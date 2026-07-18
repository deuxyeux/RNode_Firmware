// Copyright (C) 2024, Mark Qvist

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <esp_now.h>
#include <esp_wifi.h>

#define ESPNOW_QUEUE_SIZE 8

typedef struct {
        size_t len;
        int rssi;
        int snr_raw;
        uint8_t mac[6];
        uint8_t data[];
} espnow_packet_t;

static xQueueHandle espnow_packet_queue = NULL;

bool espnow_ready = false;

const uint8_t espnow_broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// vport 0 is the real LoRa radio (untouched by any of this); vport 1 is
// this ESP-NOW transport, masquerading to the host as a second LoRa radio
// per RNode Multi-Interface (see plan). selected_vport tracks which vport
// the host last selected via CMD_SEL_INT - it's the host-write-direction
// context for CMD_FREQUENCY/CMD_DATA/etc. Config sent to vport 1 is only
// ever stored here and echoed back, never applied to real hardware.
uint8_t selected_vport = 0;
typedef struct {
  uint32_t frequency;
  uint32_t bandwidth;
  int8_t   txpower;
  uint8_t  sf;
  uint8_t  cr;
  uint16_t st_alock;
  uint16_t lt_alock;
  uint8_t  radio_state;
} espnow_vport_cfg_t;
espnow_vport_cfg_t espnow_vport_cfg = {0, 0, 0, 0, 0, 0, 0, 0};

// npset(), serial_write(), escaped_serial_write() and kiss_select_interface()
// are all defined later in Utilities.h (after this file's include point) -
// forward-declared here rather than reordering includes.
void kiss_select_interface(uint8_t vport);
void serial_write(uint8_t byte);
void escaped_serial_write(uint8_t byte);

// True once the connected host has explicitly turned vport 1 "on"
// (CMD_RADIO_STATE), exactly paralleling how radio_online (Config.h) only
// becomes true after CMD_RADIO_STATE for vport 0 succeeds (startRadio(),
// RNode_Firmware.ino) - not merely "ESP-NOW hardware is up". Drives the
// display/NeoPixel's ESP-NOW-specific behavior (Display.h, loop()'s LED
// dispatch in RNode_Firmware.ino) without ever touching radio_online
// itself, which stays reserved for the real LoRa radio - reusing it here
// would break startRadio()'s !radio_online re-entrancy check and let
// transmit()/setTXPower()/setBandwidth()/setFrequency() (RNode_Firmware.ino/
// Utilities.h) touch an uninitialized LoRa driver object.
bool espnow_ui_active() {
  return espnow_enabled && espnow_ready && espnow_vport_cfg.radio_state == RADIO_STATE_ON;
}

// Momentary "an RX/TX just happened" flags for Display.h's ESP-NOW waterfall
// marks - set below, consumed and cleared once by the display code each
// refresh (mirrors display_tx's role for the LoRa path, Display.h).
bool espnow_display_rx = false;
bool espnow_display_tx = false;

#if HAS_NP == true
  void npset(uint8_t r, uint8_t g, uint8_t b);
  #define ESPNOW_FLASH_MS 120
  uint32_t espnow_flash_until = 0;
  // Distinct from LoRa's RX (blue)/TX (orange) NeoPixel colors used by
  // led_rx_on()/led_tx_on() (Utilities.h) - ESP-NOW gets its own yellow/
  // orange pair so the two transports stay visually distinguishable even
  // though the TX color happens to match LoRa's.
  void espnow_flash_rx() { npset(0xFF, 0xFF, 0x00); espnow_flash_until = millis() + ESPNOW_FLASH_MS; }
  void espnow_flash_tx() { npset(0xFF, 0x50, 0x00); espnow_flash_until = millis() + ESPNOW_FLASH_MS; }
  // Drives the NeoPixel while ESP-NOW is the active "radio" (espnow_ui_active(),
  // loop()'s LED dispatch, RNode_Firmware.ino) in place of led_indicate_standby()'s
  // idle white breathing pulse - momentary colored flashes on RX/TX (above) that
  // decay back to off, rather than a continuous pulse, since ESP-NOW has no
  // sustained on-air state the way LoRa's led_rx_on()/led_tx_on() bracket a
  // transmit/receive window. Only called from loop() while espnow_ui_active(),
  // so it never fights with led_indicate_standby()'s own npset() calls.
  void espnow_led_update() {
    if (espnow_flash_until != 0 && (int32_t)(millis() - espnow_flash_until) >= 0) {
      npset(0, 0, 0);
      espnow_flash_until = 0;
    } else if (espnow_flash_until == 0) {
      npset(0, 0, 0);
    }
  }
