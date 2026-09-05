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

// WebSocket KISS listener - lets browser-based tools (which cannot open raw
// TCP sockets) reach the same KISS byte stream Remote.h serves over raw TCP
// (port 7633) for desktop Reticulum. This carries the exact same KISS bytes
// (FEND/FESC intact, see Framing.h) as every other transport - it is "just
// another byte pipe" into serial_callback()/serial_write(), not a distinct
// on-wire protocol.

#include <WebSocketsServer.h>

// Forward declaration - Ethernet.h (which defines this) is #include'd after
// this file (Utilities.h's include chain), same reason OTA.h forward-
// declares menu_is_open(). Needed by ws_remote_init()'s network-readiness
// check below.
#if HAS_ETHERNET == true
  extern bool eth_disabled;
#endif

#define WS_LISTEN_PORT 7634

#define WS_STATE_NA        0xff
#define WS_STATE_OFF       0x00
#define WS_STATE_ON        0x01
#define WS_STATE_CONNECTED 0x02

WebSocketsServer wsServer(WS_LISTEN_PORT);
uint8_t ws_state = WS_STATE_OFF;
// ID (per the WebSockets library's own numbering) of the single client
// currently allowed to drive the KISS engine, or -1 if none. Only one at a
// time, mirroring Remote.h's single-connection model - see the cross-
// transport exclusivity check in webSocketEvent() below.
int16_t ws_client_num = -1;

// webSocketEvent() (fired from wsServer.loop(), itself called from
// update_ws() below) hands us a whole reassembled message at once, but
// buffer_serial() (RNode_Firmware.ino) drains every network transport one
// byte at a time via *_available()/*_read(), matching wifi_remote_available()
// /wifi_remote_read() (Remote.h) - which in turn lean on WiFiClient's own
// internal receive buffer. WebSocketsServer has no equivalent pull-style
// buffer, so incoming message bytes are queued here first. Sized for a
// single KISS frame at worst-case FESC-escaping (SINGLE_MTU*2, Config.h)
// plus headroom, not for multiple queued frames.
#define WS_RX_BUF_SIZE 1024
uint8_t ws_rx_buf[WS_RX_BUF_SIZE];
size_t ws_rx_head = 0;
size_t ws_rx_tail = 0;

bool ws_rx_isfull()  { return ((ws_rx_head + 1) % WS_RX_BUF_SIZE) == ws_rx_tail; }
bool ws_rx_isempty() { return ws_rx_head == ws_rx_tail; }
void ws_rx_push(uint8_t b) {
  if (!ws_rx_isfull()) { ws_rx_buf[ws_rx_head] = b; ws_rx_head = (ws_rx_head + 1) % WS_RX_BUF_SIZE; }
}
uint8_t ws_rx_pop() {
  uint8_t b = ws_rx_buf[ws_rx_tail];
  ws_rx_tail = (ws_rx_tail + 1) % WS_RX_BUF_SIZE;
  return b;
}

// Output side: buffer bytes and flush as one WS binary message per KISS
// frame (delimited by FEND) instead of one sendBIN() call per byte - the
// latter would emit one WS frame, with header overhead, per payload byte.
// Mirrors two existing precedents in this codebase: the nRF52+BLE branch of
// serial_write() (Utilities.h) that batches SerialBT.flushTXD() the same
// way, and wifi_remote_write_buf() (Remote.h), the bulk-send path ESP-NOW's
// kiss_write_espnow_packet() uses instead of per-byte writes.
#define WS_TX_BUF_SIZE 2048
uint8_t ws_tx_buf[WS_TX_BUF_SIZE];
size_t ws_tx_len = 0;
bool ws_tx_in_frame = false;

bool ws_host_is_connected() { return ws_state == WS_STATE_CONNECTED; }

// wsServer.sendBIN() used to be called directly from here, i.e. from
// whichever task happened to be calling serial_write() at the time (any
// of loopTask, kiss_tx_task, housekeeping_task - anything that can emit a
// KISS byte). A clean, fully decoded live crash (stack-canary watchpoint
// on kiss_tx_task, sized 2048 at the time) traced through this exact call
// into the WebSockets library -> lwIP TCP/IP stack -> WiFi TX - a 30+
// frame chain (esp_wifi_internal_tx, etharp_output, ip4_output_if,
// tcp_output_segment, lwip_netconn_do_write, ...) that only actually
// executes once a WebSocket remote client is connected, so no earlier
// measurement window (all done with no WS client attached) ever caught
// it. Bumping every caller task's stack to cover this worst case was
// tried and worked, but cost ~10KB combined and still left loopTask
// itself unprotected (it calls serial_write() constantly too, and was
// never explicitly sized with this hazard in mind). Isolating the actual
// network call behind its own queue+dedicated task (g_ws_tx_queue/
// ws_tx_task, this file) - the same pattern already used for Serial/
// Serial0 writes (debug_log_task/cmd_log_task history, RNode_Firmware.ino)
// - means only that one task needs the large stack, and every caller of
// serial_write() (including loopTask) is protected uniformly instead of
// needing this reasoned about per call site.
//
// wifi_remote_write() (Remote.h, the raw-TCP KISS remote on port 7633)
// has the exact same exposure - it also calls connection.write() straight
// into the same lwIP/WiFi stack, per-byte, with no isolation at all.
// Deliberately not fixed here too - needs its own dedicated testing pass
// with that transport actually connected, not bundled into this change
// untested.
// ws_tx_task() (formerly its own dedicated FreeRTOS task, pinned to core
// 0, draining a queue this function fed) removed 2026-08-15 - ws_tx_flush()
// now calls wsServer.sendBIN() directly, synchronously, matching
// microReticulum_Firmware's zero-extra-task architecture (testing/ruling
// out a cross-core data race as this session's stack-canary crash cause -
// see loop()'s own comment, RNode_Firmware.ino, for the full trail).
// Reintroduces the original "large stack needed at the wsServer.sendBIN()
// call site" risk this task/queue split existed to isolate - accepted as
// a known tradeoff for this test, not something newly missed.
void ws_tx_flush() {
  if (ws_tx_len > 0 && ws_client_num >= 0) {
    wsServer.sendBIN((uint8_t)ws_client_num, ws_tx_buf, ws_tx_len);
  }
  ws_tx_len = 0;
}

