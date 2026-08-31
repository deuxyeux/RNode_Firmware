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

#include "Boards.h"

#if PLATFORM != PLATFORM_NRF52
#if HAS_BLE

#include "BLESerial.h"

uint32_t bt_passkey_callback();
void bt_passkey_notify_callback(uint32_t passkey);
bool bt_security_request_callback();
#if defined(CONFIG_BLUEDROID_ENABLED)
void bt_authentication_complete_callback(esp_ble_auth_cmpl_t auth_result);
#elif defined(CONFIG_NIMBLE_ENABLED)
void bt_authentication_complete_callback(ble_gap_conn_desc *desc);
#endif
bool bt_confirm_pin_callback(uint32_t pin);
void bt_connect_callback(BLEServer *server);
void bt_disconnect_callback(BLEServer *server);
bool bt_client_authenticated();
extern bool bt_just_works_enabled;

uint32_t BLESerial::onPassKeyRequest() { return bt_passkey_callback(); }
void BLESerial::onPassKeyNotify(uint32_t passkey) { bt_passkey_notify_callback(passkey); }
bool BLESerial::onSecurityRequest() { return bt_security_request_callback(); }
#if defined(CONFIG_BLUEDROID_ENABLED)
void BLESerial::onAuthenticationComplete(esp_ble_auth_cmpl_t auth_result) { bt_authentication_complete_callback(auth_result); }
#elif defined(CONFIG_NIMBLE_ENABLED)
void BLESerial::onAuthenticationComplete(ble_gap_conn_desc *desc) { bt_authentication_complete_callback(desc); }
#endif
void BLESerial::onConnect(BLEServer *server) { bt_connect_callback(server); }
void BLESerial::onDisconnect(BLEServer *server) { bt_disconnect_callback(server); ble_server->startAdvertising(); }
bool BLESerial::onConfirmPIN(uint32_t pin) { return bt_confirm_pin_callback(pin); };
bool BLESerial::connected() { return bt_client_authenticated(); }

int BLESerial::read() {
  int result = this->rx_buffer.pop();
  if (result == '\n') { this->numAvailableLines--; }
  return result;
}

size_t BLESerial::readBytes(uint8_t *buffer, size_t bufferSize) {
  int i = 0;
  while (i < bufferSize && available()) { buffer[i] = (uint8_t)this->rx_buffer.pop(); i++; }
  return i;
}

int BLESerial::peek() {
  if (this->rx_buffer.getLength() == 0) return -1;
  return this->rx_buffer.get(0);
}

int BLESerial::available() { return this->rx_buffer.getLength(); }

size_t BLESerial::print(const char *str) {
  if (!bt_client_authenticated()) return 0;
  size_t written = 0; for (size_t i = 0; str[i] != '\0'; i++)  { written += this->write(str[i]); }
  flush();

  return written;
}

size_t BLESerial::write(const uint8_t *buffer, size_t bufferSize) {
  if (!bt_client_authenticated()) { return 0; } else {
    size_t written = 0; for (int i = 0; i < bufferSize; i++) { written += this->write(buffer[i]); }
    flush();

    return written;
  }
}

size_t BLESerial::write(uint8_t byte) {
  if (bt_client_authenticated()) {
    this->transmitBuffer[this->transmitBufferLength] = byte;
    this->transmitBufferLength++;
    if (this->transmitBufferLength == maxTransferSize) { flush(); }
    return 1;
  } else {
    return 0;
  }
}

