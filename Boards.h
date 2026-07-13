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
    #if BOARD_MODEL == BOARD_RAK4631
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

  #define HAS_DISPLAY false
  #define HAS_BLUETOOTH false
  #define HAS_BLE false
  #define HAS_WIFI false
  #define HAS_ETHERNET false
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

    #define GPS_BAUD_RATE 9600
    #define PIN_GPS_TX 12
    #define PIN_GPS_RX 34

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
      #define HAS_DISPLAY true
      #define HAS_NP true
      #define HAS_BLUETOOTH false
      #define HAS_BLE true
      #define HAS_WIFI true
      #define HAS_ETHERNET true
      #define HAS_CONSOLE false
      #define HAS_EEPROM true
      #define HAS_BUSY true
      #define HAS_INPUT true
      #define HAS_TCXO true
      #define MODEM SX1262
      #define DIO2_AS_RF_SWITCH true
      #define HAS_RF_SWITCH_RX_TX false
      #define HAS_LORA_LNA true
      #define LORA_LNA_GAIN  30
      #define LORA_LNA_GVT   14

      const int pin_cs = 17;
      const int pin_sclk = 16;
      const int pin_miso = 15;
      const int pin_mosi = 18;
      const int pin_reset = 3;
      const int pin_busy = 2;
      const int pin_dio = 1;
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

    #elif BOARD_MODEL == BOARD_MESHADVENTURER_S3
      #define IS_ESP32S3 true
      #define HAS_DISPLAY true
      #define HAS_NP true
      #define HAS_BLUETOOTH false
      #define HAS_BLE true
      #define HAS_WIFI true
      #define HAS_CONSOLE true
      #define HAS_EEPROM true
      #define HAS_BUSY true
      #define HAS_INPUT true
      #define HAS_TCXO true
      #define MODEM SX1262
      #define DIO2_AS_RF_SWITCH true
      #define HAS_RF_SWITCH_RX_TX false
      #define HAS_LORA_LNA true
      #define LORA_LNA_GAIN  30
      #define LORA_LNA_GVT   14
      #define HAS_BUZZER true
      #define HAS_ENCODER true
      #define HAS_VSENSE true
      #define PIN_VSENSE 6
      // R3 100k to VCC, R2 10k to GND: Vin = Vpin * (R2+R3)/R2 = Vpin * 11.0
      #define VSENSE_DIVIDER_RATIO_DEFAULT 11.0

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
      #define LORA_LNA_GAIN  30
      #define LORA_LNA_GVT   14

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
      #define LORA_LNA_GAIN  30
      #define LORA_LNA_GVT   14

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
        #define OCP_TUNED 0x28
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
      #define OCP_TUNED 0x28

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
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH false
      #define HAS_BLE true
      #define HAS_WIFI true
      #define HAS_PMU true
      #define HAS_CONSOLE true
      #define HAS_EEPROM true
      #define HAS_INPUT true
      #define HAS_SLEEP true
      #define HAS_LORA_PA true
      #define HAS_LORA_LNA true
      #define PIN_WAKEUP GPIO_NUM_0
      #define WAKEUP_LEVEL 0
      #define OCP_TUNED 0x28
      #define Vext GPIO_NUM_36
      #define LORA_PA_MODEL LORA_PA_UNKNOWN

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
      #define OCP_TUNED 0x28

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
      #define OCP_TUNED 0x18

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
      #define MODEM SX1262
      #define HAS_EEPROM false
      #define HAS_DISPLAY true
      #define HAS_BLUETOOTH false
      #define HAS_BLE true
      #define HAS_CONSOLE false
      #define HAS_PMU true
      #define HAS_NP true
      #define HAS_SD false
      #define HAS_TCXO true
      #define HAS_BUSY true
      #define HAS_INPUT true
      #define HAS_SLEEP true
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
      #define DISPLAY_SCALE 2
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
      #define MODEM SX1262
      #define HAS_EEPROM false
      #define HAS_DISPLAY true
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
      #define DIO2_AS_RF_SWITCH true
      #define CONFIG_UART_BUFFER_SIZE 6144
      #define CONFIG_QUEUE_SIZE 6144
      #define CONFIG_QUEUE_MAX_LENGTH 200
      #define EEPROM_SIZE 296
      #define EEPROM_OFFSET EEPROM_SIZE-EEPROM_RESERVED
      #define BLE_MANUFACTURER "Heltec"
      #define BLE_MODEL "T096"

      #define HAS_LORA_PA true
      #define HAS_LORA_LNA true
      #define OCP_TUNED 0x28
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

    #elif BOARD_MODEL == BOARD_PROMICRO
      //TODO:
      // - Fix low output power
      // - Make compatible with non-TCXO radios
      // - Add PMU
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
      #define HAS_RF_SWITCH_RX_TX true
      #define DIO2_AS_RF_SWITCH true
      #define OCP_TUNED 0x38
      #define HAS_SLEEP true
      #define BLE_MANUFACTURER "DIY"
      #define BLE_MODEL "ProMicro"

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
    #define SOUND_ENABLED_DEFAULT true
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

  // Default OCP value if not specified
  // in board configuration
  #ifndef OCP_TUNED
    #define OCP_TUNED 0x28
  #endif

  #ifndef NP_M
    #define NP_M 0.15
  #endif

#endif