#endif

// Serial is the host KISS connection on these boards - never write debug
// text to it (see Remote.h's wifi_dbg(), whose call sites are all disabled
// for exactly this reason). DEBUG_LOG() is the safe idiom: routes to a
// dedicated UART on HAS_DEBUG_UART boards, no-ops everywhere else.

// Classic ESP-NOW hard-caps a single transmission at 250 bytes, well under
// this firmware's Reticulum-facing MTU (508, Config.h) - a real Reticulum
// Link packet routinely exceeds 250 bytes. Confirmed on hardware: without
// fragmentation, oversized packets were silently truncated to exactly 250
// bytes at the TX accumulation stage, corrupting the ciphertext and
// causing 100% reproducible "Token HMAC was invalid" decryption failures
// on the receiving end - not a crypto/LXMF bug, a truncation bug here.
//
// Fix mirrors the LoRa path's own existing split-packet scheme (transmit()/
// receive_callback(), RNode_Firmware.ino): each raw ESP-NOW frame is
// prefixed with a 1-byte header - top nibble is a random per-logical-packet
// sequence tag, bit 0 is set on every chunk except the last ("more chunks
// follow"). This is a separate, independent framing from LoRa's own
// NIBBLE_SEQ/FLAG_SPLIT (Framing.h) - reusing the same byte-layout idea,
// but LoRa's scheme hardcodes exactly two chunks (matches its 254-byte
// payload cap fitting the 508 MTU in two), which doesn't fit ESP-NOW's
// smaller 249-byte usable-payload-per-frame; this one supports any number
// of chunks.
#define ESPNOW_RAW_MTU     250
#define ESPNOW_CHUNK_SIZE   (ESPNOW_RAW_MTU-1)
#define ESPNOW_FRAG_MORE    0x01

uint8_t  espnow_rx_buf[MTU];
uint16_t espnow_rx_len = 0;
uint8_t  espnow_rx_seq = SEQ_UNSET;

void espnow_recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len <= 1) return;

  uint8_t header = data[0];
  uint8_t seq = header & NIBBLE_SEQ;
  bool more = header & ESPNOW_FRAG_MORE;
  const uint8_t *chunk = data+1;
  int chunk_len = len-1;

  if (seq != espnow_rx_seq) {
    // First chunk of a new logical packet (or a stray/interrupted
    // continuation chunk with no matching start - either way, this is
    // the start of what we now track).
    espnow_rx_len = 0;
    espnow_rx_seq = seq;
  }

  if (espnow_rx_len + chunk_len > sizeof(espnow_rx_buf)) {
    // Reassembly overflow - drop what we had and wait for the next
    // fresh sequence rather than writing out of bounds.
    espnow_rx_len = 0;
    espnow_rx_seq = SEQ_UNSET;
    return;
  }

  memcpy(espnow_rx_buf+espnow_rx_len, chunk, chunk_len);
  espnow_rx_len += chunk_len;

  if (more) return;

  // Last chunk received - the logical packet is complete, hand it to the
  // queue exactly as before. RSSI/SNR reflect this final chunk only.
  espnow_rx_seq = SEQ_UNSET;

  espnow_packet_t *espnow_packet = (espnow_packet_t*)malloc(sizeof(espnow_packet_t) + espnow_rx_len);
  if (!espnow_packet) { memory_low = true; espnow_rx_len = 0; return; }

  espnow_packet->len = espnow_rx_len;
  espnow_packet->rssi = info->rx_ctrl ? info->rx_ctrl->rssi : 0;

  // ESP-NOW/WiFi has no native SNR register like LoRa, but ESP32-S3's
  // wifi_pkt_rx_ctrl_t does expose a real noise_floor (dBm) - rssi minus
  // that gives a genuine SNR estimate. Encoded in the same quarter-dB
  // fixed-point units as LoRa's packetSnrRaw() so it decodes correctly
  // on the host side (RNodeSubInterface: byte * 0.25).
  int snr_db = 0;
  if (info->rx_ctrl) { snr_db = info->rx_ctrl->rssi - info->rx_ctrl->noise_floor; }
  int snr_quarter_db = snr_db * 4;
  if (snr_quarter_db > 127) snr_quarter_db = 127;
  if (snr_quarter_db < -128) snr_quarter_db = -128;
  espnow_packet->snr_raw = snr_quarter_db;

  memcpy(espnow_packet->mac, info->src_addr, 6);
  memcpy(espnow_packet->data, espnow_rx_buf, espnow_rx_len);
  espnow_rx_len = 0;

  if (!espnow_packet_queue || xQueueSendFromISR(espnow_packet_queue, &espnow_packet, NULL) != pdPASS) {
    free(espnow_packet);
  }
}