void BLESerial::flush() {
  if (this->transmitBufferLength > 0) {
    TxCharacteristic->setValue(this->transmitBuffer, this->transmitBufferLength);
    #if defined(CONFIG_NIMBLE_ENABLED)
      // BLECharacteristic::notify() (ESP32 Arduino BLE library,
      // BLECharacteristic.cpp) internally checks
      // getService()->getServer()->getConnectedCount() == 0 and silently
      // no-ops if so - and that counter never increments for these
      // connections, because BLEServer's own BLE_GAP_EVENT_CONNECT handler
      // only increments it (and sets m_connId) on a status==0 event, which
      // never fires here (see bt_security_request_callback's own comment,
      // Bluetooth.h). setValue() above isn't gated by that counter, which is
      // why a direct read_gatt_char() from a test client always showed the
      // correct reply while a real subscribed notify callback never fired
      // once - confirmed live, 0 deliveries, every time, against a genuine
      // Windows/bleak client (and by extension RNS's RNodeInterface, which
      // only ever listens via notify, never polls). Bypass notify() and
      // getConnId() entirely and call the underlying NimBLE host API
      // directly with ble_conn_handle (Bluetooth.h) - populated from the
      // encryption-change event instead, which isn't affected by the same
      // bug.
      extern uint16_t ble_conn_handle;
      if (ble_conn_handle != 0xFFFF) {
        os_mbuf *om = ble_hs_mbuf_from_flat(this->transmitBuffer, this->transmitBufferLength);
        if (om != nullptr) { ble_gatts_notify_custom(ble_conn_handle, TxCharacteristic->getHandle(), om); }
      }
    #else
      TxCharacteristic->notify(true);
    #endif
    this->transmitBufferLength = 0;
    this->lastFlushTime = millis();
  }
}

void BLESerial::disconnect() {
  if (bt_client_authenticated()) {
    #if defined(CONFIG_NIMBLE_ENABLED)
      // See BLESerial::flush()'s own comment above - getConnId()/m_connId
      // is populated by the same broken BLE_GAP_EVENT_CONNECT bookkeeping,
      // so it's not reliable here either.
      extern uint16_t ble_conn_handle;
      uint16_t conn_id = ble_conn_handle;
    #else
      uint16_t conn_id = ble_server->getConnId();
    #endif
    // Serial.printf("Have connected: %d\n", conn_id);
    ble_server->disconnect(conn_id);
    // Serial.println("Disconnected");
  } else {
    // Serial.println("No connected");
  }
}

bool BLESerial::begin(const char *name) {
  ConnectedDeviceCount = 0;
  // BLEDevice::init() (Arduino core) returns false on failure - e.g.
  // nimble_port_init() failing due to a WiFi/BLE controller coexistence
  // race - but the original code here discarded that return value and
  // proceeded regardless. Confirmed live on hardware, decoded via
  // addr2line: esp_ble_tx_power_set() right below then derefs NimBLE host
  // state that was never actually initialized, crashing with an unhandled
  // LoadProhibited exception. Bail out here instead, leaving the BLE
  // stack untouched so the caller (bt_start(), Bluetooth.h) can revert
  // bt_state back to OFF rather than getting stuck ON with nothing
  // actually running.
  if (!BLEDevice::init(name)) {
    return false;
  }

  // Preferred ATT MTU for this device - just an offer, not a guarantee: the
  // actual negotiated value settles to whichever is lower once the central
  // requests its own exchange, so asking for the BLE spec's own ceiling
  // (517, ESP_GATT_MAX_MTU_SIZE) can only help, never force anything on a
  // phone that offers less. Left at the library default of 23 otherwise,
  // capping every read/write/notify to individual 20-byte payloads - the
  // nRF52/Bluefruit side already asks for the same ceiling on every connect
  // (requestMtuExchange(512+3), Bluetooth.h) instead of a global one-time
  // preference, since Bluefruit's API is per-connection, not global.
  BLEDevice::setMTU(517);

  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9);

  ble_server = BLEDevice::createServer();
  ble_server->setCallbacks(this);
  BLEDevice::setSecurityCallbacks(this);

  SetupSerialService();
  this->startAdvertising();
  return true;
}

void BLESerial::startAdvertising() {
  ble_adv = BLEDevice::getAdvertising();
  ble_adv->addServiceUUID(BLE_SERIAL_SERVICE_UUID);
  ble_adv->setMinPreferred(0x20);
  ble_adv->setMaxPreferred(0x40);
  ble_adv->setScanResponse(true);
  ble_adv->start();
}

void BLESerial::stopAdvertising() {
  ble_adv = BLEDevice::getAdvertising();
  ble_adv->stop();
}

void BLESerial::end() { BLEDevice::deinit(); }

