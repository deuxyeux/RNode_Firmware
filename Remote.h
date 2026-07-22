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

#include <WiFi.h>

#if CONFIG_IDF_TARGET_ESP32
  #include "esp32/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32S2
  #include "esp32s2/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32C3
  #include "esp32c3/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32S3
  #include "esp32s3/rom/rtc.h"
#else 
  #error Target CONFIG_IDF_TARGET is not supported
#endif

#define WIFI_UPDATE_INTERVAL_MS 500
#define WR_SOCKET_TIMEOUT 6
#define WR_READ_TIMEOUT_MS 6500
#define WR_RECONNECT_INTERVAL_MS 10000

uint32_t wifi_update_interval_ms = WIFI_UPDATE_INTERVAL_MS;
uint32_t last_wifi_update = 0;
uint32_t wr_last_connect_try = 0;
uint32_t wr_last_read = 0;

WiFiClient connection;
WiFiServer remote_listener(7633, 1);
IPAddress ap_ip(10, 0, 0, 1);
IPAddress ap_nm(255, 255, 255, 0);
IPAddress wr_device_ip;
char wr_hostname[11];
wl_status_t wr_wifi_status = WL_IDLE_STATUS;

uint8_t wifi_mode = WIFI_OFF;
bool wifi_init_ran = false;
bool wifi_initialized = false;

char wr_ssid[33];
char wr_psk[33];

extern void host_disconnected();

void wifi_dbg(String msg) { Serial.print("[WiFi] "); Serial.println(msg); }

uint8_t wifi_remote_mode() { return wifi_mode; }

bool wifi_is_connected() { return (wr_wifi_status == WL_CONNECTED); }
bool wifi_host_is_connected() { if (connection) { return true; } else { return false; } }

void wifi_remote_start_ap() {
  WiFi.mode(WIFI_AP);
  if (wr_ssid[0] != 0x00) {
    if (wr_psk[0] != 0x00) { WiFi.softAP(wr_ssid, wr_psk, wr_channel); }
    else                   { WiFi.softAP(wr_ssid, NULL, wr_channel); }
  } else {
    if (wr_psk[0] != 0x00) { WiFi.softAP(bt_devname, wr_psk, wr_channel); }
    else                   { WiFi.softAP(bt_devname, NULL, wr_channel); }
  }
  delay(150);
  WiFi.softAPConfig(ap_ip, ap_ip, ap_nm);
  wifi_initialized = true;
}

void wifi_remote_start_sta() {
  WiFi.mode(WIFI_STA);

  uint8_t ip[4]; bool ip_ok = true;
  for (uint8_t i = 0; i < 4; i++) { ip[i]  = EEPROM.read(config_addr(ADDR_CONF_IP+i)); }
  if (ip[0]==0x00 && ip[1]==0x00 && ip[2]==0x00 && ip[3]==0x00) { ip_ok = false; }
  if (ip[0]==0xFF && ip[1]==0xFF && ip[2]==0xFF && ip[3]==0xFF) { ip_ok = false; }

  uint8_t nm[4]; bool nm_ok = true;
  for (uint8_t i = 0; i < 4; i++) { nm[i]  = EEPROM.read(config_addr(ADDR_CONF_NM+i)); }
  if (nm[0]==0x00 && nm[1]==0x00 && nm[2]==0x00 && nm[3]==0x00) { nm_ok = false; }
  if (nm[0]==0xFF && nm[1]==0xFF && nm[2]==0xFF && nm[3]==0xFF) { nm_ok = false; }

  if (ip_ok && nm_ok) {
    IPAddress sta_ip(ip[0], ip[1], ip[2], ip[3]);
    IPAddress sta_nm(nm[0], nm[1], nm[2], nm[3]);

    // Gateway/DNS (ADDR_CONF_GW/DNS, ROM.h) - unset (addr4_read() returns
    // false) means 0.0.0.0, i.e. genuinely no gateway/DNS, not sta_ip
    // itself (the previous behavior here) - that never routed anywhere,
    // it just silently broke anything needing to leave the local subnet
    // (e.g. rtc_sync_ntp(), RTC.h).
    uint8_t gw[4] = {0, 0, 0, 0}; addr4_read(ADDR_CONF_GW, gw);
    uint8_t dns[4] = {0, 0, 0, 0}; addr4_read(ADDR_CONF_DNS, dns);
    IPAddress sta_gw(gw[0], gw[1], gw[2], gw[3]);
    IPAddress sta_dns(dns[0], dns[1], dns[2], dns[3]);
    WiFi.config(sta_ip, sta_gw, sta_nm, sta_dns);
  }

  delay(100);
  if (wr_ssid[0] != 0x00) {
    if (wr_psk[0] != 0x00) { WiFi.begin(wr_ssid, wr_psk); }
    else                   { WiFi.begin(wr_ssid); }
  }
  
  delay(500);
  wr_wifi_status = WiFi.status(); 
  wifi_initialized = true;
  wr_last_connect_try = millis();
}

