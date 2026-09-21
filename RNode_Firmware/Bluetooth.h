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
      // For bt_debond_all()'s own direct NVS wipe below - see that
      // function's comment for why ble_store_clear() alone isn't enough.
      #include "nvs_flash.h"
      #include "nvs.h"
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
  // Battery Level Status (0x2BED, BAS v1.1) - BLEBas only covers plain
  // Battery Level (0x2A19), so this is hand-built the same way BLEBas
  // itself is (BLECharacteristic attaches to whichever BLEService most
  // recently called begin() - BLEService::lastService, set by blebas.begin()
  // in bt_start() below - so this must stay ordered right after it). See
  // BLESerial.cpp's UpdateBatteryLevelStatus() for the MCU_ESP32 twin and
  // the bit-packing this mirrors.
  BLECharacteristic blebas_status((uint16_t)0x2BED);
  bool SerialBT_init = false;
#endif

#define BT_PAIRING_TIMEOUT 35000
#define BLE_FLUSH_TIMEOUT 20
uint32_t bt_pairing_started = 0;

#define BT_DEV_ADDR_LEN 6
uint8_t dev_bt_mac[BT_DEV_ADDR_LEN];
char bt_da[BT_DEV_ADDR_LEN];

#if MCU_VARIANT == MCU_ESP32
  // bt_connection_callback()/bt_connect_callback() (below, classic SPP and
  // NimBLE respectively) used to call set_rns_link_state() directly, from
  // their own stack's callback context - Bluedroid's BTC task for SPP,
  // NimBLE's single host task (BLEDevice::host_task -> nimble_port_run(),
  // see bt_pending_pairing_disconnect's own comment below) for BLE -
  // neither of which is loopTask. nRF52's Bluefruit side hit a real,
  // confirmed crash from this identical pattern (see
  // bt_pending_rns_link_state's own comment, MCU_NRF52 branch below -
  // "intermittently wedged the whole node... a full lockup") and was fixed
  // by deferring through a flag instead of calling straight from the
  // callback. ESP32 was never observed to crash from this, but the hazard
  // shape is the same - deferred here too rather than waiting for a first
  // report. Sentinel -1 means "nothing pending" (a real RNS_LINK_STATE_*
  // value is always >= 0).
  volatile int8_t bt_pending_rns_link_state = -1;

  // kiss_indicate_btpin() (Utilities.h) ends in a chain of serial_write()/
  // Serial.write() calls, one byte at a time, with no locking against
  // loopTask's own concurrent KISS output (periodic stat/telemetry frames,
  // radio replies, etc.) - the Arduino core's Serial object isn't
  // synchronized for multi-task access. Called directly from these BLE
  // stacks' own callback context (BTC task for SPP, NimBLE's host task -
  // see this file's own comment above, neither is loopTask) it can
  // interleave byte-for-byte with whatever loopTask is mid-writing,
  // corrupting both frames - the CMD_BT_PIN frame most visibly, since a
  // host reading KISS never resyncs mid-frame and just silently drops the
  // torn bytes. Deferred through this flag instead, same pattern as
  // bt_pending_rns_link_state just above, so the actual indicate always
  // happens from update_bt() on loopTask instead.
  volatile bool bt_pending_pin_indicate = false;

  #if HAS_BLUETOOTH == true

    // How long the passkey has to stay on screen before it's auto-accepted
    // (see bt_confirm_pending below) - long enough for the user to actually
    // read and compare it against their phone's own prompt.
    #define BT_CONFIRM_DISPLAY_MS 2500
    bool bt_confirm_pending = false;
    uint32_t bt_confirm_pending_since = 0;

    void bt_confirm_pairing(uint32_t numVal) {
      bt_ssp_pin = numVal;
      bt_pending_pin_indicate = true;
      if (!bt_allow_pairing) {
        // Peer-initiated pairing (e.g. a phone/PC's own Bluetooth stack
        // starting SSP on its own, before the on-device button-hold gesture
        // - bt_enable_pairing() - was ever used) - accept it and enter the
        // same pairing-display state that gesture would, so the passkey
        // shows up automatically. Mirrors the ESP32 HAS_BLE
        // bt_security_request_callback() and MCU_NRF52 bt_passkey_callback()
        // below, which do the same for their own stacks.
        bt_allow_pairing = true;
        bt_pairing_started = millis();
        bt_state = BT_STATE_PAIRING;
      }
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
        // Only on an actual OFF transition, not on every call - see
        // bt_start()'s own comment below for the caller-side bug this fixes.
        buzzer_bt_off_melody();
      }
    }

    void bt_start() {
      display_unblank();
      if (bt_state == BT_STATE_OFF) {
        SerialBT.begin(bt_devname);
        bt_state = BT_STATE_ON;
        // Played here, gated on bt_state actually reaching ON, rather than
        // by the caller (e.g. the button handler, RNode_Firmware.ino)
        // right after merely calling bt_start() - callers can't tell a
        // real state change from a silent no-op (this variant always
        // succeeds once bt_state was OFF, but the NimBLE bt_start() below
        // can silently no-op for several real reasons, and previously
        // still played the "on" chirp regardless).
        buzzer_bt_on_melody();
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
        // See bt_pending_rns_link_state's own comment (above) for why this
        // is deferred instead of a direct call.
        bt_pending_rns_link_state = RNS_LINK_STATE_DISCONNECTED;
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
      // See bt_pending_rns_link_state's own comment (above) for why this is
      // applied here instead of directly from bt_connection_callback().
      if (bt_pending_rns_link_state != -1) {
        set_rns_link_state((uint8_t)bt_pending_rns_link_state);
        bt_pending_rns_link_state = -1;
      }
      if (bt_pending_pin_indicate) {
        bt_pending_pin_indicate = false;
        kiss_indicate_btpin();
      }
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
    bool bt_setup_hw(); void bt_security_setup(); void bt_update_passkey();
    BLESecurity *ble_security = new BLESecurity();
    bool ble_authenticated = false;
    uint32_t pairing_pin = 0;
    // Whether pairing_pin already carries a properly-entropic value - false
    // until the first time it's rerolled with the BT radio actually up (see
    // bt_connect_callback()'s and bt_enable_pairing()'s own comments on the
    // boot-time weak-entropy bug this guards against). Sticky for the rest
    // of the runtime once true: rerolling again after that point would just
    // invalidate a pairing_pin some host/display has already committed to
    // showing, for no entropy benefit.
    bool bt_passkey_entropy_ok = false;

    // The live connection handle for BLESerial::flush() (BLESerial.cpp) to
    // call ble_gatts_notify_custom() directly with, bypassing
    // BLECharacteristic::notify()'s own internal getConnectedCount()==0
    // check - see BLESerial.cpp's own comment for why that check is
    // unreliable here. server->getConnId()/m_connId (BLEServer, same
    // library) is populated by that same broken bookkeeping and can't be
    // used either; desc->conn_handle here comes from the encryption-change
    // event instead, which fires correctly regardless.
    #if defined(CONFIG_NIMBLE_ENABLED)
    uint16_t ble_conn_handle = 0xFFFF; // BLE_HS_CONN_HANDLE_NONE
    #endif

    // Opt-in compatibility flag - see ADDR_CONF_BT_LEGACY_PAIRING (ROM.h)
    // and bt_security_setup() below. Default OFF, current SC-forced
    // behavior unchanged out of the box.
    bool bt_legacy_pairing_enabled = false;

    // Opt-in security tradeoff - see ADDR_CONF_BT_JUST_WORKS (ROM.h) for
    // the full reasoning. Read by bt_authentication_complete_callback()
    // below and BLESerial.cpp's SetupSerialService(). Default OFF, current
    // strict-MITM behavior unchanged out of the box.
    bool bt_just_works_enabled = false;

    // Whether BLE auto-starts at boot instead of requiring a manual
    // trigger every power cycle - see ADDR_CONF_BT_AUTO_START (ROM.h) for
    // the full reasoning. Read by the one-shot boot check in
    // RNode_Firmware.ino's loop() (same "static bool ...ed = false; if
    // (enabled && !...ed && millis() > threshold)" shape as the LXMF
    // one-shot boot announce, Messenger.h - has to wait past
    // BT_START_MIN_UPTIME_MS the same way a manual press does). Default
    // OFF, current manual-only behavior unchanged out of the box.
    bool bt_auto_start_enabled = false;

    // Deferred post-pairing disconnect (bt_authentication_complete_callback()
    // below) - GAP callbacks run on the single dedicated NimBLE host task
    // (BLEDevice::host_task -> nimble_port_run()), so a blocking delay()
    // there freezes the entire BLE stack's event processing for its
    // duration, precisely during the window right after pairing completes
    // when the phone's own stack is doing its post-pairing setup (MTU
    // exchange, service discovery, connection parameter negotiation) - the
    // same class of bug fixed for the nRF52/Bluefruit side in a169a1c
    // (SoftDevice's own event-dispatch task has the same single-task
    // constraint). The 2s wait itself is kept as-is (undocumented original
    // intent - no commit/comment anywhere explains why pairing-mode
    // connections are deliberately dropped), just moved off the callback.
    bool bt_pending_pairing_disconnect = false;
    uint32_t bt_pairing_disconnect_at = 0;

    void bt_flush() { if (bt_state == BT_STATE_CONNECTED) { SerialBT.flush(); } }

    // Battery level changes far slower than serial TX, so this is a coarse
    // interval, not per-loop like bt_flush() above.
    #define BLE_BATTERY_UPDATE_INTERVAL 30000
    uint32_t bt_last_battery_update = 0;
    void bt_update_battery_service() {
      if (!bt_battery_service_enabled || bt_state != BT_STATE_CONNECTED) return;
      if (millis()-bt_last_battery_update < BLE_BATTERY_UPDATE_INTERVAL) return;
      bt_last_battery_update = millis();
      SerialBT.UpdateBatteryLevel((uint8_t)battery_percent);
      SerialBT.UpdateBatteryLevelStatus();
    }

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
      display_unblank();
      if (bt_state == BT_STATE_OFF) {
        // DIAGNOSTIC ONLY - raw NVS dump of the "nimble_bond" namespace,
        // BEFORE BLEDevice::init()/ble_store_config_init() gets a chance to
        // touch anything. Investigating why a successful pairing (bond_
        // count=1 right after) doesn't survive a reboot (bond_count=0
        // again). This tells us whether the raw stored bytes themselves
        // survive the reboot (a storage-layer problem if they don't) or
        // survive but something in NimBLE's own reload logic fails to load
        // them back into the in-RAM store (a logic-layer problem if they
        // do). Remove after the investigation concludes.
        #if defined(CONFIG_NIMBLE_ENABLED)
        {
          nvs_iterator_t bleiag_it = NULL;
          esp_err_t bleiag_find_rc = nvs_entry_find(NVS_DEFAULT_PART_NAME, "nimble_bond", NVS_TYPE_ANY, &bleiag_it);
          int bleiag_raw_count = 0;
          DEBUG_LOG("[BLEDIAG] raw NVS scan of 'nimble_bond' namespace before init: find_rc=%d\n", (int)bleiag_find_rc);
          while (bleiag_find_rc == ESP_OK && bleiag_it != NULL) {
            nvs_entry_info_t bleiag_info;
            nvs_entry_info(bleiag_it, &bleiag_info);
            DEBUG_LOG("[BLEDIAG]   raw key='%s' type=%d\n", bleiag_info.key, (int)bleiag_info.type);
            bleiag_raw_count++;
            bleiag_find_rc = nvs_entry_next(&bleiag_it);
          }
          nvs_release_iterator(bleiag_it);
          DEBUG_LOG("[BLEDIAG] raw NVS scan done: %d entries found\n", bleiag_raw_count);
        }
        #endif
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
          // DIAGNOSTIC ONLY - checking whether the NimBLE bond/RPA-record
          // store on THIS board is already at/near CONFIG_BT_NIMBLE_MAX_BONDS
          // capacity from real testing history (BLE MITM/multi-client
          // sessions, this same keyboard's own 4+ rotating per-OS-mode
          // identities) - see BLEKeyboardSpike.h investigation. A full RPA
          // record store (ble_store_config.c: ble_store_config_rpa_recs[],
          // sized to MYNEWT_VAL(BLE_STORE_MAX_BONDS)) has already been
          // confirmed to fail writes on this exact board once
          // (bt_debond_all()'s own comment, this file: "rc=27, ESTORE_CAP").
          // Remove after checking.
          #if defined(CONFIG_NIMBLE_ENABLED)
            int bleiag_bond_count = 0;
            ble_store_util_count(BLE_STORE_OBJ_TYPE_PEER_SEC, &bleiag_bond_count);
            DEBUG_LOG("[BLEDIAG] bond_count=%d (CONFIG_BT_NIMBLE_MAX_BONDS=%d)\n", bleiag_bond_count, (int)MYNEWT_VAL(BLE_STORE_MAX_BONDS));
          #endif
          // Gated on bt_state actually reaching ON, not on bt_start() merely
          // being called - this is the variant that can genuinely no-op
          // above (BT_START_MIN_UPTIME_MS) or right here (SerialBT.begin()
          // returning false), most visibly
          // right after boot (the ~10s BT_START_MIN_UPTIME_MS window) - the
          // button handler (RNode_Firmware.ino) used to play the "on" chirp
          // regardless of whether this actually happened, misleadingly
          // suggesting BT had turned on when it hadn't.
          buzzer_bt_on_melody();
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
        buzzer_bt_off_melody();
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
        // ble_store_clear() -> ble_hs_lock() derefs NimBLE host state that
        // only exists once the host has actually started (same landmine as
        // bt_bond_count(), see bt_security_setup()'s comment on it) - the
        // direct NVS erase below is independent of host state either way,
        // so just skip this call rather than crash when bt_state is OFF
        // (e.g. bt_start() silently no-op'd from Menu.h's BT_LIST entry
        // point - BT_START_MIN_UPTIME_MS window, or SerialBT.begin() itself
        // failing).
        if (bt_state != BT_STATE_OFF) ble_store_clear();
        // ble_store_clear() (NimBLE's own "delete every known object type")
        // doesn't reliably clear RPA identity-resolution records too, at
        // least in the exact esp-nimble/esp-idf version bundled with this
        // project - confirmed live: even right after this same call,
        // pairing a SECOND, different central still failed
        // ("ble_store_config_write_rpa_rec rc=27", BLE_HS_ESTORE_CAP - that
        // record store already full from earlier pairing attempts), and it
        // corrupted the FIRST device's own bond in the process (its next
        // reconnect came back fully unencrypted - encrypted=0). Every
        // NimBLE bond/identity record - RPA records ("rpa_rec_N" keys)
        // included - is persisted under one shared "nimble_bond" NVS
        // namespace (ble_store_nvs.c), so erasing that whole namespace
        // directly is a robust guarantee independent of ble_store_clear()'s
        // own object-type coverage in this specific build.
        nvs_handle_t nimble_nvs_handle;
        if (nvs_open("nimble_bond", NVS_READWRITE, &nimble_nvs_handle) == ESP_OK) {
          nvs_erase_all(nimble_nvs_handle);
          nvs_commit(nimble_nvs_handle);
          nvs_close(nimble_nvs_handle);
        }
        // The NimBLE host keeps its own in-RAM copy of every one of these
        // tables (ble_store_config.c's ble_store_config_*_recs[] arrays)
        // and only reloads them from NVS at its own init time
        // (ble_store_config_init(), called from BLEDevice::init()) - so the
        // NVS erase above is invisible to the CURRENTLY running host until
        // it restarts. Cycle it here (only if it was actually running) so
        // Forget Bonds takes full effect immediately, without requiring the
        // user to separately power-cycle the device afterward.
        if (bt_state != BT_STATE_OFF) {
          bt_stop();
          bt_start();
        }
      #endif
    }

    int bt_bond_count() {
      #if defined(CONFIG_BLUEDROID_ENABLED)
        return esp_ble_get_bond_device_num();
      #elif defined(CONFIG_NIMBLE_ENABLED)
        int count = 0;
        ble_store_util_count(BLE_STORE_OBJ_TYPE_PEER_SEC, &count);
        return count;
      #else
        return 0;
      #endif
    }

    void bt_enable_pairing() {
      // Serial.println("BT enable pairing");
      display_unblank();
      if (bt_state == BT_STATE_OFF) bt_start();

      // Set before bt_security_setup() below so it forces a fresh
      // authentication challenge for this pairing window - see that
      // function's own comment on setForceAuthentication().
      bt_allow_pairing = true;
      // Fix the boot-time weak-entropy pairing_pin (see
      // bt_passkey_entropy_ok's own comment) right here, before committing
      // to and announcing a value below - the BT radio has been up for a
      // while by the time anything explicitly arms pairing, so entropy is
      // fine now. Once this call (or bt_connect_callback()'s own, for a
      // peer-initiated pairing that arrives before this was ever called)
      // has done it once, pairing_pin is never rerolled again - manually
      // arming pairing always shows and keeps the same code, rather than a
      // peer connecting later silently swapping it for a different one.
      if (!bt_passkey_entropy_ok) { bt_update_passkey(); bt_passkey_entropy_ok = true; }
      bt_security_setup();

      bt_pairing_started = millis();
      bt_state = BT_STATE_PAIRING;
      bt_ssp_pin = pairing_pin;
      // Unlike the classic-Bluetooth SPP/SSP flow (bt_confirm_pairing,
      // above) and Bluefruit's own passkey callback (nRF52, below), this
      // ESP_IO_CAP_OUT/setPassKey() setup (bt_security_setup() above) is a
      // static passkey configured entirely up front - the NimBLE-Arduino
      // wrapper never calls back into app code with it at pairing time
      // (bt_passkey_callback() below is dead code for this reason, despite
      // looking like the right hook). The passkey is already fully known
      // the moment pairing mode is armed here, so indicate it immediately
      // instead of waiting for a callback that will never come - this is
      // also the only KISS notification a peer ever gets for this flow,
      // since a peer that already knows the code (typed from the OLED)
      // never triggers any further app-level callback either.
      bt_pending_pin_indicate = true;
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
      // This is the callback that actually fires for RNode's DisplayOnly IO
      // capability under NimBLE (BLE_GAP_EVENT_PASSKEY_ACTION's
      // BLE_SM_IOACT_DISP case calls onPassKeyNotify(), not onPassKeyRequest()
      // - see BLEServer.cpp, framework-arduinoespressif32/libraries/BLE/src -
      // onPassKeyRequest()/bt_passkey_callback() below is only reached for
      // BLE_SM_IOACT_INPUT, i.e. a device with a keyboard capability, which
      // RNode never has). bt_security_request_callback()'s own auto-accept
      // (above) is Bluedroid-only (ESP_GAP_BLE_SEC_REQ_EVT, BLEDevice.cpp -
      // gated #if CONFIG_BLUEDROID_ENABLED, dead code under this project's
      // actual NimBLE backend) and never runs, so THIS is the real gate a
      // peer-initiated pairing has to clear - unconditionally disconnecting
      // here whenever bt_allow_pairing was still false (the pre-auto-pairing
      // behavior) tore down the link before the passkey was ever shown or a
      // bond could form, on every peer-initiated attempt, regardless of
      // bt_just_works_enabled (which only affects GATT permission bits and
      // bt_authentication_complete_callback()'s acceptance criteria, not
      // this gate).
      if (!bt_allow_pairing) {
        bt_allow_pairing = true;
        bt_state = BT_STATE_PAIRING;
      }
      bt_ssp_pin = passkey;
      bt_pairing_started = millis();
      bt_pending_pin_indicate = true;
    }

    bool bt_confirm_pin_callback(uint32_t pin) {
      // Serial.printf("Confirm PIN callback: %d\n", pin);
      return true;
    }

    void bt_update_passkey() {
      // Serial.println("Updating passkey");
      // esp_random() directly, NOT the Arduino random() wrapper - confirmed
      // live that the same passkey was recurring across repeated reboots
      // even after regenerating on every connect (bt_connect_callback()
      // above). Root cause: RNode_Firmware.ino's own setup() seeds the CSMA
      // R-value selector with randomSeed((unsigned long)esp_random()) very
      // early on - and WMath.cpp's randomSeed() unconditionally sets
      // s_useRandomHW = false the moment it's ever called, for the rest of
      // the firmware's life, switching every subsequent random() call
      // anywhere in the firmware (this one included) from the hardware TRNG
      // over to a plain seeded rand(). If that one early esp_random() seed
      // itself lacks enough boot-to-boot variation (the same "too early for
      // real entropy" hazard, just relocated to that one call site instead
      // of this one), every random() call downstream of it - including this
      // one, no matter when or how often it's called afterward - replays
      // the same deterministic sequence every boot. esp_random() itself is
      // ESP-IDF's own TRNG API, entirely unrelated to Arduino's random()/
      // randomSeed() layer, so calling it directly here sidesteps the
      // problem regardless of what randomSeed() elsewhere has done -
      // without touching that CSMA seeding, which is a separate concern.
      pairing_pin = 100000 + (esp_random() % 900000);
      bt_ssp_pin = pairing_pin;
    }

    uint32_t bt_passkey_callback() {
      // Serial.println("API passkey request");
      if (pairing_pin == 0) { bt_update_passkey(); }
      // Surface the passkey we're about to display over KISS too (CMD_BT_PIN,
      // Utilities.h) - this is the "we generate and display our own passkey"
      // path (Passkey Entry, responder displays - the RNode's fixed
      // DisplayOnly IO capability), distinct from bt_passkey_notify_callback()
      // above (the "host displays, we confirm" case), which already did this.
      // Without it there was no way to read the passkey except physically
      // looking at the OLED.
      bt_pending_pin_indicate = true;
      return pairing_pin;
    }

    bool bt_client_authenticated() {
      return ble_authenticated;
    }

    bool bt_security_request_callback() {
      // Serial.println("Accepting security request");
      if (!bt_allow_pairing) {
        // Peer (central) initiated a pairing/security request on its own,
        // e.g. a phone/PC's BLE stack bonding automatically the moment it
        // tries to read/write the ENC_MITM-gated RX/TX characteristics
        // (SetupSerialService(), BLESerial.cpp) - before the on-device
        // button-hold gesture (bt_enable_pairing()) was ever used. Accept it
        // and enter the same pairing-display state that gesture would, the
        // same way Meshtastic nodes auto-display a passkey for a
        // client-initiated pairing rather than requiring a physical step
        // first. setForceAuthentication (bt_security_setup() below) is
        // deliberately left alone here - this only changes whether an
        // incoming request is granted, not whether the RNode proactively
        // sends one on every connect (see that function's own comment for
        // why forcing that unconditionally previously broke bonded
        // reconnects).
        bt_allow_pairing = true;
        bt_pairing_started = millis();
        bt_state = BT_STATE_PAIRING;
      }
      return true;
    }

    #if defined(CONFIG_BLUEDROID_ENABLED)
    void bt_authentication_complete_callback(esp_ble_auth_cmpl_t auth_result) {
      if (auth_result.success == true) {
        // Serial.println("Authentication success");
        ble_authenticated = true;
        // A successful authentication - whether this was an explicit
        // pairing-mode attempt or an ordinary reconnect - ends the window
        // that needs a forced fresh challenge. bt_allow_pairing (below)
        // resets the app-level flag, but BLESecurity::m_forceSecurity is a
        // separate static the library never re-syncs on its own - leaving
        // it stuck true (from bt_enable_pairing()) makes the RNode keep
        // proactively sending a Security Request on every future ordinary
        // reconnect, racing that reconnect's own passive bond-resume
        // (LE Start Encryption using the stored LTK) and corrupting the
        // bond - confirmed via btmon: a real reconnect's "LE Start
        // Encryption" (old LTK) raced a stray "SMP: Security Request" from
        // this device, which forced BlueZ into a fresh "SMP: Pairing
        // Request" that then failed ("Authentication requirements"), and
        // the vendored library's BLE_GAP_EVENT_REPEAT_PAIRING handler
        // (BLEServer.cpp) unconditionally deletes the existing bond the
        // moment a peer re-attempts pairing on an already-bonded link -
        // leaving zero bonds and every subsequent GATT write rejected with
        // Insufficient Authentication.
        BLESecurity::setForceAuthentication(false);
        if (bt_state == BT_STATE_PAIRING) {
          // Serial.println("Pairing complete, disconnecting");
          // See bt_pending_pairing_disconnect's own comment (above) for why
          // this doesn't disconnect synchronously here.
          bt_pending_pairing_disconnect = true;
          bt_pairing_disconnect_at = millis() + 2000;
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
      // desc->sec_state has 3 separate bits: encrypted, authenticated (MITM
      // specifically), and bonded. Gated on .authenticated by default -
      // RX/TX's own GATT permissions (BLESerial.cpp, SetupSerialService())
      // require real MITM too by default, and that's a deliberate policy
      // choice (see that function's own comment) - a Just Works pairing
      // (all that's achievable against a central whose IO capability caps
      // out at DisplayYesNo, e.g. KDE's bluedevil) will correctly take the
      // "failure" branch below despite the link being encrypted, since it
      // doesn't meet the security level this device requires by default.
      // bt_just_works_enabled (ADDR_CONF_BT_JUST_WORKS, ROM.h) is the
      // explicit opt-in to relax this to plain .encrypted instead - kept in
      // sync with BLESerial.cpp's own permission level so the RNode's app
      // state (ble_authenticated) never disagrees with what the stack will
      // actually allow - reporting "connected" and then failing every real
      // write would be worse than failing here, consistently, the same way
      // bt_security_setup()'s retry-with-forced-auth (bt_allow_pairing
      // still true mid-pairing) already expects.
      if (desc->sec_state.authenticated || (bt_just_works_enabled && desc->sec_state.encrypted)) {
        ble_authenticated = true;
        ble_conn_handle = desc->conn_handle;
        // Re-sync BLESecurity's own forced-auth flag now that this
        // authentication has succeeded - see this function's own comment
        // above for the full btmon-confirmed failure mode this fixes.
        BLESecurity::setForceAuthentication(false);
        if (bt_state == BT_STATE_PAIRING) {
          // See bt_pending_pairing_disconnect's own comment (above) for why
          // this doesn't disconnect synchronously here.
          bt_pending_pairing_disconnect = true;
          bt_pairing_disconnect_at = millis() + 2000;
        } else {
          bt_state = BT_STATE_CONNECTED;
          // Actively negotiate PHY/data length now that security has
          // settled, rather than leaving both at whatever the central
          // defaults to - same intent as the nRF52/Bluefruit side's own
          // requestPHY()/requestDataLengthUpdate() on connect (Bluetooth.h,
          // further down). No Espressif-library wrapper exists for either
          // call (BLEServer/BLEDevice only expose MTU/connParams helpers),
          // so these go straight to the raw NimBLE host API already visible
          // via BLEDevice.h's own <host/ble_gap.h> include.
          //
          // Deliberately NOT done from bt_connect_callback() (right on
          // connect) - confirmed live via CORE_DEBUG_LEVEL=5 capture that
          // firing these HCI-level GAP commands while a resumed/bonded
          // connection's own encryption procedure is still in flight
          // collides on the controller's single HCI command queue
          // ("NimBLE: HCI wait for ack returned 19", BLE_HS_ETIMEOUT_HCI) -
          // reliably corrupting that encryption procedure and reporting
          // authenticated=false, which this file's own failure branch below
          // then treated as a real auth failure, endlessly rotating the
          // passkey and re-disconnecting on every reconnect attempt. A
          // fresh pairing has enough slack (interactive passkey exchange)
          // to not visibly collide, which is why this only ever showed up
          // on the *second* (resumed) connection - only visible with a
          // debug UART capturing the underlying HCI trace, not from
          // anything at the KISS/application layer. This is a real
          // regression a prior session (this one) introduced by adding
          // this negotiation directly in bt_connect_callback() - moving it
          // here (after the security procedure that raced it has already
          // finished) avoids the collision while keeping the negotiation.
          ble_gap_set_prefered_le_phy(desc->conn_handle, BLE_GAP_LE_PHY_2M_MASK, BLE_GAP_LE_PHY_2M_MASK, BLE_GAP_LE_PHY_CODED_ANY);
          ble_gap_set_data_len(desc->conn_handle, 251, 2120); // BLE spec max payload/tx-time
        }
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
      // Only if pairing_pin's boot-time weak entropy was never already
      // fixed (bt_passkey_entropy_ok) - covers a peer-initiated pairing
      // that arrives before bt_enable_pairing() was ever explicitly called
      // (nobody's shown/committed to a value yet, so this is the first safe
      // opportunity - BLE_GAP_EVENT_PASSKEY_ACTION, BLEServer.cpp, reads
      // BLESecurity::getPassKey() and locks it in before our own
      // bt_passkey_notify_callback() ever runs, so it can only be changed
      // here, ahead of time, not reactively once displayed). Once fixed
      // once (here or in bt_enable_pairing()), never rerolls again -
      // manually arming pairing (bt_enable_pairing()) always shows and
      // keeps that same code, rather than every connection after it
      // (including an ordinary reconnect from an already-bonded peer, which
      // lands here too and would never go through an interactive passkey
      // exchange at all) silently swapping it for a different one.
      if (!bt_passkey_entropy_ok) {
        bt_update_passkey();
        bt_passkey_entropy_ok = true;
        BLESecurity::setPassKey(true, pairing_pin);
        // Indicate over KISS too - this is the only such notification a
        // peer-initiated pairing (bt_enable_pairing() never called) gets.
        bt_pending_pin_indicate = true;
      }
      // See bt_pending_rns_link_state's own comment (above) for why this is
      // deferred instead of a direct call - this callback runs on NimBLE's
      // own host task, not loopTask.
      bt_pending_rns_link_state = RNS_LINK_STATE_DISCONNECTED;
    }

    void bt_disconnect_callback(BLEServer *server) {
      uint16_t conn_id = server->getConnId();
      // Serial.printf("Disconnected: %d\n", conn_id);
      display_unblank();
      ble_authenticated = false;
      ble_conn_handle = 0xFFFF; // BLE_HS_CONN_HANDLE_NONE
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
        if (EEPROM.read(ADDR_CONF_BT_LEGACY_PAIRING) == BT_LEGACY_PAIRING_ENABLE_BYTE) {
          bt_legacy_pairing_enabled = true;
        } else {
          bt_legacy_pairing_enabled = false;
        }
        if (EEPROM.read(ADDR_CONF_BT_JUST_WORKS) == BT_JUST_WORKS_ENABLE_BYTE) {
          bt_just_works_enabled = true;
        } else {
          bt_just_works_enabled = false;
        }
        if (EEPROM.read(ADDR_CONF_BT_AUTO_START) == BT_AUTO_START_ENABLE_BYTE) {
          bt_auto_start_enabled = true;
        } else {
          bt_auto_start_enabled = false;
        }
        // Inverted polarity vs the three reads above - erased/never-written
        // (0xFF) means ENABLED, same as ADDR_CONF_RADIO_AUTO_START. See
        // ADDR_CONF_BT_BATTERY_SERVICE (ROM.h) for why.
        if (EEPROM.read(ADDR_CONF_BT_BATTERY_SERVICE) == BT_BATTERY_SERVICE_DISABLE_BYTE) {
          bt_battery_service_enabled = false;
        } else {
          bt_battery_service_enabled = true;
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
      BLESecurity::setAuthenticationMode(bt_legacy_pairing_enabled ? ESP_LE_AUTH_REQ_BOND_MITM : ESP_LE_AUTH_REQ_SC_MITM_BOND);
      BLESecurity::setCapability(ESP_IO_CAP_OUT);
      BLESecurity::setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
      BLESecurity::setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
      BLESecurity::setKeySize(16);
      BLESecurity::setPassKey(true, pairing_pin);
      // true only during the explicit on-device pairing window
      // (bt_allow_pairing, set by bt_enable_pairing() before calling this) -
      // that's what makes the RNode proactively send a Security Request on
      // connect, which is what makes a generic OS pairing dialog (KDE/GNOME
      // Bluetooth settings, "click device -> asks for PIN") actually prompt.
      // Forcing it unconditionally broke that flow (regression) - it must
      // stay false for an ordinary reconnect to an already-bonded host
      // (Windows/bleak reconnecting via a cached OS-level bond), which is
      // the bug this flag was introduced to fix in the first place - see
      // "Fix BLE reconnect and notification delivery on Windows".
      //
      // This function also runs at boot (bt_setup_hw(), before bt_start()
      // has ever brought the NimBLE host up - see bt_init()'s own comment
      // on why that's deferred) - the bond-count half of this decision
      // (bt_connect_callback() below layers in bt_bond_count()==0 on top of
      // this same flag, for connections specifically) MUST NOT live here:
      // bt_bond_count() -> ble_store_util_count() -> ble_hs_lock() derefs
      // NimBLE host state that doesn't exist yet at boot time - confirmed
      // live, a hard LoadProhibited crash-loop (never reaches BT ready)
      // when this was tried directly in this function.
      BLESecurity::setForceAuthentication(bt_allow_pairing);
    }

    void update_bt() {
      // See bt_pending_rns_link_state's own comment (above) for why this is
      // applied here instead of directly from bt_connect_callback().
      if (bt_pending_rns_link_state != -1) {
        set_rns_link_state((uint8_t)bt_pending_rns_link_state);
        bt_pending_rns_link_state = -1;
      }
      if (bt_pending_pin_indicate) {
        bt_pending_pin_indicate = false;
        kiss_indicate_btpin();
      }
      if (bt_pending_pairing_disconnect && (int32_t)(millis()-bt_pairing_disconnect_at) >= 0) {
        bt_pending_pairing_disconnect = false;
        SerialBT.disconnect();
      }
      if (bt_allow_pairing && millis()-bt_pairing_started >= BT_PAIRING_TIMEOUT) {
        bt_disable_pairing();
      }
      if (bt_state == BT_STATE_CONNECTED && millis()-SerialBT.lastFlushTime >= BLE_FLUSH_TIMEOUT) {
        if (SerialBT.transmitBufferLength > 0) {
          bt_flush();
        }
      }
      bt_update_battery_service();
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
  // link-state chirps were added) did non-reentrant work - tone()/noTone()
  // on the shared PWM peripheral plus several buzzer_async_* globals
  // (Utilities.h) that loop()'s own buzzer_update() polled and mutated
  // every iteration with no locking, since it was always written assuming
  // a single caller task. Racing loop() for that hardware/state from a
  // second task intermittently wedged the whole node (observed as a full
  // lockup, not just BLE misbehaving, once a BLE central actually
  // connected/paired and this path started firing) - deferred through
  // this flag instead, so the actual set_rns_link_state() call happens
  // from update_bt(), polled from loop() like everything else that
  // touches shared firmware state.
  //
  // The buzzer itself no longer has this hazard - it's now its own
  // FreeRTOS task with a queue any caller can post to safely (Utilities.h,
  // buzzer_request_melody()), and set_rns_link_state() (Utilities.h) does
  // nothing else besides that chirp and a plain rns_link_state assignment
  // - so calling it straight from this callback would be safe again now.
  // Left deferred through this flag anyway rather than removed: fully
  // removing it (going back to a direct call) is a separate, untested
  // simplification, not this one's job. The ESP32 side has the identical
  // pattern, in both its Bluetooth stack variants (classic SPP's
  // bt_connection_callback(), NimBLE's bt_connect_callback()) - see its
  // own bt_pending_rns_link_state (MCU_ESP32 branch, above) for the
  // matching fix, added once this nRF52 history made the hazard shape
  // obvious there too, even though it had never been observed to crash.
  volatile int8_t bt_pending_rns_link_state = -1;

  // See the MCU_ESP32 branch's own bt_pending_pin_indicate for why -
  // Bluefruit's passkey/security callbacks run on their own SoftDevice
  // event-handling task, not loopTask, and kiss_indicate_btpin() ending in
  // unsynchronized Serial.write() calls can interleave with loopTask's own
  // concurrent KISS output otherwise.
  volatile bool bt_pending_pin_indicate = false;

  uint8_t eeprom_read(uint32_t mapped_addr);

  void bt_stop() {
    // Serial.println("BT Stop");
    if (bt_state != BT_STATE_OFF) {
      bt_allow_pairing = false;
      // See the ESP32 HAS_BLUETOOTH bt_stop() for why this is needed.
      bt_ssp_pin = 0;
      // Also needed, unlike the ESP32 HAS_BLUETOOTH version: bt_get_passkey()
      // (below) only calls bt_update_passkey() - the only place that sets
      // bt_ssp_pin non-zero again - when pairing_pin is 0. Without this,
      // pairing_pin survives this cancel at its old nonzero value, so the
      // next bt_enable_pairing() -> bt_get_passkey() skips regenerating it,
      // leaving bt_ssp_pin stuck at 0 forever even though bt_state correctly
      // returns to BT_STATE_PAIRING - which silently kept the on-device
      // pairing banner (Display.h, gates on bt_ssp_pin != 0) from ever
      // reappearing after a cancel (button short-tap, CMD_BT_CTRL 0x00, or
      // the long-hold console-entry path - anything that calls bt_stop()
      // rather than bt_disable_pairing(), which already reset this).
      pairing_pin = 0;
      bt_state = BT_STATE_OFF;
      buzzer_bt_off_melody();
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
    // Display the passkey the SoftDevice actually generated for this
    // pairing attempt (6 ASCII digit bytes, same format Bluefruit's setPIN()
    // takes) rather than trusting our own separately-tracked pairing_pin -
    // by the time this callback fires the SoftDevice has already committed
    // to whichever value setPIN() last configured, and on a peer-initiated
    // pairing (below) that may not have been refreshed since the last
    // manual bt_enable_pairing() call. Same numVal-from-the-stack pattern
    // the ESP32 classic-Bluetooth SPP path already uses (bt_confirm_pairing,
    // above).
    uint32_t numeric_passkey = 0;
    for (int i = 0; i < 6; i++) { numeric_passkey = numeric_passkey * 10 + (passkey[i] - '0'); }
    bt_ssp_pin = numeric_passkey;
    bt_pending_pin_indicate = true;
    if (!bt_allow_pairing) {
      // Peer-initiated pairing (e.g. a phone/PC's BLE stack bonding on its
      // own) before the on-device button-hold gesture (bt_enable_pairing())
      // was ever used - accept it and enter the same pairing-display state
      // that gesture would. Mirrors the ESP32 HAS_BLE
      // bt_security_request_callback() and HAS_BLUETOOTH bt_confirm_pairing()
      // above, which do the same for their own stacks.
      bt_allow_pairing = true;
      bt_pairing_started = millis();
      bt_state = BT_STATE_PAIRING;
    }
    return true;
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
      // Inverted polarity vs ADDR_CONF_BT above - erased/never-written
      // (0xFF) means ENABLED, same as ADDR_CONF_RADIO_AUTO_START. See
      // ADDR_CONF_BT_BATTERY_SERVICE (ROM.h) for why. Raw physical byte,
      // no eeprom_addr() wrapper - same as ADDR_CONF_RADIO_AUTO_START's own
      // read (RNode_Firmware.ino).
      #if HAS_EEPROM
          if (EEPROM.read(ADDR_CONF_BT_BATTERY_SERVICE) == BT_BATTERY_SERVICE_DISABLE_BYTE) {
      #else
          if (eeprom_read(ADDR_CONF_BT_BATTERY_SERVICE) == BT_BATTERY_SERVICE_DISABLE_BYTE) {
      #endif
        bt_battery_service_enabled = false;
      } else {
        bt_battery_service_enabled = true;
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
      // Skippable outright - blebas is never referenced by Advertising or
      // any other module, so leaving it unregistered fully hides the
      // Battery Service rather than just leaving it unwritten. See
      // bt_battery_service_conf_save() (Utilities.h) for how a live toggle
      // takes effect (stop/start cycle).
      if (bt_battery_service_enabled) {
        blebas.begin();
        // Must stay immediately after blebas.begin() - BLECharacteristic::
        // begin() attaches to BLEService::lastService, which blebas.begin()
        // (a BLEService) just set. Any other service's begin() in between
        // would misattach this. Same READ+NOTIFY, no security requirement
        // as blebas's own Battery Level characteristic (SECMODE_OPEN).
        blebas_status.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
        blebas_status.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
        blebas_status.setFixedLen(3);
        blebas_status.begin();
      }

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
      buzzer_bt_on_melody();
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
    bt_pending_pin_indicate = true;
  }

  void bt_debond_all() {
    // Was a no-op - CMD_BT_UNPAIR silently did nothing on nRF52. Mirrors
    // the ESP32 HAS_BLE version's own "just wipe stored bond keys, don't
    // touch any active connection" semantics - Bluefruit.Periph.clearBonds()
    // wraps bond_clear_prph() (Bluefruit52Lib), which deletes and recreates
    // the peripheral-role bond directory on internal flash.
    Bluefruit.Periph.clearBonds();
  }

  int bt_bond_count() {
    // Bluefruit stores each bonded peripheral-role peer as its own file
    // under InternalFS, one directory per role (bonding.cpp,
    // BOND_DIR_PRPH="/adafruit/bond_prph") - created by bond_init() inside
    // Bluefruit.begin() (bt_setup_hw() above), so it exists once bt_ready
    // is true regardless of the BLE stack's current on/off state. Counted
    // directly via the public Adafruit_LittleFS API rather than pulling in
    // Bluefruit52Lib's own non-public utility/bonding.h.
    int count = 0;
    File bond_dir("/adafruit/bond_prph", FILE_O_READ, InternalFS);
    File bond_file(InternalFS);
    while ((bond_file = bond_dir.openNextFile(FILE_O_READ))) {
      if (!bond_file.isDirectory()) count++;
      bond_file.close();
    }
    bond_dir.close();
    return count;
  }

  void update_bt() {
    // Apply any RNS link-state transition a BLE callback deferred (see
    // bt_pending_rns_link_state's own comment above).
    if (bt_pending_pin_indicate) {
      bt_pending_pin_indicate = false;
      kiss_indicate_btpin();
    }
    if (bt_pending_rns_link_state != -1) {
      set_rns_link_state((uint8_t)bt_pending_rns_link_state);
      bt_pending_rns_link_state = -1;
    }
    if (bt_allow_pairing && millis()-bt_pairing_started >= BT_PAIRING_TIMEOUT) {
      bt_disable_pairing();
    }
  }
#endif
