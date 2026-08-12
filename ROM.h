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
  // initialize at all. The checksummed info region above (0x00-0xC7,
  // EEPROM_RESERVED bytes wide) is completely full - ADDR_CONF_WS is
  // explicitly the last free byte there. Growing EEPROM_RESERVED to make
  // room was tried and reverted: it shifts EEPROM_OFFSET, which reindexes
  // every eeprom_addr()-mapped byte, including this device's own already-
  // provisioned ADDR_PRODUCT/MODEL/HW_REV/CHKSUM - on real hardware that
  // silently misaligns those reads against data written under the old
  // offset, breaking eeprom_product_valid()/eeprom_checksum_valid() and
  // producing exactly the "missing config" state this was tested against.
  // Instead, this uses a raw, non-offset physical byte, picked per-platform
  // since "genuinely free space" is a different address on each - see
  // gnss_conf_save() (Utilities.h) and its boot-time load
  // (RNode_Firmware.ino), both of which use this value directly rather than
  // through eeprom_addr()/config_addr().
  //
  // This #if requires MCU_VARIANT (Boards.h) to already be defined -
  // Config.h includes Boards.h before this file specifically so that's
  // true. Get that order backwards and both MCU_VARIANT and
  // MCU_NRF52/MCU_ESP32 are undefined here, so the comparison below
  // degenerates to "0 == 0" and silently always takes the nRF52 branch
  // (0x00) regardless of the real target - confirmed empirically on real
  // hardware, this was a live bug for a while (see project memory
  // feedback_rom_h_include_order_undef_macro for the full writeup) that
  // corrupted the first byte of ESP32's WiFi/Ethernet config storage
  // whenever GNSS's enable byte was read or written.
  #if MCU_VARIANT == MCU_NRF52
    // The low "config" region (CONFIG_SIZE/CONFIG_OFFSET, only ever defined
    // in Boards.h's MCU_ESP32 scope) is genuinely dead space on every nRF52
    // board, since config_addr() never even compiles there.
    #define ADDR_CONF_GNSS 0x00
  #elif MCU_VARIANT == MCU_ESP32
    // CONFIG_SIZE (256, below) reserves 0x00-0xFF for WiFi/Ethernet config -
    // real usage there stops at ADDR_CONF_ETH_DNS+4=0x62, but the declared
    // 256-byte width is treated as off-limits headroom for that region, not
    // free space. EEPROM_OFFSET is 824 on the default 1024-byte ESP32
    // EEPROM_SIZE (1024-200), so 256-823 is a genuinely unclaimed gap -
    // this just takes its first byte.
    #define ADDR_CONF_GNSS 256
  #endif

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