void wifi_remote_stop() {
  WiFi.softAPdisconnect(true);
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_MODE_NULL);
  wifi_initialized = false;
}

void wifi_remote_start() {
  if      (wifi_mode == WR_WIFI_AP)  { wifi_remote_start_ap(); }
  else if (wifi_mode == WR_WIFI_STA) { wifi_remote_start_sta(); }
  else                               { wifi_remote_stop(); }

  if (wifi_initialized == true) {
    remote_listener.begin();
    remote_listener.setTimeout(WR_SOCKET_TIMEOUT);
    wr_state = WR_STATE_ON;
  } else { remote_listener.end(); wr_state = WR_STATE_OFF; }
}

void wifi_remote_init() {
  snprintf(wr_hostname, sizeof(wr_hostname), "rnode-%02x%02x", (uint8_t)bt_dh[14], (uint8_t)bt_dh[15]);
  WiFi.softAPdisconnect(true);
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_MODE_NULL);
  WiFi.setHostname(wr_hostname);

  wr_ssid[32] = 0x00; wr_psk[32] = 0x00;
  for (uint8_t i = 0; i < 32; i++) { wr_ssid[i] = EEPROM.read(config_addr(ADDR_CONF_SSID+i)); if (wr_ssid[i] == 0xFF) { wr_ssid[i] = 0x00; } }
  for (uint8_t i = 0; i < 32; i++) { wr_psk[i]  = EEPROM.read(config_addr(ADDR_CONF_PSK+i));  if (wr_psk[i]  == 0xFF) { wr_psk[i]  = 0x00; } }
  wr_channel = EEPROM.read(eeprom_addr(ADDR_CONF_WCHN)); if (wr_channel < 1 || wr_channel > 14) { wr_channel = WR_CHANNEL_DEFAULT; }
  wifi_remote_start();
  wifi_init_ran = true;
}

#if HAS_ESPNOW == true
// Lighter STA reconnect retry, used by wifi_update_status()'s retry path
// below in place of the full wifi_remote_init(). wifi_remote_init()/
// wifi_remote_stop() both do WiFi.mode(WIFI_MODE_NULL) as part of their
// teardown, which fully deinitializes the shared esp_wifi driver state
// ESP-NOW rides on top of - deregistering its peer/callbacks along with it.
// Since espnow_init() (ESPNOW.h) only ever runs once, at boot, ESP-NOW
// never recovers after that: confirmed on hardware, this reproduced every
// WR_RECONNECT_INTERVAL_MS (10s) that STA couldn't connect, silently
// breaking ESP-NOW for the rest of the session. A plain WiFi.begin() retry
// accomplishes the same reconnect attempt without ever calling
// WiFi.mode(), so ESP-NOW's driver state (and its broadcast peer) survives.
//
// Residual limitation this doesn't solve: AP mode already shares wr_channel
// with ESP-NOW by construction, so it's unaffected by any of this. STA
// mode here joins an *external* AP whose channel isn't under our control -
// if that AP's channel differs from wr_channel, ESP-NOW's broadcast peer
// (locked to wr_channel at espnow_init() time) will silently stop reaching
// other ESP-NOW nodes fixed on that channel, even once this fix lands and
// nothing crashes. Fully tracking that would mean re-registering the
// ESP-NOW peer on every WiFi.channel() change - real additional scope, and
// not needed to fix the actual reported bug (the crash/deregistration
// above), so left undone here.
void wifi_remote_reconnect() {
  if (wr_ssid[0] != 0x00) {
    if (wr_psk[0] != 0x00) { WiFi.begin(wr_ssid, wr_psk); }
    else                   { WiFi.begin(wr_ssid); }
  }
  wr_wifi_status = WiFi.status();
  wr_last_connect_try = millis();
}
#endif

