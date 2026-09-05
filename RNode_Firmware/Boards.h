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

#include "Modem.h"

#ifndef BOARDS_H
  #define BOARDS_H

  #define PLATFORM_AVR        0x90
  #define PLATFORM_ESP32      0x80
  #define PLATFORM_NRF52      0x70

  #define MCU_1284P           0x91
  #define MCU_2560            0x92
  #define MCU_ESP32           0x81
  #define MCU_NRF52           0x71

  // Products, boards and models ////
  #define PRODUCT_RNODE       0x03 // RNode devices
  #define BOARD_RNODE         0x31 // Original v1.0 RNode
  #define MODEL_A4            0xA4 // RNode v1.0, 433 MHz
  #define MODEL_A9            0xA9 // RNode v1.0, 868 MHz

  #define BOARD_RNODE_NG_20   0x40 // RNode hardware revision v2.0
  #define MODEL_A3            0xA3 // RNode v2.0, 433 MHz
  #define MODEL_A8            0xA8 // RNode v2.0, 868 MHz

  #define BOARD_RNODE_NG_21   0x41 // RNode hardware revision v2.1
  #define MODEL_A2            0xA2 // RNode v2.1, 433 MHz
  #define MODEL_A7            0xA7 // RNode v2.1, 868 MHz

  #define BOARD_T3S3          0x42 // T3S3 devices
  #define MODEL_A1            0xA1 // T3S3, 433 MHz with SX1268
  #define MODEL_A5            0xA5 // T3S3, 433 MHz with SX1278
  #define MODEL_A6            0xA6 // T3S3, 868 MHz with SX1262
  #define MODEL_AA            0xAA // T3S3, 868 MHz with SX1276
  #define MODEL_AC            0xAC // T3S3, 2.4 GHz with SX1280 and PA

  #define PRODUCT_TBEAM       0xE0 // T-Beam devices
  #define BOARD_TBEAM         0x33
  #define MODEL_E4            0xE4 // T-Beam SX1278, 433 Mhz
  #define MODEL_E9            0xE9 // T-Beam SX1276, 868 Mhz
  #define MODEL_E3            0xE3 // T-Beam SX1268, 433 Mhz
  #define MODEL_E8            0xE8 // T-Beam SX1262, 868 Mhz

  #define PRODUCT_TDECK_V1    0xD0
  #define BOARD_TDECK         0x3B
  #define MODEL_D4            0xD4 // LilyGO T-Deck, 433 MHz
  #define MODEL_D9            0xD9 // LilyGO T-Deck, 868 MHz

  #define PRODUCT_TBEAM_S_V1  0xEA
  #define PRODUCT_TBEAM_S_V3  0xEC
  #define BOARD_TBEAM_S_V1    0x3D
  #define BOARD_TBEAM_S_V3    0x43
  #define MODEL_DB            0xDB // LilyGO T-Beam Supreme, 433 MHz
  #define MODEL_DC            0xDC // LilyGO T-Beam Supreme, 868 MHz

  #define PRODUCT_TBEAM_1W    0xE1 // LilyGO T-Beam 1W devices
  #define BOARD_TBEAM_1W      0x45
  #define MODEL_E5            0xE5 // LilyGO T-Beam 1W, 433 MHz
  #define MODEL_E6            0xE6 // LilyGO T-Beam 1W, 868 MHz

  #define PRODUCT_XIAO_S3     0xEB
  #define BOARD_XIAO_S3       0x3E
  #define MODEL_DE            0xDE // Xiao ESP32S3 with Wio-SX1262 module, 433 MHz
  #define MODEL_DD            0xDD // Xiao ESP32S3 with Wio-SX1262 module, 868 MHz

  #define PRODUCT_T32_10      0xB2
  #define BOARD_LORA32_V1_0   0x39
  #define MODEL_BA            0xBA // LilyGO T3 v1.0, 433 MHz
  #define MODEL_BB            0xBB // LilyGO T3 v1.0, 868 MHz

  #define PRODUCT_T32_20      0xB0
  #define BOARD_LORA32_V2_0   0x36
  #define MODEL_B3            0xB3 // LilyGO T3 v2.0, 433 MHz
  #define MODEL_B8            0xB8 // LilyGO T3 v2.0, 868 MHz

  #define PRODUCT_T32_21      0xB1
  #define BOARD_LORA32_V2_1   0x37
  #define MODEL_B4            0xB4  // LilyGO T3 v2.1, 433 MHz
  #define MODEL_B9            0xB9  // LilyGO T3 v2.1, 868 MHz

  #define PRODUCT_H32_V2      0xC0  // Board code 0x38
  #define BOARD_HELTEC32_V2   0x38
  #define MODEL_C4            0xC4  // Heltec Lora32 v2, 433 MHz
  #define MODEL_C9            0xC9  // Heltec Lora32 v2, 868 MHz

  #define PRODUCT_H32_V3      0xC1
  #define BOARD_HELTEC32_V3   0x3A
  #define MODEL_C5            0xC5 // Heltec Lora32 v3, 433 MHz
  #define MODEL_CA            0xCA // Heltec Lora32 v3, 868 MHz

  #define PRODUCT_H32_V4      0xC3
  #define BOARD_HELTEC32_V4   0x3F
  #define MODEL_C8            0xC8 // Heltec Lora32 v3, 850-950 MHz, 28dBm

  #define PRODUCT_HELTEC_T114 0xC2 // Heltec Mesh Node T114
  #define BOARD_HELTEC_T114   0x3C
  #define MODEL_C6            0xC6 // Heltec Mesh Node T114, 470-510 MHz
  #define MODEL_C7            0xC7 // Heltec Mesh Node T114, 863-928 MHz

  #define PRODUCT_HELTEC_T096 0xD1 // Heltec Mesh Node T096
  #define BOARD_HELTEC_T096   0xD2
  #define MODEL_D3            0xD3 // Heltec Mesh Node T096, 470-510 MHz
  #define MODEL_D5            0xD5 // Heltec Mesh Node T096, 863-928 MHz

  #define PRODUCT_TECHO       0x15 // LilyGO T-Echo devices
  #define BOARD_TECHO         0x44
  #define MODEL_16            0x16 // T-Echo 433 MHz
  #define MODEL_17            0x17 // T-Echo 868/915 MHz

  #define PRODUCT_RAK4631     0x10
  #define BOARD_RAK4631       0x51
  #define MODEL_11            0x11 // RAK4631, 433 Mhz
  #define MODEL_12            0x12 // RAK4631, 868 Mhz

  // RAK WisMesh 1W Booster (RAK3401 WisBlock Core + RAK13302 SX1262 +
  // SKY66122-11 1W FEM module). No physical unit available to validate
  // against - pin mapping and FEM control scheme cross-checked between
  // Meshtastic's own variant def (~/Development/meshtastic_firmware/
  // variants/nrf52840/rak3401_1watt) and MeshCore's (~/Development/MeshCore/
  // variants/rak3401), which agree on every pin and on the FEM topology -
  // same no-hardware-yet precedent as BOARD_TBEAM_1W/BOARD_HELTEC_T1 above.
  // Single MODEL byte only - the SKY66122-11 FEM is a 863-928 MHz-only part
  // (no 433 MHz RAK13302 SKU exists), unlike RAK4631's bare-SX1262 pair.
  #define PRODUCT_RAK3401     0x18
  #define BOARD_RAK3401       0x48
  #define MODEL_13            0x13 // RAK3401 1W Booster (RAK13302), 863-928 MHz

  #define PRODUCT_HMBRW       0xF0
  #define BOARD_HMBRW         0x32
  #define BOARD_HUZZAH32      0x34
  #define BOARD_GENERIC_ESP32 0x35
  #define BOARD_GENERIC_NRF52 0x50
  #define MODEL_FD            0xFD // Homebrew board with E22-xxxM33S, max 33dBm output power (clamped max txpower to 8)
  #define MODEL_FE            0xFE // Homebrew board, max 17dBm output power
  #define MODEL_FF            0xFF // Homebrew board, max 14dBm output power

  #define BOARD_MESHPOE_S3        0xF1 // MeshPoE-S3
  #define BOARD_MESHADVENTURER_S3 0xF2 // MeshAdventurer-S3
  #define BOARD_AETHERNODE        0xF3 // Aethernode
  #define BOARD_MESHADVENTURER    0xF4 // MeshAdventurer
  #define BOARD_PROMICRO          0xF5 // FakeTec (Promicro)
  #define BOARD_DIY_V1            0xF6 // DIY-V1
  #define BOARD_AETHERNODE_S3     0xF7 // Aethernode-S3

  // Heltec Wireless Tracker V2 (ESP32-S3FN8 + SX1262 + KCT8103L FEM +
  // ST7735S 160x80 TFT + UC6580 GNSS - same display/GNSS/FEM as
  // BOARD_HELTEC_T096, just on ESP32-S3 instead of nRF52). No physical unit
  // available to validate against - pin mapping cross-checked between
  // Meshtastic's own variant def (~/Development/meshtastic_firmware/
  // variants/esp32s3/heltec_wireless_tracker_v2) and MeshCore's
  // (~/Development/MeshCore/variants/heltec_tracker_v2), which agree on
  // every pin - same no-hardware-yet precedent as BOARD_TBEAM_1W above.
  #define PRODUCT_HELTEC_WTRACKER_V2  0xD8 // Heltec Wireless Tracker V2
  #define BOARD_HELTEC_WTRACKER_V2    0x46
  #define MODEL_D6                    0xD6 // Heltec Wireless Tracker V2, 470-510 MHz
  #define MODEL_D7                    0xD7 // Heltec Wireless Tracker V2, 863-928 MHz

  // Heltec Mesh Node T1 (nRF52840, HT-mesh-node-t1 core - already installed
  // locally, unlike T096/T114's own CI-mirrored core). Same display/GNSS
  // chip as BOARD_HELTEC_T096 (ST7735 160x80 + UC6580), no external FEM
  // (bare SX1262, same as BOARD_HELTEC_T114), plus a buzzer behind a
  // voltage-doubler circuit neither T096 nor T114 has. No physical unit
  // available - pin mapping cross-checked between Meshtastic's own variant
  // def (~/Development/meshtastic_firmware/variants/nrf52840/
  // heltec_mesh_node_t1), MeshCore's (~/Development/MeshCore/variants/
  // heltec_t1), and the vendor board package itself (~/.arduino15/
  // packages/Heltec_nRF52/hardware/Heltec_nRF52/variants/HT-mesh-node-t1) -
  // all three agree except GPS_RX_PIN/GPS_TX_PIN, where MeshCore has them
  // swapped relative to the other two (see HAS_GPS below).
  #define PRODUCT_HELTEC_T1 0xDA
  #define BOARD_HELTEC_T1   0x47
  #define MODEL_DF          0xDF // Heltec Mesh Node T1, 470-510 MHz
  #define MODEL_E2          0xE2 // Heltec Mesh Node T1, 863-928 MHz

  #if defined(__AVR_ATmega1284P__)
    #define PLATFORM PLATFORM_AVR
    #define MCU_VARIANT MCU_1284P
  #elif defined(__AVR_ATmega2560__)
    #define PLATFORM PLATFORM_AVR
    #define MCU_VARIANT MCU_2560
  #elif defined(ESP32)
    #define PLATFORM PLATFORM_ESP32
    #define MCU_VARIANT MCU_ESP32
  #elif defined(NRF52840_XXAA)
    #include <variant.h>
    #define PLATFORM PLATFORM_NRF52
    #define MCU_VARIANT MCU_NRF52
  #else
      #error "The firmware cannot be compiled for the selected MCU variant"
  #endif

  #ifndef MODEM
    #if BOARD_MODEL == BOARD_RAK4631 || BOARD_MODEL == BOARD_RAK3401
      #define MODEM SX1262
    #elif BOARD_MODEL == BOARD_GENERIC_NRF52
      #define MODEM SX1262
    #else
      #define MODEM SX1276
    #endif
  #endif

  #define LORA_PA_UNKNOWN  0x00
  #define LORA_PA_GC1109   0x01
  #define LORA_PA_KCT8103L 0x02

  #define GPS_MODEL_UNKNOWN 0x00
  #define GPS_MODEL_UC6580  0x01
  #define GPS_MODEL_L76K    0x02
  #define GPS_MODEL_AT6558  0x03

  #define HAS_DISPLAY false
  #define HAS_BLUETOOTH false
  #define HAS_BLE false
  #define HAS_WIFI false
  #define HAS_ESPNOW false
  #define HAS_ETHERNET false
  #define HAS_OTA false
  // BUILD_NUMBER isn't OTA-specific - it's a plain git-commit-count build
  // identity, injected into every board's compiler.cpp.extra_flags by the
  // Makefile (git rev-list --count HEAD). This fallback only applies to
  // non-Makefile builds (e.g. the Arduino IDE). Only HAS_OTA boards actually
  // display/query it at runtime (OTA.h, Menu.h's F/W Update page, Display.h's
  // VERSION banner) - on every other board it's compiled in but otherwise
  // unused, which is harmless.
  #ifndef BUILD_NUMBER
    #define BUILD_NUMBER 0
  #endif
  #define HAS_TCXO false
  #define HAS_PMU false
  #define HAS_NP false
  #define HAS_EEPROM false
  #define HAS_INPUT false
  #define HAS_SLEEP false
  #define HAS_LORA_PA false
  #define HAS_LORA_LNA false
  #define PIN_DISP_SLEEP -1
  #define VALIDATE_FIRMWARE true

  #if defined(ENABLE_TCXO)
      #define HAS_TCXO true
  #endif

  #if MCU_VARIANT == MCU_1284P
    const int pin_cs = 4;
    const int pin_reset = 3;
    const int pin_dio = 2;
    const int pin_led_rx = 12;
    const int pin_led_tx = 13;

    #define BOARD_MODEL BOARD_RNODE
    #define HAS_EEPROM true
    #define CONFIG_UART_BUFFER_SIZE 6144
    #define CONFIG_QUEUE_SIZE 6144
    #define CONFIG_QUEUE_MAX_LENGTH 200
    #define EEPROM_SIZE 4096
    #define EEPROM_OFFSET EEPROM_SIZE-EEPROM_RESERVED
  
  #elif MCU_VARIANT == MCU_2560
    const int pin_cs = 5;
    const int pin_reset = 4;
    const int pin_dio = 2;
    const int pin_led_rx = 12;
    const int pin_led_tx = 13;

    #define BOARD_MODEL BOARD_HMBRW
    #define HAS_EEPROM true
    #define CONFIG_UART_BUFFER_SIZE 768
    #define CONFIG_QUEUE_SIZE 5120
    #define CONFIG_QUEUE_MAX_LENGTH 24
    #define EEPROM_SIZE 4096
    #define EEPROM_OFFSET EEPROM_SIZE-EEPROM_RESERVED

  #elif MCU_VARIANT == MCU_ESP32

    // Board models for ESP32 based builds are
    // defined by the build target in the makefile.
    // If you are not using make to compile this
    // firmware, you can manually define model here.
    //
    // #define BOARD_MODEL BOARD_GENERIC_ESP32
    #define CONFIG_UART_BUFFER_SIZE 6144
    #define CONFIG_QUEUE_SIZE 6144
    #define CONFIG_QUEUE_MAX_LENGTH 200

    #define EEPROM_SIZE 1024
    #define EEPROM_OFFSET EEPROM_SIZE-EEPROM_RESERVED
    #define CONFIG_OFFSET 0

    #if BOARD_MODEL == BOARD_GENERIC_ESP32
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH true
      #define HAS_WIFI true
      #define HAS_CONSOLE true
      #define HAS_EEPROM true
      #define HAS_BUSY true
      #define HAS_INPUT true
      #define HAS_TCXO true
      #define MODEM SX1262
      #define DIO2_AS_RF_SWITCH true
      #define HAS_RF_SWITCH_RX_TX false
      const int pin_cs = 5;
      const int pin_sclk = 18;
      const int pin_miso = 19;
      const int pin_mosi = 23;
      const int pin_busy = 32;
      const int pin_reset = 34;
      const int pin_dio = 33;
      const int pin_txen = -1;
      const int pin_rxen = -1;
      const int pin_tcxo_enable = -1;

      const int pin_btn_usr1 = 39;
      const int pin_led_rx = 2;
      const int pin_led_tx = 4;

    #elif BOARD_MODEL == BOARD_MESHPOE_S3
      #define IS_ESP32S3 true
      // Second HAS_URNS board (meshpoe-s3-urns branch), reusing the same
      // vendored microReticulum/microStore/microLXMF stack and platformio.ini
      // build as MeshAdventurer-S3 - see the global-defaults block below for
      // what this flag does.
      #define HAS_URNS true
      // Re-enabled after root-causing the heap-corruption crash to a real
      // bug in the vendored LXMessage::unpack_from_bytes() field-count
      // handling (lib/microLXMF/src/LXMF/LXMessage.cpp) - fixed there, not
      // something to work around by leaving this feature off.
      #define HAS_LXMF true
      // See global-defaults block's own comment on URNS_ENABLED_DEFAULT -
      // this board's onboard node ran unconditionally before the toggle
      // existed, so already-deployed units need to keep booting with URNS
      // on rather than losing it silently on their next update.
      #define URNS_ENABLED_DEFAULT true
      #undef HAS_DISPLAY
      #define HAS_DISPLAY true
      #undef HAS_NP
      #define HAS_NP true
      #define HAS_BLUETOOTH false
      #undef HAS_BLE
      #define HAS_BLE true
      #undef HAS_WIFI
      #define HAS_WIFI true
      #undef HAS_ESPNOW
      #define HAS_ESPNOW true
      // Was forced false 2026-08-17 to rule Ethernet out as a contributor to
      // the endPacket() TX-poll Interrupt-WDT/stack-canary crash (checkpoint
      // sx126x::endPacket()->poll, ~every few min on an announce) - the W5500
      // runs on its own SPI bus + IRQ (pin_eth_int) next to the radio's SPI/
      // DIO0 ISR. Ruled out (feedback_meshpoe_s3_ethernet_not_the_crash
      // memory: "HAS_ETHERNET off still crashes") - the actual cause was
      // fixed separately (b39cb958, deferred DIO0 RX handling out of the TX
      // call stack), so this is back to true.
      #undef HAS_ETHERNET
      #define HAS_ETHERNET true
      // Disabled on this branch (meshpoe-s3-urns) - same reasoning as
      // MeshAdventurer-S3's own HAS_CONSOLE false above: the button-hold
      // web console (Console.h) calls bt_stop() before starting on
      // HAS_BLUETOOTH||HAS_BLE boards, which reliably crashes NimBLE on
      // reinit (assert failed: ble_hs_init, ble_hs.c:1001) and corrupts the
      // LittleFS partition URNS.h's onboard node keeps its identity in.
      // MeshPoE-S3 has the same HAS_BLUETOOTH false/HAS_BLE true NimBLE-only
      // setup as MeshAdventurer-S3, so it's exposed to the identical crash
      // now that HAS_URNS is enabled here too. See
      // feedback_console_ble_crash_littlefs_corruption memory.
      #define HAS_CONSOLE false
      // Network OTA firmware updates (OTA.h) - see BUILD_NUMBER's own
      // comment above for why that fallback lives outside this board block.
      // Was forced false alongside HAS_ETHERNET above in the same 2026-08-18
      // TX-crash isolation sweep (b39cb958), but never had its own "restore
      // me" marker - that crash is fixed now (same commit), so re-enabled.
      // This was the original MeshPoE-S3-only OTA implementation before
      // MeshAdventurer-S3 got its own; needs its own OTA_BOARD_NAME now that
      // OTA_VERSION_URL/OTA_BIN_URL (OTA.h) are built per board instead of a
      // single hardcoded name.
      #undef HAS_OTA
      #define HAS_OTA true
      #define OTA_BOARD_NAME "meshpoe_s3"
      #undef HAS_EEPROM
      #define HAS_EEPROM true
      #define HAS_BUSY true
      #undef HAS_INPUT
      #define HAS_INPUT true
      #undef HAS_TCXO
      #define HAS_TCXO true
      #undef MODEM
      #define MODEM SX1262
      // FEM: EBYTE E22P-868M30S. RFEN (pin_rxen, GPIO39) is held HIGH
      // continuously (merged LNA+PA enable - see beginPacket()'s MeshPoE
      // special case in sx126x.cpp), and TXEN (pin_txen, GPIO40) is driven
      // directly by the MCU - HIGH to transmit, LOW to receive - so DIO2 is
      // NOT used as the RF switch on this board. (Was true, with GPIO40 left
      // at -1 on the theory it had to be bridged to DIO2 on-module; testing
      // the direct-drive topology per the module's actual TXEN/RFEN wiring.)
      #define DIO2_AS_RF_SWITCH false
      #define HAS_RF_SWITCH_RX_TX true
      #undef HAS_LORA_LNA
      #define HAS_LORA_LNA true
      #define LORA_LNA_GAIN  17
      #define LORA_LNA_GVT   12

      // RNode Settings menu (Menu.h), button-only navigation (tap = next,
      // double-tap = back, hold = select/open - see menu_button_press()).
      // No encoder, buzzer, or voltage divider on this board, so
      // HAS_ENCODER/HAS_BUZZER/HAS_VSENSE/HAS_BATTERY_DIVIDER all stay at
      // their default false (Boards.h fallback block below) - the Hardware
      // page still shows CPU temp via IS_ESP32S3 above.
      #define HAS_MENU true

      const int pin_cs = 17;
      const int pin_sclk = 16;
      const int pin_miso = 15;
      const int pin_mosi = 18;
      const int pin_reset = 3;
      const int pin_busy = 2;
      const int pin_dio = 1;
      // GPIO40 is the E22P-868M30S module's TXEN pin, driven directly by the
      // MCU (DIO2_AS_RF_SWITCH is false above): HIGH to transmit, LOW to
      // receive - see rxAntEnable()/beginPacket() in sx126x.cpp. GPIO39
      // (RXEN below) is this module's merged RFEN (LNA+PA enable), held HIGH
      // continuously whenever the radio is active - the beginPacket() special
      // case for BOARD_MESHPOE_S3 keeps it from being dropped on TX.
      const int pin_txen = 40;
      const int pin_rxen = 39;
      const int pin_tcxo_enable = -1;

      const int pin_eth_rst = 9;
      const int pin_eth_int = 10;
      const int pin_eth_mosi = 11;
      const int pin_eth_miso = 12;
      const int pin_eth_sclk = 13;
      const int pin_eth_cs = 14;

      const int pin_btn_usr1 = 42;
      const int pin_np = 21;

      #if HAS_NP == false
        const int pin_led_rx = -1;
        const int pin_led_tx = -1;
      #endif

      // DS3231MZ RTC, sharing the OLED's I2C bus (SDA_OLED/SCL_OLED,
      // Display.h) - display_init() (Display.h) brings the bus up before
      // rtc_init() (RTC.h) runs (see setup(), RNode_Firmware.ino), so
      // RTC.h doesn't need (and doesn't declare) pins of its own here.
      #define HAS_RTC true

      // Free debug UART - Serial (KISS/console link) is remapped to the
      // native USB CDC device (ARDUINO_USB_CDC_ON_BOOT=1, see Makefile),
      // which leaves Serial0 (UART0, fixed to GPIO43 TX / GPIO44 RX on
      // ESP32-S3) fully unused and unclaimed by any pin above - a free
      // debug channel that can't collide with the KISS protocol on Serial.
      // See DEBUG_UART_BEGIN()/DEBUG_LOG() below for how this gets used.
      #define HAS_DEBUG_UART true

    #elif BOARD_MODEL == BOARD_MESHADVENTURER_S3
      #define IS_ESP32S3 true
      // See global-defaults block below for what this is - MeshPoE-S3 (see
      // its own block above) is the other HAS_URNS board.
      #define HAS_URNS true
      // HAS_LXMF defaults false regardless of HAS_URNS (global-defaults
      // block below) - this board's Messenger app is the established,
      // tested feature this whole branch was built around, so opt in
      // explicitly rather than silently losing it now that the default
      // flipped.
      #define HAS_LXMF true
      // See global-defaults block's own comment on URNS_ENABLED_DEFAULT -
      // same reasoning as HAS_LXMF just above, for the same already-
      // deployed-units reason.
      #define URNS_ENABLED_DEFAULT true
      #undef HAS_DISPLAY
      #define HAS_DISPLAY true
      #undef HAS_NP
      #define HAS_NP true
      #define HAS_BLUETOOTH false
      #undef HAS_BLE
      #define HAS_BLE true
      #undef HAS_WIFI
      #define HAS_WIFI true
      #undef HAS_ESPNOW
      #define HAS_ESPNOW true
      // Disabled - the button-hold-triggered web console (Console.h) calls
      // bt_stop() before starting on HAS_BLUETOOTH||HAS_BLE boards, and on
      // this board (HAS_BLUETOOTH false, HAS_BLE true) that reliably
      // crashes NimBLE on reinit (assert failed: ble_hs_init, ble_hs.c:1001)
      // and reboots the device - which then corrupts the LittleFS partition
      // URNS.h's onboard node keeps its identity in, silently discarding it.
      // Not needed for this variant anyway.
      #define HAS_CONSOLE false
      // Network OTA firmware updates (OTA.h) - see BUILD_NUMBER's own
      // comment (Boards.h global-defaults block) for why that fallback
      // isn't duplicated per board. WiFi-only here (no HAS_ETHERNET on this
      // board) - OTA.h's network-up check already covers WiFi-only boards.
      #undef HAS_OTA
      #define HAS_OTA true
      // Board-specific identity for OTA_VERSION_URL/OTA_BIN_URL (OTA.h) -
      // every HAS_OTA board defines its own, so those URLs are built per
      // board instead of a single hardcoded name shared (and silently
      // wrong) across all of them.
      #define OTA_BOARD_NAME "meshadventurer_s3"
      #undef HAS_EEPROM
      #define HAS_EEPROM true
      #define HAS_BUSY true
      #undef HAS_INPUT
      #define HAS_INPUT true
      #undef HAS_TCXO
      #define HAS_TCXO true
      #undef MODEM
      #define MODEM SX1262
      #define DIO2_AS_RF_SWITCH true
      #define HAS_RF_SWITCH_RX_TX false
      #undef HAS_LORA_LNA
      #define HAS_LORA_LNA true
      #define LORA_LNA_GAIN  17
      #define LORA_LNA_GVT   12
      #define HAS_BUZZER true
      #define HAS_ENCODER true
      #define HAS_VSENSE true
      #define PIN_VSENSE 6
      // Empirically calibrated default (real resistor tolerances drift from
      // the nominal R3 100k/R2 10k math) - still overridable per-board via
      // the Settings menu's Voltage Divider Ratio field (vsense_divider_ratio,
      // Config.h), persisted to EEPROM (ADDR_CONF_VSR) via CMD_VSENSE_DIV.
      #define VSENSE_DIVIDER_RATIO_DEFAULT 12.9

      // Optional ATGM336H GNSS module (AT6558 chipset) - a "high power"
      // unit with no enable/power-control pin at all, unlike either nRF52
      // HAS_GPS board, so neither PIN_GPS_EN nor PIN_GPS_STANDBY apply here
      // - the Settings menu's Enabled toggle (GNSS.h) only starts/stops
      // GPS_SERIAL on this board, nothing hardware-side to power-cycle.
      // ESP32's UART peripherals bind pins at .begin() time (not at compile
      // time like the nRF52 boards' Serial2), so GPS_SERIAL is Serial1 (the
      // only free hardware UART - Serial0/GPIO43-44 is this board's own
      // HAS_DEBUG_UART channel below, and the default Serial is the native
      // USB CDC KISS link) with its rx/tx pins passed explicitly - see
      // gnss_set_enabled(), GNSS.h.
      #define HAS_GPS true
      #define GPS_MODEL GPS_MODEL_AT6558
      // Verbose GNSS diagnostics (Menu.h) - second board to opt in, after
      // T114's L76K. GSA/GSV support on the AT6558's default NMEA sentence
      // set is unverified as of this writing, same open risk as T114 (see
      // gnss_gsv_update()'s own comment in GNSS.h) - PDOP/VDOP/Sats In View
      // should degrade to N/A/No Data cleanly if the module doesn't emit
      // them. This board has no PIN_GPS_EN/PIN_GPS_STANDBY (see comment
      // above), so it's not GNSS_DUTY_CYCLE_CAPABLE - the Diagnostics
      // page's duty-cycle rows and the main page's Module->Duty State swap
      // both compile out here, leaving the main page's Module row
      // untouched.
      #define HAS_GNSS_DEBUG_MENU true
      #define GPS_SERIAL Serial1
      #define GPS_BAUD_RATE 9600 // AT6558's factory-default NMEA baud
      #define PIN_GPS_RX 3
      #define PIN_GPS_TX 16
      #define PIN_GPS_PPS 5
      // This is an optional, not-always-populated add-on (unlike T096/
      // T114's built-in receivers) - defaults off so a board without the
      // module installed doesn't sit there listening to an unconnected
      // UART by default.
      #define GNSS_ENABLED_DEFAULT false

      const int pin_sclk = 1;
      const int pin_reset = 2;
      const int pin_miso = 40;
      const int pin_cs = 39;
      const int pin_mosi = 18;
      const int pin_busy = 7;
      const int pin_dio = 15;
      const int pin_txen = 21;
      const int pin_rxen = 8;
      const int pin_tcxo_enable = -1;

      const int pin_btn_usr1 = 4;
      const int pin_np = 48;
      #define PIN_BUZZER 17
      #define PIN_ENCODER_UP 9
      #define PIN_ENCODER_DOWN 10
      #define PIN_ENCODER_PRESS 11

      #if HAS_NP == false
        #if defined(EXTERNAL_LEDS)
          const int pin_led_rx = 48;
          const int pin_led_tx = 48;
        #else
          const int pin_led_rx = 48;
          const int pin_led_tx = 48;
        #endif
      #endif

      // DS3231SN RTC, sharing the OLED's I2C bus (SDA_OLED/SCL_OLED,
      // Display.h) - same register map/I2C address as the DS3231MZ on
      // MeshPoE-S3 (RTC.h only touches the 0x00-0x06 time registers, which
      // are identical across every DS3231 variant), just a different
      // package/oscillator, so no driver changes are needed. display_init()
      // brings the bus up before rtc_init() runs, same as MeshPoE-S3 above.
      #define HAS_RTC true

      // Free debug UART - same reasoning as BOARD_MESHPOE_S3 above: this
      // board also builds with CDCOnBoot=cdc (native USB CDC for the KISS
      // Serial link), and none of this board's pins above claim GPIO43/44,
      // so Serial0 (UART0) is a free debug channel here too.
      #define HAS_DEBUG_UART true

    #elif BOARD_MODEL == BOARD_MESHADVENTURER
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH true
      #define HAS_WIFI true
      #define HAS_CONSOLE true
      #define HAS_EEPROM true
      #define HAS_BUSY true
      #define HAS_INPUT true
      #define HAS_TCXO true
      #define MODEM SX1262
      #define DIO2_AS_RF_SWITCH false
      #define HAS_RF_SWITCH_RX_TX true
      #define HAS_LORA_LNA true
      #define LORA_LNA_GAIN  17
      #define LORA_LNA_GVT   12
      #define HAS_BUZZER true
      #define HAS_ENCODER true

      // RNode Settings menu (Menu.h), button-only navigation (tap = next,
      // double-tap = back, hold = select/open - see menu_button_press()).
      // No voltage divider on this board, so HAS_VSENSE/HAS_BATTERY_DIVIDER
      // stay at their default false (Boards.h fallback block below).
      #define HAS_MENU true

      // Optional ATGM336H GNSS module (AT6558 chipset), same as
      // MeshAdventurer-S3's - no enable/power-control pin, so neither
      // PIN_GPS_EN nor PIN_GPS_STANDBY apply here - the Settings menu's
      // Enabled toggle (GNSS.h) only starts/stops GPS_SERIAL on this board.
      // This board's KISS serial link runs over UART0 (Serial, native
      // USB-serial bridge, not native USB CDC like the S3), so Serial1 is
      // free for GPS_SERIAL with its rx/tx pins passed explicitly - see
      // gnss_set_enabled(), GNSS.h.
      #define HAS_GPS true
      #define GPS_MODEL GPS_MODEL_AT6558
      #define GPS_SERIAL Serial1
      #define GPS_BAUD_RATE 9600 // AT6558's factory-default NMEA baud
      #define PIN_GPS_RX 12 // MCU RX - wired to GPS TX-out
      #define PIN_GPS_TX 15 // MCU TX - wired to GPS RX-in
      // This is an optional, not-always-populated add-on - defaults off so
      // a board without the module installed doesn't sit there listening
      // to an unconnected UART by default.
      #define GNSS_ENABLED_DEFAULT false

      const int pin_cs = 18;
      const int pin_sclk = 5;
      const int pin_miso = 19;
      const int pin_mosi = 27;
      const int pin_busy = 32;
      const int pin_reset = 23;
      const int pin_dio = 33;
      const int pin_txen = 13;
      const int pin_rxen = 14;
      const int pin_tcxo_enable = -1;

      const int pin_btn_usr1 = 39;
      const int pin_led_rx = 2;
      const int pin_led_tx = 2;

      #define PIN_BUZZER 26
      #define PIN_ENCODER_UP 16
      #define PIN_ENCODER_DOWN 17
      #define PIN_ENCODER_PRESS 4

    #elif BOARD_MODEL == BOARD_TBEAM_1W
      // LilyGO T-Beam 1W (ESP32-S3 + SX1262 driving an XY16P35 1W PA/LNA
      // module). No physical unit available to validate against - the pin
      // mapping and RF-switch topology below are cross-checked between
      // Meshtastic's board/variant defs (~/Development/meshtastic_firmware/
      // variants/esp32s3/t-beam-1w) and MeshCore's own
      // (~/Development/MeshCore/variants/lilygo_tbeam_1w), which agree on
      // every pin except TCXO voltage and the battery ADC scale - both
      // called out where they're set below.
      #define IS_ESP32S3 true
      #undef HAS_DISPLAY
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH false
      #undef HAS_BLE
      #define HAS_BLE true
      #undef HAS_WIFI
      #define HAS_WIFI true
      #define HAS_CONSOLE true
      #undef HAS_EEPROM
      #define HAS_EEPROM true
      #define HAS_BUSY true
      #undef HAS_INPUT
      #define HAS_INPUT true
      #undef HAS_TCXO
      #define HAS_TCXO true
      #undef MODEM
      #define MODEM SX1262

      // XY16P35 module: DIO2 drives the PA leg of the RF switch directly
      // (SX126X_DIO2_AS_RF_SWITCH in both reference firmwares); CTRL/RXEN
      // (GPIO21) is a separate, MCU-driven LNA-enable line - exactly the
      // "DIO2_AS_RF_SWITCH true with a real pin_rxen" pattern this firmware
      // already uses for BOARD_MESHADVENTURER_S3. rxAntEnable()/
      // beginPacket() (sx126x.cpp) already drive that pin HIGH on receive()
      // and LOW at the start of every TX with no board-specific code needed
      // - see those functions' own comments for why.
      #define DIO2_AS_RF_SWITCH true
      #define HAS_RF_SWITCH_RX_TX false

      // RSSI correction for the module's built-in LNA - no datasheet figure
      // available for the XY16P35, so this reuses the same generic
      // external-LNA default every other FEM-equipped board in this
      // codebase falls back to (MeshPoE-S3, MeshAdventurer-S3,
      // MeshAdventurer, DIY-V1) rather than inventing an unverified number.
      #undef HAS_LORA_LNA
      #define HAS_LORA_LNA true
      #define LORA_LNA_GAIN  17
      #define LORA_LNA_GVT   12

      // RNode Settings menu (Menu.h), button-only navigation (tap = next,
      // double-tap = back, hold = select/open) - no encoder on this board.
      #define HAS_MENU true

      // Direct resistor-divider battery ADC (2S 7.4V LiPo pack), no fuel
      // gauge/PMU - same mechanism as BOARD_MESHADVENTURER_S3's
      // HAS_VSENSE/PIN_VSENSE (Power.h's update_vsense()). Meshtastic's and
      // MeshCore's variant.h disagree on the exact multiplier for what both
      // describe as the same divider (2.9333 vs 3.0) - 3.0 is used here as
      // a clean starting point; it's a runtime-overridable default
      // (vsense_divider_ratio, CMD_VSENSE_DIV) either way, so field
      // calibration against a real pack corrects it.
      #define HAS_VSENSE true
      #define PIN_VSENSE 4
      #define VSENSE_DIVIDER_RATIO_DEFAULT 3.0

      // Built-in Quectel L76K GNSS - same chip and enable-line style as
      // BOARD_HELTEC_T114's. GPS_EN_PIN (GPIO16) is active-HIGH on this
      // board (MeshCore's PIN_GPS_EN_ACTIVE defaults HIGH and nothing
      // overrides it for this variant), the opposite polarity from this
      // firmware's generic PIN_GPS_EN macro (T096-style, active-LOW - see
      // GNSS.h) - so it's wired as PIN_GPS_STANDBY instead (HIGH=awake,
      // LOW=allow sleep, GNSS.h's gnss_set_enabled()), matching T114's own
      // L76K wiring exactly.
      #define HAS_GPS true
      #define GPS_MODEL GPS_MODEL_L76K
      #define GPS_SERIAL Serial1
      #define GPS_BAUD_RATE 9600 // L76K's factory-default NMEA baud
      #define PIN_GPS_RX 5       // MCU RX - wired to GPS TX-out
      #define PIN_GPS_TX 6       // MCU TX - wired to GPS RX-in
      #define PIN_GPS_PPS 7
      #define PIN_GPS_STANDBY 16 // active HIGH - see comment above
      #define GNSS_DUTY_CYCLE_CAPABLE true // PIN_GPS_STANDBY only - soft-sleep

      const int pin_cs = 15;
      const int pin_sclk = 13;
      const int pin_miso = 12;
      const int pin_mosi = 11;
      const int pin_busy = 38;
      const int pin_reset = 3;
      const int pin_dio = 1;
      // No discrete TXEN GPIO on this module - DIO2 (above) drives the PA
      // leg directly; only the LNA leg (RXEN) is MCU-controlled.
      const int pin_txen = -1;
      const int pin_rxen = 21;
      const int pin_tcxo_enable = -1;

      // BOOT button. A 2nd button (ALT, GPIO17) exists on this board but
      // isn't wired to anything - this firmware has no generic
      // second-user-button concept (see pin_btn_usr1's own use sites).
      const int pin_btn_usr1 = 0;

      const int pin_led_rx = 18;
      const int pin_led_tx = 18;

      // GPIO40/41 aren't modeled by any existing pin_* concept in this
      // firmware - they're handled directly in setup() (RNode_Firmware.ino)
      // instead of through the generic radio-pin machinery above.
      //   pin_radio_en (GPIO40): LDO enable for the whole XY16P35 module
      //     (SX1262 + PA + LNA) - must be driven HIGH before any SPI/radio
      //     access, per both reference firmwares.
      //   pin_fan_en (GPIO41): PA cooling fan, driven on unconditionally at
      //     boot (matches MeshCore's TBeam1WBoard::begin(): "on by default -
      //     1W PA can overheat"). No thermal sensing to duty-cycle it
      //     against, so this errs toward always-on rather than silent.
      const int pin_radio_en = 40;
      const int pin_fan_en = 41;

    #elif BOARD_MODEL == BOARD_DIY_V1
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH true
      #define HAS_WIFI true
      #define HAS_CONSOLE true
      #define HAS_EEPROM true
      #define HAS_BUSY true
      #define HAS_INPUT true
      #define HAS_TCXO true
      #define MODEM SX1262
      #define DIO2_AS_RF_SWITCH false
      #define HAS_RF_SWITCH_RX_TX true
      #define HAS_LORA_LNA true
      #define LORA_LNA_GAIN  17
      #define LORA_LNA_GVT   12

      // RNode Settings menu (Menu.h), button-only navigation (tap = next,
      // double-tap = back, hold = select/open - see menu_button_press()).
      // No encoder, buzzer, or voltage divider on this board, so
      // HAS_ENCODER/HAS_BUZZER/HAS_VSENSE/HAS_BATTERY_DIVIDER all stay at
      // their default false (Boards.h fallback block below).
      #define HAS_MENU true

      const int pin_cs = 18;
      const int pin_sclk = 5;
      const int pin_miso = 19;
      const int pin_mosi = 27;
      const int pin_busy = 32;
      const int pin_reset = 23;
      const int pin_dio = 33;
      const int pin_txen = 13;
      const int pin_rxen = 14;
      const int pin_tcxo_enable = -1;

      const int pin_btn_usr1 = 39;
      const int pin_led_rx = 2;
      const int pin_led_tx = 2;

    #elif BOARD_MODEL == BOARD_AETHERNODE
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH true
      #define HAS_WIFI true
      #define HAS_CONSOLE true
      #define HAS_EEPROM true
      #define HAS_BUSY true
      #define HAS_INPUT true
      #define HAS_TCXO true
      #define MODEM SX1262
      #define DIO2_AS_RF_SWITCH true
      #define HAS_RF_SWITCH_RX_TX false
      const int pin_cs = 5;
      const int pin_sclk = 18;
      const int pin_miso = 19;
      const int pin_mosi = 23;
      const int pin_busy = 32;
      const int pin_reset = 25;
      const int pin_dio = 33;
      const int pin_rxen = 16;
      const int pin_txen = 17;
      const int pin_tcxo_enable = -1;

      const int pin_btn_usr1 = 39;
      const int pin_led_rx = 2;
      const int pin_led_tx = 2;

    #elif BOARD_MODEL == BOARD_AETHERNODE_S3
      #define IS_ESP32S3 true
      #define HAS_DISPLAY true
      #define HAS_NP true
      #define HAS_BLUETOOTH false
      #define HAS_BLE true
      #define HAS_WIFI true
      #define HAS_CONSOLE true
      #define HAS_EEPROM true
      #define HAS_BUSY true
      #define HAS_INPUT false
      #define HAS_TCXO true
      #define MODEM SX1262
      #define DIO2_AS_RF_SWITCH true
      #define HAS_RF_SWITCH_RX_TX false
      #define HAS_LORA_LNA true
      #define LORA_LNA_GAIN  17
      #define LORA_LNA_GVT   12

      const int pin_cs = 10;
      const int pin_sclk = 13;
      const int pin_miso = 12;
      const int pin_mosi = 11;
      const int pin_busy = 37;
      const int pin_reset = 35;
      const int pin_dio = 36;
      const int pin_txen = 38;
      const int pin_rxen = 39;
      const int pin_tcxo_enable = -1;

      const int pin_btn_usr1 = -1;
      const int pin_np = 48;

      #if HAS_NP == false
        #if defined(EXTERNAL_LEDS)
          const int pin_led_rx = 48;
          const int pin_led_tx = 48;
        #else
          const int pin_led_rx = 48;
          const int pin_led_tx = 48;
        #endif
      #endif

    #elif BOARD_MODEL == BOARD_TBEAM
      #define HAS_DISPLAY true
      #define HAS_PMU true
      #define HAS_BLUETOOTH true
      #define HAS_WIFI true
      #define HAS_CONSOLE true
      #define HAS_SD false
      #define HAS_EEPROM true
      #define I2C_SDA 21
      #define I2C_SCL 22
      #define PMU_IRQ 35

      #define HAS_INPUT true
      const int pin_btn_usr1 = 38;

      const int pin_cs = 18;
      const int pin_reset = 23;
      const int pin_led_rx = 2;
      const int pin_led_tx = 2;

      #if MODEM == SX1262
        #define HAS_TCXO true
        #define HAS_BUSY true
        #define DIO2_AS_RF_SWITCH true
        // FIXED (ported from microReticulum_Firmware's unmerged
        // origin/rak4631 branch, commit 1926f15 "Fix SX1262 init order,
        // OCP, and regulator mode"): 0x28 (80mA) trips the SX1262's own
        // overcurrent limiter on every transmit - the chip draws ~118mA
        // at +14dBm and ~158mA at +22dBm per the Semtech datasheet
        // (confirmed against RadioLib and Meshtastic's SX126xInterface,
        // which both use 0x38/140mA). Applied uniformly across every
        // SX1262 board in this firmware, not just this one - see
        // sx126x.cpp's enableTCXO()/begin() ordering fix, same commit,
        // for the sibling issue this branch was investigating.
        #define OCP_TUNED 0x38
        const int pin_busy = 32;
        const int pin_dio = 33;
        const int pin_tcxo_enable = -1;
      #else
        const int pin_dio = 26;
      #endif

    #elif BOARD_MODEL == BOARD_HUZZAH32
      #define HAS_BLUETOOTH true
      #define HAS_CONSOLE true
      #define HAS_EEPROM true
      const int pin_cs = 4;
      const int pin_reset = 33;
      const int pin_dio = 39;
      const int pin_led_rx = 14;
      const int pin_led_tx = 32;

    #elif BOARD_MODEL == BOARD_LORA32_V1_0
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH true
      #define HAS_WIFI true
      #define HAS_CONSOLE true
      #define HAS_EEPROM true
      const int pin_cs = 18;
      const int pin_reset = 14;
      const int pin_dio = 26;
      #if defined(EXTERNAL_LEDS)
        const int pin_led_rx = 25;
        const int pin_led_tx = 2;
      #else
        const int pin_led_rx = 2;
        const int pin_led_tx = 2;
      #endif

    #elif BOARD_MODEL == BOARD_LORA32_V2_0
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH true
      #define HAS_WIFI true
      #define HAS_CONSOLE true
      #define HAS_EEPROM true
      const int pin_cs = 18;
      const int pin_reset = 12;
      const int pin_dio = 26;
      #if defined(EXTERNAL_LEDS)
        const int pin_led_rx = 2;
        const int pin_led_tx = 0;
      #else
        const int pin_led_rx = 22;
        const int pin_led_tx = 22;
      #endif

    #elif BOARD_MODEL == BOARD_LORA32_V2_1
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH true
      #define HAS_WIFI true
      #define HAS_PMU true
      #define HAS_CONSOLE true
      #define HAS_EEPROM true
      const int pin_cs = 18;
      const int pin_reset = 23;
      const int pin_dio = 26;
      #if HAS_TCXO == true
        const int pin_tcxo_enable = 33;
      #endif
      #if defined(EXTERNAL_LEDS)
        const int pin_led_rx = 15;
        const int pin_led_tx = 4;
      #else
        const int pin_led_rx = 25;
        const int pin_led_tx = 25;
      #endif

    #elif BOARD_MODEL == BOARD_HELTEC32_V2
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH true
      #define HAS_CONSOLE true
      #define HAS_EEPROM true
      #define HAS_INPUT true
      #define HAS_SLEEP true
      #define PIN_WAKEUP GPIO_NUM_0
      #define WAKEUP_LEVEL 0

      const int pin_btn_usr1 = 0;

      const int pin_cs = 18;
      const int pin_reset = 14;
      const int pin_dio = 26;
      #if defined(EXTERNAL_LEDS)
        const int pin_led_rx = 36;
        const int pin_led_tx = 37;
      #else
        const int pin_led_rx = 25;
        const int pin_led_tx = 25;
      #endif

    #elif BOARD_MODEL == BOARD_HELTEC32_V3
      #define IS_ESP32S3 true
      #define HAS_DISPLAY true
      #define HAS_WIFI true
      #define HAS_BLUETOOTH false
      #define HAS_BLE true
      #define HAS_PMU true
      #define HAS_CONSOLE true
      #define HAS_EEPROM true
      #define HAS_INPUT true
      #define HAS_SLEEP true
      #define PIN_WAKEUP GPIO_NUM_0
      #define WAKEUP_LEVEL 0
      #define OCP_TUNED 0x38

      const int pin_btn_usr1 = 0;

      #if defined(EXTERNAL_LEDS)
        const int pin_led_rx = 13;
        const int pin_led_tx = 14;
      #else
        const int pin_led_rx = 35;
        const int pin_led_tx = 35;
      #endif

      #define MODEM SX1262
      #define HAS_TCXO true
      const int pin_tcxo_enable = -1;
      #define HAS_BUSY true
      #define DIO2_AS_RF_SWITCH true

      // Following pins are for the SX1262
      const int pin_cs = 8;
      const int pin_busy = 13;
      const int pin_dio = 14;
      const int pin_reset = 12;
      const int pin_mosi = 10;
      const int pin_miso = 11;
      const int pin_sclk = 9;

    #elif BOARD_MODEL == BOARD_HELTEC32_V4
      #define IS_ESP32S3 true
      #undef HAS_DISPLAY
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH false
      #undef HAS_BLE
      #define HAS_BLE true
      #undef HAS_WIFI
      #define HAS_WIFI true
      #undef HAS_PMU
      #define HAS_PMU true
      // Guarded (unlike every other board's unconditional HAS_CONSOLE
      // define) so a build-flag variant env can force it off with
      // -DHAS_CONSOLE=false - see [env:heltec32v4pa_urns] (platformio.ini),
      // same #ifndef mechanism as HAS_URNS's own global fallback below.
      #ifndef HAS_CONSOLE
        #define HAS_CONSOLE true
      #endif
      #undef HAS_EEPROM
      #define HAS_EEPROM true
      #undef HAS_INPUT
      #define HAS_INPUT true
      #undef HAS_SLEEP
      #define HAS_SLEEP true
      #undef HAS_LORA_PA
      #define HAS_LORA_PA true
      #undef HAS_LORA_LNA
      #define HAS_LORA_LNA true
      #define PIN_WAKEUP GPIO_NUM_0
      #define WAKEUP_LEVEL 0
      #define OCP_TUNED 0x38
      #define Vext GPIO_NUM_36
      #define LORA_PA_MODEL LORA_PA_UNKNOWN

      // Built-in L76K GNSS - unlike T114's board revision, this one has a
      // real dedicated enable pin (not shared with Vext/the display), plus
      // a working reset pin and the same soft-standby line - all fully
      // wired (not commented out) in the vendor reference design, so this
      // is a standard always-populated feature, not an optional add-on -
      // GNSS_ENABLED_DEFAULT stays at its global default (true).
      // gnss_set_enabled() (GNSS.h) drives both PIN_GPS_EN and
      // PIN_GPS_STANDBY together on Enabled toggle - harmless overlap
      // (STANDBY is moot with EN off; both HIGH when on matches the
      // reference driver's own behavior of using both independently).
      #define HAS_GPS true
      #define GPS_MODEL GPS_MODEL_L76K
      #define GPS_SERIAL Serial1
      #define GPS_BAUD_RATE 9600 // L76K's factory-default NMEA baud

      #define PIN_GPS_RX 39      // MCU RX - wired to GPS TX-out
      #define PIN_GPS_TX 38      // MCU TX - wired to GPS RX-in
      #define PIN_GPS_PPS 41
      #define PIN_GPS_EN 34      // active LOW
      #define PIN_GPS_RESET 42   // active LOW, needs a >100ms hold to reset
      #define PIN_GPS_STANDBY 40 // HIGH=force wake, LOW=allow sleep
      #define GNSS_DUTY_CYCLE_CAPABLE true // both pins - soft-sleep short intervals, hard power-off long ones

      // RNode Settings menu (Menu.h), button-only navigation (tap = next,
      // double-tap = back, hold = select/open - see menu_button_press()).
      // No encoder, buzzer, or voltage divider on this board, so
      // HAS_ENCODER/HAS_BUZZER/HAS_VSENSE/HAS_BATTERY_DIVIDER all stay at
      // their default false (Boards.h fallback block below) - the Hardware
      // page still shows CPU Temp via IS_ESP32S3 above, and WiFi IP/
      // Netmask/MAC via HAS_WIFI above. The panel's normal Orientation is
      // portrait (see disp_mode/setRotation(1) below in Display.h), but the
      // menu itself always forces landscape while open regardless (generic
      // update_display() handling, Display.h) - same as every other HAS_MENU
      // board, no board-specific menu layout needed here.
      #define HAS_MENU true

      const int pin_btn_usr1 = 0;

      #if defined(EXTERNAL_LEDS)
        const int pin_led_rx = 13;
        const int pin_led_tx = 14;
      #else
        const int pin_led_rx = 35;
        const int pin_led_tx = 35;
      #endif

      #undef MODEM
      #define MODEM SX1262
      #undef HAS_TCXO
      #define HAS_TCXO true
      const int pin_tcxo_enable = -1;
      #define HAS_BUSY true
      #define DIO2_AS_RF_SWITCH true
      #define LNA_GD_THRSHLD (-109)
      #define LNA_GD_LIMIT   (-89)

      #define LORA_LNA_GAIN  17
      #define LORA_LNA_GVT   12
      #define LORA_PA_PWR_EN  7
      #define LORA_PA_CSD     2 // Same pin on GC1109
      #define LORA_PA_CPS    46 // Same pin on GC1109
      #define LORA_PA_CTX     5 // Only used on KCT8103

      #define PA_MAX_OUTPUT  28
      #define PA_GAIN_POINTS 22
      
      #define LORA_LNA_KCT8103L_GAIN 21
      const int PA_GC1109_VALUES[PA_GAIN_POINTS] =   {11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 10,  9, 9, 8, 7};
      const int PA_KCT8103L_VALUES[PA_GAIN_POINTS] = {13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 12, 11, 11, 10, 9, 8, 7};

      const int pin_cs = 8;
      const int pin_busy = 13;
      const int pin_dio = 14;
      const int pin_reset = 12;
      const int pin_mosi = 10;
      const int pin_miso = 11;
      const int pin_sclk = 9;

    #elif BOARD_MODEL == BOARD_RNODE_NG_20
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH true
      #define HAS_NP true
      #define HAS_CONSOLE true
      #define HAS_EEPROM true
      const int pin_cs = 18;
      const int pin_reset = 12;
      const int pin_dio = 26;
      const int pin_np = 4;
      #if HAS_NP == false
        #if defined(EXTERNAL_LEDS)
          const int pin_led_rx = 2;
          const int pin_led_tx = 0;
        #else
          const int pin_led_rx = 22;
          const int pin_led_tx = 22;
        #endif
      #endif

    #elif BOARD_MODEL == BOARD_RNODE_NG_21
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH true
      #define HAS_CONSOLE true
      #define HAS_PMU true
      #define HAS_NP true
      #define HAS_SD false
      #define HAS_EEPROM true
      const int pin_cs = 18;
      const int pin_reset = 23;
      const int pin_dio = 26;
      const int pin_np = 12;
      const int pin_dac = 25;
      const int pin_adc = 34;
      const int SD_MISO = 2;
      const int SD_MOSI = 15;
      const int SD_CLK = 14;
      const int SD_CS = 13;
      #if HAS_NP == false
        #if defined(EXTERNAL_LEDS)
          const int pin_led_rx = 12;
          const int pin_led_tx = 4;
        #else
          const int pin_led_rx = 25;
          const int pin_led_tx = 25;
        #endif
      #endif

    #elif BOARD_MODEL == BOARD_T3S3
      #define IS_ESP32S3 true
      #define HAS_DISPLAY true
      #define HAS_CONSOLE true
      #define HAS_WIFI true
      #define HAS_BLUETOOTH false
      #define HAS_BLE true
      #define HAS_PMU true
      #define HAS_NP false
      #define HAS_SD false
      #define HAS_EEPROM true

      #define HAS_INPUT true
      #define HAS_SLEEP true
      #define PIN_WAKEUP GPIO_NUM_0
      #define WAKEUP_LEVEL 0
      const int pin_btn_usr1 = 0;

      const int pin_cs = 7;
      const int pin_reset = 8;
      const int pin_sclk = 5;
      const int pin_mosi = 6;
      const int pin_miso = 3;
      
      #if MODEM == SX1262
        #define DIO2_AS_RF_SWITCH true
        #define HAS_BUSY true
        #define HAS_TCXO true
        const int pin_busy = 34;
        const int pin_dio = 33;
        const int pin_tcxo_enable = -1;
      #elif MODEM == SX1280
        #define CONFIG_QUEUE_SIZE 6144
        #define DIO2_AS_RF_SWITCH false
        #define HAS_BUSY true
        #define HAS_TCXO true
        #define HAS_PA true
        const int pa_max_input = 3;

        #define HAS_RF_SWITCH_RX_TX true
        const int pin_rxen = 21;
        const int pin_txen = 10;
        
        const int pin_busy = 36;
        const int pin_dio = 9;
        const int pin_tcxo_enable = -1;
      #else
        const int pin_dio = 9;
      #endif
      
      const int pin_np = 38;
      const int pin_dac = 25;
      const int pin_adc = 1;

      const int SD_MISO = 2;
      const int SD_MOSI = 11;
      const int SD_CLK = 14;
      const int SD_CS = 13;

      #if HAS_NP == false
        #if defined(EXTERNAL_LEDS)
          const int pin_led_rx = 37;
          const int pin_led_tx = 37;
        #else
          const int pin_led_rx = 37;
          const int pin_led_tx = 37;
        #endif
      #endif

    #elif BOARD_MODEL == BOARD_TDECK
      #define IS_ESP32S3 true
      #define MODEM SX1262
      #define DIO2_AS_RF_SWITCH true
      #define HAS_BUSY true
      #define HAS_TCXO true

      #define HAS_DISPLAY false
      #define HAS_CONSOLE false
      #define HAS_WIFI true
      #define HAS_BLUETOOTH false
      #define HAS_BLE true
      #define HAS_PMU true
      #define HAS_NP false
      #define HAS_SD false
      #define HAS_EEPROM true

      #define HAS_INPUT true
      #define HAS_SLEEP true
      #define PIN_WAKEUP GPIO_NUM_0
      #define WAKEUP_LEVEL 0

      const int pin_poweron = 10;
      const int pin_btn_usr1 = 0;

      const int pin_cs = 9;
      const int pin_reset = 17;
      const int pin_sclk = 40;
      const int pin_mosi = 41;
      const int pin_miso = 38;
      const int pin_tcxo_enable = -1;
      const int pin_dio = 45;
      const int pin_busy = 13;
      
      const int SD_MISO = 38;
      const int SD_MOSI = 41;
      const int SD_CLK = 40;
      const int SD_CS = 39;

      const int DISPLAY_DC = 11;
      const int DISPLAY_CS = 12;
      const int DISPLAY_MISO = 38;
      const int DISPLAY_MOSI = 41;
      const int DISPLAY_CLK = 40;
      const int DISPLAY_BL_PIN = 42;

      #if HAS_NP == false
        #if defined(EXTERNAL_LEDS)
          const int pin_led_rx = 43;
          const int pin_led_tx = 43;
        #else
          const int pin_led_rx = 43;
          const int pin_led_tx = 43;
        #endif
      #endif

    #elif BOARD_MODEL == BOARD_TBEAM_S_V1
      #define IS_ESP32S3 true
      #define MODEM SX1262
      #define DIO2_AS_RF_SWITCH true
      #define HAS_BUSY true
      #define HAS_TCXO true
      #define OCP_TUNED 0x38

      #define HAS_DISPLAY true
      #define HAS_CONSOLE true
      #define HAS_WIFI true
      #define HAS_BLUETOOTH false
      #define HAS_BLE true
      #define HAS_PMU true
      #define HAS_NP false
      #define HAS_SD false
      #define HAS_EEPROM true

      #define HAS_INPUT true
      #define HAS_SLEEP false
      
      #define PMU_IRQ 40
      #define I2C_SCL 41
      #define I2C_SDA 42

      const int pin_btn_usr1 = 0;

      const int pin_cs = 10;
      const int pin_reset = 5;
      const int pin_sclk = 12;
      const int pin_mosi = 11;
      const int pin_miso = 13;
      const int pin_tcxo_enable = -1;
      const int pin_dio = 1;
      const int pin_busy = 4;
      
      const int SD_MISO = 37;
      const int SD_MOSI = 35;
      const int SD_CLK = 36;
      const int SD_CS = 47;

      const int IMU_CS = 34;

      #if HAS_NP == false
        #if defined(EXTERNAL_LEDS)
          const int pin_led_rx = 43;
          const int pin_led_tx = 43;
        #else
          const int pin_led_rx = 43;
          const int pin_led_tx = 43;
        #endif
      #endif

    #elif BOARD_MODEL == BOARD_TBEAM_S_V3
      #define IS_ESP32S3 true
      #define MODEM SX1262
      #define DIO2_AS_RF_SWITCH true
      #define HAS_BUSY true
      #define HAS_TCXO true
      #define OCP_TUNED 0x38

      #define HAS_DISPLAY true
      #define HAS_CONSOLE true
      #define HAS_WIFI true
      #define HAS_BLUETOOTH false
      #define HAS_BLE true
      #define HAS_PMU true
      #define HAS_NP false
      #define HAS_SD false
      #define HAS_EEPROM true

      #define HAS_INPUT true
      #define HAS_SLEEP false
      
      #define PMU_IRQ 40
      #define I2C_SCL 41
      #define I2C_SDA 42

      const int pin_btn_usr1 = 0;

      const int pin_cs = 10;
      const int pin_reset = 5;
      const int pin_sclk = 12;
      const int pin_mosi = 11;
      const int pin_miso = 13;
      const int pin_tcxo_enable = -1;
      const int pin_dio = 1;
      const int pin_busy = 4;
      
      const int SD_MISO = 37;
      const int SD_MOSI = 35;
      const int SD_CLK = 36;
      const int SD_CS = 47;

      const int IMU_CS = 34;

      #if HAS_NP == false
        #if defined(EXTERNAL_LEDS)
          const int pin_led_rx = 43;
          const int pin_led_tx = 43;
        #else
          const int pin_led_rx = 43;
          const int pin_led_tx = 43;
        #endif
      #endif

    #elif BOARD_MODEL == BOARD_XIAO_S3
      #define IS_ESP32S3 true
      #define MODEM SX1262
      #define DIO2_AS_RF_SWITCH true
      #define HAS_BUSY true
      #define HAS_TCXO true

      #define HAS_DISPLAY false
      #define HAS_CONSOLE true
      #define HAS_WIFI true
      #define HAS_BLUETOOTH false
      #define HAS_BLE true
      #define HAS_NP false
      #define HAS_SD false
      #define HAS_EEPROM true

      #define HAS_INPUT false
      #define HAS_SLEEP false

      const int pin_dio = 2;
      const int pin_reset = 3;
      const int pin_busy = 4;
      const int pin_cs = 5;
      const int pin_sclk = 7;
      const int pin_miso = 8;
      const int pin_mosi = 9;
      const int pin_tcxo_enable = -1;

      #if HAS_NP == false
        #if defined(EXTERNAL_LEDS)
          const int pin_led_rx = 48;
          const int pin_led_tx = 48;
        #else
          const int pin_led_rx = 48;
          const int pin_led_tx = 48;
        #endif
      #endif

    #elif BOARD_MODEL == BOARD_HELTEC_WTRACKER_V2
      // Heltec Wireless Tracker V2 (ESP32-S3FN8 + SX1262 + KCT8103L FEM +
      // ST7735S 160x80 TFT + UC6580 GNSS - same display/GNSS/FEM as
      // BOARD_HELTEC_T096, just on ESP32-S3 instead of nRF52). No physical
      // unit available to validate against - pin mapping cross-checked
      // between Meshtastic's own variant def
      // (~/Development/meshtastic_firmware/variants/esp32s3/
      // heltec_wireless_tracker_v2) and MeshCore's
      // (~/Development/MeshCore/variants/heltec_tracker_v2), which agree
      // on every pin - same no-hardware-yet precedent as BOARD_TBEAM_1W
      // above.
      #define IS_ESP32S3 true
      #define MODEM SX1262
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH false
      #define HAS_BLE true
      #define HAS_WIFI true
      #define HAS_CONSOLE true
      #define HAS_EEPROM true
      #define HAS_PMU true
      #define HAS_NP false
      #define HAS_SD false
      #define HAS_TCXO true
      #define HAS_BUSY true
      #define HAS_INPUT true
      #define HAS_SLEEP true
      // Wake from deep sleep on the same button used for menu/input (GPIO0,
      // pin_btn_usr1 below) - active LOW, same convention as every other
      // HAS_SLEEP ESP32-S3 board in this codebase (e.g. BOARD_HELTEC32_V4/
      // BOARD_T3S3).
      #define PIN_WAKEUP GPIO_NUM_0
      #define WAKEUP_LEVEL 0

      // RNode Settings menu (Menu.h), button-only navigation - same pattern
      // as BOARD_HELTEC_T096 (no encoder on this board either). Shares
      // T096's 160x80 ST7735 menu rendering path (Menu.h/Display.h's own
      // BOARD_HELTEC_T096-grouped blocks) since it's the identical panel.
      #define HAS_MENU true

      #define DIO2_AS_RF_SWITCH true
      #define CONFIG_UART_BUFFER_SIZE 6144
      #define CONFIG_QUEUE_SIZE 6144
      #define CONFIG_QUEUE_MAX_LENGTH 200
      #define BLE_MANUFACTURER "Heltec"
      #define BLE_MODEL "WTrackerV2"

      #define HAS_LORA_PA true
      #define HAS_LORA_LNA true
      #define OCP_TUNED 0x38
      // Same KCT8103L FEM chip as BOARD_HELTEC_T096 (confirmed in both
      // Meshtastic's and MeshCore's reference variant.h for this board) -
      // reuses that board's gain table/thresholds verbatim.
      #define LORA_PA_MODEL LORA_PA_KCT8103L
      #define LNA_GD_THRSHLD (-109)
      #define LNA_GD_LIMIT   (-89)
      #define LORA_LNA_GAIN  21
      #define LORA_LNA_GVT   12

      // KCT8103L FEM: VFEM LDO enable on GPIO7, CSD on GPIO4, CTX on GPIO5
      // (Meshtastic variant.h: LORA_PA_PWR_EN 7, LORA_KCT8103L_PA_CSD 4,
      // LORA_KCT8103L_PA_CTX 5). CPS is wired to SX1262 DIO2 and switched
      // automatically via DIO2_AS_RF_SWITCH, same as every other
      // DIO2-driven FEM in this codebase.
      #define LORA_PA_PWR_EN 7
      #define LORA_PA_CPS    -1
      #define LORA_PA_CSD    4
      #define LORA_PA_CTX    5

      #define PA_MAX_OUTPUT  28
      #define PA_GAIN_POINTS 22

      #define LORA_LNA_KCT8103L_GAIN 21
      const int PA_KCT8103L_VALUES[PA_GAIN_POINTS] = {13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 12, 11, 11, 10, 9, 8, 7};

      // Single onboard LED (Meshtastic variant.h: LED_POWER 18) - no
      // separate RX/TX indicators, same shared-LED pattern as T096.
      const int pin_led_rx = 18;
      const int pin_led_tx = 18;

      // SPI (LoRa) - Meshtastic/MeshCore variant.h: LORA_SCK 9, LORA_MISO
      // 11, LORA_MOSI 10, LORA_CS 8, LORA_RESET 12, LORA_DIO1 14 (IRQ),
      // LORA_DIO2 13 (their macro name for the BUSY line, not a real DIO2 -
      // SX126X_BUSY is #defined to it in both reference firmwares).
      const int pin_cs = 8;
      const int pin_sclk = 9;
      const int pin_mosi = 10;
      const int pin_miso = 11;
      const int pin_reset = 12;
      const int pin_busy = 13;
      const int pin_dio = 14;
      const int pin_tcxo_enable = -1;

      const int pin_btn_usr1 = 0;

      // pin_vbat/pin_ctrl (battery ADC) are declared in Power.h's own
      // BOARD_HELTEC_WTRACKER_V2 block, same as BOARD_HELTEC_T096 - not
      // here, to match that board's existing split.

      // ST7735S 160x80 TFT, identical panel family to BOARD_HELTEC_T096 -
      // shares that board's rendering pipeline (Display.h/Menu.h/Graphics.h)
      // wholesale, just different pins and an ESP32 SPIClass object instead
      // of the nRF52 core's pre-wired SPI1. Meshtastic/MeshCore variant.h:
      // ST7735_CS 38, ST7735_RS(DC) 40, ST7735_SDA(MOSI) 42, ST7735_SCK 41,
      // ST7735_RESET 39, TFT_BL 21.
      #define DISPLAY_SCALE 1
      #define USE_COLOR_DISPLAY true
      #define PIN_WTV2_TFT_MOSI 42
      #define PIN_WTV2_TFT_SCK 41
      #define PIN_WTV2_TFT_SS 38
      #define PIN_WTV2_TFT_DC 40
      #define PIN_WTV2_TFT_RST 39
      #define PIN_WTV2_TFT_BLGT 21

      // VEXT_ENABLE (GPIO3, active HIGH) - powers the GPS, GPS LNA, and the
      // TFT together (Meshtastic variant.h: "VEXT_ENABLE 3 // active HIGH -
      // powers the GPS, GPS LNA and OLED"). Unlike T096, there's no separate
      // per-peripheral enable pin, so this one line is both this board's
      // "TFT_EN" (driven from display_init(), Display.h) and its GPS power -
      // see HAS_GPS below for why PIN_GPS_EN is deliberately left undefined.
      #define PIN_WTV2_VEXT_EN 3

      const int DISPLAY_DC = PIN_WTV2_TFT_DC;
      const int DISPLAY_CS = PIN_WTV2_TFT_SS;
      const int DISPLAY_MOSI = PIN_WTV2_TFT_MOSI;
      const int DISPLAY_CLK = PIN_WTV2_TFT_SCK;
      const int DISPLAY_BL_PIN = PIN_WTV2_TFT_BLGT;
      const int DISPLAY_RST = PIN_WTV2_TFT_RST;

      // Built-in UC6580 GNSS - same chipset as BOARD_HELTEC_T096, so
      // GNSS.h's existing GPS_MODEL_UC6580 parser needs no changes.
      // Meshtastic variant.h: GPS_RX_PIN 33, GPS_TX_PIN 34, PIN_GPS_RESET
      // 35, PIN_GPS_PPS 36, GPS_BAUDRATE 115200. Unlike T096's dedicated
      // nRF52 Serial2, ESP32's UART peripherals bind pins at .begin() time,
      // so GPS_SERIAL is Serial1 with explicit rx/tx args - same portability
      // indirection as BOARD_MESHADVENTURER_S3's own HAS_GPS block.
      #define HAS_GPS true
      #define GPS_MODEL GPS_MODEL_UC6580
      #define GPS_SERIAL Serial1
      #define GPS_BAUD_RATE 115200
      #define PIN_GPS_RX 33 // MCU RX - wired to GPS TX-out
      #define PIN_GPS_TX 34 // MCU TX - wired to GPS RX-in
      #define PIN_GPS_PPS 36
      #define PIN_GPS_RESET 35 // active LOW (Meshtastic: GPS_RESET_MODE LOW)
      // No PIN_GPS_EN here - GPS power is the shared PIN_WTV2_VEXT_EN line
      // above (also used by the TFT), not a dedicated GPS enable pin, so
      // toggling GNSS off in the Settings menu must not touch this pin (it
      // would also kill the display) - same reasoning as
      // BOARD_MESHADVENTURER_S3's own "no PIN_GPS_EN/PIN_GPS_STANDBY apply
      // here" GNSS block. Always-populated onboard receiver (not an
      // optional add-on like MeshAdventurer-S3's), so GNSS_ENABLED_DEFAULT
      // stays at its global default (true).

    #else
      #error An unsupported ESP32 board was selected. Cannot compile RNode firmware.
    #endif

  #elif MCU_VARIANT == MCU_NRF52
    #if BOARD_MODEL == BOARD_RAK4631
      #define HAS_EEPROM false
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH false
      #define HAS_BLE true
      #define HAS_CONSOLE false
      #define HAS_PMU false
      #define HAS_NP false
      #define HAS_SD false
      #define HAS_TCXO true
      #define HAS_RF_SWITCH_RX_TX true
      #define HAS_BUSY true
      #define HAS_INPUT true
      #define DIO2_AS_RF_SWITCH true
      #define CONFIG_UART_BUFFER_SIZE 6144
      #define CONFIG_QUEUE_SIZE 6144
      #define CONFIG_QUEUE_MAX_LENGTH 200
      #define EEPROM_SIZE 296
      #define EEPROM_OFFSET EEPROM_SIZE-EEPROM_RESERVED
      #define BLE_MANUFACTURER "RAK Wireless"
      #define BLE_MODEL "RAK4640"

      const int pin_btn_usr1 = 9;

      // Following pins are for the sx1262
      const int pin_rxen = 37;
      const int pin_txen = -1;
      const int pin_reset = 38;
      const int pin_cs = 42;
      const int pin_sclk = 43;
      const int pin_mosi = 44;
      const int pin_miso = 45;
      const int pin_busy = 46;
      const int pin_dio = 47;
      const int pin_led_rx = LED_BLUE;
      const int pin_led_tx = LED_GREEN;
      const int pin_tcxo_enable = -1;

    #elif BOARD_MODEL == BOARD_RAK3401
      // Reuses RAK4630/RAK5005-O WisBlock mechanicals (same OLED/GPS
      // IO-slot pins as BOARD_RAK4631 above), but the radio itself is NOT
      // pin-compatible with RAK4631 - the SX1262 sits on a second SPI bus
      // (pins 3/29/30, normally this module's QSPI flash pins - the RAK3401
      // Core has no onboard flash populated, so both reference firmwares
      // repurpose them as plain GPIO for the radio instead).
      #define HAS_EEPROM false
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH false
      #define HAS_BLE true
      #define HAS_CONSOLE false
      #define HAS_PMU false
      #define HAS_NP false
      #define HAS_SD false
      #define HAS_TCXO true
      #define HAS_BUSY true
      // No digital user button on this Core module - the RAK5005-O
      // baseboard's only button is an analog resistor-divider
      // (PIN_USER_BTN_ANA, P0.31 in both reference firmwares), which this
      // firmware's pin_btn_usr1 concept (Input.h, OTA.h) assumes is a plain
      // digitalRead()/INPUT_PULLUP switch - wiring it up as one would misread
      // the divider's intermediate voltage as a bogus press. Left
      // unsupported until this firmware gains analog-button handling; same
      // pin_btn_usr1 = -1 tradeoff as other buttonless boards below.
      #define HAS_INPUT false
      #define DIO2_AS_RF_SWITCH true
      #define HAS_RF_SWITCH_RX_TX false
      #define CONFIG_UART_BUFFER_SIZE 6144
      #define CONFIG_QUEUE_SIZE 6144
      #define CONFIG_QUEUE_MAX_LENGTH 200
      #define EEPROM_SIZE 296
      #define EEPROM_OFFSET EEPROM_SIZE-EEPROM_RESERVED
      #define BLE_MANUFACTURER "RAK Wireless"
      #define BLE_MODEL "RAK3401"

      // SKY66122-11 FEM on the RAK13302 module: CSD+CPS are tied together on
      // the module PCB and routed to a single enable pin (WisBlock IO3,
      // P0.21) - driven HIGH once at boot (RNode_Firmware.ino's setup()) and
      // left alone, unlike this codebase's GC1109/KCT8103L autodetect path
      // (sx126x.cpp) which actively toggles a CTX pin every TX/RX
      // transition. Here CTX is wired directly to SX1262's own DIO2 instead,
      // handled entirely in hardware via DIO2_AS_RF_SWITCH above. No
      // verified SKY66122 gain curve to build a PA_GAIN_VALUES table from,
      // so - same call as BOARD_TBEAM_1W's unverified XY16P35 - HAS_LORA_PA
      // is left at its default false; the RNode Settings power slider
      // reports the bare SX1262's own chip output, not the FEM-boosted
      // antenna power.
      #undef HAS_LORA_LNA
      #define HAS_LORA_LNA true
      #define LORA_LNA_GAIN  17
      #define LORA_LNA_GVT   12

      const int pin_cs = 26;
      const int pin_sclk = 3;
      const int pin_mosi = 30;
      const int pin_miso = 29;
      const int pin_busy = 9;
      const int pin_reset = 4;
      const int pin_dio = 10; // SX1262 DIO1 - this firmware's generic "pin_dio"/IRQ line
      const int pin_txen = -1;
      const int pin_rxen = -1;
      const int pin_tcxo_enable = -1;

      // P0.21 (WisBlock IO3): SKY66122 CSD+CPS FEM enable - see comment
      // above. P0.34 (WisBlock IO2/PIN_3V3_EN): switched 3V3_S peripheral
      // rail AND the RAK13302's onboard 5V boost (U5) that actually powers
      // the FEM - both must be HIGH before the FEM (and therefore any TX)
      // works, per both reference firmwares' begin() (MeshCore's
      // RAK3401Board.cpp is explicit about P0.34's dual role). Neither pin
      // fits an existing pin_* concept in this firmware, so both are driven
      // directly in setup() (RNode_Firmware.ino), same pattern as
      // BOARD_TBEAM_1W's pin_radio_en.
      const int pin_radio_en = 21;
      const int pin_3v3_en = 34;

      const int pin_btn_usr1 = -1;
      const int pin_led_rx = LED_BLUE;
      const int pin_led_tx = LED_GREEN;

    #elif BOARD_MODEL == BOARD_TECHO
      #define _PINNUM(port, pin) ((port) * 32 + (pin))
      #define MODEM SX1262
      #define HAS_EEPROM false
      #define HAS_BLUETOOTH false
      #define HAS_BLE true
      #define HAS_CONSOLE false
      #define HAS_PMU true
      #define HAS_NP false
      #define HAS_SD false
      #define HAS_TCXO true
      #define HAS_BUSY true
      #define HAS_INPUT true
      #define HAS_SLEEP true
      #define BLE_MANUFACTURER "LilyGO"
      #define BLE_MODEL "T-Echo"

      #define HAS_INPUT true
      #define EEPROM_SIZE 296
      #define EEPROM_OFFSET EEPROM_SIZE-EEPROM_RESERVED

      #define CONFIG_UART_BUFFER_SIZE 32768
      #define CONFIG_QUEUE_SIZE 6144
      #define CONFIG_QUEUE_MAX_LENGTH 200

      #define HAS_DISPLAY true
      #define HAS_BACKLIGHT true
      #define DISPLAY_SCALE 1

      #define LED_ON LOW
      #define LED_OFF HIGH
      #define PIN_LED_GREEN _PINNUM(1, 1)
      #define PIN_LED_RED   _PINNUM(1, 3)
      #define PIN_LED_BLUE  _PINNUM(0, 14)
      #define PIN_VEXT_EN _PINNUM(0, 12)

      const int pin_disp_cs = 30;
      const int pin_disp_dc = 28;
      const int pin_disp_reset = 2;
      const int pin_disp_busy = 3;
      const int pin_disp_en = -1;
      const int pin_disp_sck = 31;
      const int pin_disp_mosi = 29;
      const int pin_disp_miso = -1;
      const int pin_backlight = 43;

      const int pin_btn_usr1 = _PINNUM(1, 10);
      const int pin_btn_touch = _PINNUM(0, 11);

      const int pin_reset = 25;
      const int pin_cs = 24;
      const int pin_sclk = 19;
      const int pin_mosi = 22;
      const int pin_miso = 23;
      const int pin_busy = 17;
      const int pin_dio = 20;
      const int pin_tcxo_enable = 21;
      const int pin_led_rx = PIN_LED_BLUE;
      const int pin_led_tx = PIN_LED_RED;

    #elif BOARD_MODEL == BOARD_HELTEC_T114
      #undef MODEM
      #define MODEM SX1262
      #define HAS_EEPROM false
      #undef HAS_DISPLAY
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH false
      #undef HAS_BLE
      #define HAS_BLE true
      #define HAS_CONSOLE false
      #undef HAS_PMU
      #define HAS_PMU true
      #undef HAS_NP
      #define HAS_NP true
      #define HAS_SD false
      #undef HAS_TCXO
      #define HAS_TCXO true
      #define HAS_BUSY true
      #undef HAS_INPUT
      #define HAS_INPUT true
      #undef HAS_SLEEP
      #define HAS_SLEEP true

      // RNode Settings menu (Menu.h), button-only navigation (tap = next,
      // double-tap = back, hold = select/open - see menu_button_press()).
      // No encoder on this board, same pattern as BOARD_HELTEC_T096. The
      // panel is a color ST7789 TFT rather than a 128x64 SSD1306 OLED, so
      // the menu renders into its own off-screen canvas (Display.h,
      // BOARD_HELTEC_T114-only block) instead of drawing straight to the
      // unbuffered display - see MENU_GFX in Menu.h.
      #define HAS_MENU true

      #define DIO2_AS_RF_SWITCH true
      #define CONFIG_UART_BUFFER_SIZE 6144
      #define CONFIG_QUEUE_SIZE 6144
      #define CONFIG_QUEUE_MAX_LENGTH 200
      #define EEPROM_SIZE 296
      #define EEPROM_OFFSET EEPROM_SIZE-EEPROM_RESERVED
      #define BLE_MANUFACTURER "Heltec"
      #define BLE_MODEL "T114"

      #define PIN_T114_ADC_EN 6
      #define PIN_VEXT_EN 21

      #define HAS_GPS true
      #define GPS_MODEL GPS_MODEL_L76K

      // Verbose GNSS diagnostics (Menu.h) - enabled here first since T114
      // is actively flash-tested; GSA/GSV support on the L76K's default
      // NMEA sentence set is unverified as of this writing (see
      // gnss_gsv_update()'s own comment in GNSS.h) - if it never
      // populates PDOP/VDOP/sats-in-view, this screen should still
      // degrade to N/A rows, not misbehave.
      #define HAS_GNSS_DEBUG_MENU true

      // L76K GNSS, wired to this core's second hardware UART (Serial2 -
      // cores/nRF5/Uart.cpp auto-instantiates it since PIN_SERIAL2_RX/TX
      // are always defined on this variant,
      // ~/.arduino15/packages/Heltec_nRF52/hardware/Heltec_nRF52/variants/
      // HT-n5262/variant.h - same core family as T096's HT-n5262G). No
      // name collisions with vendor macros here (unlike T096's variant.h,
      // this one defines no GPS_*/PIN_GPS_* macros of its own), so no
      // #undef guards needed.
      #define GPS_SERIAL Serial2
      #define GPS_BAUD_RATE 9600 // L76K's factory-default NMEA baud

      #define PIN_GPS_RX 37   // Serial2 RX - MCU receives here (wired to GPS TX-out)
      #define PIN_GPS_TX 39   // Serial2 TX - MCU transmits here (wired to GPS RX-in)
      #define PIN_GPS_PPS 36

      // No dedicated GPS enable pin on this board revision - GPS shares
      // PIN_VEXT_EN above with the display, so a T096-style PIN_GPS_EN
      // power-cutoff would also kill the display. The "Enabled" toggle
      // instead drives the L76K's own PIN_GPS_STANDBY line (soft sleep/
      // wake, independent of VEXT) - see GNSS.h's gnss_set_enabled().
      #define PIN_GPS_STANDBY 34
      #define GNSS_DUTY_CYCLE_CAPABLE true // PIN_GPS_STANDBY only - soft-sleep

      // Battery voltage sensing (measure_battery(), Power.h) goes through
      // pin_vbat and a fixed volts-per-ADC-count constant, same as
      // BOARD_PROMICRO - the RNode Settings menu's Hardware page exposes a
      // recalibration knob as a %/of-default correction against this
      // default (see BATTERY_V_SCALE_DEFAULT/battery_v_scale, Config.h/
      // Power.h).
      #define HAS_BATTERY_DIVIDER true
      #define BATTERY_V_SCALE_DEFAULT 0.017165

      // LED
      #define LED_T114_GREEN 3
      #define PIN_T114_LED 14
      #define NP_M 1
      const int pin_np = PIN_T114_LED;

      // SPI
      #define PIN_T114_MOSI 22
      #define PIN_T114_MISO 23
      #define PIN_T114_SCK  19
      #define PIN_T114_SS   24

      // SX1262
      #define PIN_T114_RST  25
      #define PIN_T114_DIO1 20
      #define PIN_T114_BUSY 17

      // TFT
      #define DISPLAY_SCALE 1
      // Colourize the waterfall (RX green, TX blue) and lamps/battery/
      // banners; set to false for an all-monochrome display
      #define USE_COLOR_DISPLAY true
      #define PIN_T114_TFT_MOSI 9
      #define PIN_T114_TFT_MISO 11 // not connected
      #define PIN_T114_TFT_SCK 8
      #define PIN_T114_TFT_SS 11
      #define PIN_T114_TFT_DC 12
      #define PIN_T114_TFT_RST 2
      #define PIN_T114_TFT_EN 3
      #define PIN_T114_TFT_BLGT 15

      // pins for buttons on Heltec T114
      const int pin_btn_usr1 = 42;

      // pins for sx1262 on Heltec T114
      const int pin_reset = PIN_T114_RST;
      const int pin_cs = PIN_T114_SS;
      const int pin_sclk = PIN_T114_SCK;
      const int pin_mosi = PIN_T114_MOSI;
      const int pin_miso = PIN_T114_MISO;
      const int pin_busy = PIN_T114_BUSY;
      const int pin_dio = PIN_T114_DIO1;
      const int pin_led_rx = 35;
      const int pin_led_tx = 35;
      const int pin_tcxo_enable = -1;

      // pins for ST7789 display on Heltec T114
      const int DISPLAY_DC = PIN_T114_TFT_DC;
      const int DISPLAY_CS = PIN_T114_TFT_SS;
      const int DISPLAY_MISO = PIN_T114_TFT_MISO;
      const int DISPLAY_MOSI = PIN_T114_TFT_MOSI;
      const int DISPLAY_CLK = PIN_T114_TFT_SCK;
      const int DISPLAY_BL_PIN = PIN_T114_TFT_BLGT;
      const int DISPLAY_RST = PIN_T114_TFT_RST;

    #elif BOARD_MODEL == BOARD_HELTEC_T096
      #undef MODEM
      #define MODEM SX1262
      #define HAS_EEPROM false
      #undef HAS_DISPLAY
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH false
      #undef HAS_BLE
      #define HAS_BLE true
      #define HAS_CONSOLE false
      #undef HAS_PMU
      #define HAS_PMU true
      #define HAS_NP false
      #define HAS_SD false
      #undef HAS_TCXO
      #define HAS_TCXO true
      #define HAS_BUSY true
      #undef HAS_INPUT
      #define HAS_INPUT true
      #undef HAS_SLEEP
      #define HAS_SLEEP true

      // RNode Settings menu (Menu.h), button-only navigation (tap = next,
      // double-tap = back, hold = select/open - see menu_button_press()).
      // No encoder on this board, same pattern as BOARD_MESHPOE_S3. The
      // panel is a 160x80/80x160 color ST7735 TFT rather than a 128x64
      // SSD1306 OLED, so the menu renders into its own off-screen canvas
      // (Display.h, BOARD_HELTEC_T096-only block) instead of drawing
      // straight to the unbuffered display - see MENU_GFX in Menu.h.
      #define HAS_MENU true

      #define DIO2_AS_RF_SWITCH true
      #define CONFIG_UART_BUFFER_SIZE 6144
      #define CONFIG_QUEUE_SIZE 6144
      #define CONFIG_QUEUE_MAX_LENGTH 200
      #define EEPROM_SIZE 296
      #define EEPROM_OFFSET EEPROM_SIZE-EEPROM_RESERVED
      #define BLE_MANUFACTURER "Heltec"
      #define BLE_MODEL "T096"

      #undef HAS_LORA_PA
      #define HAS_LORA_PA true
      #undef HAS_LORA_LNA
      #define HAS_LORA_LNA true
      #define OCP_TUNED 0x38
      #define LORA_PA_MODEL LORA_PA_KCT8103L
      #define LNA_GD_THRSHLD (-109)
      #define LNA_GD_LIMIT   (-89)

      // The KCT8103L RX LNA has 21 dB gain (1.9 dB NF). The FEM model is
      // fixed at compile time on this board, so the runtime detection that
      // corrects this value on the Heltec V4 never runs — it must be right
      // here.
      #define LORA_LNA_GAIN  21
      #define LORA_LNA_GVT   12
      // KCT8103L FEM: VFEM LDO enable on P0.30, CSD on P0.12, CTX on P1.09
      // (41). CPS is wired to SX1262 DIO2 and switched automatically via
      // DIO2_AS_RF_SWITCH.
      #define LORA_PA_PWR_EN 30
      #define LORA_PA_CPS    -1
      #define LORA_PA_CSD    12
      #define LORA_PA_CTX    41

      #define PA_MAX_OUTPUT  28
      #define PA_GAIN_POINTS 22

      #define LORA_LNA_KCT8103L_GAIN 21
      const int PA_KCT8103L_VALUES[PA_GAIN_POINTS] = {13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 12, 11, 11, 10, 9, 8, 7};

      // LED
      #define PIN_T096_LED 28

      // SPI
      #define PIN_T096_MOSI 11
      #define PIN_T096_MISO 14
      #define PIN_T096_SCK  40
      #define PIN_T096_SS   5

      // SX1262
      #define PIN_T096_RST  16
      #define PIN_T096_DIO1 21
      #define PIN_T096_BUSY 19

      // TFT
      #define DISPLAY_SCALE 1
      // Colourize the waterfall (RX green, TX blue); set to false for an
      // all-monochrome display
      #define USE_COLOR_DISPLAY true
      #define PIN_T096_TFT_MOSI 17
      #define PIN_T096_TFT_SCK 20
      #define PIN_T096_TFT_SS 22
      #define PIN_T096_TFT_DC 15
      #define PIN_T096_TFT_RST 13
      #define PIN_T096_TFT_EN 26
      #define PIN_T096_TFT_BLGT 44

      #define HAS_GPS true
      #define GPS_MODEL GPS_MODEL_UC6580

      // Verbose GNSS diagnostics (Menu.h) - third board to opt in, after
      // T114's L76K and MeshAdventurer-S3's AT6558. GSA/GSV support on the
      // UC6580's default NMEA sentence set is unverified as of this
      // writing, same open risk as the other two (see gnss_gsv_update()'s
      // own comment in GNSS.h) - PDOP/VDOP/Sats In View should degrade to
      // N/A/No Data cleanly if the module doesn't emit them. Unlike T114,
      // this board is GNSS_DUTY_CYCLE_HARD_ONLY (every wake is a cold
      // start), so the main GNSS page's Module row swaps to "Duty State"
      // same as T114 - worth confirming that reads sensibly given the
      // longer, harder power cycle.
      #define HAS_GNSS_DEBUG_MENU true

      // UC6580 GNSS, wired to this core's second hardware UART (Serial2 -
      // cores/nRF5/Uart.cpp auto-instantiates it since PIN_SERIAL2_RX/TX
      // are always defined on this variant,
      // ~/.arduino15/packages/Heltec_nRF52/hardware/Heltec_nRF52/variants/
      // HT-n5262G/variant.h). GPS_SERIAL is a portability indirection so
      // GNSS.h says GPS_SERIAL instead of hardcoding Serial2 - a future
      // ESP32 GPS board would define this to Serial1 and additionally need
      // explicit rx/tx args at .begin() time.
      #define GPS_SERIAL Serial2
      #define GPS_BAUD_RATE 115200

      // No name collision with the vendor core's own GPS_RX_PIN/GPS_TX_PIN.
      // PIN_GPS_RX=23 is genuinely the MCU's receive pin (wired to GPS
      // TX-out) - the vendor header's own inline comment on this pin is
      // backwards, verified against Uart::Uart()'s real PSEL.RXD/TXD
      // assignment (cores/nRF5/Uart.cpp), not just copied from the vendor
      // header.
      #define PIN_GPS_RX 23
      #define PIN_GPS_TX 25

      // The vendor variant.h defines these exact names too (same values,
      // different token spelling, e.g. "(0+6)" vs "6") - #undef first to
      // avoid a harmless macro-redefinition warning.
      #undef PIN_GPS_EN
      #define PIN_GPS_EN 6        // active LOW
      #define GNSS_DUTY_CYCLE_CAPABLE true // PIN_GPS_EN only - hard power-off, cold start every wake
      #define GNSS_DUTY_CYCLE_HARD_ONLY true
      #undef PIN_GPS_PPS
      #define PIN_GPS_PPS 43
      #undef PIN_GPS_RESET
      #define PIN_GPS_RESET 46    // active LOW, needs a >100ms hold to reset

      // pins for buttons on Heltec T096
      const int pin_btn_usr1 = 42;

      // pins for sx1262 on Heltec T096
      const int pin_reset = PIN_T096_RST;
      const int pin_cs = PIN_T096_SS;
      const int pin_sclk = PIN_T096_SCK;
      const int pin_mosi = PIN_T096_MOSI;
      const int pin_miso = PIN_T096_MISO;
      const int pin_busy = PIN_T096_BUSY;
      const int pin_dio = PIN_T096_DIO1;
      const int pin_led_rx = 28;
      const int pin_led_tx = 28;
      const int pin_tcxo_enable = -1;

      // pins for ST7735 display on Heltec T096
      const int DISPLAY_DC = PIN_T096_TFT_DC;
      const int DISPLAY_CS = PIN_T096_TFT_SS;
      const int DISPLAY_MOSI = PIN_T096_TFT_MOSI;
      const int DISPLAY_CLK = PIN_T096_TFT_SCK;
      const int DISPLAY_BL_PIN = PIN_T096_TFT_BLGT;
      const int DISPLAY_RST = PIN_T096_TFT_RST;

    #elif BOARD_MODEL == BOARD_HELTEC_T1
      #undef MODEM
      #define MODEM SX1262
      #define HAS_EEPROM false
      #undef HAS_DISPLAY
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH false
      #undef HAS_BLE
      #define HAS_BLE true
      #define HAS_CONSOLE false
      #undef HAS_PMU
      #define HAS_PMU true
      #define HAS_NP false
      #define HAS_SD false
      #undef HAS_TCXO
      #define HAS_TCXO true
      #define HAS_BUSY true
      #undef HAS_INPUT
      #define HAS_INPUT true
      #undef HAS_SLEEP
      #define HAS_SLEEP true

      // RNode Settings menu (Menu.h), button-only navigation - same pattern
      // as BOARD_HELTEC_T096 (no encoder on this board either). Shares
      // T096's 160x80 ST7735 menu rendering path (Menu.h/Display.h's own
      // BOARD_HELTEC_T096-grouped blocks) since it's the identical panel.
      #define HAS_MENU true

      #define DIO2_AS_RF_SWITCH true
      #define CONFIG_UART_BUFFER_SIZE 6144
      #define CONFIG_QUEUE_SIZE 6144
      #define CONFIG_QUEUE_MAX_LENGTH 200
      #define EEPROM_SIZE 296
      #define EEPROM_OFFSET EEPROM_SIZE-EEPROM_RESERVED
      #define BLE_MANUFACTURER "Heltec"
      #define BLE_MODEL "T1"

      // No external FEM/PA on this board (bare SX1262, no KCT8103L or
      // similar chip in either reference variant.h) - same as
      // BOARD_HELTEC_T114, unlike T096/WTRACKER_V2's KCT8103L. HAS_LORA_PA/
      // HAS_LORA_LNA stay at their default false (Boards.h global-defaults
      // block below).

      // Piezo buzzer behind a voltage-doubler circuit - unique to this
      // board among the nRF52 Heltec boards. PIN_BUZZER is the actual
      // tone()/noTone() pin; the two multiplier pins are just held HIGH
      // once at boot to enable the boost (Utilities.h's own
      // BOARD_HELTEC_T1 branch in buzzer_init()), never toggled again -
      // matches MeshCore's T1Board::begin(), the only reference driver
      // that does anything with them.
      // Vendor variant.h defines PIN_BUZZER too (same value, different
      // token spelling, "(0+9)" vs "9") - #undef first to avoid a harmless
      // macro-redefinition warning, same as PIN_GPS_EN/PPS/RESET below.
      #define HAS_BUZZER true
      #undef PIN_BUZZER
      #define PIN_BUZZER 9
      #define PIN_T1_BUZZER_MULT1 34
      #define PIN_T1_BUZZER_MULT2 37

      // LED (P0.16) - active LOW per both reference variant.h's
      // (LED_STATE_ON=0/LOW), same polarity as BOARD_HELTEC_T114, unlike
      // T096's active-HIGH LED - see Utilities.h's led_rx_on()/etc., joined
      // to T114's branch there, not T096's.
      const int pin_led_rx = 16;
      const int pin_led_tx = 16;

      // SPI (LoRa) - Meshtastic/MeshCore/vendor variant.h agree: CS=(32+11),
      // MISO=(0+3), MOSI=(32+14), SCK=(32+13), RESET=(0+2), DIO1=(0+31,
      // IRQ), BUSY=(0+29).
      const int pin_cs = 43;
      const int pin_sclk = 45;
      const int pin_mosi = 46;
      const int pin_miso = 3;
      const int pin_reset = 2;
      const int pin_busy = 29;
      const int pin_dio = 31;
      const int pin_tcxo_enable = -1;

      const int pin_btn_usr1 = 42;

      // Battery voltage sensing (measure_battery(), Power.h) goes through
      // pin_vbat and a fixed volts-per-ADC-count constant, same mechanism
      // as BOARD_HELTEC_T114/BOARD_PROMICRO - the RNode Settings menu's
      // Hardware page exposes a recalibration knob as a %/of-default
      // correction against this default (see BATTERY_V_SCALE_DEFAULT/
      // battery_v_scale, Config.h/Power.h). Meshtastic/MeshCore variant.h:
      // BATTERY_PIN (0+5), ADC_CTRL (0+11) (active HIGH, powers the
      // divider) - both give an explicit ADC_MULTIPLIER (4.916) computed
      // against a 12-bit/3.0V-reference read they set up explicitly
      // (analogReadResolution(12)/analogReference(AR_INTERNAL_3_0)), which
      // this codebase's own battery_v_scale mechanism has never set on any
      // board (T114/T096 rely on the core's own ADC defaults instead) - so
      // that constant isn't directly reusable here. Same core/MCU as T096,
      // whose own empirically-tuned constant (0.017165) T114 already reuses
      // as its own untested default for exactly this reason - following
      // that precedent rather than introducing a differently-configured ADC
      // setup with no hardware to verify it against.
      #define HAS_BATTERY_DIVIDER true
      #define BATTERY_V_SCALE_DEFAULT 0.017165

      // ST7735S 160x80 TFT, identical panel to BOARD_HELTEC_T096 - shares
      // that board's rendering pipeline (Display.h/Menu.h/Graphics.h)
      // wholesale. This core also pre-wires a hardware SPI1 the same way
      // T096's does (vendor variant.h: SPI_32MHZ_INTERFACE 1, SS1/MOSI1/
      // MISO1/SCK1 statics), so the display object itself can reuse T096's
      // exact `&SPI1` construction (Display.h) - only the enable/backlight
      // pins and their polarity differ (active LOW here vs T096's active
      // HIGH - vendor variant.h: TFT_VDD_ENABLE=0, TFT_LEDA_ENABLE=0).
      // Meshtastic/MeshCore/vendor variant.h: ST7735_CS (0+12), ST7735_RS/
      // DC (0+22), ST7735_SDA/MOSI (0+24), ST7735_SCK (32+0), ST7735_RESET
      // (0+20), ST7735_BL (0+15), VTFT_CTRL (0+13).
      #define DISPLAY_SCALE 1
      #define USE_COLOR_DISPLAY true
      #define PIN_T1_TFT_MOSI 24
      #define PIN_T1_TFT_SCK 32
      #define PIN_T1_TFT_SS 12
      #define PIN_T1_TFT_DC 22
      #define PIN_T1_TFT_RST 20
      #define PIN_T1_TFT_BLGT 15
      // Whole-panel power enable, active LOW (unlike T096's PIN_T096_TFT_EN,
      // active HIGH) - driven from Display.h's own BOARD_HELTEC_T1 branch
      // in display_init().
      #define PIN_T1_TFT_EN 13

      const int DISPLAY_DC = PIN_T1_TFT_DC;
      const int DISPLAY_CS = PIN_T1_TFT_SS;
      const int DISPLAY_MOSI = PIN_T1_TFT_MOSI;
      const int DISPLAY_CLK = PIN_T1_TFT_SCK;
      const int DISPLAY_BL_PIN = PIN_T1_TFT_BLGT;
      const int DISPLAY_RST = PIN_T1_TFT_RST;

      // Built-in UC6580 GNSS - same chipset as BOARD_HELTEC_T096, so
      // GNSS.h's existing GPS_MODEL_UC6580 parser needs no changes. Unlike
      // T096 (dedicated Serial2), this core's vendor variant.h binds the
      // GPS UART to Serial1 (PIN_SERIAL1_RX/TX point at the GPS pins, not
      // PIN_SERIAL2_RX/TX) - Serial1 is free here (KISS link is native USB
      // CDC, same as T096), so GPS_SERIAL is set explicitly to Serial1.
      //
      // Meshtastic's and the vendor board package's own variant.h agree:
      // MCU RX (wired to GPS TX-out) = (0+8) = 8, MCU TX (wired to GPS
      // RX-in) = (0+7) = 7. MeshCore's variant.h has these swapped (RX=7,
      // TX=8) - going with the vendor+Meshtastic majority since the vendor
      // file is the primary source both almost certainly derive from.
      // Worth double-checking against real hardware if GPS never locks.
      #define HAS_GPS true
      #define GPS_MODEL GPS_MODEL_UC6580
      #define GPS_SERIAL Serial1
      #define GPS_BAUD_RATE 115200
      // Vendor variant.h defines PIN_GPS_RX/PPS/RESET/EN too (same values,
      // different token spelling, e.g. "(0+8)" vs "8") - #undef first to
      // avoid a harmless macro-redefinition warning, same as T096's own
      // pattern above.
      #undef PIN_GPS_RX
      #define PIN_GPS_RX 8  // MCU RX - wired to GPS TX-out
      #define PIN_GPS_TX 7  // MCU TX - wired to GPS RX-in
      #undef PIN_GPS_PPS
      #define PIN_GPS_PPS 41 // (32+9)
      #undef PIN_GPS_RESET
      #define PIN_GPS_RESET 26 // active LOW, needs a >100ms hold to reset
      #undef PIN_GPS_EN
      #define PIN_GPS_EN 4      // active LOW, standalone (not shared with
                                 // the display, unlike WTRACKER_V2)
      #define GNSS_DUTY_CYCLE_CAPABLE true // PIN_GPS_EN only - hard power-off, cold start every wake
      #define GNSS_DUTY_CYCLE_HARD_ONLY true

    #elif BOARD_MODEL == BOARD_PROMICRO
      //TODO:
      // - Fix low output power
      // - Make compatible with non-TCXO radios
      // - Add PMU
      #undef MODEM
      #define MODEM SX1262
      #define HAS_EEPROM false
      #define HAS_BLUETOOTH false
      #undef HAS_BLE
      #define HAS_BLE true
      #define HAS_CONSOLE false
      #undef HAS_PMU
      #define HAS_PMU true
      #define HAS_NP false
      #define HAS_SD false
      #undef HAS_TCXO
      #define HAS_TCXO true
      #define HAS_BUSY true
      #define HAS_RF_SWITCH_RX_TX true
      #define DIO2_AS_RF_SWITCH true
      #define OCP_TUNED 0x38
      #undef HAS_SLEEP
      #define HAS_SLEEP true
      #define BLE_MANUFACTURER "DIY"
      #define BLE_MODEL "ProMicro"

      #undef HAS_INPUT
      #define HAS_INPUT true
      #define EEPROM_SIZE 296
      #define EEPROM_OFFSET EEPROM_SIZE-EEPROM_RESERVED

      #define CONFIG_UART_BUFFER_SIZE 6144
      #define CONFIG_QUEUE_SIZE 6144
      #define CONFIG_QUEUE_MAX_LENGTH 200

      // RNode Settings menu (Menu.h). Main button navigation (tap = next,
      // double-tap = back, hold = select/open - see menu_button_press())
      // always works regardless of the below. Encoder support is compiled
      // in unconditionally - on builds with no physical encoder wired up,
      // this is harmless: the pins just sit as pulled-up inputs that never
      // see a transition, so the ISR never fires. The three pins below are
      // just a starting point, not a wiring requirement - they're
      // reassignable at runtime via Hardware > GPIO on this board (see
      // HAS_GPIO_MENU below), so there's no need to know your exact wiring
      // ahead of time. Still overridable via build flags (e.g.
      // -DPIN_ENCODER_UP=...) for anyone who'd rather fix them at compile
      // time instead.
      #define HAS_MENU true
      // TEMPORARY diagnostic guard - defaults true (unconditional, as
      // decided), but overridable via -DHAS_ENCODER=false for isolating
      // the boot-hang report. Revert to a plain `#define HAS_ENCODER true`
      // once the actual cause is confirmed/fixed.
      #ifndef HAS_ENCODER
        #define HAS_ENCODER true
      #endif
      #ifndef PIN_ENCODER_UP
        #define PIN_ENCODER_UP 0
      #endif
      #ifndef PIN_ENCODER_DOWN
        #define PIN_ENCODER_DOWN 1
      #endif
      #ifndef PIN_ENCODER_PRESS
        #define PIN_ENCODER_PRESS 9
      #endif

      // Battery voltage sensing (measure_battery(), Power.h) goes through
      // pin_vbat and a fixed volts-per-ADC-count constant, not a VSENSE
      // resistor divider - there's no clean way to isolate "the divider"
      // out of that constant, so the RNode Settings menu's Hardware page
      // exposes a recalibration knob as a %/of-default correction instead
      // (see BATTERY_V_SCALE_DEFAULT/battery_v_scale, Config.h/Power.h).
      #define HAS_BATTERY_DIVIDER true
      #define BATTERY_V_SCALE_DEFAULT 0.006263

      // Buzzer (and, if HAS_ENCODER, the encoder's Up/Down/Press
      // pins). This is a DIY board - builders solder these to whichever
      // spare pads they used, so each is user-selectable at runtime via
      // the RNode Settings menu (Hardware > GPIO - see HAS_GPIO_MENU,
      // Menu.h) rather than fixed here. PIN_BUZZER/PIN_ENCODER_UP/DOWN/
      // PRESS are just compiled-in defaults (overridden from EEPROM at
      // boot if ever changed via the menu - see buzzer_pin/pin_encoder_up/
      // down/press, Utilities.h). The menu also refuses to assign the same
      // pin to two of these functions at once.
      //
      // The candidate list below is deliberately short and curated, not
      // "every pin this firmware doesn't already use":
      //  - D9/D18/D19/D20 have no competing role at all (LoRa SPI/DIO/BUSY,
      //    display I2C, button, battery ADC, LED, VEXT claim the rest -
      //    see the pin list above), confirmed genuinely free by an
      //    independent source, Meshtastic's own variant for this exact
      //    physical board (nrf52_promicro_diy_tcxo/variant.h), which labels
      //    them "Free pin" outright.
      //  - D3/D4/D5 are that same variant's GPS_TX/GPS_RX/PIN_GPS_EN -
      //    unused by RNode_Firmware (no GPS support), so free here, but
      //    only included because that role is a named, known one a builder
      //    can consciously decide to give up, not an arbitrary pin.
      //  - D0/D1 are this core's Serial1/UART TX/RX (nice_nano/variant.h:
      //    D0 = P0.06 = TX, D1 = P0.08 = RX) - not used by this firmware's
      //    console/KISS link (that's USB CDC, a separate peripheral), so
      //    also free, on the same "named, known role" basis as the GPS
      //    pins above.
      // Most bare ProMicro/FakeTec builds don't have a buzzer installed at
      // all (it's a DIY add-on, easy to skip), so - unlike boards where
      // it's a standard always-populated feature - Sound defaults OFF
      // here rather than the global default of ON (see SOUND_ENABLED_DEFAULT
      // fallback below, sound_enabled, Utilities.h).
      #define SOUND_ENABLED_DEFAULT false
      #define HAS_BUZZER true
      #define PIN_BUZZER 5
      #define HAS_GPIO_MENU true
      #define GPIO_FREE_PIN_CANDIDATE_COUNT 9
      const uint8_t gpio_free_pin_candidates[GPIO_FREE_PIN_CANDIDATE_COUNT] = {0, 1, 3, 4, 5, 9, 18, 19, 20};

      //Confused with the pin numbers??
      //https://github.com/pdcook/nRFMicro-Arduino-Core/blob/a83161e619da8668f726b52578a3dd89c1ef5956/variants/nice_nano/variant.h#L59

      #undef HAS_DISPLAY
      #define HAS_DISPLAY true
      #define I2C_SDA 8 //P1.04
      #define I2C_SCL 7 //P0.11

      #define PIN_LED_RED   22 //P0.15
      const int pin_led_rx = PIN_LED_RED;
      const int pin_led_tx = PIN_LED_RED;

      #define PIN_VEXT_EN 21 //P0.13

      const int pin_btn_usr1 = 6; //P1.00

      const int pin_reset = 10; //P0.09
      const int pin_cs    = 13; //P1.13
      const int pin_sclk  = 12; //P1.11
      const int pin_mosi  = 14; //P1.15
      const int pin_miso  = 15; //P0.02
      const int pin_busy  = 16; //P0.29
      const int pin_dio   = 11; //P0.10
      const int pin_rxen  = 2;  //P0.17
      const int pin_txen  = -1;
      const int pin_tcxo_enable = -1;

    #else
      #error An unsupported nRF board was selected. Cannot compile RNode firmware.
    #endif

  #endif

  #ifndef DISPLAY_SCALE
    #define DISPLAY_SCALE 1
  #endif

  #ifndef HAS_RF_SWITCH_RX_TX
    const int pin_rxen = -1;
    const int pin_txen = -1;
  #endif

  #ifndef HAS_BUSY
    const int pin_busy = -1;
  #endif

  #ifndef HAS_BUZZER
    #define HAS_BUZZER false
  #endif

  #ifndef HAS_ENCODER
    #define HAS_ENCODER false
  #endif

  // Whether the RNode Settings menu (Menu.h) is compiled in at all. Boards
  // with a rotary encoder always get it; boards without one can still opt
  // in and navigate the menu with just the main button (see
  // menu_button_press()/menu_button_process(), Menu.h) by defining
  // HAS_MENU true explicitly in their board block above.
  #ifndef HAS_MENU
    #define HAS_MENU HAS_ENCODER
  #endif

  #ifndef HAS_VSENSE
    #define HAS_VSENSE false
  #endif

  #ifndef HAS_BATTERY_DIVIDER
    #define HAS_BATTERY_DIVIDER false
  #endif

  // Whether an external RTC chip (e.g. DS3231MZ) is present. RTC.h doesn't
  // bring up its own I2C bus - whatever brings up Wire for this board
  // (typically display_init(), Display.h, on boards that share the OLED's
  // bus) must run before rtc_init() does - see BOARD_MESHPOE_S3's block
  // above for the pattern. A board with no display would need its own
  // explicit Wire.begin() somewhere before rtc_init() runs.
  #ifndef HAS_RTC
    #define HAS_RTC false
  #endif

  // Whether a GNSS receiver (GNSS.h) is present. No global PIN_GPS_*
  // fallback is needed - GNSS.h is only ever #include'd when HAS_GPS is
  // true, same reasoning as HAS_RTC's pins above never needing one.
  #ifndef HAS_GPS
    #define HAS_GPS false
  #endif

  // Whether the verbose GNSS diagnostics subscreen (Menu.h's
  // MENU_STATE_GNSS_DIAG/MENU_STATE_GNSS_DIAG_SATS) is compiled in.
  // Opt-in per board, independent of HAS_GPS itself (meaningless without
  // it, but not automatically turned on by it) - this is a debug/verbose
  // surface, not something every HAS_GPS board's end user needs by
  // default, and it adds real runtime cost (TinyGPSCustom PDOP/VDOP/GSV
  // parsing on every gnss_update() byte) that boards which haven't
  // verified GSA/GSV support on their specific chip shouldn't pay for.
  #ifndef HAS_GNSS_DEBUG_MENU
    #define HAS_GNSS_DEBUG_MENU false
  #endif

  // Whether an onboard microReticulum node (URNS.h) runs alongside the
  // normal KISS/host modem path, giving the device its own Identity that
  // can originate/receive Reticulum packets directly (sensor telemetry,
  // GNSS wardrive beacons) over the same shared radio. Experimental,
  // branch-only - not wired into the arduino-cli/Makefile toolchain, only
  // the platformio.ini envs at the repo root. MeshAdventurer-S3
  // (microreticulum-onboard-node branch) and MeshPoE-S3
  // (meshpoe-s3-urns branch) only for now.
  #ifndef HAS_URNS
    #define HAS_URNS false
  #endif

  // Whether the LXMF messenger app (Messenger.h) and its onboard LXMRouter
  // run on top of the microReticulum node above. Split out from HAS_URNS so
  // a board can run the bare RNS transport (Identity, Transport, path
  // table, Provisioning) without the messenger/LXMF stack on top - defaults
  // false regardless of HAS_URNS, so a board must explicitly opt in with its
  // own "#define HAS_LXMF true" after defining HAS_URNS true (see
  // BOARD_MESHADVENTURER_S3's block for the pattern). Meaningless (and
  // unused) when HAS_URNS is false, since URNS.h/Messenger.h are only
  // #include'd behind HAS_URNS in the first place (Utilities.h).
  #ifndef HAS_LXMF
    #define HAS_LXMF false
  #endif

  // Compiled default for urns_enabled (URNS.h) when its EEPROM byte
  // (ADDR_CONF_URNS) has never been explicitly written - same "erased
  // EEPROM means never touched, so fall back to this compiled default"
  // convention that byte already uses. Defaults false: URNS should be an
  // explicit opt-in on a newly flashed/provisioned board, not silently
  // running out of the box. MeshAdventurer-S3/MeshPoE-S3 override this
  // back to true in their own blocks below, same "explicit opt-back-in"
  // pattern as HAS_LXMF above - those two boards' onboard node ran
  // unconditionally before either toggle existed, so their own
  // already-deployed units (which have never touched this EEPROM byte
  // either) need to keep booting with URNS on, not silently lose it on
  // their next firmware update. Meaningless when HAS_URNS is false.
  #ifndef URNS_ENABLED_DEFAULT
    #define URNS_ENABLED_DEFAULT false
  #endif

  // Whether this board has a free UART broken out to a header/pins that
  // isn't used for anything else (Serial/KISS included) - see
  // BOARD_MESHPOE_S3's block above for the pattern (Serial0/GPIO43-44).
  // The actual DEBUG_UART_BEGIN()/DEBUG_LOG() macros that use this live in
  // Utilities.h, not here - Boards.h only ever declares capabilities, it
  // doesn't implement behavior (same reasoning as every other HAS_* flag).
  #ifndef HAS_DEBUG_UART
    #define HAS_DEBUG_UART false
  #endif

  #ifndef BATTERY_V_SCALE_DEFAULT
    #define BATTERY_V_SCALE_DEFAULT 0.0
  #endif

  // Whether the RNode Settings menu's Hardware page has a "GPIO" submenu
  // for reassigning peripherals (currently just the buzzer) to a different
  // physical pin at runtime - see PROMICRO's board block for the pattern.
  #ifndef HAS_GPIO_MENU
    #define HAS_GPIO_MENU false
  #endif

  // Whether the buzzer defaults to enabled at first boot (before the user
  // has ever touched Sound in the menu) - see sound_enabled, Utilities.h.
  #ifndef SOUND_ENABLED_DEFAULT
    #define SOUND_ENABLED_DEFAULT false
  #endif

  // Whether the GNSS receiver defaults to enabled at first boot (before the
  // user has ever touched Enabled on the Settings menu's GNSS page) - see
  // gnss_enabled, GNSS.h.
  #ifndef GNSS_ENABLED_DEFAULT
    #define GNSS_ENABLED_DEFAULT false
  #endif

  // Duty-cycled GNSS acquisition (GNSS.h) - default interval, 0 = Continuous
  // (today's always-on behavior), the safe default on every board. Opt-in
  // only, via the Settings menu's GNSS > Update Interval field.
  #ifndef GNSS_UPDATE_INTERVAL_DEFAULT
    #define GNSS_UPDATE_INTERVAL_DEFAULT 0
  #endif

  // Whether this board can actually power the GNSS receiver down between
  // duty-cycle wakes at all - only true where a board-specific GPS block
  // above defined PIN_GPS_EN and/or PIN_GPS_STANDBY. Boards with neither
  // (the receiver's power is either unswitched, like MeshAdventurer-S3's
  // AT6558 add-on, or shares a rail with the display, like Heltec
  // WTracker V2's UC6580) get no benefit from duty-cycling - the interval
  // setting stays hidden there and Continuous is the only mode.
  #ifndef GNSS_DUTY_CYCLE_CAPABLE
    #define GNSS_DUTY_CYCLE_CAPABLE false
  #endif

  // Whether this board's only gating pin is PIN_GPS_EN (a hard power
  // switch) with no PIN_GPS_STANDBY - every wake is therefore a cold start,
  // so the Settings menu's selectable interval range starts at 5 min
  // instead of 1 min there (a 1-min duty cycle would mostly be spent
  // waiting on a cold start, defeating the point).
  #ifndef GNSS_DUTY_CYCLE_HARD_ONLY
    #define GNSS_DUTY_CYCLE_HARD_ONLY false
  #endif

  #ifndef VSENSE_DIVIDER_RATIO_DEFAULT
    #define VSENSE_DIVIDER_RATIO_DEFAULT 11.0
  #endif

  #ifndef LED_ON
    #define LED_ON HIGH
  #endif
  
  #ifndef LED_OFF
    #define LED_OFF LOW
  #endif

  #ifndef DIO2_AS_RF_SWITCH
    #define DIO2_AS_RF_SWITCH false
  #endif

  // Default OCP value if not specified in board configuration - 0x38
  // (140mA) matches the SX1262's real current draw (~118mA at +14dBm,
  // ~158mA at +22dBm per the Semtech datasheet, confirmed against
  // RadioLib/Meshtastic), not just this board's tuning - see the
  // per-board #define OCP_TUNED sites for the full "ported from
  // microReticulum_Firmware" explanation. sx127x-based boards never
  // reference OCP_TUNED at all in this fork, so this default is
  // effectively SX1262-only regardless of which boards fall through to it.
  #ifndef OCP_TUNED
    #define OCP_TUNED 0x38
  #endif

  // NeoPixel intensity scalar's *default* (Utilities.h: np_intensity/npi,
  // Menu.h: the user-facing NeoPixel Brightness setting, EEPROM-persisted
  // via np_int_conf_save()) - not a hard brightness ceiling, just the
  // out-of-box starting point before a user ever touches that setting.
  // Was 0.15 (15%) with no comment explaining why - other than one board
  // (BOARD_HELTEC_T114) explicitly overriding to 1 (full), every NeoPixel
  // board fell through to this same conservative fallback, which starved
  // any effect that further divides its own intensity range (e.g.
  // led_indicate_standby()'s RGB split) down to only a handful of
  // distinct achievable output levels - confirmed live: 100 possible
  // intensity levels collapsed to ~5 real ones after /3 (RGB split) *
  // 0.15. Matching the T114 override instead, since users who want it
  // dimmer already have a real, working way to set that themselves.
  #ifndef NP_M
    #define NP_M 1
  #endif

  // OTA firmware updates (OTA.h) fundamentally depend on a real, comparable
  // BUILD_NUMBER - ota_get_target_build_info()'s update check is
  // *newer_out = remote_build > BUILD_NUMBER, and with BUILD_NUMBER stuck
  // at its unset fallback (0, see BUILD_NUMBER's own comment above) there's
  // no way to tell a genuine build 0 from "the git-commit-count injection
  // never ran" - every remote build would look newer, and downgrade
  // protection would be meaningless. Rather than ship that, OTA is
  // disabled outright and compiled out entirely. This has to run down
  // here, after every #elif BOARD_MODEL branch above, not next to
  // BUILD_NUMBER's own fallback near the top of this file - that runs
  // before BOARD_MESHADVENTURER_S3's own "#undef HAS_OTA / #define
  // HAS_OTA true", which would just re-override an early false right back
  // to true.
  #if BUILD_NUMBER == 0
    #undef HAS_OTA
    #define HAS_OTA false
  #endif

#endif