void BLESerial::onWrite(BLECharacteristic *characteristic) {
  if (characteristic->getUUID().toString() == BLE_RX_UUID) {
    auto value = characteristic->getValue();
    for (int i = 0; i < value.length(); i++) { rx_buffer.push(value[i]); }
  }
}

void BLESerial::SetupSerialService() {
  SerialService = ble_server->createService(BLE_SERIAL_SERVICE_UUID);

  // setAccessPermissions(ESP_GATT_PERM_*_ENC_MITM) below is the real
  // enforcement mechanism under Bluedroid, but a silent no-op under NimBLE
  // (BLECharacteristic::setAccessPermissions()'s whole body only exists
  // under CONFIG_BLUEDROID_ENABLED - our actual backend is NimBLE, so it
  // was doing nothing there). The PROPERTY_*_ENC/PROPERTY_*_AUTHEN flags
  // passed into createCharacteristic() below are NimBLE's own real
  // mechanism instead (mapped to BLE_GATT_CHR_F_*_ENC/_AUTHEN) - they're
  // literal no-op 0 under Bluedroid (BLECharacteristic.h's own comment:
  // "Not supported by Bluedroid. Use setAccessPermissions() instead"), so
  // passing both here covers whichever backend is actually compiled in.
  //
  // Deliberately MITM-authenticated by default, not just plain encryption -
  // a Just Works bond (all RNode can ever get against a central whose own
  // IO capability caps out at DisplayYesNo, e.g. KDE's bluedevil/bluez-qt -
  // confirmed via extracting the string literals out of libKF6BluezQt.so:
  // DisplayOnly/DisplayYesNo/KeyboardOnly/NoInputNoOutput are present,
  // KeyboardDisplay is not) has no protection against an active attacker
  // during the pairing handshake itself. bt_just_works_enabled
  // (ADDR_CONF_BT_JUST_WORKS, ROM.h) is the explicit, off-by-default opt-in
  // to relax this to plain _ENCRYPTED instead, which is what actually lets
  // Just-Works-only bonds (e.g. KDE's native Bluetooth settings) stop dying
  // on every real GATT write - any central that can negotiate real Passkey
  // Entry (Windows, Android, or Linux's own `bluetoothctl` with an
  // explicitly registered KeyboardDisplay agent) already clears the
  // stricter bar regardless of this setting, so it's a pure widening, never
  // a downgrade, for hosts that already do better. Read once here rather
  // than checked per-connection because NimBLE bakes characteristic
  // permissions in at creation time - changing this setting requires a
  // bt_stop()/bt_start() cycle to rebuild the GATT service under the new
  // permission level (bt_just_works_conf_save(), Utilities.h), same as
  // toggling Bluetooth off and back on.
  uint32_t rx_props = BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_ENC;
  uint32_t tx_props = BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_READ_ENC;
  esp_gatt_perm_t rx_perm = ESP_GATT_PERM_WRITE_ENCRYPTED;
  esp_gatt_perm_t tx_perm = ESP_GATT_PERM_READ_ENCRYPTED;
  if (!bt_just_works_enabled) {
    rx_props = rx_props | BLECharacteristic::PROPERTY_WRITE_AUTHEN;
    tx_props = tx_props | BLECharacteristic::PROPERTY_READ_AUTHEN;
    rx_perm = ESP_GATT_PERM_WRITE_ENC_MITM;
    tx_perm = ESP_GATT_PERM_READ_ENC_MITM;
  }

  RxCharacteristic = SerialService->createCharacteristic(BLE_RX_UUID, rx_props);
  RxCharacteristic->setAccessPermissions(rx_perm);
  RxCharacteristic->setWriteProperty(true);
  RxCharacteristic->setCallbacks(this);

  // NimBLE auto-adds the 2902 (CCCD) descriptor whenever a characteristic
  // has notify/indicate enabled - manually adding one is deprecated
  // (BLE2902 will be removed) and was already redundant here.
  TxCharacteristic = SerialService->createCharacteristic(BLE_TX_UUID, tx_props);
  TxCharacteristic->setAccessPermissions(tx_perm);
  TxCharacteristic->setNotifyProperty(true);
  TxCharacteristic->setReadProperty(true);

  SerialService->start();
}

BLESerial::BLESerial() { }

#endif
#endif