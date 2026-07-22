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

void ws_remote_init() {
  if (ws_enabled) {
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
