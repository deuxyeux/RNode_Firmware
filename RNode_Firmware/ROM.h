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

#ifndef ROM_H
  #define ROM_H
  #define CHECKSUMMED_SIZE  0x0B

  // ROM address map ///////////////
  #define ADDR_PRODUCT   0x00
  #define ADDR_MODEL     0x01
  #define ADDR_HW_REV    0x02
  #define ADDR_SERIAL    0x03
  #define ADDR_MADE      0x07
  #define ADDR_CHKSUM    0x0B
  #define ADDR_SIGNATURE 0x1B
  #define ADDR_INFO_LOCK 0x9B

  #define ADDR_CONF_SF   0x9C
  #define ADDR_CONF_CR   0x9D
  #define ADDR_CONF_TXP  0x9E
  #define ADDR_CONF_BW   0x9F
  #define ADDR_CONF_FREQ 0xA3
  #define ADDR_CONF_OK   0xA7

  #define ADDR_CONF_BT   0xB0
  #define ADDR_CONF_DSET 0xB1
  #define ADDR_CONF_DINT 0xB2
  #define ADDR_CONF_DADR 0xB3
  #define ADDR_CONF_DBLK 0xB4
  #define ADDR_CONF_DROT 0xB8
  #define ADDR_CONF_PSET 0xB5
  #define ADDR_CONF_PINT 0xB6
  #define ADDR_CONF_BSET 0xB7
  #define ADDR_CONF_DIA  0xB9
  #define ADDR_CONF_WIFI 0xBA
  #define ADDR_CONF_WCHN 0xBB
  #define ADDR_CONF_SND  0xBC
  #define ADDR_CONF_VSR  0xBD
  #define ADDR_CONF_BVS  0xBE
  #define ADDR_CONF_BUZ  0xBF
  #define ADDR_CONF_EUP  0xC0
  #define ADDR_CONF_EDN  0xC1
  #define ADDR_CONF_EPR  0xC2
  #define ADDR_CONF_ENA  0xC3
  #define ADDR_CONF_ETHSPD 0xC4
  // Display-only UTC offset for the RTC (RTC.h/Menu.h) - raw byte, not a
  // plain signed value (see rtc_get_tz_offset_qh(), RTC.h) so 0x00/0xFF
  // (unset/erased EEPROM) stay unambiguous, same convention as
  // ADDR_CONF_VSR/BVS above.
  #define ADDR_CONF_TZ   0xC5
  // Whether the ESP-NOW virtual interface (vport 1, ESPNOW.h) is allowed to
  // initialize at all - only 0xC6/0xC7 remained free in this region before
  // this, so there's no headroom left after claiming this byte.
  #define ADDR_CONF_ESPNOW 0xC6
  // Whether the WebSocket KISS listener (WebSocketRemote.h) is allowed to
  // initialize at all - this is the last free byte in this checksummed info
  // region (EEPROM_RESERVED starts at 0xC8 below); no further single-byte
  // flags fit here after this one.
  #define ADDR_CONF_WS 0xC7

  #define INFO_LOCK_BYTE 0x73
  #define CONF_OK_BYTE   0x73
  #define BT_ENABLE_BYTE 0x73
  #define SND_ENABLE_BYTE  0x01
  #define SND_DISABLE_BYTE 0x00
  #define ENC_ENABLE_BYTE  0x01
  #define ENC_DISABLE_BYTE 0x00
  #define ESPNOW_ENABLE_BYTE  0x01
  #define ESPNOW_DISABLE_BYTE 0x00
  #define WS_ENABLE_BYTE  0x01
  #define WS_DISABLE_BYTE 0x00
  #define GNSS_ENABLE_BYTE  0x01
  #define GNSS_DISABLE_BYTE 0x00

  #define EEPROM_RESERVED 200

  // Whether the GNSS receiver (HAS_GPS boards, GNSS.h) is allowed to
  // initialize at all, and its duty-cycle update interval (GNSS.h). These
  // previously lived at raw, non-offset physical bytes 0x00/0x01, picked
  // per-MCU-variant on the belief that low memory was "genuinely dead
  // space" on nRF52 (config_addr()'s CONFIG_SIZE region is an ESP32-only
  // concept, so that specific reasoning was correct) - but that reasoning
  // never checked Device.h's own raw addressing scheme for the same low
  // region. On every nRF52 board (EEPROM_SIZE=296, EEPROM_RESERVED=200):
  // DEV_FWHASH_OFFSET = EEPROM_SIZE-EEPROM_RESERVED-DEV_SIG_LEN-DEV_HASH_LEN
  // = 296-200-64-32 = 0, and DEV_SIG_OFFSET = 296-200-64 = 32 - i.e.
  // dev_firmware_hash_target occupies raw bytes 0-31 and dev_sig occupies
  // 32-95, filling the entire "dead" region. Raw byte 0x00 (the old
  // ADDR_CONF_GNSS) was literally dev_firmware_hash_target[0]; toggling
  // GNSS Enabled overwrote it, so the next boot's device_firmware_ok()
  // comparison failed and displayed "Firmware Corrupt" - confirmed on a
  // real T114 (see project memory feedback_gnss_eeprom_fwhash_collision
  // for the full writeup). ADDR_CONF_GNSS_INTERVAL (0x01) had the same
  // flaw against dev_firmware_hash_target[1], just not yet shipped when
  // caught.
  //
  // Fix: despite the comment on ADDR_CONF_WS above claiming the
  // checksummed info region (0x00-0xC7, EEPROM_RESERVED bytes wide) is
  // "completely full," there's an 8-byte gap - 0xA8-0xAF - between
  // ADDR_CONF_OK (0xA7) and ADDR_CONF_BT (0xB0) that was never actually
  // claimed by anything (verified by grepping every ADDR_CONF_* value in
  // this file - it was simply overlooked, not intentionally reserved).
  // These now live there instead, through the normal eeprom_addr() offset
  // like every other ADDR_CONF_* setting - no more raw physical bytes, no
  // more per-MCU_VARIANT branching, and no possibility of colliding with
  // Device.h's raw addressing since that scheme only ever claims bytes
  // below EEPROM_OFFSET, never inside the EEPROM_RESERVED window these
  // sit in. See gnss_conf_save()/gnss_interval_conf_save() (Utilities.h)
  // and their boot-time load (RNode_Firmware.ino), both of which now wrap
  // these in eeprom_addr() like ADDR_CONF_VSR/BVS/TZ above.
  #define ADDR_CONF_GNSS          0xA8
  #define ADDR_CONF_GNSS_INTERVAL 0xA9

  // ESP-NOW's vport 1 (ESPNOW.h) has two independent axes, each its own raw
  // physical byte in the same genuinely-unclaimed 256-823 gap as
  // ADDR_CONF_GNSS just above - NOT through eeprom_addr()/config_addr().
  // Deliberately unconditional, no MCU_VARIANT guard - HAS_ESPNOW is
  // ESP32-only today, so there's nothing to branch on. (Boards.h is now
  // included before this file, Config.h, so MCU_VARIANT would actually be
  // reliable here if ever needed - see that include-order fix's own
  // comment for why ADDR_CONF_GNSS's guard used to silently misresolve.)
  //
  // ADDR_CONF_ESPNOW_MODE: wire framing - v1 (classic, chunked with a
  // 1-byte sequence header) or v2 (single unfragmented frame, no header -
  // required for byte-for-byte interop with attermann/microReticulum's
  // ESPNOWInterface). Purely a framing choice, independent of PHY rate.
  #define ADDR_CONF_ESPNOW_MODE 257
  #define ESPNOW_MODE_V1 0x00
  #define ESPNOW_MODE_V2 0x01

  // ADDR_CONF_ESPNOW_LR: whether 802.11 LR mode (WIFI_PROTOCOL_LR,
  // ESPNOW.h) is active - a PHY/rate choice (range for throughput),
  // independent of ADDR_CONF_ESPNOW_MODE above. "Interop mode" with the
  // reticulum-espnow project specifically means v2 + LR together, but
  // either axis can be set independently - e.g. v1 or v2 with LR off
  // (normal WiFi range/rate), or v1 with LR on (extended range between two
  // RNode_Firmware boards, keeping classic framing).
  #define ADDR_CONF_ESPNOW_LR 258
  #define ESPNOW_LR_ENABLE_BYTE  0x01
  #define ESPNOW_LR_DISABLE_BYTE 0x00

  // Master switch for the onboard microReticulum node (URNS.h) - same
  // unclaimed 256-823 gap as ADDR_CONF_GNSS/ESPNOW_MODE/LR above, raw
  // physical byte, not through eeprom_addr(). Deliberately unconditional,
  // no MCU_VARIANT guard - HAS_URNS is ESP32-only (MeshAdventurer-S3 only)
  // today, same reasoning as ESPNOW_MODE/LR.
  #define ADDR_CONF_URNS 259
  #define URNS_ENABLE_BYTE  0x01
  #define URNS_DISABLE_BYTE 0x00

  // Whether the onboard node participates in RNS transport (relays other
  // nodes' traffic, stores/forwards paths, etc. - RNS::Reticulum::
  // transport_enabled(), same flag ~/Development/microReticulum_Firmware
  // sets unconditionally true as a provisioning default). Ours defaults
  // OFF - this board is deliberately a leaf/client node (see
  // project_microreticulum_onboard_node memory), so relaying is an
  // explicit opt-in via RNode Settings > URNS > Transport Mode, not the
  // out-of-the-box behavior. Same unclaimed 256-823 gap as ADDR_CONF_URNS
  // above.
  #define ADDR_CONF_URNS_TRANSPORT 260
  #define URNS_TRANSPORT_ENABLE_BYTE  0x01
  #define URNS_TRANSPORT_DISABLE_BYTE 0x00

  // RNode Settings > URNS > Link MTU Discovery/Remote Management/Probe
  // Destination - RNS::Reticulum::link_mtu_discovery()/remote_management_
  // enabled()/probe_destination_enabled() (urns_init(), URNS.h). Same
  // unclaimed 256-823 gap as ADDR_CONF_URNS/_TRANSPORT above, same "staged,
  // no self-reboot until SAVE & EXIT" shape.
  #define ADDR_CONF_URNS_LINK_MTU_DISCOVERY 261
  #define URNS_LINK_MTU_DISCOVERY_ENABLE_BYTE  0x01
  #define URNS_LINK_MTU_DISCOVERY_DISABLE_BYTE 0x00

  #define ADDR_CONF_URNS_REMOTE_MGMT 262
  #define URNS_REMOTE_MGMT_ENABLE_BYTE  0x01
  #define URNS_REMOTE_MGMT_DISABLE_BYTE 0x00

  #define ADDR_CONF_URNS_PROBE_DEST 263
  #define URNS_PROBE_DEST_ENABLE_BYTE  0x01
  #define URNS_PROBE_DEST_DISABLE_BYTE 0x00

  // Opt-in escape hatch that makes the ESP32 NimBLE/BLE GATT link
  // (Bluetooth.h, bt_security_setup()) negotiate LE Legacy Pairing instead
  // of forcing LE Secure Connections, so pre-BT-4.2 host controllers
  // (which never support SC) can still pair. Same unclaimed 256-823 gap as
  // ADDR_CONF_URNS/_TRANSPORT/etc above, raw physical byte, no
  // eeprom_addr() wrapper, deliberately unconditional despite being
  // ESP32/HAS_BLE-only (same reasoning as ADDR_CONF_URNS). Default OFF -
  // current SC-forced behavior is unchanged out of the box.
  #define ADDR_CONF_BT_LEGACY_PAIRING 264
  #define BT_LEGACY_PAIRING_ENABLE_BYTE  0x01
  #define BT_LEGACY_PAIRING_DISABLE_BYTE 0x00

  // Opt-in security tradeoff for the ESP32 NimBLE/BLE GATT link
  // (Bluetooth.h, bt_authentication_complete_callback()/BLESerial.cpp,
  // SetupSerialService()): accept a Just Works (encrypted but not
  // MITM-authenticated) bond as sufficient for the RX/TX characteristics,
  // instead of requiring real MITM. Exists because RNode's fixed
  // DisplayOnly IO capability can never actually achieve MITM against a
  // central whose own capability caps out at DisplayYesNo (confirmed via
  // btmon against real hardware - e.g. KDE's bluedevil/bluez-qt) - with
  // MITM required, those bonds get created successfully but then every real
  // GATT read/write is rejected "Insufficient Authentication", which makes
  // the host correctly try to re-pair to upgrade security, an upgrade
  // that's structurally impossible given the same IO capabilities, and the
  // failed re-pair lets the vendored BLE library's own
  // BLE_GAP_EVENT_REPEAT_PAIRING handler delete the bond it just created -
  // i.e. BLE is entirely unusable from such a host with this off. Same
  // unclaimed 256-823 gap as ADDR_CONF_BT_LEGACY_PAIRING above. Default
  // OFF - current strict-MITM behavior is unchanged out of the box; this
  // is a deliberate, explicit security-vs-compatibility choice left to the
  // user, not a bug fix.
  #define ADDR_CONF_BT_JUST_WORKS 265
  #define BT_JUST_WORKS_ENABLE_BYTE  0x01
  #define BT_JUST_WORKS_DISABLE_BYTE 0x00

  // Messenger app's (Messenger.h) LXMF outbound delivery retry count -
  // RNode Settings > Messenger > Settings > Retries in the menu
  // (MENU_STATE_MSNGR_SETTINGS/_EDIT, Menu.h). Same unclaimed 256-823 gap
  // as ADDR_CONF_URNS/BT_LEGACY_PAIRING/BT_JUST_WORKS above, raw physical
  // byte, no eeprom_addr() wrapper. Valid range is 0-5 (see
  // LXMRouter::set_max_delivery_attempts()); an out-of-range/erased
  // (0xFF) value leaves msngr_max_retries at its compiled default (5,
  // same as the Python reference implementation) - see the boot-time load
  // in messenger_init() (Messenger.h), same "if raw < COUNT, use it"
  // shape as ADDR_CONF_GNSS_INTERVAL.
  #define ADDR_CONF_MSNGR_RETRIES 266

  // Whether RNode_Firmware.ino's existing one-shot post-boot LXMF announce
  // (urns_announce_lxmf(), URNS.h, fired ~8s after boot) actually runs -
  // that call itself is unconditional today; this just gates it. NOT
  // wired to LXMRouter's own _announce_at_start/set_announce_at_start()
  // (LXMRouter.h/cpp) - that path fires announce() synchronously inside
  // the constructor, before the radio/TX queue is up, which is exactly
  // the race the existing millis()>8000 one-shot was built to avoid (see
  // urns_announce_lxmf()'s own comment). Default ON (byte value 0x01,
  // not the usual 0x00/absent-EEPROM-is-off convention) since the
  // existing one-shot already fires unconditionally for every board
  // today - an erased/never-configured EEPROM must preserve that, not
  // silently go quiet. Same unclaimed 256-823 gap as ADDR_CONF_MSNGR_
  // RETRIES above.
  #define ADDR_CONF_MSNGR_ANNOUNCE_AT_START 267
  #define MSNGR_ANNOUNCE_AT_START_ENABLE_BYTE  0x01
  #define MSNGR_ANNOUNCE_AT_START_DISABLE_BYTE 0x00

  // Periodic LXMF re-announce interval - stores a preset index (Off/15m/
  // 30m/1h/2h/3h/6h/12h, msngr_announce_interval_presets_s[], Messenger.h)
  // into LXMRouter::set_announce_interval() (seconds), same "preset index,
  // not a raw value" shape as ADDR_CONF_GNSS_INTERVAL. Out-of-range/erased
  // (0xFF) leaves msngr_announce_interval_idx at its compiled default (0 =
  // Off, matching that no periodic auto-announce exists at all before
  // this feature). Same unclaimed 256-823 gap as the two above.
  #define ADDR_CONF_MSNGR_ANNOUNCE_INTERVAL 268

  // Delay (seconds) between outbound LXMF delivery retries -
  // LXMRouter::set_outbound_retry_delay(), raw value not a preset index
  // (unlike Announce Interval above - no natural small preset set for
  // this one). Out-of-range/erased (0xFF, or anything outside the menu's
  // 1-60 range) leaves msngr_retry_delay_s at its compiled default (10,
  // matching LXMRouter's own compiled default). Same unclaimed 256-823
  // gap as the three above.
  #define ADDR_CONF_MSNGR_RETRY_DELAY 269

  // Whether BLE auto-starts at boot (bt_start(), Bluetooth.h) instead of
  // requiring a manual button-hold/menu trigger every power cycle - RNode
  // Settings > Bluetooth > Settings > Auto Start in the menu (Menu.h),
  // alongside Legacy Pairing/Just Works (moved into this same new
  // submenu). Same unclaimed 256-823 gap as ADDR_CONF_BT_LEGACY_PAIRING/
  // BT_JUST_WORKS above, raw physical byte, no eeprom_addr() wrapper.
  // Default OFF (byte 0x00/absent-EEPROM-is-off, the usual convention
  // here) - a new feature must not silently change existing boot
  // behavior; auto-starting BLE costs real internal DRAM (see
  // project_ble_wifi_mutual_exclusivity memory), so this stays an
  // explicit opt-in.
  #define ADDR_CONF_BT_AUTO_START 270
  #define BT_AUTO_START_ENABLE_BYTE  0x01
  #define BT_AUTO_START_DISABLE_BYTE 0x00

  // Whether a configured radio automatically comes online at boot (classic
  // TNC-mode auto-start, RNode_Firmware.ino) - RNode Settings > Radio >
  // Auto Start (Menu.h). Not gated on HAS_URNS - see MENU_STATE_URNS_
  // RADIO_LIST's own comment (Menu.h) for why the Radio menu itself isn't
  // either. Same unclaimed 256-823 gap as the toggles above, raw physical
  // byte, no eeprom_addr() wrapper. Unlike ADDR_CONF_BT_AUTO_START above,
  // erased/never-written (0xFF) means ENABLED (radio_auto_start_enabled's
  // compiled default, Config.h) - this preserves every already-deployed
  // board's existing behavior (a configured RNode has always come up hot),
  // opposite of BT_AUTO_START's "new feature, must opt in" polarity.
  #define ADDR_CONF_RADIO_AUTO_START 271
  #define RADIO_AUTO_START_ENABLE_BYTE  0x01
  #define RADIO_AUTO_START_DISABLE_BYTE 0x00

  #define CONFIG_SIZE     256
  #define ADDR_CONF_SSID 0x00
  #define ADDR_CONF_PSK  0x21
  #define ADDR_CONF_IP   0x42
  #define ADDR_CONF_NM   0x46
  // Wired Ethernet's own static IP/netmask (MeshPoE-S3 only, HAS_ETHERNET) -
  // deliberately separate from ADDR_CONF_IP/NM above, which are WiFi STA-only
  // (see wifi_remote_start_sta(), Remote.h) - a board can have both a WiFi
  // static IP and a wired Ethernet static IP configured independently.
  #define ADDR_CONF_ETH_IP 0x4A
  #define ADDR_CONF_ETH_NM 0x4E
  // Gateway/DNS for each static config above - unset (all-zero/all-0xFF,
  // same convention as addr4_read(), Utilities.h) means "no gateway/DNS
  // configured," not "use DHCP" (DHCP isn't running at all once IP/NM are
  // static) - needed for anything that has to leave the local subnet, e.g.
  // NTP sync (rtc_sync_ntp(), RTC.h) while on a static IP.
  #define ADDR_CONF_GW      0x52
  #define ADDR_CONF_DNS     0x56
  #define ADDR_CONF_ETH_GW  0x5A
  #define ADDR_CONF_ETH_DNS 0x5E
  //////////////////////////////////

#endif
