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

#if MCU_VARIANT == MCU_ESP32

#elif MCU_VARIANT == MCU_NRF52
#endif

#if MCU_VARIANT == MCU_ESP32
  #if HAS_BLUETOOTH == true
    #include "BluetoothSerial.h"
    #include "esp_mac.h"
    BluetoothSerial SerialBT;
  #elif HAS_BLE == true
    #include "esp_mac.h"
    #include "BLESerial.h"
    #if defined(CONFIG_NIMBLE_ENABLED)
      #include <host/ble_store.h>
    #endif
    BLESerial SerialBT;
  #else
    #include "esp_mac.h"
  #endif

#elif MCU_VARIANT == MCU_NRF52
  #include <bluefruit.h>
  #include <math.h>
  #define BLE_RX_BUF 6144
  BLEUart SerialBT(BLE_RX_BUF);
  BLEDis  bledis;
  BLEBas  blebas;
  bool SerialBT_init = false;
#endif

#define BT_PAIRING_TIMEOUT 35000
#define BLE_FLUSH_TIMEOUT 20
uint32_t bt_pairing_started = 0;

#define BT_DEV_ADDR_LEN 6
uint8_t dev_bt_mac[BT_DEV_ADDR_LEN];
char bt_da[BT_DEV_ADDR_LEN];