void ws_remote_write(uint8_t byte) {
  if (!ws_host_is_connected()) return;

  if (ws_tx_len >= WS_TX_BUF_SIZE) { ws_tx_flush(); } // overflow safety valve
  ws_tx_buf[ws_tx_len++] = byte;

  if (byte == FEND) {
    if (ws_tx_in_frame) { ws_tx_flush(); ws_tx_in_frame = false; }
    else                { ws_tx_in_frame = true; }
  }
}

bool ws_remote_available() { return !ws_rx_isempty(); }
uint8_t ws_remote_read()   { return ws_rx_pop(); }

void ws_remote_close_all() {
  if (ws_client_num >= 0) { wsServer.disconnect((uint8_t)ws_client_num); }
  ws_client_num = -1;
  ws_tx_len = 0;
  ws_tx_in_frame = false;
  if (ws_state == WS_STATE_CONNECTED) { ws_state = WS_STATE_ON; }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      if ((int16_t)num == ws_client_num) { ws_remote_close_all(); }
      break;

    case WStype_CONNECTED:
      // Cross-transport exclusivity: at most one active network host (TCP-
      // Remote or WS) at a time. buffer_serial()/serial_callback()
      // (RNode_Firmware.ino) share one global KISS parser state, so two
      // simultaneously connected hosts feeding bytes into it would
      // interleave mid-frame and corrupt both streams. See the symmetric
      // guard in wifi_remote_available() (Remote.h).
      if (wifi_host_is_connected() || ws_client_num >= 0) {
        wsServer.disconnect(num);
      } else {
        ws_client_num = num;
        ws_state = WS_STATE_CONNECTED;
      }
      break;

    case WStype_BIN:
      if ((int16_t)num == ws_client_num) {
        for (size_t i = 0; i < length; i++) { ws_rx_push(payload[i]); }
      }
      break;

    default:
      break;
  }
}

// wsServer.begin() needs lwIP's TCP/IP task already running - it doesn't
// gracefully fail if nothing's brought that up yet, it hard-asserts
// ("assert failed: xQueueSemaphoreTake queue.c") deep inside NetworkServer::
// begin(), a hard bootloop rather than just being unreachable. Same failure
// class OTA.h's own ota_server_init() call site already guards against
// (see its own comment) - WiFi.mode() (via wifi_remote_init()/espnow_init())
// or Ethernet's ETH.begin() (init_ethernet(), Ethernet.h) are the only two
// things that bring lwIP up. Checked here, not just at the boot call site,
// so the same guard also covers ws_conf_save() below - CMD_WS_ENABLE can
// arrive over USB serial with no network interface ever having come up at
// all (WiFi-only boards with WiFi Mode off), independent of boot order.
#if HAS_ETHERNET == true
  bool ws_net_stack_ready() { return (WiFi.getMode() != WIFI_MODE_NULL || !eth_disabled); }
#else
  bool ws_net_stack_ready() { return (WiFi.getMode() != WIFI_MODE_NULL); }
#endif

void ws_remote_init() {
  if (ws_enabled && ws_net_stack_ready()) {
    wsServer.begin();
    wsServer.onEvent(webSocketEvent);
    // Without an active heartbeat, the library only detects a disconnect
    // via a clean WS close handshake or the next failed read/write on a
    // dead TCP socket - an abruptly-closed browser tab, a dropped WiFi
    // link, or a crashed client leaves ws_client_num stuck indefinitely,
    // permanently rejecting every future connection attempt (the exact
    // symptom this was debugged from). Ping every 5s, allow 3s for a pong,
    // disconnect after 2 misses - so a dead peer's slot frees within ~11s
    // instead of never.
    wsServer.enableHeartbeat(5000, 3000, 2);
    ws_state = WS_STATE_ON;
  } else {
    wsServer.close();
    ws_client_num = -1;
    ws_tx_len = 0;
    ws_tx_in_frame = false;
    ws_state = WS_STATE_OFF;
  }
}

// Persists whether the WebSocket KISS listener is allowed to run
// (ADDR_CONF_WS, ROM.h). Unlike espnow_conf_save() (Utilities.h), this takes
// effect immediately - WebSocketsServer supports begin()/close() at runtime
// same as Remote.h's WiFiServer, so no reboot is needed.
void ws_conf_save(uint8_t val) {
  #if HAS_EEPROM
    uint8_t stored = EEPROM.read(eeprom_addr(ADDR_CONF_WS));
  #elif MCU_VARIANT == MCU_NRF52
    uint8_t stored = eeprom_read(eeprom_addr(ADDR_CONF_WS));
  #endif
  eeprom_update(eeprom_addr(ADDR_CONF_WS), val);
  #if !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
    eeprom_flush();
  #endif
  (void)stored;
  ws_enabled = (val == WS_ENABLE_BYTE);
  ws_remote_init();
}

void update_ws() {
  if (ws_enabled) { wsServer.loop(); }
}