void wifi_remote_close_all() {
  // wifi_dbg("Close all"); // TODO: Remove debug
  if (connection) { connection.stop(); }
  WiFiClient client = remote_listener.available();
  while (client) { client.stop(); client = remote_listener.available(); }
  wr_state = WR_STATE_ON;
}

void wifi_remote_check_active() {
  if (millis()-wr_last_read >= WR_READ_TIMEOUT_MS) {
    // wifi_dbg("Connection activity timed out"); // TODO: Remove debug
    if (connection && connection.connected()) {
      connection.stop();
      wifi_remote_close_all();
      host_disconnected();
    }
  }
}

bool wifi_remote_available() {
  if (connection) {
    if (connection.connected()) {
      if (connection.available()) { wr_last_read = millis(); return true; }
      else                        { wifi_remote_check_active(); return false; }
    } else {
      // wifi_dbg("Client disconnected"); // TODO: Remove debug
      wifi_remote_close_all();
      host_disconnected();
      return false;
    }
  } else {
    WiFiClient client = remote_listener.available();
    if (!client) { return false; }
    else {
      #if HAS_WIFI
      // Cross-transport exclusivity: at most one active network host (TCP-
      // Remote or WS) at a time - see the symmetric guard in
      // WebSocketRemote.h's webSocketEvent(). buffer_serial()/
      // serial_callback() share one global KISS parser state, so a second
      // live host feeding it at once would interleave bytes mid-frame and
      // corrupt both streams.
      if (ws_host_is_connected()) { client.stop(); return false; }
      #endif
      // wifi_dbg("Client connected"); // TODO: Remove debug
      connection = client;
      wr_state = WR_STATE_CONNECTED;
      wr_last_read = millis();
      if (connection.available()) { return true; }
      else                        { return false; }
    }
  }
}

uint8_t wifi_remote_read() {
  if (connection && connection.available()) { return connection.read(); }
  else {
    // wifi_dbg("Error: No data to read from TCP socket"); // TODO: Remove debug
    if (connection) { wifi_remote_close_all(); host_disconnected(); }
    return 0xC0;
  }
}

void wifi_remote_write(uint8_t byte) { if (connection) { connection.write(byte); } }

// Bulk variant - callers with a whole frame ready (e.g. ESPNOW.h's
// kiss_write_espnow_packet()) should use this instead of looping
// wifi_remote_write() one byte at a time. WiFiClient::write(buf, size) is a
// single TCP send() under the hood, so this avoids both the per-byte call
// overhead and, unlike serial_write()'s byte-at-a-time path, can't stall
// mid-frame waiting on the host to drain a one-byte-at-a-time stream.
size_t wifi_remote_write_buf(const uint8_t *buf, size_t len) {
  if (connection) { return connection.write(buf, len); }
  return 0;
}

void wifi_update_status() {
  wr_wifi_status = WiFi.status();
  if (wr_wifi_status == WL_CONNECTED) { wr_device_ip = WiFi.localIP(); }
  if (wifi_mode == WR_WIFI_AP && wifi_initialized) { wr_device_ip = WiFi.softAPIP(); wr_wifi_status = WL_CONNECTED; }
  if (wifi_init_ran && wifi_mode == WR_WIFI_STA && wr_wifi_status != WL_CONNECTED) {
    if (millis()-wr_last_connect_try >= WR_RECONNECT_INTERVAL_MS) {
      #if HAS_ESPNOW == true
        wifi_remote_reconnect();
      #else
        wifi_remote_init();
      #endif
    }
  }
}

void update_wifi() {
  if (millis()-last_wifi_update >= wifi_update_interval_ms) {
    wifi_update_status();
    last_wifi_update = millis();
  }
}