#if MCU_VARIANT == MCU_ESP32
  #if HAS_BLUETOOTH == true

    // How long the passkey has to stay on screen before it's auto-accepted
    // (see bt_confirm_pending below) - long enough for the user to actually
    // read and compare it against their phone's own prompt.
    #define BT_CONFIRM_DISPLAY_MS 2500
    bool bt_confirm_pending = false;
    uint32_t bt_confirm_pending_since = 0;

    void bt_confirm_pairing(uint32_t numVal) {
      bt_ssp_pin = numVal;
      kiss_indicate_btpin();
      if (bt_allow_pairing) {
        // Deferred to update_bt() rather than calling confirmReply(true)
        // right here - esp_bt_gap_ssp_confirm_reply() (which this wraps)
        // doesn't have to be called synchronously from within this GAP
        // callback, and Bluedroid just leaves the negotiation paused until
        // it is. Confirming immediately let auth complete (and
        // bt_pairing_complete()/bt_disable_pairing() zero bt_ssp_pin again)
        // before the display's own ~7fps update_display() cadence
        // necessarily landed a redraw in between - the passkey only ever
        // reliably made it on screen if that race happened to go its way,
        // otherwise it flashed for well under a second or not at all.
        bt_confirm_pending = true;
        bt_confirm_pending_since = millis();
      } else {
        SerialBT.confirmReply(false);
      }
    }

    void bt_stop() {
      display_unblank();
      if (bt_state != BT_STATE_OFF) {
        SerialBT.end();
        bt_allow_pairing = false;
        bt_confirm_pending = false;
        // Otherwise a pairing interrupted after the confirmation PIN is set
        // (bt_confirm_pairing()) but before bt_disable_pairing()/
        // bt_pairing_complete() runs leaves bt_ssp_pin stuck non-zero forever -
        // which permanently forces the normal UI in draw_disp_area() regardless
        // of disp_ext_fb, since it doesn't check bt_state at all.
        bt_ssp_pin = 0;
        bt_state = BT_STATE_OFF;
      }
    }

    void bt_start() {
      display_unblank();
      if (bt_state == BT_STATE_OFF) {
        SerialBT.begin(bt_devname);
        bt_state = BT_STATE_ON;
       }
    }

    void bt_enable_pairing() {
      display_unblank();
      if (bt_state == BT_STATE_OFF) bt_start();
      bt_allow_pairing = true;
      bt_pairing_started = millis();
      bt_state = BT_STATE_PAIRING;
    }

    void bt_disable_pairing() {
      display_unblank();
      bt_allow_pairing = false;
      bt_confirm_pending = false;
      bt_ssp_pin = 0;
      bt_state = BT_STATE_ON;
    }

    void bt_pairing_complete(boolean success) {
      display_unblank();
      if (success) {
        bt_disable_pairing();
      } else {
        bt_ssp_pin = 0;
      }
    }

    void bt_connection_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
      display_unblank();
      if(event == ESP_SPP_SRV_OPEN_EVT) {
        bt_state = BT_STATE_CONNECTED;
        set_rns_link_state(RNS_LINK_STATE_DISCONNECTED);
      }
       
      if(event == ESP_SPP_CLOSE_EVT ){
        bt_state = BT_STATE_ON;
      }
    }

    bool bt_setup_hw() {
      if (!bt_ready) {
        if (EEPROM.read(eeprom_addr(ADDR_CONF_BT)) == BT_ENABLE_BYTE) {
          bt_enabled = true;
        } else {
          bt_enabled = false;
        }
        uint8_t mac[BT_DEV_ADDR_LEN];
        esp_read_mac(mac, ESP_MAC_BT);
        char *data = (char*)malloc(BT_DEV_ADDR_LEN+1);
        for (int i = 0; i < BT_DEV_ADDR_LEN; i++) { data[i] = mac[i]; }
        data[BT_DEV_ADDR_LEN] = EEPROM.read(eeprom_addr(ADDR_SIGNATURE));
        unsigned char *hash = MD5::make_hash(data, BT_DEV_ADDR_LEN);
        memcpy(bt_dh, hash, BT_DEV_HASH_LEN);
        sprintf(bt_devname, "RNode %02X%02X", bt_dh[14], bt_dh[15]);
        free(data);

        SerialBT.enableSSP();
        SerialBT.onConfirmRequest(bt_confirm_pairing);
        SerialBT.onAuthComplete(bt_pairing_complete);
        SerialBT.register_callback(bt_connection_callback);

        bt_ready = true;
        return true;
      } else { return false; }
    }

    bool bt_init() {
        bt_state = BT_STATE_OFF;
        if (bt_setup_hw()) {
          if (bt_enabled && !console_active) bt_start();
          return true;
        } else {
          return false;
        }
    }

    void update_bt() {
      if (bt_confirm_pending && millis()-bt_confirm_pending_since >= BT_CONFIRM_DISPLAY_MS) {
        bt_confirm_pending = false;
        // bt_allow_pairing may have gone false since the request came in
        // (e.g. BT_PAIRING_TIMEOUT firing mid-delay, bt_disable_pairing()
        // above already clears bt_confirm_pending too, but re-check here
        // in case of any other path that flips it) - honor that instead of
        // blindly accepting.
        SerialBT.confirmReply(bt_allow_pairing);
      }
      if (bt_allow_pairing && millis()-bt_pairing_started >= BT_PAIRING_TIMEOUT) {
        bt_disable_pairing();
      }
    }

  #elif HAS_BLE == true
    #if HAS_URNS == true && (HAS_WIFI == true || HAS_ETHERNET == true)
      // BLE and WiFi/Ethernet networking can't both be up at once on
      // HAS_URNS boards - the internal-DRAM budget doesn't have room for
      // both. Confirmed live: URNS/Reticulum's own init costs ~53KB,
      // WiFi Remote's STA/lwIP bring-up costs ~50KB more (not ESP-NOW
      // itself, which is cheap - it's the underlying WiFi.mode() call),
      // leaving too little margin for bt_start()'s own ~78-82KB
      // nimble_port_init() spike. A failed/partial bt_start() doesn't
      // fail cleanly either - it drains the heap enough (observed: under
      // 1KB free afterward) to also break the still-running WiFi stack,
      // which is worse than just refusing up front. See
      // project_meshpoe_s3_ble_urns_memory_crunch memory for the full
      // budget breakdown.
      //
      // One-directional by design (matches what was actually asked for):
      // this only blocks BLE from coming up while networking is active.
      // It does NOT tear down BLE if WiFi/Ethernet gets turned on while
      // BLE is already running - that reverse case can still recreate the
      // same conflict and isn't guarded here.
      #if HAS_WIFI == true
        #include <WiFi.h>
      #endif
      #if HAS_ETHERNET == true
        extern bool eth_disabled;
      #endif
      bool ble_networking_conflict() {
        #if HAS_WIFI == true
          if (WiFi.getMode() != WIFI_MODE_NULL) return true;
        #endif
        #if HAS_ETHERNET == true
          if (!eth_disabled) return true;
        #endif
        return false;
      }
    #endif
    bool bt_setup_hw(); void bt_security_setup();
    BLESecurity *ble_security = new BLESecurity();
    bool ble_authenticated = false;
    uint32_t pairing_pin = 0;

    void bt_flush() { if (bt_state == BT_STATE_CONNECTED) { SerialBT.flush(); } }

    // Minimum uptime before it's safe to actually bring up the NimBLE
    // stack (SerialBT.begin() below, which wraps BLEDevice::init() ->
    // nimble_port_init()). Confirmed live on hardware: a button press in
    // roughly the first 6-7s after boot lands while WiFi's own driver
    // init/connect sequence is still active, and the two contend for the
    // shared radio/controller - nimble_port_init() fails ("rc=-1 Unknown
    // ESP_ERR error") and BLEDevice::createServer() right after it derefs
    // state that was never set up, crashing with an unhandled
    // LoadProhibited exception rather than erroring out gracefully. 10s
    // gives real margin past the observed ~6-7s danger window. A press
    // this early just silently does nothing rather than crashing - the
    // device is still mid-boot-sequence at this point anyway.
    #define BT_START_MIN_UPTIME_MS 10000
    void bt_start() {
      // Serial.println("BT start");
      if (millis() < BT_START_MIN_UPTIME_MS) return;
      #if HAS_URNS == true && (HAS_WIFI == true || HAS_ETHERNET == true)
        // See ble_networking_conflict()'s own comment (above) for why -
        // silent no-op, matching the BT_START_MIN_UPTIME_MS guard just
        // above.
        if (ble_networking_conflict()) return;
      #endif
      display_unblank();
      if (bt_state == BT_STATE_OFF) {
        // SerialBT.begin() (BLESerial::begin(), BLESerial.cpp) now returns
        // false instead of crashing when the underlying BLEDevice::init()
        // fails (confirmed live: a WiFi/BLE coexistence race can still make
        // nimble_port_init() fail even past BT_START_MIN_UPTIME_MS above -
        // that guard reduces how often this is hit, it doesn't guarantee
        // it can't happen). Only claim bt_state == ON if init actually
        // succeeded, so a failed attempt can be retried by pressing again
        // instead of getting stuck showing BT as on with nothing running.
        bool ok = SerialBT.begin(bt_devname);
        if (ok) {
          bt_state = BT_STATE_ON;
          SerialBT.setTimeout(10);
        }
      }
    }

    void bt_stop() {
      // Serial.println("BT stop");
      display_unblank();
      if (bt_state != BT_STATE_OFF) {
        bt_allow_pairing = false;
        // See the HAS_BLUETOOTH bt_stop() above for why this is needed.
        bt_ssp_pin = 0;
        bt_state = BT_STATE_OFF;
        SerialBT.end();
      }
    }

    bool bt_init() {
      // Serial.println("BT init");
      bt_state = BT_STATE_OFF;
      if (bt_setup_hw()) {
        // Deliberately NOT calling bt_start() here anymore - measured
        // live that it alone accounts for ~82KB of the ~137KB BLE+
        // ESP-NOW together were consuming
        // out of ~185KB available after display init, leaving only
        // ~30KB for everything else (LXMF/RNS processing, crypto, etc.)
        // - the direct cause of this session's whole crash investigation
        // (heap exhaustion, confirmed via mbedTLS/esp-aes allocation
        // failures under load). bt_setup_hw() above (MAC/name/security
        // config) is cheap and still runs unconditionally so the device
        // is ready to pair the moment it's asked to - bt_start() (the
        // actual SerialBT.begin(), which is what allocates the BLE
        // stack) now only happens on-demand, the same way it already
        // does when re-enabling via bt_enable_pairing() (the button-hold
        // gesture) after an explicit bt_stop(). bt_enabled (EEPROM
        // preference) still gates whether pairing is allowed at all, via
        // bt_enable_pairing()'s own check - it just no longer means
        // "start BLE automatically at every boot".
        return true;
      } else {
        return false;
      }
    }

    void bt_debond_all() {
      // Serial.println("Debonding all");
      #if defined(CONFIG_BLUEDROID_ENABLED)
        int dev_num = esp_ble_get_bond_device_num();
        esp_ble_bond_dev_t *dev_list = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
        esp_ble_get_bond_device_list(&dev_num, dev_list);
        for (int i = 0; i < dev_num; i++) { esp_ble_remove_bond_device(dev_list[i].bd_addr); }
        free(dev_list);
      #elif defined(CONFIG_NIMBLE_ENABLED)
        ble_store_clear();
      #endif
    }

    void bt_enable_pairing() {
      // Serial.println("BT enable pairing");
      display_unblank();
      if (bt_state == BT_STATE_OFF) bt_start();

      bt_security_setup();

      bt_allow_pairing = true;
      bt_pairing_started = millis();
      bt_state = BT_STATE_PAIRING;
      bt_ssp_pin = pairing_pin;
    }

    void bt_disable_pairing() {
      // Serial.println("BT disable pairing");
      display_unblank();
      bt_allow_pairing = false;
      bt_ssp_pin = 0;
      bt_state = BT_STATE_ON;
    }

    void bt_passkey_notify_callback(uint32_t passkey) {
      // Serial.printf("Got passkey notification: %d\n", passkey);
      if (bt_allow_pairing) {
        bt_ssp_pin = passkey;
        bt_pairing_started = millis();
        kiss_indicate_btpin();
      } else {
        // Serial.println("Pairing not allowed, re-init");
        SerialBT.disconnect();
      }
    }

    bool bt_confirm_pin_callback(uint32_t pin) {
      // Serial.printf("Confirm PIN callback: %d\n", pin);
      return true;
    }

    void bt_update_passkey() {
      // Serial.println("Updating passkey");
      pairing_pin = random(899999)+100000;
      bt_ssp_pin = pairing_pin;
    }

    uint32_t bt_passkey_callback() {
      // Serial.println("API passkey request");
      if (pairing_pin == 0) { bt_update_passkey(); }
      return pairing_pin;
    }

    bool bt_client_authenticated() {
      return ble_authenticated;
    }

    bool bt_security_request_callback() {
      if (bt_allow_pairing) {
          // Serial.println("Accepting security request");
          return true;
        } else {
          // Serial.println("Rejecting security request");
          return false;
        }
    }

    #if defined(CONFIG_BLUEDROID_ENABLED)
    void bt_authentication_complete_callback(esp_ble_auth_cmpl_t auth_result) {
      if (auth_result.success == true) {
        // Serial.println("Authentication success");
        ble_authenticated = true;
        if (bt_state == BT_STATE_PAIRING) {
          // Serial.println("Pairing complete, disconnecting");
          delay(2000); SerialBT.disconnect();
        } else { bt_state = BT_STATE_CONNECTED; }
      } else {
        // Serial.println("Authentication fail");
        ble_authenticated = false;
        bt_state = BT_STATE_ON;
        bt_update_passkey();
        bt_security_setup();
      }
      bt_allow_pairing = false;
      bt_ssp_pin = 0;
    }
    #elif defined(CONFIG_NIMBLE_ENABLED)
    void bt_authentication_complete_callback(ble_gap_conn_desc *desc) {
      if (desc->sec_state.authenticated) {
        ble_authenticated = true;
        if (bt_state == BT_STATE_PAIRING) {
          delay(2000); SerialBT.disconnect();
        } else { bt_state = BT_STATE_CONNECTED; }
      } else {
        ble_authenticated = false;
        bt_state = BT_STATE_ON;
        bt_update_passkey();
        bt_security_setup();
      }
      bt_allow_pairing = false;
      bt_ssp_pin = 0;
    }
    #endif

    void bt_connect_callback(BLEServer *server) {
      uint16_t conn_id = server->getConnId();
      // Serial.printf("Connected: %d\n", conn_id);
      display_unblank();
      ble_authenticated = false;
      if (bt_state != BT_STATE_PAIRING) { bt_state = BT_STATE_CONNECTED; }
      set_rns_link_state(RNS_LINK_STATE_DISCONNECTED);
    }

    void bt_disconnect_callback(BLEServer *server) {
      uint16_t conn_id = server->getConnId();
      // Serial.printf("Disconnected: %d\n", conn_id);
      display_unblank();
      ble_authenticated = false;
      bt_state = BT_STATE_ON;
    }

    bool bt_setup_hw() {
      // Serial.println("BT setup hw");
      if (!bt_ready) {
        if (EEPROM.read(eeprom_addr(ADDR_CONF_BT)) == BT_ENABLE_BYTE) {
          bt_enabled = true;
        } else {
          bt_enabled = false;
        }
        uint8_t mac[BT_DEV_ADDR_LEN];
        esp_read_mac(mac, ESP_MAC_BT);
        char *data = (char*)malloc(BT_DEV_ADDR_LEN+1);
        for (int i = 0; i < BT_DEV_ADDR_LEN; i++) { data[i] = mac[i]; }
        data[BT_DEV_ADDR_LEN] = EEPROM.read(eeprom_addr(ADDR_SIGNATURE));
        unsigned char *hash = MD5::make_hash(data, BT_DEV_ADDR_LEN);
        memcpy(bt_dh, hash, BT_DEV_HASH_LEN);
        sprintf(bt_devname, "RNode %02X%02X", bt_dh[14], bt_dh[15]);
        free(data);

        bt_security_setup();

        bt_ready = true;
        return true;
      } else { return false; }
    }

    void bt_security_setup() {
      // Serial.println("Executing BT security setup");
      if (pairing_pin == 0) { bt_update_passkey(); }
      // Serial.printf("Passkey is %d\n", pairing_pin);
      BLESecurity::setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
      BLESecurity::setCapability(ESP_IO_CAP_OUT);
      BLESecurity::setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
      BLESecurity::setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
      BLESecurity::setKeySize(16);
      BLESecurity::setPassKey(true, pairing_pin);
      BLESecurity::setForceAuthentication(true);
    }

    void update_bt() {
      if (bt_allow_pairing && millis()-bt_pairing_started >= BT_PAIRING_TIMEOUT) {
        bt_disable_pairing();
      }
      if (bt_state == BT_STATE_CONNECTED && millis()-SerialBT.lastFlushTime >= BLE_FLUSH_TIMEOUT) {
        if (SerialBT.transmitBufferLength > 0) {
          bt_flush();
        }
      }
    }
  #else
    bool bt_init() {
      uint8_t mac[BT_DEV_ADDR_LEN];
      esp_read_mac(mac, ESP_MAC_BT);
      char *data = (char*)malloc(BT_DEV_ADDR_LEN+1);
      for (int i = 0; i < BT_DEV_ADDR_LEN; i++) { data[i] = mac[i]; }
      data[BT_DEV_ADDR_LEN] = EEPROM.read(eeprom_addr(ADDR_SIGNATURE));
      unsigned char *hash = MD5::make_hash(data, BT_DEV_ADDR_LEN);
      memcpy(bt_dh, hash, BT_DEV_HASH_LEN);
      sprintf(bt_devname, "RNode %02X%02X", bt_dh[14], bt_dh[15]);
      free(data);
      return true;
    }
    void update_bt() { }
  #endif