// esp_now_send() is async: a successful return only means the packet was
// queued with the WiFi driver, not that it was actually transmitted over
// the air. Real TX outcome only arrives via this callback - without it we
// have zero visibility into silent TX failures.
void espnow_send_cb(const esp_now_send_info_t *tx_info, esp_now_send_status_t status) {
  DEBUG_LOG("[ESP-NOW] TX result: %s\n", status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

bool espnow_send_chunk(const uint8_t *data, uint8_t len) {
  if (!espnow_ready) return false;
  esp_err_t err = esp_now_send(espnow_broadcast_mac, data, len);
  if (err != ESP_OK) { DEBUG_LOG("[ESP-NOW] esp_now_send() queue failed: %d\n", err); }
  return err == ESP_OK;
}

// Fragmenting entry point - splits any logical payload up to the full
// Reticulum MTU into <=ESPNOW_RAW_MTU raw frames (see espnow_recv_cb()
// for the matching reassembly and why this exists). A single-chunk
// packet still gets the 1-byte header (MORE clear), keeping the wire
// format uniform regardless of size.
bool espnow_send(const uint8_t *data, uint16_t len) {
  uint8_t header = random(256) & NIBBLE_SEQ;
  uint8_t chunk_buf[ESPNOW_RAW_MTU];
  uint16_t offset = 0;
  bool all_ok = true;

  do {
    uint16_t remaining = len - offset;
    uint16_t chunk_len = (remaining > ESPNOW_CHUNK_SIZE) ? ESPNOW_CHUNK_SIZE : remaining;
    bool more = (offset + chunk_len) < len;

    chunk_buf[0] = header | (more ? ESPNOW_FRAG_MORE : 0);
    memcpy(chunk_buf+1, data+offset, chunk_len);

    all_ok = espnow_send_chunk(chunk_buf, chunk_len+1) && all_ok;
    offset += chunk_len;
  } while (offset < len);

  espnow_display_tx = true;
  #if HAS_NP == true
    if (espnow_ui_active()) espnow_flash_tx();
  #endif

  return all_ok;
}

// TX-side accumulation buffer for vport 1's CMD_DATA frames, filled
// byte-by-byte by serial_callback() (RNode_Firmware.ino) and flushed via
// espnow_send() on the frame's closing FEND. Sized to this firmware's
// full Reticulum-facing MTU (508, Config.h) - espnow_send() fragments
// anything over ESP-NOW's smaller per-frame hardware limit automatically.
#define ESPNOW_TX_BUF_SIZE MTU
uint8_t espnow_tx_buf[ESPNOW_TX_BUF_SIZE];
uint16_t espnow_tx_len = 0;

// CMD_INTERFACES: queried once at connect (bundled into the host's
// detect() write alongside CMD_DETECT/CMD_FW_VERSION/CMD_PLATFORM/CMD_MCU).
// Host expects one frame per vport, in vport-index order, host-side
// interface_type_to_str() only knows real LoRa radio codes - vport 1 has
// to claim one too (matching this board's real MODEM, KISS_SX1262) since
// there's no non-LoRa vport concept on the host.
void kiss_indicate_interfaces() {
  serial_write(FEND);
  serial_write(CMD_INTERFACES);
  serial_write(0x00);
  serial_write(KISS_SX1262);
  serial_write(FEND);

  serial_write(FEND);
  serial_write(CMD_INTERFACES);
  serial_write(0x01);
  serial_write(KISS_SX1262);
  serial_write(FEND);
}

// vport 1's own "radio" indicate functions - mirror the shape of the
// vport-0 kiss_indicate_*() functions in Utilities.h exactly, but read
// from espnow_vport_cfg (accepted-and-stored only) instead of the real
// lora_* globals. Needed so the host's RNodeSubInterface.validateRadioState()
// sees its configured values echoed back and marks vport 1 online, even
// though none of it is ever applied to real hardware.
void kiss_indicate_v1_frequency() {
  kiss_select_interface(1);
  serial_write(FEND);
  serial_write(CMD_FREQUENCY);
  escaped_serial_write(espnow_vport_cfg.frequency>>24);
  escaped_serial_write(espnow_vport_cfg.frequency>>16);
  escaped_serial_write(espnow_vport_cfg.frequency>>8);
  escaped_serial_write(espnow_vport_cfg.frequency);
  serial_write(FEND);
}

void kiss_indicate_v1_bandwidth() {
  kiss_select_interface(1);
  serial_write(FEND);
  serial_write(CMD_BANDWIDTH);
  escaped_serial_write(espnow_vport_cfg.bandwidth>>24);
  escaped_serial_write(espnow_vport_cfg.bandwidth>>16);
  escaped_serial_write(espnow_vport_cfg.bandwidth>>8);
  escaped_serial_write(espnow_vport_cfg.bandwidth);
  serial_write(FEND);
}

void kiss_indicate_v1_txpower() {
  kiss_select_interface(1);
  serial_write(FEND);
  serial_write(CMD_TXPOWER);
  serial_write((uint8_t)espnow_vport_cfg.txpower);
  serial_write(FEND);
}

void kiss_indicate_v1_sf() {
  kiss_select_interface(1);
  serial_write(FEND);
  serial_write(CMD_SF);
  serial_write(espnow_vport_cfg.sf);
  serial_write(FEND);
}

void kiss_indicate_v1_cr() {
  kiss_select_interface(1);
  serial_write(FEND);
  serial_write(CMD_CR);
  serial_write(espnow_vport_cfg.cr);
  serial_write(FEND);
}

void kiss_indicate_v1_radiostate() {
  kiss_select_interface(1);
  serial_write(FEND);
  serial_write(CMD_RADIO_STATE);
  serial_write(espnow_vport_cfg.radio_state);
  serial_write(FEND);
}

void kiss_indicate_v1_st_alock() {
  kiss_select_interface(1);
  serial_write(FEND);
  serial_write(CMD_ST_ALOCK);
  escaped_serial_write(espnow_vport_cfg.st_alock>>8);
  escaped_serial_write(espnow_vport_cfg.st_alock);
  serial_write(FEND);
}

void kiss_indicate_v1_lt_alock() {
  kiss_select_interface(1);
  serial_write(FEND);
  serial_write(CMD_LT_ALOCK);
  escaped_serial_write(espnow_vport_cfg.lt_alock>>8);
  escaped_serial_write(espnow_vport_cfg.lt_alock);
  serial_write(FEND);
}

// Per-packet RSSI/SNR indicators for vport 1, mirroring kiss_indicate_stat_rssi()/
// kiss_indicate_stat_snr() (Utilities.h) - called right before each received
// packet's kiss_write_espnow_packet(), same as the LoRa RX path does with
// kiss_write_packet(). Take the values as parameters (from the just-drained
// espnow_packet_t) rather than reading persistent globals, since there's no
// equivalent of last_rssi/last_snr_raw needed here.
void kiss_indicate_v1_stat_rssi(int rssi) {
  kiss_select_interface(1);
  serial_write(FEND);
  serial_write(CMD_STAT_RSSI);
  escaped_serial_write((uint8_t)(rssi + rssi_offset));
  serial_write(FEND);
}

void kiss_indicate_v1_stat_snr(int snr_raw) {
  kiss_select_interface(1);
  serial_write(FEND);
  serial_write(CMD_STAT_SNR);
  escaped_serial_write((uint8_t)snr_raw);
  serial_write(FEND);
}

// ESP-NOW RX -> KISS for vport 1. Deliberately a standalone function
// reading straight from the drained espnow_packet_t buffer rather than
// reusing the shared pbuf/host_write_len globals, which belong to the
// LoRa RX path (RNode_Firmware.ino) - reusing them here would risk
// clobbering an in-flight LoRa RX in a single-threaded loop().
//
// Built into a local buffer and sent with one bulk Serial.write() rather
// than serial_write()'s usual byte-at-a-time calls. Confirmed on hardware:
// byte-at-a-time writes over native USB CDC can each block briefly waiting
// for the host to drain its buffer: fine on a fast host, but on a loaded
// Raspberry Pi (rnsd+lxmd+nomadnet competing for CPU) this accumulated
// enough delay mid-frame to blow past the host's 100ms per-frame KISS read
// timeout ("serial read timeout in command 0" in RNodeMultiInterface.py),
// silently discarding the whole packet.
void kiss_write_espnow_packet(uint8_t *data, size_t len) {
  kiss_select_interface(1);

  static uint8_t frame_buf[2*MTU+8];
  size_t n = 0;
  frame_buf[n++] = FEND;
  frame_buf[n++] = CMD_DATA;
  for (size_t i = 0; i < len && n < sizeof(frame_buf)-2; i++) {
    uint8_t byte = data[i];
    if (byte == FEND) { frame_buf[n++] = FESC; byte = TFEND; }
    if (byte == FESC) { frame_buf[n++] = FESC; byte = TFESC; }
    frame_buf[n++] = byte;
  }
  frame_buf[n++] = FEND;

  size_t written = Serial.write(frame_buf, n);
  if (written != n) {
    DEBUG_LOG("[ESP-NOW] kiss_write_espnow_packet: short write, wrote %d of %d bytes\n", (int)written, (int)n);
  }
}

void espnow_init() {
  DEBUG_LOG("[ESP-NOW] init: start\n");
  espnow_packet_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(espnow_packet_t*));
  DEBUG_LOG("[ESP-NOW] init: queue created\n");

  if (WiFi.getMode() == WIFI_MODE_NULL) {
    DEBUG_LOG("[ESP-NOW] init: setting WiFi.mode(WIFI_STA)\n");
    WiFi.mode(WIFI_STA);
    DEBUG_LOG("[ESP-NOW] init: WiFi.mode() returned\n");
  }

  if (WiFi.status() != WL_CONNECTED) {
    DEBUG_LOG("[ESP-NOW] init: calling esp_wifi_set_channel(%d)\n", wr_channel);
    esp_err_t cherr = esp_wifi_set_channel(wr_channel, WIFI_SECOND_CHAN_NONE);
    DEBUG_LOG("[ESP-NOW] init: esp_wifi_set_channel() returned %d\n", cherr);
  }

  DEBUG_LOG("[ESP-NOW] init: calling esp_now_init()\n");
  esp_err_t initerr = esp_now_init();
  DEBUG_LOG("[ESP-NOW] init: esp_now_init() returned %d\n", initerr);
  if (initerr != ESP_OK) { DEBUG_LOG("[ESP-NOW] Could not initialise\n"); return; }
  esp_now_register_recv_cb(espnow_recv_cb);
  esp_now_register_send_cb(espnow_send_cb);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, espnow_broadcast_mac, 6);
  peer.channel = wr_channel;
  peer.encrypt = false;
  DEBUG_LOG("[ESP-NOW] init: calling esp_now_add_peer()\n");
  esp_err_t peererr = esp_now_add_peer(&peer);
  DEBUG_LOG("[ESP-NOW] init: esp_now_add_peer() returned %d\n", peererr);
  if (peererr != ESP_OK) { DEBUG_LOG("[ESP-NOW] Could not add broadcast peer\n"); return; }

  espnow_ready = true;
  DEBUG_LOG("[ESP-NOW] Initialised on channel %d\n", wr_channel);
}

void update_espnow() {
  espnow_packet_t *espnow_packet = NULL;
  if (espnow_packet_queue && xQueueReceive(espnow_packet_queue, &espnow_packet, 0) == pdTRUE && espnow_packet) {
    DEBUG_LOG("[ESP-NOW] RX from %02X:%02X:%02X:%02X:%02X:%02X, rssi %d, snr_raw %d, len %d\n",
      espnow_packet->mac[0], espnow_packet->mac[1], espnow_packet->mac[2], espnow_packet->mac[3], espnow_packet->mac[4], espnow_packet->mac[5],
      espnow_packet->rssi, espnow_packet->snr_raw, (int)espnow_packet->len);
    kiss_indicate_v1_stat_rssi(espnow_packet->rssi);
    kiss_indicate_v1_stat_snr(espnow_packet->snr_raw);
    kiss_write_espnow_packet(espnow_packet->data, espnow_packet->len);
    espnow_display_rx = true;
    #if HAS_NP == true
      if (espnow_ui_active()) espnow_flash_rx();
    #endif
    free(espnow_packet);
    espnow_packet = NULL;
  }
}