#elif MCU_VARIANT == MCU_NRF52
    uint32_t pairing_pin = 0;

  // bt_connect_callback()/bt_pairing_complete() (below) are Bluefruit
  // Security/Periph callbacks - the Adafruit nRF52 core dispatches these
  // from its own SoftDevice event-handling task, not from loop()'s task.
  // They used to call set_rns_link_state() directly, which (since the RNS
  // link-state chirps were added) does non-reentrant work - tone()/
  // noTone() on the shared PWM peripheral plus several buzzer_async_*
  // globals (Utilities.h) - that loop()'s own buzzer_update() polls and
  // mutates every iteration with no locking, since it was always written
  // assuming a single caller task. Racing loop() for that hardware/state
  // from a second task intermittently wedged the whole node (observed as
  // a full lockup, not just BLE misbehaving, once a BLE central actually
  // connected/paired and this path started firing) - deferred through
  // this flag instead, so the actual set_rns_link_state() call happens
  // from update_bt(), polled from loop() like everything else that
  // touches shared firmware state.
  volatile int8_t bt_pending_rns_link_state = -1;

  uint8_t eeprom_read(uint32_t mapped_addr);

  void bt_stop() {
    // Serial.println("BT Stop");
    if (bt_state != BT_STATE_OFF) {
      bt_allow_pairing = false;
      // See the ESP32 HAS_BLUETOOTH bt_stop() for why this is needed.
      bt_ssp_pin = 0;
      bt_state = BT_STATE_OFF;
    }
  }

  void bt_flush() { if (bt_state == BT_STATE_CONNECTED) { SerialBT.flushTXD(); } }

  void bt_disable_pairing() {
    // Serial.println("BT Disable pairing");
    bt_allow_pairing = false;
    pairing_pin = 0;
    bt_ssp_pin = 0;
    bt_state = BT_STATE_ON;
  }

  void bt_pairing_complete(uint16_t conn_handle, uint8_t auth_status) {
    // Serial.println("BT pairing complete");
    BLEConnection* connection = Bluefruit.Connection(conn_handle);
    if (auth_status == BLE_GAP_SEC_STATUS_SUCCESS) {
      ble_gap_conn_sec_mode_t security = connection->getSecureMode();
      // Serial.println("Bonding success");

      // On the NRF52 it is not possible with the Arduino library to reject
      // requests from devices with no IO capabilities, which would allow
      // bypassing pin entry through pairing using the "just works" mode.
      // Therefore, we must check the security level of the connection after
      // pairing to ensure "just works" has not been used. If it has, we need
      // to disconnect, unpair and delete any bonding information immediately.
      // Settings on the SerialBT service should prevent unauthorised access to
      // the serial port anyway, but this is still wise to do regardless.
      //
      // Note: It may be nice to have this done in the BLESecurity class in the
      // future, but as it stands right now I'd have to fork the BSP to do
      // that, which I don't fancy doing. Impact on security is likely minimal.
      // Requires investigation.

      if (security.sm == 1 && security.lv >= 3) {
          // Serial.println("Auth level success");
          bt_state = BT_STATE_CONNECTED;
          bt_pending_rns_link_state = RNS_LINK_STATE_DISCONNECTED;
          connection->disconnect();
          bt_disable_pairing();
      } else {
          // Serial.println("Auth level failure, debonding");
          if (connection->bonded()) { connection->removeBondKey(); }
          connection->disconnect();
          bt_disable_pairing();
      }
    } else {
      // Serial.println("Bonding failure");
      connection->disconnect();
      bt_disable_pairing();
    }
  }

  bool bt_passkey_callback(uint16_t conn_handle, uint8_t const passkey[6], bool match_request) {
    // Serial.println("Passkey callback");
    if (bt_allow_pairing) {
      return true;
    }
    return false;
  }

  void bt_connect_callback(uint16_t conn_handle) {
    // Serial.println("Connect callback");
    bt_state = BT_STATE_CONNECTED;
    bt_pending_rns_link_state = RNS_LINK_STATE_DISCONNECTED;

    BLEConnection* conn = Bluefruit.Connection(conn_handle);
    conn->requestPHY(BLE_GAP_PHY_2MBPS);
    conn->requestMtuExchange(512+3);
    conn->requestDataLengthUpdate();
  }

  void bt_disconnect_callback(uint16_t conn_handle, uint8_t reason) {
    // Serial.println("Disconnect callback");
    if (reason != BLE_GAP_SEC_STATUS_SUCCESS) {
        bt_state = BT_STATE_ON;
    }
  }

  void bt_update_passkey() {
    // Serial.println("Update passkey");
    pairing_pin = random(899999)+100000;
    bt_ssp_pin = pairing_pin;
  }

  uint32_t bt_get_passkey() {
    // Serial.println("API passkey request");
    if (pairing_pin == 0) { bt_update_passkey(); }
    return pairing_pin;
  }

  bool bt_setup_hw() {
    // Serial.println("Setup HW");
    if (!bt_ready) {
      #if HAS_EEPROM 
          if (EEPROM.read(eeprom_addr(ADDR_CONF_BT)) == BT_ENABLE_BYTE) {
      #else
          if (eeprom_read(eeprom_addr(ADDR_CONF_BT)) == BT_ENABLE_BYTE) {
      #endif
        bt_enabled = true;
      } else {
        bt_enabled = false;
      }
      Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
      Bluefruit.autoConnLed(false);
      if (Bluefruit.begin()) {
        uint32_t pin = bt_get_passkey();
        char pin_char[6];
        sprintf(pin_char,"%lu", pin);

        Bluefruit.setTxPower(8);    // Check bluefruit.h for supported values
        Bluefruit.Security.setIOCaps(true, false, false); // display, yes; yes / no, no; keyboard, no
        // This device is indeed capable of yes / no through the pairing mode
        // being set, but I have chosen to set it thus to force the input of the
        // pin on the device initiating the pairing.

        Bluefruit.Security.setMITM(true);
        Bluefruit.Security.setPairPasskeyCallback(bt_passkey_callback);
        Bluefruit.Security.setSecuredCallback(bt_connect_callback);
        Bluefruit.Security.setPIN(pin_char);
        Bluefruit.Periph.setDisconnectCallback(bt_disconnect_callback);
        Bluefruit.Security.setPairCompleteCallback(bt_pairing_complete);
        Bluefruit.Periph.setConnInterval(6, 12); // 7.5 - 15 ms

        const ble_gap_addr_t gap_addr = Bluefruit.getAddr();
        char *data = (char*)malloc(BT_DEV_ADDR_LEN+1);
        for (int i = 0; i < BT_DEV_ADDR_LEN; i++) {
            data[i] = gap_addr.addr[i];
        }
        #if HAS_EEPROM 
            data[BT_DEV_ADDR_LEN] = EEPROM.read(eeprom_addr(ADDR_SIGNATURE));
        #else
            data[BT_DEV_ADDR_LEN] = eeprom_read(eeprom_addr(ADDR_SIGNATURE));
        #endif
        unsigned char *hash = MD5::make_hash(data, BT_DEV_ADDR_LEN);
        memcpy(bt_dh, hash, BT_DEV_HASH_LEN);
        sprintf(bt_devname, "RNode %02X%02X", bt_dh[14], bt_dh[15]);
        free(data);

        bt_ready = true;
        return true;

      } else { return false; }
    } else { return false; }
  }

  void bt_start() {
    // Serial.println("BT Start");
    if (bt_state == BT_STATE_OFF) {
      Bluefruit.setName(bt_devname);
      bledis.setManufacturer(BLE_MANUFACTURER);
      bledis.setModel(BLE_MODEL);
      // start device information service
      bledis.begin();
      blebas.begin();

      // Guard to ensure SerialBT service is not duplicated through BT being power cycled
      if (!SerialBT_init) {
          SerialBT.bufferTXD(true); // enable buffering

          SerialBT.setPermission(SECMODE_ENC_WITH_MITM, SECMODE_ENC_WITH_MITM); // enable encryption for BLE serial
          SerialBT.begin();
          SerialBT_init = true;
      }

      Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
      Bluefruit.Advertising.addTxPower();

      // Include bleuart 128-bit uuid
      Bluefruit.Advertising.addService(SerialBT);

      // There is no room for Name in Advertising packet
      // Use Scan response for Name
      Bluefruit.ScanResponse.addName();

      Bluefruit.Advertising.start(0);

      bt_state = BT_STATE_ON;
     }
  }

  bool bt_init() {
    // Serial.println("BT init");
    bt_state = BT_STATE_OFF;
    if (bt_setup_hw()) {
      if (bt_enabled && !console_active) bt_start();
      return true;
    } else {
      return false;
    }
  }

  void bt_enable_pairing() {
    // Serial.println("BT enable pairing");
    if (bt_state == BT_STATE_OFF) bt_start();

    uint32_t pin = bt_get_passkey();
    char pin_char[6];
    sprintf(pin_char,"%lu", pin);
    Bluefruit.Security.setPIN(pin_char);

    bt_allow_pairing = true;
    bt_pairing_started = millis();
    bt_state = BT_STATE_PAIRING;
    kiss_indicate_btpin();
  }

  void bt_debond_all() { }

  void update_bt() {
    // Apply any RNS link-state transition a BLE callback deferred (see
    // bt_pending_rns_link_state's own comment above) - safe here, loop()'s
    // own task, same one buzzer_update() runs on.
    if (bt_pending_rns_link_state != -1) {
      set_rns_link_state((uint8_t)bt_pending_rns_link_state);
      bt_pending_rns_link_state = -1;
    }
    if (bt_allow_pairing && millis()-bt_pairing_started >= BT_PAIRING_TIMEOUT) {
      bt_disable_pairing();
    }
  }
#endif
