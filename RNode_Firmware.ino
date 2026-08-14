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

#include <Arduino.h>
#include <SPI.h>
#include "Utilities.h"

#if MCU_VARIANT == MCU_ESP32
  #include <esp_task_wdt.h>
  #include <esp_heap_caps.h>
  #include <esp_system.h>
  #include <mbedtls/platform.h>

  // Printed once at boot (see "RNode starting" below) so a reboot we didn't
  // witness live still leaves a record of whether it was a plain power-on,
  // a panic/abort, a watchdog trip, or something else, once the debug UART
  // capture picks back up on the new boot.
  const char* esp_reset_reason_str() {
    switch (esp_reset_reason()) {
      case ESP_RST_POWERON:   return "POWERON";
      case ESP_RST_EXT:       return "EXT";
      case ESP_RST_SW:        return "SW";
      case ESP_RST_PANIC:     return "PANIC";
      case ESP_RST_INT_WDT:   return "INT_WDT";
      case ESP_RST_TASK_WDT:  return "TASK_WDT";
      case ESP_RST_WDT:       return "WDT";
      case ESP_RST_DEEPSLEEP: return "DEEPSLEEP";
      case ESP_RST_BROWNOUT:  return "BROWNOUT";
      case ESP_RST_SDIO:      return "SDIO";
      default:                return "UNKNOWN";
    }
  }

  // Pinpointing the still-open loopTask task-watchdog stall (see
  // feedback_sx126x_tx_rx_spi_mutex_race.md / project_sx1262_tx_poll_yield_
  // fix.md) - a real, uncorrupted capture finally caught one, but the panic
  // dump only shows both cores already idle by the time it fires (loopTask
  // had silently blocked and never come back), not where. RTC_NOINIT_ATTR
  // survives the watchdog's own software reset (unlike regular RAM), so the
  // last checkpoint loopTask reached survives into the next boot, where it
  // gets printed once, right after the reset-reason line above - cheap
  // enough (two word writes, no serial I/O) to bracket every major loop()
  // section without perturbing the timing that's supposed to be measured.
  #define CP_MAGIC 0xC0FFEE42
  // volatile is load-bearing here, not defensive: cp_report_last()'s one
  // read happens (in this compilation's view of program order) before any
  // of loop()'s writes below it - the compiler has no way to know a reset
  // and reboot sits between them, so without volatile it's free to treat
  // every CP() write as a dead store never observed within this execution,
  // and reorder/coalesce/drop them. That's almost certainly why the first
  // real capture reported a checkpoint far older than the last heartbeat
  // before the crash - not a logic bug in CP() itself, a missing volatile.
  RTC_NOINIT_ATTR volatile uint32_t g_cp_magic;
  RTC_NOINIT_ATTR volatile uint16_t g_cp_id;
  RTC_NOINIT_ATTR volatile uint32_t g_cp_millis;

  enum {
    CP_NONE = 0,
    CP_LOOP_TOP,
    CP_URNS_RETICULUM_LOOP,
    CP_URNS_LXMF_LOOP,
    CP_MSNGR_PING,
    CP_MSNGR_SEND,
    CP_MSNGR_SEND_RESULT,
    CP_MSNGR_HEARTBEAT,
    CP_URNS_ANNOUNCE,
    CP_DIO0_PENDING,
    CP_MODEM_QUEUE_DRAIN,
    CP_TX_QUEUE_HANDLER,
    CP_CHECK_MODEM_STATUS,
    CP_LED_STANDBY,
    CP_SERIAL_BUFFER_POLL,
    CP_DISPLAY_UPDATE,
    CP_BUZZER_UPDATE,
    CP_PMU_UPDATE,
    CP_VSENSE_UPDATE,
    CP_BT_UPDATE,
    CP_WIFI_UPDATE,
    CP_OTA_LOOP,
    CP_ESPNOW_UPDATE,
    CP_GNSS_UPDATE,
    CP_INPUT_READ,
    CP_ENCODER_PROCESS,
    CP_MENU_PROCESS,
    CP_TXQ_FLUSH_QUEUE,
    CP_TXQ_POP_QUEUE,
  };

  const char* cp_name(uint16_t id) {
    switch (id) {
      case CP_NONE:                  return "NONE";
      case CP_LOOP_TOP:              return "loop_top";
      case CP_URNS_RETICULUM_LOOP:   return "urns_reticulum.loop()";
      case CP_URNS_LXMF_LOOP:        return "urns_lxmf_loop()";
      case CP_MSNGR_PING:            return "messenger_ping_process()";
      case CP_MSNGR_SEND:            return "messenger_send_process()";
      case CP_MSNGR_SEND_RESULT:     return "msngr_send_result_process()";
      case CP_MSNGR_HEARTBEAT:       return "messenger_heartbeat_process()";
      case CP_URNS_ANNOUNCE:         return "urns_announce()";
      case CP_DIO0_PENDING:          return "handleDio0IfPending()";
      case CP_MODEM_QUEUE_DRAIN:     return "modem_packet_queue drain";
      case CP_TX_QUEUE_HANDLER:      return "tx_queue_handler()";
      case CP_CHECK_MODEM_STATUS:    return "check_modem_status()";
      case CP_LED_STANDBY:           return "led_indicate_standby()/npset()";
      case CP_SERIAL_BUFFER_POLL:    return "buffer_serial()/serial_poll()";
      case CP_DISPLAY_UPDATE:        return "update_display()";
      case CP_BUZZER_UPDATE:         return "buzzer_update()";
      case CP_PMU_UPDATE:            return "update_pmu()";
      case CP_VSENSE_UPDATE:         return "update_vsense()";
      case CP_BT_UPDATE:             return "update_bt()";
      case CP_WIFI_UPDATE:           return "update_wifi()/update_ws()";
      case CP_OTA_LOOP:              return "ota_loop()";
      case CP_ESPNOW_UPDATE:         return "update_espnow()";
      case CP_GNSS_UPDATE:           return "gnss_update()";
      case CP_INPUT_READ:            return "input_read()";
      case CP_ENCODER_PROCESS:       return "encoder_process()";
      case CP_MENU_PROCESS:          return "menu_*_process()";
      case CP_TXQ_FLUSH_QUEUE:       return "tx_queue_handler->flush_queue()";
      case CP_TXQ_POP_QUEUE:         return "tx_queue_handler->pop_queue()";
      default:                       return "UNKNOWN";
    }
  }

  #define CP(x) do { g_cp_id = (x); g_cp_millis = millis(); } while (0)

  // Call once at boot, right after the reset-reason line - prints where
  // loopTask was about to go last time, iff this boot followed a reset type
  // that could plausibly be this stall (panic/watchdog), and the RTC_NOINIT
  // region actually holds a checkpoint from a real prior boot (not power-on
  // garbage, guarded by CP_MAGIC).
  void cp_report_last() {
    if (g_cp_magic != CP_MAGIC) {
      g_cp_magic = CP_MAGIC;
      g_cp_id = CP_NONE;
      g_cp_millis = 0;
      DEBUG_LOG("[Boot] no prior checkpoint (cold boot)\r\n");
      return;
    }
    DEBUG_LOG("[Boot] last checkpoint before this boot: %s (id=%u) at millis=%lu\r\n", cp_name(g_cp_id), g_cp_id, (unsigned long)g_cp_millis);
  }

  // mbedTLS (used for every RNS::Link decrypt) can fail its own small
  // internal allocations under internal-DIRAM pressure even when PSRAM is
  // merged into the general allocator - ESP-IDF's own "always internal
  // below this size" threshold keeps small allocations (mbedTLS's AES/
  // crypto contexts included) pinned to internal RAM regardless of PSRAM
  // headroom. Pointing mbedTLS's own allocator directly at PSRAM sidesteps
  // that threshold for exactly the allocations that were failing, without
  // needing to touch every other allocation site throughout the vendored
  // microReticulum/microLXMF stack.
  void* mbedtls_psram_calloc(size_t n, size_t size) {
    void* p = heap_caps_calloc(n, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!p) { p = heap_caps_calloc(n, size, MALLOC_CAP_DEFAULT); }
    return p;
  }
  void mbedtls_psram_free(void* p) { heap_caps_free(p); }
#else
  // CP() is sprinkled through loop() below regardless of MCU_VARIANT (most
  // of that code is shared across boards) - on non-ESP32 boards there's no
  // task watchdog stall to chase and no RTC_NOINIT_ATTR checkpoint state to
  // update, so it's just a no-op here.
  #define CP(x) do {} while (0)
#endif

#if MCU_VARIANT == MCU_ESP32 && HAS_URNS == true
  // See update_airtime()'s own comment (this file) for why this exists -
  // moves the one remaining, still-not-fully-explained hang (a Serial.
  // write() call that occasionally never returns, despite USBCDC::
  // write()'s own internal timeout logic) off loopTask entirely, so it
  // can only ever block itself, not the radio/LXMF path or the
  // watchdog's own reset call. This is purely a "did update_airtime() ask
  // for a report" flag, not a tight loop.
  volatile bool g_kiss_stats_pending = false;

  // housekeeping_task() - its own dedicated task, not folded onto loopTask.
  // It briefly was (matching microReticulum_Firmware's zero-extra-task
  // architecture, to recover DRAM for BLE), but that reintroduced a real,
  // confirmed hang on real hardware: the same "Serial.write() occasionally
  // never returns" USBCDC quirk this task was originally isolated to
  // protect against (see below) is real, not theoretical - moving it back
  // off loopTask is what actually fixes it, not a smaller DRAM budget
  // being worth reintroducing this hang for.
  unsigned long g_last_kiss_stats_check_ms = 0;
  void housekeeping_task(void *param) {
    for (;;) {
      char debug_buf[DEBUG_LOG_MSG_LEN];
      if (xQueueReceive(g_debug_log_queue, debug_buf, pdMS_TO_TICKS(50)) == pdTRUE) {
        Serial0.print(debug_buf);
      }
      char cmd_buf[CMD_LOG_MSG_LEN];
      if (xQueueReceive(g_cmd_log_queue, cmd_buf, 0) == pdTRUE) {
        serial_write(FEND);
        serial_write(CMD_LOG);
        size_t len = strnlen(cmd_buf, CMD_LOG_MSG_LEN);
        for (size_t i = 0; i < len; i++) { escaped_serial_write(cmd_buf[i]); }
        serial_write(FEND);
      }

      unsigned long now = millis();
      if (now - g_last_kiss_stats_check_ms >= 500) {
        g_last_kiss_stats_check_ms = now;
        if (g_kiss_stats_pending) {
          g_kiss_stats_pending = false;
          kiss_indicate_channel_stats();
        }
      }
    }
  }
#endif

FIFOBuffer serialFIFO;
uint8_t serialBuffer[CONFIG_UART_BUFFER_SIZE+1];

FIFOBuffer16 packet_starts;
uint16_t packet_starts_buf[CONFIG_QUEUE_MAX_LENGTH+1];

FIFOBuffer16 packet_lengths;
uint16_t packet_lengths_buf[CONFIG_QUEUE_MAX_LENGTH+1];

uint8_t packet_queue[CONFIG_QUEUE_SIZE];

volatile uint8_t queue_height = 0;
volatile uint16_t queued_bytes = 0;
volatile uint16_t queue_cursor = 0;
volatile uint16_t current_packet_start = 0;
volatile bool serial_buffering = false;
#if MCU_VARIANT == MCU_ESP32 || HAS_BLUETOOTH || HAS_BLE == true
  bool bt_init_ran = false;
#endif

#if HAS_CONSOLE
  #include "Console.h"
#endif

#if PLATFORM == PLATFORM_ESP32 || PLATFORM == PLATFORM_NRF52
  #define MODEM_QUEUE_SIZE 8
  typedef struct {
          size_t len;
          int rssi;
          int snr_raw;
          uint8_t data[];
  } modem_packet_t;
  static xQueueHandle modem_packet_queue = NULL;
#endif

#if MCU_VARIANT == MCU_ESP32 && HAS_URNS == true
  // kiss_tx_task() - back on its own dedicated task, not folded onto
  // loopTask (see housekeeping_task()'s own comment for why - the same
  // "Serial.write() occasionally never returns" hang applies here too, and
  // this is the highest-frequency Serial-writing call site in the firmware
  // (actual received-packet delivery over KISS), so if anything it's the
  // higher-risk of the two to leave on loopTask. Blocks indefinitely on the
  // queue instead of polling - unlike loop(), a dedicated task can afford
  // to block waiting for a packet that may never come.
  //
  // Deliberately does NOT also move urns_stage_incoming() here - that's a
  // plain bounded memcpy into a single-slot staging buffer, still done
  // synchronously on loopTask right where modem_packet is dequeued (this
  // file's loop()) before ownership is handed to g_kiss_tx_queue below; see
  // URNS.h's own comment on urns_stage_incoming for why that side stays on
  // loopTask. The two are independent - only the actual host write moves.
  QueueHandle_t g_kiss_tx_queue = NULL;
  void kiss_tx_task(void *param) {
    for (;;) {
    modem_packet_t *mp = NULL;
    if (xQueueReceive(g_kiss_tx_queue, &mp, portMAX_DELAY) == pdTRUE && mp) {
      uint8_t rssi_val = (uint8_t)(mp->rssi + rssi_offset);
      #if HAS_ESPNOW == true
        kiss_select_interface(0);
      #endif
      serial_write(FEND); serial_write(CMD_STAT_RSSI); escaped_serial_write(rssi_val); serial_write(FEND);
      #if HAS_ESPNOW == true
        kiss_select_interface(0);
      #endif
      serial_write(FEND); serial_write(CMD_STAT_SNR); escaped_serial_write((uint8_t)mp->snr_raw); serial_write(FEND);

      packet_rx_count++;
      serial_write(FEND);
      serial_write(CMD_DATA);
      for (uint16_t i = 0; i < mp->len; i++) {
        uint8_t byte = mp->data[i];
        if (byte == FEND) { serial_write(FESC); byte = TFEND; }
        if (byte == FESC) { serial_write(FESC); byte = TFESC; }
        serial_write(byte);
      }
      serial_write(FEND);
      #if HAS_BLE
        bt_flush();
      #endif

      free(mp);
      mp = NULL;
    }
    }
  }
#endif


char sbuf[128];

void setup() {
  #if MCU_VARIANT == MCU_ESP32 && HAS_URNS == true
    // The queues need to exist this early, before the very first
    // DEBUG_LOG() call anywhere below - see g_debug_log_queue/
    // g_cmd_log_queue (Utilities.h).
    // xQueueSend() doesn't need a consumer running yet; entries just sit
    // queued until housekeeping_task() (this file) starts draining them -
    // started right away below anyway, no real gap in practice.
    g_debug_log_queue = xQueueCreate(DEBUG_LOG_QUEUE_DEPTH, DEBUG_LOG_MSG_LEN);
    g_cmd_log_queue = xQueueCreate(CMD_LOG_QUEUE_DEPTH, CMD_LOG_MSG_LEN);
    // Stack measured empirically at ~2032B used (see memory) - sized with
    // headroom, not guessed from scratch. Internal-RAM stack, not PSRAM -
    // see bt_start()'s own ble_networking_conflict() comment: BLE and
    // WiFi/Ethernet are now mutually exclusive on HAS_URNS boards, so this
    // ~3KB no longer needs to compete with BLE's own margin, and there's
    // no need to take on PSRAM-backed-stack's open question (does it race
    // safely against flash writes on the other core?) for no remaining
    // benefit.
    xTaskCreatePinnedToCore(housekeeping_task, "housekeep", 3072, nullptr, 1, nullptr, 0);
  #endif
  #if HAS_OTA == true
    // Field recovery path for a bad OTA update - must run before anything
    // else in boot (radio/display/network init), see OTA.h.
    ota_check_recovery_button();
  #endif

  DEBUG_UART_BEGIN();
  DEBUG_LOG("RNode starting\r\n");

  #if MCU_VARIANT == MCU_ESP32
    DEBUG_LOG("[Boot] reset reason: %s\r\n", esp_reset_reason_str());
    cp_report_last();
  #endif

  #if MCU_VARIANT == MCU_ESP32 && HAS_URNS == true
    // See this file's own top-of-file comment (near mbedtls_psram_calloc)
    // for the full story - as early as possible, before any RNS::Link
    // decrypt could need it.
    mbedtls_platform_set_calloc_free(mbedtls_psram_calloc, mbedtls_psram_free);
  #endif

  #if MCU_VARIANT == MCU_ESP32 && HAS_URNS == true
    // Safety net for the still-open SX1262 SPI task-watchdog stall
    // (feedback_sx126x_tx_rx_spi_mutex_race memory) - without this, a
    // loop() stall just sits there silently (unresponsive to input, e.g.
    // the button no longer dismissing a menu popup) until it happens to
    // clear on its own, instead of getting cut short by a clean panic +
    // reboot. 25s is comfortably above LORA_MODEM_TIMEOUT_MS (20s,
    // sx126x.h) - the longest legitimate single blocking wait in the
    // normal TX path - so a real max-length TX timeout doesn't
    // false-trigger it.
    esp_task_wdt_config_t twdt_config = {
      .timeout_ms = 25000,
      .idle_core_mask = 0,
      .trigger_panic = true,
    };
    if (esp_task_wdt_init(&twdt_config) != ESP_OK) {
      esp_task_wdt_reconfigure(&twdt_config);
    }
    esp_task_wdt_add(NULL);
  #endif

  #if MCU_VARIANT == MCU_ESP32
    boot_seq();
    EEPROM.begin(EEPROM_SIZE);
    Serial.setRxBufferSize(CONFIG_UART_BUFFER_SIZE);

    #if BOARD_MODEL == BOARD_TDECK
      pinMode(pin_poweron, OUTPUT);
      digitalWrite(pin_poweron, HIGH);

      pinMode(SD_CS, OUTPUT);
      pinMode(DISPLAY_CS, OUTPUT);
      digitalWrite(SD_CS, HIGH);
      digitalWrite(DISPLAY_CS, HIGH);

      pinMode(DISPLAY_BL_PIN, OUTPUT);
    #endif
  #endif

  #if MCU_VARIANT == MCU_NRF52
    #if BOARD_MODEL == BOARD_TECHO
      delay(200);
      pinMode(PIN_VEXT_EN, OUTPUT);
      digitalWrite(PIN_VEXT_EN, HIGH);
      pinMode(pin_btn_usr1, INPUT_PULLUP);
      pinMode(pin_btn_touch, INPUT_PULLUP);
      pinMode(PIN_LED_RED, OUTPUT);
      pinMode(PIN_LED_GREEN, OUTPUT);
      pinMode(PIN_LED_BLUE, OUTPUT);
      delay(200);
    #endif
    #if BOARD_MODEL == BOARD_PROMICRO
      delay(200);
      pinMode(PIN_VEXT_EN, OUTPUT);
      digitalWrite(PIN_VEXT_EN, HIGH);
      delay(200);
    #endif

    if (!eeprom_begin()) { Serial.write("EEPROM initialisation failed.\r\n"); }
  #endif

  // Seed the PRNG for CSMA R-value selection
  #if MCU_VARIANT == MCU_ESP32
    // On ESP32, get the seed value from the
    // hardware RNG
    unsigned long seed_val = (unsigned long)esp_random();
  #elif MCU_VARIANT == MCU_NRF52
    // On nRF, get the seed value from the
    // hardware RNG
    unsigned long seed_val = get_rng_seed();
  #else
    // Otherwise, get a pseudo-random seed
    // value from an unconnected analog pin
    //
    // CAUTION! If you are implementing the
    // firmware on a platform that does not
    // have a hardware RNG, you MUST take
    // care to get a seed value with enough
    // entropy at each device reset!
    unsigned long seed_val = analogRead(0);
  #endif
  randomSeed(seed_val);

  // Initialise serial communication
  memset(serialBuffer, 0, sizeof(serialBuffer));
  fifo_init(&serialFIFO, serialBuffer, CONFIG_UART_BUFFER_SIZE);

  Serial.begin(serial_baudrate);

  #if HAS_NP
    led_init();
  #endif

  #if MCU_VARIANT == MCU_NRF52 && HAS_NP == true
    boot_seq();
  #endif

  #if BOARD_MODEL != BOARD_RAK4631 && BOARD_MODEL != BOARD_HELTEC_T114 && BOARD_MODEL != BOARD_HELTEC_T096 && BOARD_MODEL != BOARD_MESHPOE_S3 && BOARD_MODEL != BOARD_MESHADVENTURER_S3 && BOARD_MODEL != BOARD_PROMICRO && BOARD_MODEL != BOARD_AETHERNODE_S3 && BOARD_MODEL != BOARD_TECHO && BOARD_MODEL != BOARD_T3S3 && BOARD_MODEL != BOARD_TBEAM_S_V1 && BOARD_MODEL != BOARD_TBEAM_S_V3 && BOARD_MODEL != BOARD_HELTEC32_V4
    // Some boards need to wait until the hardware UART is set up before booting
    // the full firmware. In the case of the RAK4631 and Heltec T114, the line below will wait
    // until a serial connection is actually established with a master. Thus, it
    // is disabled on this platform.
    while (!Serial);
  #endif
  
  serial_interrupt_init();

  // Configure input and output pins
  #if HAS_INPUT
    input_init();
  #endif

  #if HAS_ENCODER == true
    encoder_init();
  #endif

  #if HAS_ENCODER == true
    #if HAS_GPIO_MENU == true
      // Must run before encoder_init(), which reads pin_encoder_up/down/
      // press to set up the actual hardware pins.
      #if HAS_EEPROM
        uint8_t eup_raw = EEPROM.read(eeprom_addr(ADDR_CONF_EUP));
        uint8_t edn_raw = EEPROM.read(eeprom_addr(ADDR_CONF_EDN));
        uint8_t epr_raw = EEPROM.read(eeprom_addr(ADDR_CONF_EPR));
      #elif MCU_VARIANT == MCU_NRF52
        uint8_t eup_raw = eeprom_read(eeprom_addr(ADDR_CONF_EUP));
        uint8_t edn_raw = eeprom_read(eeprom_addr(ADDR_CONF_EDN));
        uint8_t epr_raw = eeprom_read(eeprom_addr(ADDR_CONF_EPR));
      #endif
      // Only 0xFF (erased EEPROM) means "unset" here - unlike VSR/BVS's
      // ratio/percentage settings, 0x00 is a legitimate value (D0 is a
      // real candidate pin), so it must NOT be treated as a sentinel.
      if (eup_raw != 0xFF) {
        for (uint8_t i = 0; i < GPIO_FREE_PIN_CANDIDATE_COUNT; i++) {
          if (gpio_free_pin_candidates[i] == eup_raw) { pin_encoder_up = eup_raw; break; }
        }
      }
      if (edn_raw != 0xFF) {
        for (uint8_t i = 0; i < GPIO_FREE_PIN_CANDIDATE_COUNT; i++) {
          if (gpio_free_pin_candidates[i] == edn_raw) { pin_encoder_down = edn_raw; break; }
        }
      }
      if (epr_raw != 0xFF) {
        for (uint8_t i = 0; i < GPIO_FREE_PIN_CANDIDATE_COUNT; i++) {
          if (gpio_free_pin_candidates[i] == epr_raw) { pin_encoder_press = epr_raw; break; }
        }
      }
    #endif
    encoder_init();
  #endif

  #if HAS_NP == false
    pinMode(pin_led_rx, OUTPUT);
    pinMode(pin_led_tx, OUTPUT);
  #endif

  #if HAS_TCXO == true
    if (pin_tcxo_enable != -1) {
        pinMode(pin_tcxo_enable, OUTPUT);
        digitalWrite(pin_tcxo_enable, HIGH);
    }
  #endif

  // Initialise buffers
  memset(pbuf, 0, sizeof(pbuf));
  memset(cmdbuf, 0, sizeof(cmdbuf));
  
  memset(packet_queue, 0, sizeof(packet_queue));

  memset(packet_starts_buf, 0, sizeof(packet_starts_buf));
  fifo16_init(&packet_starts, packet_starts_buf, CONFIG_QUEUE_MAX_LENGTH);
  
  memset(packet_lengths_buf, 0, sizeof(packet_starts_buf));
  fifo16_init(&packet_lengths, packet_lengths_buf, CONFIG_QUEUE_MAX_LENGTH);

  #if PLATFORM == PLATFORM_ESP32 || PLATFORM == PLATFORM_NRF52
    modem_packet_queue = xQueueCreate(MODEM_QUEUE_SIZE, sizeof(modem_packet_t*));
  #endif
  #if MCU_VARIANT == MCU_ESP32 && HAS_URNS == true
    // See kiss_tx_task()'s own comment (this file) for why this exists.
    // Created before radio bring-up, well before any packet could arrive.
    g_kiss_tx_queue = xQueueCreate(MODEM_QUEUE_SIZE, sizeof(modem_packet_t*));
    // Stack measured empirically at ~1044B used (see memory) - sized with
    // headroom, not guessed from scratch. Internal-RAM stack - see
    // housekeeping_task's own task-creation comment (above, this file) for
    // why this isn't PSRAM-backed.
    xTaskCreatePinnedToCore(kiss_tx_task, "kisstx", 2048, nullptr, 1, nullptr, 0);
  #endif

  // Set chip select, reset and interrupt
  // pins for the LoRa module
  #if MODEM == SX1276 || MODEM == SX1278
  LoRa->setPins(pin_cs, pin_reset, pin_dio, pin_busy);
  #elif MODEM == SX1262
  LoRa->setPins(pin_cs, pin_reset, pin_dio, pin_busy, pin_rxen, pin_txen);
  #elif MODEM == SX1280
  LoRa->setPins(pin_cs, pin_reset, pin_dio, pin_busy, pin_rxen, pin_txen);
  #endif
  
  #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
    init_channel_stats();

    #if BOARD_MODEL == BOARD_T3S3
      #if MODEM == SX1280
        delay(300);
        LoRa->reset();
        delay(100);
      #endif
    #endif

    #if BOARD_MODEL == BOARD_XIAO_S3
      // Improve wakeup from sleep
      delay(300);
      LoRa->reset();
      delay(100);
    #endif

    // Check installed transceiver chip and
    // probe boot parameters.
    if (LoRa->preInit()) {
      modem_installed = true;
      
      #if HAS_INPUT
        // Skip quick-reset console activation
      #else
        uint32_t lfr = LoRa->getFrequency();
        if (lfr == 0) {
          // Normal boot
        } else if (lfr == M_FRQ_R) {
          // Quick reboot
          #if HAS_CONSOLE
            if (rtc_get_reset_reason(0) == POWERON_RESET) {
              console_active = true;
            }
          #endif
        } else {
          // Unknown boot
        }
        LoRa->setFrequency(M_FRQ_S);
      #endif

    } else {
      modem_installed = false;
    }
  #else
    // Older variants only came with SX1276/78 chips,
    // so assume that to be the case for now.
    modem_installed = true;
  #endif

  #if HAS_DISPLAY
    #if HAS_EEPROM
    if (EEPROM.read(eeprom_addr(ADDR_CONF_DSET)) != CONF_OK_BYTE) {
    #elif MCU_VARIANT == MCU_NRF52
    if (eeprom_read(eeprom_addr(ADDR_CONF_DSET)) != CONF_OK_BYTE) {
    #endif
      eeprom_update(eeprom_addr(ADDR_CONF_DSET), CONF_OK_BYTE);
      #if BOARD_MODEL == BOARD_TECHO
        eeprom_update(eeprom_addr(ADDR_CONF_DINT), 0x03);
      #else
        eeprom_update(eeprom_addr(ADDR_CONF_DINT), 0xFF);
      #endif
    }
    #if BOARD_MODEL == BOARD_TECHO
      display_add_callback(work_while_waiting);
    #endif

    display_unblank();
    disp_ready = display_init();
    update_display();
  #endif

  #if HAS_RTC == true
    rtc_init();
  #endif

  #if HAS_SENSORS == true
    sensors_init();
  #endif

  #if HAS_GPS == true
    // ADDR_CONF_GNSS is a raw physical byte, not offset via eeprom_addr() -
    // it resolves to a different (platform-appropriate) genuinely-free
    // address per MCU_VARIANT - see its own comment, ROM.h.
    #if HAS_EEPROM
      uint8_t gnss_raw = EEPROM.read(ADDR_CONF_GNSS);
    #elif MCU_VARIANT == MCU_NRF52
      uint8_t gnss_raw = eeprom_read(ADDR_CONF_GNSS);
    #endif
    // Explicit ON/OFF only ever get written as GNSS_ENABLE_BYTE/
    // GNSS_DISABLE_BYTE (see gnss_conf_save()) - any other value (erased
    // EEPROM reads 0xFF) means "never touched", so leave gnss_enabled at
    // its compiled default (GNSS.h, defaults true) instead of forcing it
    // either way.
    if (gnss_raw == GNSS_ENABLE_BYTE) gnss_enabled = true;
    else if (gnss_raw == GNSS_DISABLE_BYTE) gnss_enabled = false;
    gnss_init();
  #endif

  #if HAS_URNS == true
    // Raw physical byte, not through eeprom_addr() - see ADDR_CONF_URNS
    // (ROM.h), same convention as ADDR_CONF_GNSS just above. Explicit
    // ON/OFF only ever get written as URNS_ENABLE_BYTE/URNS_DISABLE_BYTE
    // (Menu.h) - any other value (erased EEPROM reads 0xFF) means "never
    // touched", so leave urns_enabled at its compiled default (true,
    // URNS.h) instead of forcing it either way.
    uint8_t urns_raw = EEPROM.read(ADDR_CONF_URNS);
    if (urns_raw == URNS_ENABLE_BYTE) urns_enabled = true;
    else if (urns_raw == URNS_DISABLE_BYTE) urns_enabled = false;

    // Same "never touched" convention as urns_raw above.
    uint8_t urns_transport_raw = EEPROM.read(ADDR_CONF_URNS_TRANSPORT);
    if (urns_transport_raw == URNS_TRANSPORT_ENABLE_BYTE) urns_transport_enabled = true;
    else if (urns_transport_raw == URNS_TRANSPORT_DISABLE_BYTE) urns_transport_enabled = false;

    // Same "never touched" convention as urns_raw above, for the three
    // uReticulum General Config toggles (URNS.h).
    uint8_t urns_link_mtu_raw = EEPROM.read(ADDR_CONF_URNS_LINK_MTU_DISCOVERY);
    if (urns_link_mtu_raw == URNS_LINK_MTU_DISCOVERY_ENABLE_BYTE) urns_link_mtu_discovery = true;
    else if (urns_link_mtu_raw == URNS_LINK_MTU_DISCOVERY_DISABLE_BYTE) urns_link_mtu_discovery = false;

    uint8_t urns_remote_mgmt_raw = EEPROM.read(ADDR_CONF_URNS_REMOTE_MGMT);
    if (urns_remote_mgmt_raw == URNS_REMOTE_MGMT_ENABLE_BYTE) urns_remote_management_enabled = true;
    else if (urns_remote_mgmt_raw == URNS_REMOTE_MGMT_DISABLE_BYTE) urns_remote_management_enabled = false;

    uint8_t urns_probe_dest_raw = EEPROM.read(ADDR_CONF_URNS_PROBE_DEST);
    if (urns_probe_dest_raw == URNS_PROBE_DEST_ENABLE_BYTE) urns_probe_destination_enabled = true;
    else if (urns_probe_dest_raw == URNS_PROBE_DEST_DISABLE_BYTE) urns_probe_destination_enabled = false;

    if (urns_enabled) {
      // Identity/persistence only - doesn't touch the radio, so it's fine
      // this early. urns_radio_bringup() is deferred to after
      // validate_status() below (hw_ready isn't actually set until then -
      // startRadio() would otherwise always hit its not-ready branch).
      urns_init();
      #if HAS_LXMF == true
        // Messenger app (Messenger.h) - bookmarks/message store/announce
        // handler. Only meaningful once urns_init() actually succeeded
        // (urns_ready) - a mount failure there leaves nothing for this to
        // build on.
        if (urns_ready) messenger_init();
      #endif
      // Provisioning.h - local KISS (CMD_PROVISION_REQ/RSP) + RNS-remote
      // (remote.management destination, already enabled above inside
      // urns_init()) config/management. Same urns_ready guard as
      // messenger_init() just above.
      if (urns_ready) provisioning_init();

      // Final authoritative safety check, run last and regardless of how
      // probe_destination_enabled ended up set - not just the EEPROM
      // read/Menu.h gate above. provisioning_init() (Provisioner::begin()
      // -> apply_loaded_to_runtime(), Provisioning.cpp) re-applies whatever
      // a Provisioning client (KISS or RNS-remote/"webconsole") previously
      // committed to its own separate storage file on the urns partition,
      // completely independent of and after our own EEPROM read above -
      // so a client can silently re-enable this via Provisioning even when
      // our own EEPROM/Menu.h says it should be off. Confirmed live on
      // hardware: Transport::start() (both this vendored port and upstream
      // Python RNS, ~/Development/Reticulum/RNS/Transport.py) only ever
      // constructs the actual probe-responder Destination inside its own
      // transport_enabled() branch - enabling this without Transport Mode
      // also on crashes (Interrupt WDT panic on Core 1).
      if (RNS::Reticulum::probe_destination_enabled() && !RNS::Reticulum::transport_enabled()) {
        RNS::Reticulum::probe_destination_enabled(false);
        urns_probe_destination_enabled = false;
      }
    }
  #endif

  #if HAS_BUZZER == true
    #if HAS_EEPROM
      uint8_t snd_raw = EEPROM.read(eeprom_addr(ADDR_CONF_SND));
    #elif MCU_VARIANT == MCU_NRF52
      uint8_t snd_raw = eeprom_read(eeprom_addr(ADDR_CONF_SND));
    #endif
    // Explicit ON/OFF only ever get written as SND_ENABLE_BYTE/
    // SND_DISABLE_BYTE (see the KISS handler and snd_conf_save()) - any
    // other value (erased EEPROM reads 0xFF, not 0x00) means "never
    // touched", so leave sound_enabled at its board default
    // (SOUND_ENABLED_DEFAULT, Boards.h) instead of forcing it either way.
    if (snd_raw == SND_ENABLE_BYTE) sound_enabled = true;
    else if (snd_raw == SND_DISABLE_BYTE) sound_enabled = false;
    #if HAS_GPIO_MENU == true
      // Must run before buzzer_init(), which reads buzzer_pin to set up
      // the actual hardware pin.
      #if HAS_EEPROM
        uint8_t buz_raw = EEPROM.read(eeprom_addr(ADDR_CONF_BUZ));
      #elif MCU_VARIANT == MCU_NRF52
        uint8_t buz_raw = eeprom_read(eeprom_addr(ADDR_CONF_BUZ));
      #endif
      // Only 0xFF (erased EEPROM) means "unset" here - see the equivalent
      // comment on the encoder-pin loads above, same reasoning (D0 is a
      // legitimate candidate pin, not a sentinel).
      if (buz_raw != 0xFF) {
        for (uint8_t i = 0; i < GPIO_FREE_PIN_CANDIDATE_COUNT; i++) {
          if (gpio_free_pin_candidates[i] == buz_raw) { buzzer_pin = buz_raw; break; }
        }
      }
    #endif
    buzzer_init();
    buzzer_boot_melody();
  #endif

  #if HAS_ENCODER == true
    // No board-specific default here (unlike Sound above) - an encoder is
    // never a guaranteed always-there feature on any board that has this
    // setting, so encoder_enabled's declared default (false) always
    // applies unless explicitly turned on via the menu.
    #if HAS_EEPROM
      uint8_t enc_en_raw = EEPROM.read(eeprom_addr(ADDR_CONF_ENA));
    #elif MCU_VARIANT == MCU_NRF52
      uint8_t enc_en_raw = eeprom_read(eeprom_addr(ADDR_CONF_ENA));
    #endif
    encoder_enabled = (enc_en_raw == ENC_ENABLE_BYTE);
  #endif

  #if HAS_VSENSE == true
    #if HAS_EEPROM
      uint8_t vsr_raw = EEPROM.read(eeprom_addr(ADDR_CONF_VSR));
    #elif MCU_VARIANT == MCU_NRF52
      uint8_t vsr_raw = eeprom_read(eeprom_addr(ADDR_CONF_VSR));
    #endif
    if (vsr_raw != 0x00 && vsr_raw != 0xFF) { vsense_divider_ratio = (float)vsr_raw / 10.0; }
  #endif

  #if HAS_BATTERY_DIVIDER == true
    #if HAS_EEPROM
      uint8_t bvs_raw = EEPROM.read(eeprom_addr(ADDR_CONF_BVS));
    #elif MCU_VARIANT == MCU_NRF52
      uint8_t bvs_raw = eeprom_read(eeprom_addr(ADDR_CONF_BVS));
    #endif
    if (bvs_raw != 0x00 && bvs_raw != 0xFF) { battery_v_scale = BATTERY_V_SCALE_DEFAULT * ((float)bvs_raw / 100.0); }
  #endif

  #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
    #if HAS_PMU == true || IS_ESP32S3
      pmu_ready = init_pmu();
    #endif

    #if MCU_VARIANT == MCU_ESP32 || HAS_BLUETOOTH || HAS_BLE == true
      bt_init();
      bt_init_ran = true;
    #endif

    if (console_active) {
      #if HAS_CONSOLE
        console_start();
      #else
        kiss_indicate_reset();
      #endif
    } else {
      #if HAS_ESPNOW == true
        // Read (not yet init) ahead of the HAS_WIFI block below - LR mode
        // must veto wifi_remote_init() entirely this boot (see that gate's
        // own comment for why WiFi-mode churn and ESP-NOW don't mix), so
        // espnow_enabled/espnow_lr_enabled need to be known before that
        // gate is evaluated, not after.
        uint8_t espnow_en_raw = EEPROM.read(eeprom_addr(ADDR_CONF_ESPNOW));
        // Explicit ON/OFF only ever get written as ESPNOW_ENABLE_BYTE/
        // ESPNOW_DISABLE_BYTE (see the KISS handler and espnow_conf_save())
        // - any other value (erased EEPROM reads 0xFF) means "never
        // touched", so leave espnow_enabled at its board default (off,
        // Config.h) instead of forcing it either way - same convention as
        // sound_enabled above.
        if (espnow_en_raw == ESPNOW_ENABLE_BYTE) espnow_enabled = true;
        else if (espnow_en_raw == ESPNOW_DISABLE_BYTE) espnow_enabled = false;

        // Raw physical byte, not through eeprom_addr() - see ADDR_CONF_ESPNOW_MODE
        // (ROM.h). Any value other than ESPNOW_MODE_V2 (including erased
        // EEPROM, 0xFF) means v1, matching ESPNOW_MODE_V1's 0x00 default.
        uint8_t espnow_mode_raw = EEPROM.read(ADDR_CONF_ESPNOW_MODE);
        espnow_mode = (espnow_mode_raw == ESPNOW_MODE_V2) ? ESPNOW_MODE_V2 : ESPNOW_MODE_V1;

        // Independent axis, own raw byte - see ADDR_CONF_ESPNOW_LR (ROM.h).
        // Any value other than ESPNOW_LR_ENABLE_BYTE (including erased
        // EEPROM) means off, matching espnow_lr_enabled's false default.
        uint8_t espnow_lr_raw = EEPROM.read(ADDR_CONF_ESPNOW_LR);
        espnow_lr_enabled = (espnow_lr_raw == ESPNOW_LR_ENABLE_BYTE);
      #endif

      #if HAS_WIFI
        wifi_mode = EEPROM.read(eeprom_addr(ADDR_CONF_WIFI));
        #if HAS_ESPNOW == true
          // LR mode needs the shared WiFi radio to itself (WIFI_PROTOCOL_LR,
          // ESPNOW.h) - never let a normal STA/AP association come up
          // alongside it. Independent of espnow_mode (v1/v2 framing) -
          // v2 alone doesn't touch the WiFi protocol bitmask at all, only
          // LR does. On WiFi-only boards (no HAS_ETHERNET) this means no
          // WiFi-remote host connection and no OTA reachability while LR
          // is active this boot - expected, not an error. espnow_wifi_disabled()
          // (ESPNOW.h) is the single source of truth for this veto - also
          // reused by Display.h's status icon so the two can't drift apart.
          bool espnow_lr_active = espnow_wifi_disabled();
        #else
          bool espnow_lr_active = false;
        #endif
        // Loaded here unconditionally, not left to wifi_remote_init() below,
        // because espnow_lr_active vetoes that call entirely - ESP-NOW still
        // needs wr_channel (espnow_init() locks its own channel to it) even
        // when WiFi's own STA/AP stack never comes up this boot. Without
        // this, LR mode left wr_channel stuck at its compiled-in default
        // (WR_CHANNEL_DEFAULT, Config.h) every boot, silently discarding
        // whatever channel was saved to EEPROM.
        wr_channel = EEPROM.read(eeprom_addr(ADDR_CONF_WCHN)); if (wr_channel < 1 || wr_channel > 14) { wr_channel = WR_CHANNEL_DEFAULT; }
        if (!espnow_lr_active && (wifi_mode == WR_WIFI_STA || wifi_mode == WR_WIFI_AP)) { wifi_remote_init(); }
      #endif
      #if HAS_ESPNOW == true
        if (espnow_enabled) espnow_init();
      #endif
      #if HAS_ETHERNET == true
        eth_speed_mode = EEPROM.read(eeprom_addr(ADDR_CONF_ETHSPD));
        if (eth_speed_mode > ETH_SPEED_OFF) eth_speed_mode = ETH_SPEED_AUTO; // erased EEPROM (0xFF) => default
        init_ethernet();
      #endif
      #if HAS_WIFI
        // Loaded/started here, after WiFi's own bring-up above and after
        // init_ethernet() just above, not inside the HAS_WIFI block up
        // there - ws_remote_init() (WebSocketRemote.h) needs lwIP's TCP/IP
        // task already running (same requirement, and same crash if it
        // isn't, as ota_server_init() below - see that call site's own
        // comment), which on a WiFi-off, Ethernet-equipped board only
        // happens once init_ethernet() has actually run. ws_remote_init()
        // itself still checks readiness (WebSocketRemote.h) since a WiFi-
        // only board reaches this same line with Ethernet never having
        // existed at all - this ordering just makes sure that check sees
        // accurate state instead of Ethernet's still-unset defaults.
        uint8_t ws_en_raw = EEPROM.read(eeprom_addr(ADDR_CONF_WS));
        // Only ever written as WS_ENABLE_BYTE/WS_DISABLE_BYTE (see the KISS
        // handler and ws_conf_save()) - any other value (erased EEPROM
        // reads 0xFF) means "never touched", so leave ws_enabled at its
        // default (on, Config.h - unlike espnow_en_raw above, this listener
        // needs to be reachable out of the box for browser-based tools).
        if (ws_en_raw == WS_ENABLE_BYTE) ws_enabled = true;
        else if (ws_en_raw == WS_DISABLE_BYTE) ws_enabled = false;
        // See ws_tx_flush()'s own comment (WebSocketRemote.h) for why this
        // exists - created before ws_remote_init() so the queue exists no
        // matter how soon a client could connect.
        #if MCU_VARIANT == MCU_ESP32
          g_ws_tx_queue = xQueueCreate(2, sizeof(ws_tx_item_t));
          xTaskCreatePinnedToCore(ws_tx_task, "wstx", 8192, nullptr, 1, nullptr, 0);
        #endif
        ws_remote_init();
      #endif
      #if HAS_OTA == true
        // WebServer::begin() (called from ota_server_init()) needs the
        // lwIP TCP/IP stack already brought up by something first -
        // WiFi.mode() (called by wifi_remote_init() in STA/AP mode, or by
        // espnow_init() regardless of wifi_mode) or Ethernet's own init
        // (above). If none of those actually ran this boot (WiFi Mode OFF
        // and ESP-NOW disabled/never initialized, and on HAS_ETHERNET
        // boards, Ethernet Speed also OFF/eth_disabled), nothing ever
        // brings up lwIP, and WebServer::begin() crashes hard instead of
        // just not being reachable - confirmed on real MeshAdventurer-S3
        // hardware: "assert failed: xQueueSemaphoreTake queue.c" inside
        // NetworkServer::begin(), a hard bootloop, not a graceful failure.
        // Skip OTA's web server entirely in that case - it wouldn't be
        // reachable over the network anyway with no interface up.
        #if HAS_ETHERNET == true
          if (WiFi.getMode() != WIFI_MODE_NULL || !eth_disabled) { ota_server_init(); }
        #else
          if (WiFi.getMode() != WIFI_MODE_NULL) { ota_server_init(); }
        #endif
      #endif
      kiss_indicate_reset();
    }
  #endif

  #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
    #if MODEM == SX1280
      avoid_interference = false;
    #else
      #if HAS_EEPROM
        uint8_t ia_conf = EEPROM.read(eeprom_addr(ADDR_CONF_DIA));
        if (ia_conf == 0x00) { avoid_interference = true; }
        else                 { avoid_interference = false; }
      #elif MCU_VARIANT == MCU_NRF52
        uint8_t ia_conf = eeprom_read(eeprom_addr(ADDR_CONF_DIA));
        if (ia_conf == 0x00) { avoid_interference = true; }
        else                 { avoid_interference = false; }
      #endif
    #endif
  #endif

  // Validate board health, EEPROM and config
  validate_status();

  if (op_mode != MODE_TNC) LoRa->setFrequency(0);

  // Not unconditional - validate_status() above has several failure
  // branches (invalid EEPROM checksum/config, unprovisioned device,
  // incorrect boot vector, ...) that set hw_ready = false but fall
  // through to here rather than hard_reset()'ing, so "ready" would be
  // actively misleading exactly when something's actually wrong.
  if (hw_ready) { DEBUG_LOG("RNode ready\r\n"); }

  #if HAS_URNS == true
    if (urns_enabled) {
      // Must come after both validate_status() (hw_ready isn't meaningful
      // before this) and the LoRa->setFrequency(0) idle-reset just above -
      // calling it any earlier would get immediately undone by that reset.
      urns_radio_bringup();
    }
  #endif
}

void lora_receive() {
  if (!implicit) {
    LoRa->receive();
  } else {
    LoRa->receive(implicit_l);
  }
}

inline void kiss_write_packet() {
  packet_rx_count++;

  #if HAS_URNS == true
    // Deliberately NOT calling urns_lora_interface.handle_incoming()
    // directly here anymore - this function runs deep inside the radio
    // driver's DIO0 interrupt handler, and handle_incoming() fans out
    // into the full Transport/LXMF/MessageStore stack, none of it safe to
    // run from interrupt context. See urns_stage_incoming()'s own comment
    // (URNS.h) for the real story - this just snapshots the bytes; the
    // actual processing happens later, from urns_lxmf_loop() in loop().
    if (urns_ready) { urns_stage_incoming(pbuf, host_write_len); }
  #endif

  serial_write(FEND);
  serial_write(CMD_DATA);
  
  for (uint16_t i = 0; i < host_write_len; i++) {
    #if MCU_VARIANT == MCU_NRF52
      portENTER_CRITICAL();
      uint8_t byte = pbuf[i];
      portEXIT_CRITICAL();
    #else
      uint8_t byte = pbuf[i];
    #endif

    if (byte == FEND) { serial_write(FESC); byte = TFEND; }
    if (byte == FESC) { serial_write(FESC); byte = TFESC; }
    serial_write(byte);
  }

  serial_write(FEND);
  host_write_len = 0;

  #if MCU_VARIANT == MCU_ESP32
    #if HAS_BLE
      bt_flush();
    #endif
  #endif
}

inline void getPacketData(uint16_t len) {
  #if MCU_VARIANT != MCU_NRF52
    while (len-- && read_len < MTU) {
      pbuf[read_len++] = LoRa->read();
    }  
  #else
    BaseType_t int_mask = taskENTER_CRITICAL_FROM_ISR();
    while (len-- && read_len < MTU) {
      pbuf[read_len++] = LoRa->read();
    }
    taskEXIT_CRITICAL_FROM_ISR(int_mask);
  #endif
}

void ISR_VECT receive_callback(int packet_size) {
  #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
    BaseType_t int_mask;
  #endif

  // Shared between both branches below (see markqvist/RNode_Firmware#108) -
  // promiscuous mode previously set a separate, never-read packet_ready
  // global instead of this flag, so its captured packets on ESP32/NRF52
  // were never actually enqueued to the host via modem_packet_queue.
  bool ready = false;

  if (!promisc) {
    // The standard operating mode allows large
    // packets with a payload up to 500 bytes,
    // by combining two raw LoRa packets.
    // We read the 1-byte header and extract
    // packet sequence number and split flags
    uint8_t header   = LoRa->read(); packet_size--;
    uint8_t sequence = packetSequence(header);

    if (isSplitPacket(header) && seq == SEQ_UNSET) {
      // This is the first part of a split
      // packet, so we set the seq variable
      // and add the data to the buffer
      #if MCU_VARIANT == MCU_NRF52
        int_mask = taskENTER_CRITICAL_FROM_ISR(); read_len = 0; taskEXIT_CRITICAL_FROM_ISR(int_mask);
      #else
        read_len = 0;
      #endif
      
      seq = sequence;

      #if MCU_VARIANT != MCU_ESP32 && MCU_VARIANT != MCU_NRF52
        last_rssi = LoRa->packetRssi();
        last_snr_raw = LoRa->packetSnrRaw();
      #endif

      getPacketData(packet_size);

    } else if (isSplitPacket(header) && seq == sequence) {
      // This is the second part of a split
      // packet, so we add it to the buffer
      // and set the ready flag.
      #if MCU_VARIANT != MCU_ESP32 && MCU_VARIANT != MCU_NRF52
        last_rssi = (last_rssi+LoRa->packetRssi())/2;
        last_snr_raw = (last_snr_raw+LoRa->packetSnrRaw())/2;
      #endif

      getPacketData(packet_size);
      seq = SEQ_UNSET;
      ready = true;

    } else if (isSplitPacket(header) && seq != sequence) {
      // This split packet does not carry the
      // same sequence id, so we must assume
      // that we are seeing the first part of
      // a new split packet.
      #if MCU_VARIANT == MCU_NRF52
        int_mask = taskENTER_CRITICAL_FROM_ISR(); read_len = 0; taskEXIT_CRITICAL_FROM_ISR(int_mask);
      #else
        read_len = 0;
      #endif
      seq = sequence;

      #if MCU_VARIANT != MCU_ESP32 && MCU_VARIANT != MCU_NRF52
        last_rssi = LoRa->packetRssi();
        last_snr_raw = LoRa->packetSnrRaw();
      #endif

      getPacketData(packet_size);

    } else if (!isSplitPacket(header)) {
      // This is not a split packet, so we
      // just read it and set the ready
      // flag to true.

      if (seq != SEQ_UNSET) {
        // If we already had part of a split
        // packet in the buffer, we clear it.
        #if MCU_VARIANT == MCU_NRF52
          int_mask = taskENTER_CRITICAL_FROM_ISR(); read_len = 0; taskEXIT_CRITICAL_FROM_ISR(int_mask);
        #else
          read_len = 0;
        #endif
        seq = SEQ_UNSET;
      }

      #if MCU_VARIANT != MCU_ESP32 && MCU_VARIANT != MCU_NRF52
        last_rssi = LoRa->packetRssi();
        last_snr_raw = LoRa->packetSnrRaw();
      #endif

      getPacketData(packet_size);
      ready = true;
    }

  } else {
    // In promiscuous mode, raw packets are
    // output directly to the host
    read_len = 0;

    #if MCU_VARIANT != MCU_ESP32 && MCU_VARIANT != MCU_NRF52
      last_rssi = LoRa->packetRssi();
      last_snr_raw = LoRa->packetSnrRaw();
      getPacketData(packet_size);

      // We first signal the RSSI of the
      // recieved packet to the host.
      kiss_indicate_stat_rssi();
      kiss_indicate_stat_snr();

      // And then write the entire packet
      kiss_write_packet();

    #else
      getPacketData(packet_size);
      ready = true;
    #endif
  }

  if (ready) {
    #if MCU_VARIANT != MCU_ESP32 && MCU_VARIANT != MCU_NRF52
      // We first signal the RSSI of the
      // recieved packet to the host.
      kiss_indicate_stat_rssi();
      kiss_indicate_stat_snr();

      // And then write the entire packet
      host_write_len = read_len;
      kiss_write_packet(); read_len = 0;

    #else
      // Allocate packet struct, but abort if there
      // is not enough memory available.
      modem_packet_t *modem_packet = (modem_packet_t*)malloc(sizeof(modem_packet_t) + read_len);
      if(!modem_packet) { memory_low = true; return; }

      // Get packet RSSI and SNR
      #if MCU_VARIANT == MCU_ESP32
        modem_packet->snr_raw = LoRa->packetSnrRaw();
        modem_packet->rssi = LoRa->packetRssi(modem_packet->snr_raw);
      #endif

      // Send packet to event queue, but free the
      // allocated memory again if the queue is
      // unable to receive the packet.
      modem_packet->len = read_len;
      memcpy(modem_packet->data, pbuf, read_len); read_len = 0;
      // receive_callback() (this function) is shared across every modem
      // driver's _onReceive target, not just sx126x - only sx126x.cpp's
      // own onDio0Rise() was changed to defer to task context
      // (handleDio0IfPending()); sx127x/sx128x still call their own
      // handleDioXRise() directly from true ISR context on every
      // platform, same as sx126x used to. So this must be keyed on the
      // actual modem in use, not just MCU_VARIANT - an ESP32 board using
      // SX1276/SX1280 still needs the ISR-safe xQueueSendFromISR() here.
      #if MCU_VARIANT == MCU_ESP32 && MODEM == SX1262
        bool queue_ok = modem_packet_queue && xQueueSend(modem_packet_queue, &modem_packet, 0) == pdPASS;
      #else
        bool queue_ok = modem_packet_queue && xQueueSendFromISR(modem_packet_queue, &modem_packet, NULL) == pdPASS;
      #endif
      if (!queue_ok) {
          free(modem_packet);
      }
    #endif
  }
}

#if HAS_URNS == true
void urns_sync_time_from_rtc() {
  #if HAS_RTC == true
    if (!rtc_time_valid()) {
      DEBUG_LOG("[URNS] time sync: RTC not present/valid, skipping\r\n");
      return;
    }
    uint64_t current_ltime_ms = RNS::Utilities::OS::ltime();
    uint64_t target_ltime_ms = (uint64_t)rtc_get_unixtime() * 1000ULL;
    // Adjust (not overwrite) the existing offset - ltime() already folds
    // in whatever offset Reticulum::start() set (its own +1ms monotonic
    // guard, RNS_USE_PERSISTED_TIME_OFFSET-style), so this lands exactly
    // on target_ltime_ms on the next ltime() call regardless of what that
    // was, including safely under uint64_t wraparound (modular arithmetic
    // still nets out correctly).
    uint64_t new_offset = RNS::Utilities::OS::getTimeOffset() + (target_ltime_ms - current_ltime_ms);
    RNS::Utilities::OS::setTimeOffset(new_offset);
    // Persist immediately rather than waiting for Reticulum::loop()'s own
    // periodic writeTimeOffset() (every 600s of uptime, Reticulum.cpp) -
    // without this, a device that's power-cycled inside that window never
    // gets a chance to save a real offset, so every single boot starts
    // Transport::start() (and so _path_store/_known_store's init()-time
    // sweep(), FileStore.h) with a bogus near-zero clock again. See the
    // readTimeOffset() "FIXED (local patch...)" comment (Reticulum.cpp)
    // for the other half of this bug (a units mismatch that discarded
    // any real persisted offset as "corrupt" even once one existed).
    RNS::Reticulum::writeTimeOffset();
    // microStore has its OWN independent clock (microStore::time(),
    // Utility.h) with its own offset variable, seeded ONCE from
    // RNS::Utilities::OS::getTimeOffset() when _path_store/_known_store's
    // init() ran (Transport.cpp/Identity.cpp, both inside Transport::
    // start() - i.e. before this RTC sync ever gets a chance to run) and
    // never touched again afterward. Without re-seeding it here too,
    // every path/announce record's stored timestamp (and every later
    // "now" microStore::time() call, e.g. is_ttl_expired() at compact()
    // time, FileStore.h) stays anchored to that stale pre-sync value
    // forever - both sides of the TTL comparison end up wrong by the
    // same amount, so is_ttl_expired() always says "not expired yet"
    // even for entries the UI's _expires field (built from the correctly-
    // synced RNS::Utilities::OS::time()) already shows as "Expired" -
    // confirmed live: compact() ran successfully but removed 0 of 9 path
    // entries despite some already showing Expired in the Path Table menu.
    microStore::set_time_offset(RNS::Utilities::OS::getTimeOffset() / 1000);
    DEBUG_LOG("[URNS] time sync: RTC unixtime=%lu, OS::time() now=%.0f\r\n",
      (unsigned long)rtc_get_unixtime(), RNS::Utilities::OS::time());
  #else
    DEBUG_LOG("[URNS] time sync: no RTC on this board, skipping\r\n");
  #endif
}

bool urns_enqueue_outgoing(const uint8_t* data, uint16_t len) {
  // A host KISS frame is currently being assembled byte-by-byte into this
  // same circular buffer (serial_callback(), driven from serial_poll() in
  // loop() - same thread, no ISR involved, but queue_cursor mid-frame
  // isn't a safe insertion point) - drop and let Reticulum re-announce/
  // retry later rather than corrupt it.
  //
  // Only an in-progress CMD_DATA frame actually touches packet_queue/
  // queue_cursor (serial_callback(), the `command == CMD_DATA` branch) -
  // every other KISS command (CMD_FREQUENCY, CMD_SYNC_WORD, etc.) buffers
  // into its own separate small buffer (e.g. cmdbuf) instead. IN_FRAME
  // itself, though, is plain "we're somewhere between two FENDs" and only
  // ever gets explicitly cleared by a CMD_DATA or CMD_SYNC_WORD frame's
  // closing FEND (serial_callback()) - closing any *other* command frame
  // falls through to the generic "a FEND either closes the previous frame
  // or opens the next one" branch, which sets IN_FRAME back to true
  // rather than false. In real usage that leaves IN_FRAME stuck true
  // (blocking every future call here) for as long as the host doesn't
  // happen to send another CMD_DATA/CMD_SYNC_WORD frame - previously rare
  // enough not to matter with only occasional manual test sends, but with
  // Messenger's real announce/message traffic this was observed to wedge
  // outgoing LXMF sends indefinitely (recoverable only by a reboot,
  // which resets IN_FRAME to its declared default). Checking `command`
  // too narrows this to the one case that's actually unsafe.
  // TEMPORARY instrumentation - see project_microreticulum_onboard_node memory.
  DEBUG_LOG("[URNS] enqueue: len=%u IN_FRAME=%d command=%d queue_height=%u queued_bytes=%u starts_full=%d\r\n",
    len, (int)IN_FRAME, (int)command, (unsigned)queue_height, (unsigned)queued_bytes, (int)fifo16_isfull(&packet_starts));
  if (IN_FRAME && command == CMD_DATA) return false;
  if (len < MIN_L || len > MTU) return false;
  if (fifo16_isfull(&packet_starts) || queue_height >= CONFIG_QUEUE_MAX_LENGTH) return false;
  if (queued_bytes + len > CONFIG_QUEUE_SIZE) return false;

  uint16_t start = queue_cursor;
  for (uint16_t i = 0; i < len; i++) {
    packet_queue[queue_cursor++] = data[i];
    if (queue_cursor == CONFIG_QUEUE_SIZE) queue_cursor = 0;
  }
  queued_bytes += len;
  queue_height++;
  fifo16_push(&packet_starts, start);
  fifo16_push(&packet_lengths, len);
  current_packet_start = queue_cursor;

  return true;
}

// Nothing in this codebase has ever needed a standalone default - lora_freq/
// lora_bw/lora_sf/lora_cr/lora_txp are normally only ever set by a host's
// CMD_FREQUENCY/CMD_BANDWIDTH/CMD_SF/CMD_CR/CMD_TXPOWER, and radio_locked
// (update_radio_lock()) stays true - so startRadio() silently refuses to
// run - until all four of the first five are set. Without this, the
// onboard node's queued packets (urns_enqueue_outgoing() above) just sit
// in packet_queue forever: tx_queue_handler() itself only runs inside
// loop()'s `if (radio_online)` branch. User-specified values, not a
// project default - real RF, real regulatory/interference stakes.
void urns_radio_bringup() {
  if (!urns_ready || radio_online) return;

  lora_freq = 868825000; // 868.825 MHz
  lora_bw   = 125000;    // 125 kHz
  lora_sf   = 10;
  lora_cr   = 7;
  lora_txp  = 17;         // dBm

  startRadio();
}
#endif

bool startRadio() {
  update_radio_lock();
  if (!radio_online && !console_active) {
    if (!radio_locked && hw_ready) {
      if (!LoRa->begin(lora_freq)) {
        // The radio could not be started.
        // Indicate this failure over both the
        // serial port and with the onboard LEDs
        radio_error = true;
        kiss_indicate_error(ERROR_INITRADIO);
        led_indicate_error(0);
        return false;
      } else {
        radio_online = true;
        DEBUG_LOG("[Radio] radio_online=true (startRadio, millis=%lu)\r\n", (unsigned long)millis());

        init_channel_stats();

        setTXPower();
        setBandwidth();
        setSpreadingFactor();
        setCodingRate();
        setSyncWord();
        getFrequency();

        LoRa->enableCrc();
        LoRa->onReceive(receive_callback);
        lora_receive();

        // Flash an info pattern to indicate
        // that the radio is now on
        kiss_indicate_radiostate();
        if (!LED_DISPLAY_BLANKED) {
          led_indicate_info(3);
        }
        return true;
      }

    } else {
      // Flash a warning pattern to indicate
      // that the radio was locked, and thus
      // not started
      radio_online = false;
      DEBUG_LOG("[Radio] radio_online=false (startRadio, locked, millis=%lu)\r\n", (unsigned long)millis());
      kiss_indicate_radiostate();
      led_indicate_warning(3);
      return false;
    }
  } else {
    // If radio is already on, we silently
    // ignore the request.
    kiss_indicate_radiostate();
    return true;
  }
}

void stopRadio() {
  LoRa->end();
  radio_online = false;
  DEBUG_LOG("[Radio] radio_online=false (stopRadio, millis=%lu)\r\n", (unsigned long)millis());
}

void update_radio_lock() {
  if (lora_freq != 0 && lora_bw != 0 && lora_txp != 0xFF && lora_sf != 0) {
    radio_locked = false;
  } else {
    radio_locked = true;
  }
}

bool queue_full() { return (queue_height >= CONFIG_QUEUE_MAX_LENGTH || queued_bytes >= CONFIG_QUEUE_SIZE); }

volatile bool queue_flushing = false;
void flush_queue(void) {
  if (!queue_flushing) {
    queue_flushing = true;
    if (!LED_DISPLAY_BLANKED) { led_tx_on(); }
    #if HAS_DISPLAY && BOARD_MODEL == BOARD_HELTEC_T096
      display_indicate_tx();
    #endif

    #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
    while (!fifo16_isempty(&packet_starts)) {
    #else
    while (!fifo16_isempty_locked(&packet_starts)) {
    #endif

      uint16_t start = fifo16_pop(&packet_starts);
      uint16_t length = fifo16_pop(&packet_lengths);

      if (length >= MIN_L && length <= MTU) {
        for (uint16_t i = 0; i < length; i++) {
          uint16_t pos = (start+i)%CONFIG_QUEUE_SIZE;
          tbuf[i] = packet_queue[pos];
        }

        transmit(length);
      }

      // Root-caused via CP()/checkpoint instrumentation (2026-08-14): this
      // loop drains the *entire* backlog in one call, transmitting each
      // queued packet in turn - the outer loop()'s single esp_task_wdt_
      // reset() call, once per top-level iteration, doesn't cover it. Under
      // heavy traffic queue_height can reach the high teens before a flush
      // is triggered; even without any single transmit() call hanging, that
      // many back-to-back TX cycles can cumulatively outrun the 25s task
      // watchdog budget, which then aborts loopTask entirely - confirmed by
      // two independent captures, both landing here with queue_height in
      // the 15-18 range right before the trigger. Feed it per packet so a
      // long-but-legitimate flush is never mistaken for a hang.
      #if MCU_VARIANT == MCU_ESP32 && HAS_URNS == true
        esp_task_wdt_reset();
      #endif
    }

    lora_receive(); if (!LED_DISPLAY_BLANKED) { led_tx_off(); }
  }

  queue_height = 0;
  queued_bytes = 0;

  #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
    update_airtime();
  #endif

  queue_flushing = false;

  #if HAS_DISPLAY && BOARD_MODEL != BOARD_HELTEC_T096
    // on the T096 this is handled by display_indicate_tx() pre-transmit
    display_tx = true;
  #endif
}

void pop_queue() {
  if (!queue_flushing) {
    queue_flushing = true;
    if (!LED_DISPLAY_BLANKED) { led_tx_on(); }
    #if HAS_DISPLAY && BOARD_MODEL == BOARD_HELTEC_T096
      display_indicate_tx();
    #endif

    #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
    if (!fifo16_isempty(&packet_starts)) {
    #else
    if (!fifo16_isempty_locked(&packet_starts)) {
    #endif

      uint16_t start = fifo16_pop(&packet_starts);
      uint16_t length = fifo16_pop(&packet_lengths);
      if (length >= MIN_L && length <= MTU) {
        for (uint16_t i = 0; i < length; i++) {
          uint16_t pos = (start+i)%CONFIG_QUEUE_SIZE;
          tbuf[i] = packet_queue[pos];
        }

        transmit(length);
      }
      queue_height -= 1;
      queued_bytes -= length;
    }

    lora_receive();
    if (!LED_DISPLAY_BLANKED) { led_tx_off(); }
  }

  #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
    update_airtime();
  #endif

  queue_flushing = false;

  #if HAS_DISPLAY && BOARD_MODEL != BOARD_HELTEC_T096
    // on the T096 this is handled by display_indicate_tx() pre-transmit
    display_tx = true;
  #endif
}

void add_airtime(uint16_t written) {
  #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
    float lora_symbols = 0;
    float packet_cost_ms = 0.0;
    int ldr_opt = 0; if (lora_low_datarate) ldr_opt = 1;

    #if MODEM == SX1276 || MODEM == SX1278
      lora_symbols += (8*written + PHY_CRC_LORA_BITS - 4*lora_sf + 8 + PHY_HEADER_LORA_SYMBOLS);
      lora_symbols /=                          4*(lora_sf-2*ldr_opt);
      lora_symbols *= lora_cr;
      lora_symbols += lora_preamble_symbols + 0.25 + 8;
      packet_cost_ms += lora_symbols * lora_symbol_time_ms;
      
    #elif MODEM == SX1262 || MODEM == SX1280
      if (lora_sf < 7) {
        lora_symbols += (8*written + PHY_CRC_LORA_BITS - 4*lora_sf + PHY_HEADER_LORA_SYMBOLS);
        lora_symbols /=                              4*lora_sf;
        lora_symbols *= lora_cr;
        lora_symbols += lora_preamble_symbols + 2.25 + 8;
        packet_cost_ms += lora_symbols * lora_symbol_time_ms;

      } else {
        lora_symbols += (8*written + PHY_CRC_LORA_BITS - 4*lora_sf + 8 + PHY_HEADER_LORA_SYMBOLS);
        lora_symbols /=                         4*(lora_sf-2*ldr_opt);
        lora_symbols *= lora_cr;
        lora_symbols += lora_preamble_symbols + 0.25 + 8;
        packet_cost_ms += lora_symbols * lora_symbol_time_ms;
      }
    
    #endif

    uint16_t cb = current_airtime_bin();
    uint16_t nb = cb+1; if (nb == AIRTIME_BINS) { nb = 0; }
    airtime_bins[cb] += packet_cost_ms;
    airtime_bins[nb] = 0;

  #endif
}

#if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
  // Explicit forward declaration: PlatformIO's Arduino prototype generator
  // (unlike arduino-cli's) misses functions defined inside a later #if block,
  // and update_csma_parameters() (defined further down) is called from
  // update_airtime() below.
  void update_csma_parameters();
#endif

void update_airtime() {
  #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
    uint16_t cb = current_airtime_bin();
    uint16_t pb = cb-1; if (cb-1 < 0) { pb = AIRTIME_BINS-1; }
    uint16_t nb = cb+1; if (nb == AIRTIME_BINS) { nb = 0; }
    airtime_bins[nb] = 0; airtime = (float)(airtime_bins[cb]+airtime_bins[pb])/(2.0*AIRTIME_BINLEN_MS);

    uint32_t longterm_airtime_sum = 0;
    for (uint16_t bin = 0; bin < AIRTIME_BINS; bin++) { longterm_airtime_sum += airtime_bins[bin]; }
    longterm_airtime = (float)longterm_airtime_sum/(float)AIRTIME_LONGTERM_MS;

    float longterm_channel_util_sum = 0.0;
    for (uint16_t bin = 0; bin < AIRTIME_BINS; bin++) { longterm_channel_util_sum += longterm_bins[bin]; }
    longterm_channel_util = (float)longterm_channel_util_sum/(float)AIRTIME_BINS;

    #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
      update_csma_parameters();
    #endif

    // Some Serial.write() (native USB CDC) calls can occasionally hang
    // despite USBCDC::write()'s own internal timeout logic looking sound
    // on paper - not just a call site under active BLE/WiFi/WS traffic;
    // seen on otherwise-idle runs too. The actual write happens in
    // housekeeping_task() (this file), on its own dedicated task,
    // specifically so a hang there can't take loopTask down with it.
    #if MCU_VARIANT == MCU_ESP32
      if (millis() > 10000) {
        g_kiss_stats_pending = true;
      }
    #else
      kiss_indicate_channel_stats();
    #endif
  #endif
}

void transmit(uint16_t size) {
  if (radio_online) {
    packet_tx_count++;

    if (!promisc) {
      uint16_t  written = 0;
      uint8_t header  = random(256) & 0xF0;
      if (size > SINGLE_MTU - HEADER_L) { header = header | FLAG_SPLIT; }

      LoRa->beginPacket();
      LoRa->write(header); written++;

      for (uint16_t i=0; i < size; i++) {
        LoRa->write(tbuf[i]); written++;

        if (written == 255 && isSplitPacket(header)) {
          if (!LoRa->endPacket()) {
            kiss_indicate_error(ERROR_MODEM_TIMEOUT);
            kiss_indicate_error(ERROR_TXFAILED);
            led_indicate_error(5);
            hard_reset();
          }

          add_airtime(written);
          LoRa->beginPacket();
          LoRa->write(header);
          written = 1;
        }
      }

      if (!LoRa->endPacket()) {
        kiss_indicate_error(ERROR_MODEM_TIMEOUT);
        kiss_indicate_error(ERROR_TXFAILED);
        led_indicate_error(5);
        hard_reset();
      }

      add_airtime(written);

    } else {
      if (!LED_DISPLAY_BLANKED) { led_tx_on(); } uint16_t written = 0;
      if (size > SINGLE_MTU) { size = SINGLE_MTU; }
      if (!implicit) { LoRa->beginPacket(); }
      else           { LoRa->beginPacket(size); }
      for (uint16_t i=0; i < size; i++) { LoRa->write(tbuf[i]); written++; }
      LoRa->endPacket(); add_airtime(written);
    }

  } else { kiss_indicate_error(ERROR_TXFAILED); led_indicate_error(5); }
}

void serial_callback(uint8_t sbyte) {
  if (IN_FRAME && sbyte == FEND && command == CMD_DATA) {
    IN_FRAME = false;

    #if HAS_ESPNOW == true
    if (selected_vport == 1) {
      if (espnow_tx_len > 0) { espnow_send(espnow_tx_buf, espnow_tx_len); }
      espnow_tx_len = 0;
    } else
    #endif
    if (!fifo16_isfull(&packet_starts) && queued_bytes < CONFIG_QUEUE_SIZE) {
        uint16_t s = current_packet_start;
        int16_t e = queue_cursor-1; if (e == -1) e = CONFIG_QUEUE_SIZE-1;
        uint16_t l;

        if (s != e) { l = (s < e) ? e - s + 1 : CONFIG_QUEUE_SIZE - s + e + 1; }
        else        { l = 1; }

        if (l >= MIN_L) {
            queue_height++;
            fifo16_push(&packet_starts, s);
            fifo16_push(&packet_lengths, l);
            current_packet_start = queue_cursor;
        }
    }

  } else if (IN_FRAME && sbyte == FEND && command == CMD_SYNC_WORD && frame_len > 0) {
    IN_FRAME = false;

    if (frame_len == 1 && cmdbuf[0] == 0xFF) {
      kiss_indicate_syncword();
    } else if (frame_len == 1) {
      lora_sw = cmdbuf[0];
      if (op_mode == MODE_HOST) setSyncWord();
      kiss_indicate_syncword();
    } else {
      // Two bytes: an already nibble-interleaved sync word register pair
      // (sx126x/sx128x form, e.g. 0x14/0x24 for logical sync word 0x12 -
      // see sx126x::setSyncWord()) sent directly by the host instead of
      // the raw logical byte. Decode back to the logical byte so lora_sw
      // stays in its one canonical raw-byte form and every driver's
      // existing setSyncWord(uint8_t) keeps working unchanged.
      lora_sw = (cmdbuf[0] & 0xF0) | ((cmdbuf[1] >> 4) & 0x0F);
      if (op_mode == MODE_HOST) setSyncWord();
      kiss_indicate_syncword();
    }

  #if HAS_URNS == true
  } else if (IN_FRAME && sbyte == FEND && command == CMD_PROVISION_REQ && frame_len > 0) {
    IN_FRAME = false;
    on_provision_request(prov_req_buf, frame_len);
  #endif

  } else if (sbyte == FEND) {
    IN_FRAME = true;
    command = CMD_UNKNOWN;
    frame_len = 0;
  } else if (IN_FRAME && frame_len < MTU) {
    // Have a look at the command byte first
    if (frame_len == 0 && command == CMD_UNKNOWN) {
        command = sbyte;
    } else if (command == CMD_DATA) {
        if (bt_state != BT_STATE_CONNECTED) {
          set_rns_link_state(RNS_LINK_STATE_CONNECTED);
        }
        #if HAS_ESPNOW == true
        if (selected_vport == 1) {
          if (sbyte == FESC) {
              ESCAPE = true;
          } else {
              if (ESCAPE) {
                  if (sbyte == TFEND) sbyte = FEND;
                  if (sbyte == TFESC) sbyte = FESC;
                  ESCAPE = false;
              }
              if (espnow_tx_len < ESPNOW_TX_BUF_SIZE) {
                espnow_tx_buf[espnow_tx_len++] = sbyte;
              }
          }
        } else
        #endif
        if (sbyte == FESC) {
            ESCAPE = true;
        } else {
            if (ESCAPE) {
                if (sbyte == TFEND) sbyte = FEND;
                if (sbyte == TFESC) sbyte = FESC;
                ESCAPE = false;
            }
            if (queue_height < CONFIG_QUEUE_MAX_LENGTH && queued_bytes < CONFIG_QUEUE_SIZE) {
              queued_bytes++;
              packet_queue[queue_cursor++] = sbyte;
              if (queue_cursor == CONFIG_QUEUE_SIZE) queue_cursor = 0;
            }
        }
    } else if (command == CMD_FREQUENCY) {
      if (sbyte == FESC) {
            ESCAPE = true;
        } else {
            if (ESCAPE) {
                if (sbyte == TFEND) sbyte = FEND;
                if (sbyte == TFESC) sbyte = FESC;
                ESCAPE = false;
            }
            if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
        }

        if (frame_len == 4) {
          uint32_t freq = (uint32_t)cmdbuf[0] << 24 | (uint32_t)cmdbuf[1] << 16 | (uint32_t)cmdbuf[2] << 8 | (uint32_t)cmdbuf[3];

          #if HAS_ESPNOW == true
          if (selected_vport == 1) {
            if (freq != 0) espnow_vport_cfg.frequency = freq;
            kiss_indicate_v1_frequency();
          } else
          #endif
          if (freq == 0) {
            kiss_indicate_frequency();
          } else {
            lora_freq = freq;
            if (op_mode == MODE_HOST) setFrequency();
            kiss_indicate_frequency();
          }
        }
    } else if (command == CMD_BANDWIDTH) {
      if (sbyte == FESC) {
            ESCAPE = true;
        } else {
            if (ESCAPE) {
                if (sbyte == TFEND) sbyte = FEND;
                if (sbyte == TFESC) sbyte = FESC;
                ESCAPE = false;
            }
            if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
        }

        if (frame_len == 4) {
          uint32_t bw = (uint32_t)cmdbuf[0] << 24 | (uint32_t)cmdbuf[1] << 16 | (uint32_t)cmdbuf[2] << 8 | (uint32_t)cmdbuf[3];

          #if HAS_ESPNOW == true
          if (selected_vport == 1) {
            if (bw != 0) espnow_vport_cfg.bandwidth = bw;
            kiss_indicate_v1_bandwidth();
          } else
          #endif
          if (bw == 0) {
            kiss_indicate_bandwidth();
          } else {
            lora_bw = bw;
            if (op_mode == MODE_HOST) setBandwidth();
            kiss_indicate_bandwidth();
          }
        }
    } else if (command == CMD_TIME) {
      #if HAS_RTC == true
        if (sbyte == FESC) {
              ESCAPE = true;
          } else {
              if (ESCAPE) {
                  if (sbyte == TFEND) sbyte = FEND;
                  if (sbyte == TFESC) sbyte = FESC;
                  ESCAPE = false;
              }
              if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
          }

          if (frame_len == 4) {
            uint32_t new_time = (uint32_t)cmdbuf[0] << 24 | (uint32_t)cmdbuf[1] << 16 | (uint32_t)cmdbuf[2] << 8 | (uint32_t)cmdbuf[3];

            if (new_time == 0) {
              kiss_indicate_time();
            } else {
              rtc_set_unixtime(new_time);
              kiss_indicate_time();
            }
          }
      #endif
    } else if (command == CMD_NTP_SYNC) {
      #if MCU_VARIANT == MCU_ESP32 && HAS_RTC == true && (HAS_WIFI == true || HAS_ETHERNET == true)
        if (sbyte == 0x01) {
          uint8_t status = rtc_sync_ntp();
          kiss_indicate_ntp_sync(status);
        }
      #endif
    } else if (command == CMD_SENSOR) {
      #if HAS_SENSORS == true
        kiss_indicate_sensor();
      #endif
    } else if (command == CMD_TXPOWER) {
      #if HAS_ESPNOW == true
      if (selected_vport == 1) {
        if (sbyte != 0xFF) espnow_vport_cfg.txpower = (int8_t)sbyte;
        kiss_indicate_v1_txpower();
      } else
      #endif
      if (sbyte == 0xFF) {
        kiss_indicate_txpower();
      } else {
        int txp = sbyte;
        #if MODEM == SX1262
          #if HAS_LORA_PA
            if (txp > PA_MAX_OUTPUT) txp = PA_MAX_OUTPUT;
          #else
            if (txp > 22) txp = 22;
          #endif
        #elif MODEM == SX1280
          #if HAS_PA
            if (txp > 20) txp = 20;
          #else
            if (txp > 13) txp = 13;
          #endif
        #else
          if (txp > 20) txp = 20;
        #endif

        lora_txp = txp;
        if (op_mode == MODE_HOST) setTXPower();
        kiss_indicate_txpower();
      }
    } else if (command == CMD_SF) {
      #if HAS_ESPNOW == true
      if (selected_vport == 1) {
        if (sbyte != 0xFF) espnow_vport_cfg.sf = sbyte;
        kiss_indicate_v1_sf();
      } else
      #endif
      if (sbyte == 0xFF) {
        kiss_indicate_spreadingfactor();
      } else {
        int sf = sbyte;
        if (sf < 5) sf = 5;
        if (sf > 12) sf = 12;

        lora_sf = sf;
        if (op_mode == MODE_HOST) setSpreadingFactor();
        kiss_indicate_spreadingfactor();
      }
    } else if (command == CMD_CR) {
      #if HAS_ESPNOW == true
      if (selected_vport == 1) {
        if (sbyte != 0xFF) espnow_vport_cfg.cr = sbyte;
        kiss_indicate_v1_cr();
      } else
      #endif
      if (sbyte == 0xFF) {
        kiss_indicate_codingrate();
      } else {
        int cr = sbyte;
        if (cr < 5) cr = 5;
        if (cr > 8) cr = 8;

        lora_cr = cr;
        if (op_mode == MODE_HOST) setCodingRate();
        kiss_indicate_codingrate();
      }
    } else if (command == CMD_SYNC_WORD) {
      if (sbyte == FESC) {
        ESCAPE = true;
      } else {
        if (ESCAPE) {
          if (sbyte == TFEND) sbyte = FEND;
          if (sbyte == TFESC) sbyte = FESC;
          ESCAPE = false;
        }
        if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
      }
    #if HAS_URNS == true
    } else if (command == CMD_PROVISION_REQ) {
      if (sbyte == FESC) {
        ESCAPE = true;
      } else {
        if (ESCAPE) {
          if (sbyte == TFEND) sbyte = FEND;
          if (sbyte == TFESC) sbyte = FESC;
          ESCAPE = false;
        }
        if (frame_len < MTU) prov_req_buf[frame_len++] = sbyte;
      }
    #endif
    } else if (command == CMD_IMPLICIT) {
      set_implicit_length(sbyte);
      kiss_indicate_implicit_length();
    } else if (command == CMD_LEAVE) {
      if (sbyte == 0xFF) {
        display_unblank();
        set_rns_link_state(RNS_LINK_STATE_DISCONNECTED);
        current_rssi  = -292;
        last_rssi     = -292;
        last_rssi_raw = 0x00;
        last_snr_raw  = 0x80;
      }
    } else if (command == CMD_RADIO_STATE) {
      if (bt_state != BT_STATE_CONNECTED) {
        set_rns_link_state(RNS_LINK_STATE_CONNECTED);
        display_unblank();
      }
      #if HAS_ESPNOW == true
      if (selected_vport == 1) {
        if (sbyte != 0xFF) espnow_vport_cfg.radio_state = sbyte;
        kiss_indicate_v1_radiostate();
      } else
      #endif
      if (sbyte == 0xFF) {
        kiss_indicate_radiostate();
      } else if (sbyte == 0x00) {
        stopRadio();
        kiss_indicate_radiostate();
      } else if (sbyte == 0x01) {
        startRadio();
        kiss_indicate_radiostate();
      }
    } else if (command == CMD_ST_ALOCK) {
      if (sbyte == FESC) {
            ESCAPE = true;
        } else {
            if (ESCAPE) {
                if (sbyte == TFEND) sbyte = FEND;
                if (sbyte == TFESC) sbyte = FESC;
                ESCAPE = false;
            }
            if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
        }

        if (frame_len == 2) {
          uint16_t at = (uint16_t)cmdbuf[0] << 8 | (uint16_t)cmdbuf[1];

          #if HAS_ESPNOW == true
          if (selected_vport == 1) {
            espnow_vport_cfg.st_alock = at;
            kiss_indicate_v1_st_alock();
          } else
          #endif
          {
            if (at == 0) {
              st_airtime_limit = 0.0;
            } else {
              st_airtime_limit = (float)at/(100.0*100.0);
              if (st_airtime_limit >= 1.0) { st_airtime_limit = 0.0; }
            }
            kiss_indicate_st_alock();
          }
        }
    } else if (command == CMD_LT_ALOCK) {
      if (sbyte == FESC) {
            ESCAPE = true;
        } else {
            if (ESCAPE) {
                if (sbyte == TFEND) sbyte = FEND;
                if (sbyte == TFESC) sbyte = FESC;
                ESCAPE = false;
            }
            if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
        }

        if (frame_len == 2) {
          uint16_t at = (uint16_t)cmdbuf[0] << 8 | (uint16_t)cmdbuf[1];

          #if HAS_ESPNOW == true
          if (selected_vport == 1) {
            espnow_vport_cfg.lt_alock = at;
            kiss_indicate_v1_lt_alock();
          } else
          #endif
          {
            if (at == 0) {
              lt_airtime_limit = 0.0;
            } else {
              lt_airtime_limit = (float)at/(100.0*100.0);
              if (lt_airtime_limit >= 1.0) { lt_airtime_limit = 0.0; }
            }
            kiss_indicate_lt_alock();
          }
        }
    } else if (command == CMD_STAT_RX) {
      kiss_indicate_stat_rx();
    } else if (command == CMD_STAT_TX) {
      kiss_indicate_stat_tx();
    } else if (command == CMD_STAT_RSSI) {
      kiss_indicate_stat_rssi();
    } else if (command == CMD_RADIO_LOCK) {
      update_radio_lock();
      kiss_indicate_radio_lock();
    } else if (command == CMD_BLINK && !LED_DISPLAY_BLANKED) {
      led_indicate_info(sbyte);
    } else if (command == CMD_RANDOM) {
      kiss_indicate_random(getRandom());
    } else if (command == CMD_DETECT) {
      if (sbyte == DETECT_REQ) {
        if (bt_state != BT_STATE_CONNECTED) set_rns_link_state(RNS_LINK_STATE_CONNECTED);
        kiss_indicate_detect();
      }
    #if HAS_ESPNOW == true
    } else if (command == CMD_SEL_INT) {
      selected_vport = (sbyte <= 1) ? sbyte : 0;
    } else if (command == CMD_INTERFACES) {
      kiss_indicate_interfaces();
    #endif
    } else if (command == CMD_PROMISC) {
      if (sbyte == 0x01) {
        promisc_enable();
      } else if (sbyte == 0x00) {
        promisc_disable();
      }
      kiss_indicate_promisc();
    } else if (command == CMD_READY) {
      if (!queue_full()) {
        kiss_indicate_ready();
      } else {
        kiss_indicate_not_ready();
      }
    } else if (command == CMD_UNLOCK_ROM) {
      if (sbyte == ROM_UNLOCK_BYTE) {
        unlock_rom();
      }
    } else if (command == CMD_RESET) {
      if (sbyte == CMD_RESET_BYTE) {
        hard_reset();
      }
    } else if (command == CMD_ROM_READ) {
      kiss_dump_eeprom();
    } else if (command == CMD_CFG_READ) {
      kiss_dump_config();
    } else if (command == CMD_ROM_WRITE) {
      if (sbyte == FESC) {
            ESCAPE = true;
        } else {
            if (ESCAPE) {
                if (sbyte == TFEND) sbyte = FEND;
                if (sbyte == TFESC) sbyte = FESC;
                ESCAPE = false;
            }
            if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
        }

        if (frame_len == 2) {
          eeprom_write(cmdbuf[0], cmdbuf[1]);
        }
    } else if (command == CMD_FW_VERSION) {
      kiss_indicate_version();
    } else if (command == CMD_PLATFORM) {
      kiss_indicate_platform();
    } else if (command == CMD_MCU) {
      kiss_indicate_mcu();
    } else if (command == CMD_BOARD) {
      kiss_indicate_board();
    } else if (command == CMD_CONF_SAVE) {
      eeprom_conf_save();
    } else if (command == CMD_CONF_DELETE) {
      eeprom_conf_delete();
    } else if (command == CMD_FB_EXT) {
      #if HAS_DISPLAY == true
        if (sbyte == 0xFF) {
          kiss_indicate_fbstate();
        } else if (sbyte == 0x00) {
          ext_fb_disable();
          kiss_indicate_fbstate();
        } else if (sbyte == 0x01) {
          ext_fb_enable();
          kiss_indicate_fbstate();
        }
      #endif
    } else if (command == CMD_FB_WRITE) {
      if (sbyte == FESC) {
            ESCAPE = true;
        } else {
            if (ESCAPE) {
                if (sbyte == TFEND) sbyte = FEND;
                if (sbyte == TFESC) sbyte = FESC;
                ESCAPE = false;
            }
            if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
        }
        #if HAS_DISPLAY
          if (frame_len == 9) {
            uint8_t line = cmdbuf[0];
            if (line > 63) line = 63;
            int fb_o = line*8; 
            memcpy(fb+fb_o, cmdbuf+1, 8);
          }
        #endif
    } else if (command == CMD_FB_READ) {
      if (sbyte != 0x00) { kiss_indicate_fb(); }
    } else if (command == CMD_DISP_READ) {
      if (sbyte != 0x00) { kiss_indicate_disp(); }
    } else if (command == CMD_DEV_HASH) {
      #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
        if (sbyte != 0x00) {
          kiss_indicate_device_hash();
        }
      #endif
    } else if (command == CMD_DEV_SIG) {
      #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
        if (sbyte == FESC) {
              ESCAPE = true;
          } else {
              if (ESCAPE) {
                  if (sbyte == TFEND) sbyte = FEND;
                  if (sbyte == TFESC) sbyte = FESC;
                  ESCAPE = false;
              }
              if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
          }

          if (frame_len == DEV_SIG_LEN) {
            memcpy(dev_sig, cmdbuf, DEV_SIG_LEN);
            device_save_signature();
          }
      #endif
    } else if (command == CMD_FW_UPD) {
      if (sbyte == 0x01) {
        firmware_update_mode = true;
      } else {
        firmware_update_mode = false;
      }
    } else if (command == CMD_HASHES) {
      #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
        if (sbyte == 0x01) {
          kiss_indicate_target_fw_hash();
        } else if (sbyte == 0x02) {
          kiss_indicate_fw_hash();
        } else if (sbyte == 0x03) {
          kiss_indicate_bootloader_hash();
        } else if (sbyte == 0x04) {
          kiss_indicate_partition_table_hash();
        }
      #endif
    } else if (command == CMD_FW_HASH) {
      #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
        if (sbyte == FESC) {
              ESCAPE = true;
          } else {
              if (ESCAPE) {
                  if (sbyte == TFEND) sbyte = FEND;
                  if (sbyte == TFESC) sbyte = FESC;
                  ESCAPE = false;
              }
              if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
          }

          if (frame_len == DEV_HASH_LEN) {
            memcpy(dev_firmware_hash_target, cmdbuf, DEV_HASH_LEN);
            device_save_firmware_hash();
          }
      #endif
    } else if (command == CMD_WIFI_CHN) {
      #if HAS_WIFI
        if (sbyte > 0 && sbyte < 14) { eeprom_update(eeprom_addr(ADDR_CONF_WCHN), sbyte); }
      #endif
    } else if (command == CMD_WIFI_MODE) {
      #if HAS_WIFI
        if (sbyte == WR_WIFI_OFF || sbyte == WR_WIFI_STA || sbyte == WR_WIFI_AP) {
          wr_conf_save(sbyte);
          wifi_mode = sbyte;
          wifi_remote_init();
        }
      #endif
    } else if (command == CMD_SND) {
      #if HAS_BUZZER == true
        if (sbyte == SND_ENABLE_BYTE || sbyte == SND_DISABLE_BYTE) {
          snd_conf_save(sbyte == SND_ENABLE_BYTE);
        }
      #endif
    } else if (command == CMD_ESPNOW_ENABLE) {
      #if HAS_ESPNOW == true
        if (sbyte == ESPNOW_ENABLE_BYTE || sbyte == ESPNOW_DISABLE_BYTE) {
          espnow_conf_save(sbyte);
        }
      #endif
    } else if (command == CMD_WS_ENABLE) {
      #if HAS_WIFI
        if (sbyte == WS_ENABLE_BYTE || sbyte == WS_DISABLE_BYTE) {
          ws_conf_save(sbyte);
        }
      #endif
    } else if (command == CMD_VSENSE_DIV) {
      #if HAS_VSENSE == true
        if (sbyte == 0xFF) {
          kiss_indicate_vsense_div();
        } else {
          vsr_conf_save(sbyte);
          kiss_indicate_vsense_div();
        }
      #endif
    } else if (command == CMD_WIFI_SSID) {
      #if HAS_WIFI
        if (sbyte == FESC) { ESCAPE = true; }
        else {
          if (ESCAPE) {
            if (sbyte == TFEND) sbyte = FEND;
            if (sbyte == TFESC) sbyte = FESC;
            ESCAPE = false;
          }
          if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
        }

        if (sbyte == 0x00) {
          for (uint8_t i = 0; i<33; i++) {
            if (i<frame_len && i<32) { eeprom_update(config_addr(ADDR_CONF_SSID+i), cmdbuf[i]); }
            else                     { eeprom_update(config_addr(ADDR_CONF_SSID+i), 0x00); }
          }
        }
      #endif
    } else if (command == CMD_WIFI_PSK) {
      #if HAS_WIFI
        if (sbyte == FESC) { ESCAPE = true; }
        else {
          if (ESCAPE) {
            if (sbyte == TFEND) sbyte = FEND;
            if (sbyte == TFESC) sbyte = FESC;
            ESCAPE = false;
          }
          if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
        }

        if (sbyte == 0x00) {
          for (uint8_t i = 0; i<33; i++) {
            if (i<frame_len && i<32) { eeprom_update(config_addr(ADDR_CONF_PSK+i), cmdbuf[i]); }
            else                     { eeprom_update(config_addr(ADDR_CONF_PSK+i), 0x00); }
          }
        }
      #endif
    } else if (command == CMD_WIFI_IP) {
      #if HAS_WIFI
        if (sbyte == FESC) { ESCAPE = true; }
        else {
          if (ESCAPE) {
            if (sbyte == TFEND) sbyte = FEND;
            if (sbyte == TFESC) sbyte = FESC;
            ESCAPE = false;
          }
          if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
        }

        if (frame_len == 4) { for (uint8_t i = 0; i<4; i++) { eeprom_update(config_addr(ADDR_CONF_IP+i), cmdbuf[i]); } }
      #endif
    } else if (command == CMD_WIFI_NM) {
      #if HAS_WIFI
        if (sbyte == FESC) { ESCAPE = true; }
        else {
          if (ESCAPE) {
            if (sbyte == TFEND) sbyte = FEND;
            if (sbyte == TFESC) sbyte = FESC;
            ESCAPE = false;
          }
          if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
        }

        if (frame_len == 4) { for (uint8_t i = 0; i<4; i++) { eeprom_update(config_addr(ADDR_CONF_NM+i), cmdbuf[i]); } }
      #endif
    } else if (command == CMD_WIFI_GW) {
      #if HAS_WIFI
        if (sbyte == FESC) { ESCAPE = true; }
        else {
          if (ESCAPE) {
            if (sbyte == TFEND) sbyte = FEND;
            if (sbyte == TFESC) sbyte = FESC;
            ESCAPE = false;
          }
          if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
        }

        if (frame_len == 4) { for (uint8_t i = 0; i<4; i++) { eeprom_update(config_addr(ADDR_CONF_GW+i), cmdbuf[i]); } }
      #endif
    } else if (command == CMD_WIFI_DNS) {
      #if HAS_WIFI
        if (sbyte == FESC) { ESCAPE = true; }
        else {
          if (ESCAPE) {
            if (sbyte == TFEND) sbyte = FEND;
            if (sbyte == TFESC) sbyte = FESC;
            ESCAPE = false;
          }
          if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
        }

        if (frame_len == 4) { for (uint8_t i = 0; i<4; i++) { eeprom_update(config_addr(ADDR_CONF_DNS+i), cmdbuf[i]); } }
      #endif
    } else if (command == CMD_ETH_IP) {
      #if HAS_ETHERNET == true
        if (sbyte == FESC) { ESCAPE = true; }
        else {
          if (ESCAPE) {
            if (sbyte == TFEND) sbyte = FEND;
            if (sbyte == TFESC) sbyte = FESC;
            ESCAPE = false;
          }
          if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
        }

        if (frame_len == 4) { for (uint8_t i = 0; i<4; i++) { eeprom_update(config_addr(ADDR_CONF_ETH_IP+i), cmdbuf[i]); } }
      #endif
    } else if (command == CMD_ETH_NM) {
      #if HAS_ETHERNET == true
        if (sbyte == FESC) { ESCAPE = true; }
        else {
          if (ESCAPE) {
            if (sbyte == TFEND) sbyte = FEND;
            if (sbyte == TFESC) sbyte = FESC;
            ESCAPE = false;
          }
          if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
        }

        if (frame_len == 4) { for (uint8_t i = 0; i<4; i++) { eeprom_update(config_addr(ADDR_CONF_ETH_NM+i), cmdbuf[i]); } }
      #endif
    } else if (command == CMD_ETH_GW) {
      #if HAS_ETHERNET == true
        if (sbyte == FESC) { ESCAPE = true; }
        else {
          if (ESCAPE) {
            if (sbyte == TFEND) sbyte = FEND;
            if (sbyte == TFESC) sbyte = FESC;
            ESCAPE = false;
          }
          if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
        }

        if (frame_len == 4) { for (uint8_t i = 0; i<4; i++) { eeprom_update(config_addr(ADDR_CONF_ETH_GW+i), cmdbuf[i]); } }
      #endif
    } else if (command == CMD_ETH_DNS) {
      #if HAS_ETHERNET == true
        if (sbyte == FESC) { ESCAPE = true; }
        else {
          if (ESCAPE) {
            if (sbyte == TFEND) sbyte = FEND;
            if (sbyte == TFESC) sbyte = FESC;
            ESCAPE = false;
          }
          if (frame_len < CMD_L) cmdbuf[frame_len++] = sbyte;
        }

        if (frame_len == 4) { for (uint8_t i = 0; i<4; i++) { eeprom_update(config_addr(ADDR_CONF_ETH_DNS+i), cmdbuf[i]); } }
      #endif
    } else if (command == CMD_ETH_SPEED) {
      #if HAS_ETHERNET == true
        if (sbyte <= ETH_SPEED_OFF) { ethspd_conf_save(sbyte); }
      #endif
    } else if (command == CMD_BT_CTRL) {
      #if HAS_BLUETOOTH || HAS_BLE
        if (sbyte == 0x00) {
          bt_stop();
          bt_conf_save(false);
        } else if (sbyte == 0x01) {
          bt_start();
          bt_conf_save(true);
        } else if (sbyte == 0x02) {
          if (bt_state == BT_STATE_OFF) {
            bt_start();
            bt_conf_save(true);
          }
          if (bt_state != BT_STATE_CONNECTED) {
            bt_enable_pairing();
          }
        }
      #endif
    } else if (command == CMD_BT_UNPAIR) {
      #if HAS_BLE
        if (sbyte == 0x01) { bt_debond_all(); }
      #endif
    } else if (command == CMD_DISP_INT) {
      #if HAS_DISPLAY
        if (sbyte == FESC) {
            ESCAPE = true;
        } else {
            if (ESCAPE) {
                if (sbyte == TFEND) sbyte = FEND;
                if (sbyte == TFESC) sbyte = FESC;
                ESCAPE = false;
            }
            display_intensity = sbyte;
            di_conf_save(display_intensity);
            display_unblank();
        }
      #endif
    } else if (command == CMD_DISP_ADDR) {
      #if HAS_DISPLAY
        if (sbyte == FESC) {
            ESCAPE = true;
        } else {
            if (ESCAPE) {
                if (sbyte == TFEND) sbyte = FEND;
                if (sbyte == TFESC) sbyte = FESC;
                ESCAPE = false;
            }
            display_addr = sbyte;
            da_conf_save(display_addr);
        }

      #endif
    } else if (command == CMD_DISP_BLNK) {
      #if HAS_DISPLAY
        if (sbyte == FESC) {
            ESCAPE = true;
        } else {
            if (ESCAPE) {
                if (sbyte == TFEND) sbyte = FEND;
                if (sbyte == TFESC) sbyte = FESC;
                ESCAPE = false;
            }
            db_conf_save(sbyte);
            display_unblank();
        }
      #endif
    } else if (command == CMD_DISP_ROT) {
      #if HAS_DISPLAY
        if (sbyte == FESC) {
            ESCAPE = true;
        } else {
            if (ESCAPE) {
                if (sbyte == TFEND) sbyte = FEND;
                if (sbyte == TFESC) sbyte = FESC;
                ESCAPE = false;
            }
            drot_conf_save(sbyte);
            display_unblank();
        }
      #endif
    } else if (command == CMD_DIS_IA) {
      if (sbyte == FESC) {
          ESCAPE = true;
      } else {
          if (ESCAPE) {
              if (sbyte == TFEND) sbyte = FEND;
              if (sbyte == TFESC) sbyte = FESC;
              ESCAPE = false;
          }
          dia_conf_save(sbyte);
      }
    } else if (command == CMD_DISP_RCND) {
      #if HAS_DISPLAY
        if (sbyte == FESC) {
            ESCAPE = true;
        } else {
            if (ESCAPE) {
                if (sbyte == TFEND) sbyte = FEND;
                if (sbyte == TFESC) sbyte = FESC;
                ESCAPE = false;
            }
            if (sbyte > 0x00) recondition_display = true;
        }
      #endif
    } else if (command == CMD_NP_INT) {
      #if HAS_NP
        if (sbyte == FESC) {
            ESCAPE = true;
        } else {
            if (ESCAPE) {
                if (sbyte == TFEND) sbyte = FEND;
                if (sbyte == TFESC) sbyte = FESC;
                ESCAPE = false;
            }
            sbyte;
            led_set_intensity(sbyte);
            np_int_conf_save(sbyte);
        }

      #endif
    }
  }
}

#if MCU_VARIANT == MCU_ESP32
  portMUX_TYPE update_lock = portMUX_INITIALIZER_UNLOCKED;
#endif

bool medium_free() {
  update_modem_status();
  if (avoid_interference && interference_detected) { return false; }
  return !dcd;
}

bool noise_floor_sampled = false;
int  noise_floor_sample  = 0;
int  noise_floor_buffer[NOISE_FLOOR_SAMPLES] = {0};
void update_noise_floor() {
  #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
    if (!dcd) {
      #if BOARD_MODEL != BOARD_HELTEC32_V4
      if (!noise_floor_sampled || current_rssi < noise_floor + CSMA_INFR_THRESHOLD_DB) {
      #else
      if ((!noise_floor_sampled || current_rssi < noise_floor + CSMA_INFR_THRESHOLD_DB) || (noise_floor_sampled && (noise_floor < LNA_GD_THRSHLD && current_rssi <= LNA_GD_LIMIT))) {
      #endif
        #if HAS_LORA_LNA
          // Discard invalid samples due to gain variance
          // during LoRa LNA re-calibration
          if (current_rssi < noise_floor-LORA_LNA_GVT) { return; }
        #endif
        bool sum_noise_floor = false;
        noise_floor_buffer[noise_floor_sample] = current_rssi;
        noise_floor_sample = noise_floor_sample+1;
        if (noise_floor_sample >= NOISE_FLOOR_SAMPLES) {
          noise_floor_sample %= NOISE_FLOOR_SAMPLES;
          noise_floor_sampled = true;
          sum_noise_floor = true;
        }

        if (noise_floor_sampled && sum_noise_floor) {
          noise_floor = 0;
          for (int ni = 0; ni < NOISE_FLOOR_SAMPLES; ni++) { noise_floor += noise_floor_buffer[ni]; }
          noise_floor /= NOISE_FLOOR_SAMPLES;
        }
      }
    }
  #endif
}

#define LED_ID_TRIG 16
uint8_t led_id_filter = 0;
uint32_t interference_start = 0;
bool interference_persists = false;
void update_modem_status() {
  // dcd()/currentRssi() do raw SPI against the radio, same as beginPacket()/
  // endPacket() (sx126x.cpp) - see maskDio0()'s own comment there for the
  // DIO0-vs-SPI-mutex race this masking addresses. That's a separate
  // concern from the *real* bug found live tonight: these calls used to
  // run *inside* the portENTER_CRITICAL()/portEXIT_CRITICAL() section
  // below (pre-existing code, not something added this session) - and
  // Arduino-ESP32's SPI.beginTransaction() (called deep inside dcd()/
  // currentRssi()) blocks on its own internal semaphore. portENTER_
  // CRITICAL() disables interrupts on this core for its entire duration,
  // including the FreeRTOS tick that drives all task scheduling/wake-ups
  // - so if that SPI semaphore ever isn't immediately free, this blocks
  // waiting for it while holding a lock that has disabled the one thing
  // (the tick) needed for anything, including whoever holds that
  // semaphore, to ever make progress and release it. A genuine deadlock,
  // with interrupts off on this core - confirmed live via a task-state
  // stall monitor that itself never got a chance to run (its own
  // vTaskDelay() needs the same disabled tick), and a watchdog panic that
  // still fired (it uses a lower-level timer, not the tick) with both
  // CPUs reported idle - exactly what "the tick stopped, nothing can be
  // scheduled" looks like. This is almost certainly the actual root cause
  // behind tonight's whole "stuck at a different, unrelated call site
  // every time" pattern, once heap pressure (a real, separate, already-
  // fixed issue) is not around to explain it - once this state hits,
  // *everything* halts wherever it happened to be, so whatever the loop
  // checkpoint says is just whatever ran last before the freeze, not
  // where the actual bug is.
  //
  // Fix: never call a blocking SPI operation inside a portENTER_CRITICAL
  // section. Do the SPI work first, in normal task context (safe to
  // block/yield here), and only bring the *results* into the critical
  // section to update the shared globals atomically - which is now
  // genuinely brief, matching what portENTER_CRITICAL is actually for.
  #if MCU_VARIANT == MCU_ESP32
    #if MODEM == SX1262
      LoRa->maskDio0();
    #endif
  #endif
  bool carrier_detected = LoRa->dcd();
  int new_current_rssi = LoRa->currentRssi();
  #if MCU_VARIANT == MCU_ESP32
    #if MODEM == SX1262
      LoRa->unmaskDio0();
    #endif
  #endif

  #if MCU_VARIANT == MCU_ESP32
    portENTER_CRITICAL(&update_lock);
  #elif MCU_VARIANT == MCU_NRF52
    portENTER_CRITICAL();
  #endif

  current_rssi = new_current_rssi;
  last_status_update = millis();

  #if MCU_VARIANT == MCU_ESP32
    portEXIT_CRITICAL(&update_lock);
  #elif MCU_VARIANT == MCU_NRF52
    portEXIT_CRITICAL();
  #endif

  #if BOARD_MODEL == BOARD_HELTEC32_V4
    if (noise_floor > LNA_GD_THRSHLD)  { interference_detected = !carrier_detected && (current_rssi > (noise_floor+CSMA_INFR_THRESHOLD_DB)); }
    else                               { interference_detected = !carrier_detected && (current_rssi > LNA_GD_LIMIT); }
  #else
    interference_detected = !carrier_detected && (current_rssi > (noise_floor+CSMA_INFR_THRESHOLD_DB));
  #endif

  if (interference_detected) { if (led_id_filter < LED_ID_TRIG) { led_id_filter += 1; } }
  else                       { if (led_id_filter > 0) {led_id_filter -= 1; } }

  // Handle potential false interference detection due to
  // LNA recalibration, antenna swap, moving into new RF
  // environment or similar.
  if (interference_detected && current_rssi < CSMA_RFENV_RECAL_LIMIT_DB) {
    if (!interference_persists) { interference_persists = true; interference_start = millis(); }
    else {
      if (millis()-interference_start >= CSMA_RFENV_RECAL_MS) { noise_floor_sampled = false; interference_persists = false; }
    }
  } else { interference_persists = false; }

  if (carrier_detected) { dcd = true; } else { dcd = false; }

  dcd_led = dcd;
  if (!LED_DISPLAY_BLANKED && dcd_led) { led_rx_on(); }
  else {
    // FIXED (ported from microReticulum_Firmware commit 5f5fadb, "fix
    // stuck rx led if noise floor is not sampled"): noise_floor_sampled
    // used to only gate the inner led_id_on() call, with interference_
    // detected alone as the outer branch - so interference_detected==true
    // && noise_floor_sampled==false (e.g. right after boot/a noise-floor
    // reset, before enough samples have accumulated) fell through this
    // whole block doing nothing at all, leaving led_id/led_rx stuck
    // showing whatever they last displayed. Folding noise_floor_sampled
    // into the outer condition means that case now correctly reaches the
    // else branch below (airtime_lock indication or led_rx_off()/
    // led_id_off()) instead of being stranded in limbo.
    if (interference_detected && noise_floor_sampled) {
      if (led_id_filter >= LED_ID_TRIG && !LED_DISPLAY_BLANKED) { led_id_on(); }
    } else {
      if (airtime_lock && !LED_DISPLAY_BLANKED) { led_indicate_airtime_lock(); }
      else {
        if (!LED_DISPLAY_BLANKED) { led_rx_off(); led_id_off(); }
      }
    }
  }

  // FIXED (ported from microReticulum_Firmware commit a5393a5, "fix tx:
  // medium_free() always returned false"): update_noise_floor() used to
  // live only in check_modem_status()'s own status_interval_ms-gated
  // block, alongside this same function - but medium_free() (below) calls
  // update_modem_status() directly and unconditionally, every CSMA check
  // during TX, which itself sets last_status_update = millis(). Once TX
  // traffic starts, that refreshes faster than status_interval_ms can
  // elapse, so check_modem_status()'s gate almost never opens and
  // update_noise_floor() effectively stops running - noise_floor goes
  // stale, and medium_free() (which depends on a valid noise floor) can
  // get permanently stuck returning false, wedging the TX queue. Worst in
  // TNC mode with interference avoidance active (repeated medium_free()
  // calls). Fix: never update modem status without updating noise floor
  // too, unconditionally, in the one place both actually happen.
  update_noise_floor();
}

void check_modem_status() {
  if (millis()-last_status_update >= status_interval_ms) {
    // update_noise_floor() now happens unconditionally at the end of
    // update_modem_status() itself - see that function's own comment for
    // why calling it only here (gated on status_interval_ms) was the bug.
    update_modem_status();

    #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
      util_samples[dcd_sample] = dcd;
      dcd_sample = (dcd_sample+1)%DCD_SAMPLES;
      if (dcd_sample % UTIL_UPDATE_INTERVAL == 0) {
        int util_count = 0;
        for (int ui = 0; ui < DCD_SAMPLES; ui++) {
          if (util_samples[ui]) util_count++;
        }
        local_channel_util = (float)util_count / (float)DCD_SAMPLES;
        total_channel_util = local_channel_util + airtime;
        if (total_channel_util > 1.0) total_channel_util = 1.0;

        int16_t cb = current_airtime_bin();
        uint16_t nb = cb+1; if (nb == AIRTIME_BINS) { nb = 0; }
        if (total_channel_util > longterm_bins[cb]) longterm_bins[cb] = total_channel_util;
        longterm_bins[nb] = 0.0;

        update_airtime();
      }
    #endif
  }
}

void validate_status() {
  #if MCU_VARIANT == MCU_1284P
      uint8_t boot_flags = OPTIBOOT_MCUSR;
      uint8_t F_POR = PORF;
      uint8_t F_BOR = BORF;
      uint8_t F_WDR = WDRF;
  #elif MCU_VARIANT == MCU_2560
      uint8_t boot_flags = OPTIBOOT_MCUSR;
      if (boot_flags == 0x00) boot_flags = 0x03;
      uint8_t F_POR = PORF;
      uint8_t F_BOR = BORF;
      uint8_t F_WDR = WDRF;
  #elif MCU_VARIANT == MCU_ESP32
      // TODO: Get ESP32 boot flags
      uint8_t boot_flags = 0x02;
      uint8_t F_POR = 0x00;
      uint8_t F_BOR = 0x00;
      uint8_t F_WDR = 0x01;
  #elif MCU_VARIANT == MCU_NRF52
      // TODO: Get NRF52 boot flags
      uint8_t boot_flags = 0x02;
      uint8_t F_POR = 0x00;
      uint8_t F_BOR = 0x00;
      uint8_t F_WDR = 0x01;
  #endif

  if (hw_ready || device_init_done) {
    hw_ready = false;
    Serial.write("Error, invalid hardware check state\r\n");
    #if HAS_DISPLAY
      if (disp_ready) {
        device_init_done = true;
        update_display();
      }
    #endif
    led_indicate_boot_error();
  }

  if (boot_flags & (1<<F_POR)) {
    boot_vector = START_FROM_POWERON;
  } else if (boot_flags & (1<<F_BOR)) {
    boot_vector = START_FROM_BROWNOUT;
  } else if (boot_flags & (1<<F_WDR)) {
    boot_vector = START_FROM_BOOTLOADER;
  } else {
      Serial.write("Error, indeterminate boot vector\r\n");
      #if HAS_DISPLAY
        if (disp_ready) {
          device_init_done = true;
          update_display();
        }
      #endif
      led_indicate_boot_error();
  }

  if (boot_vector == START_FROM_BOOTLOADER || boot_vector == START_FROM_POWERON) {
    if (eeprom_lock_set()) {
      if (eeprom_product_valid() && eeprom_model_valid() && eeprom_hwrev_valid()) {
        if (eeprom_checksum_valid()) {
          eeprom_ok = true;
          if (modem_installed) {
            #if PLATFORM == PLATFORM_ESP32 || PLATFORM == PLATFORM_NRF52
              if (device_init()) {
                hw_ready = true;
              } else {
                hw_ready = false;
              }
            #else
              hw_ready = true;
            #endif
          } else {
            hw_ready = false;
            Serial.write("No radio module found\r\n");
            #if HAS_DISPLAY
              if (disp_ready) {
                device_init_done = true;
                update_display();
              }
            #endif
            LoRa->reset();
            delay(5000);
            hard_reset();
          }
          
          if (hw_ready && eeprom_have_conf()) {
            eeprom_conf_load();
            op_mode = MODE_TNC;
            startRadio();
          }
        } else {
          hw_ready = false;
          Serial.write("Invalid EEPROM checksum\r\n");
          #if HAS_DISPLAY
            if (disp_ready) {
              device_init_done = true;
              update_display();
            }
          #endif
        }
      } else {
        hw_ready = false;
        Serial.write("Invalid EEPROM configuration\r\n");
        #if HAS_DISPLAY
          if (disp_ready) {
            device_init_done = true;
            update_display();
          }
        #endif
      }
    } else {
      hw_ready = false;
      Serial.write("Device unprovisioned, no device configuration found in EEPROM\r\n");
      #if HAS_DISPLAY
        if (disp_ready) {
          device_init_done = true;
          update_display();
        }
      #endif
    }
  } else {
    hw_ready = false;
    Serial.write("Error, incorrect boot vector\r\n");
    #if HAS_DISPLAY
      if (disp_ready) {
        device_init_done = true;
        update_display();
      }
    #endif
    led_indicate_boot_error();
  }
}

#if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
  void update_csma_parameters() {
    int airtime_pct = (int)(airtime*100);
    int new_cw_band = cw_band;

    if (airtime_pct <= CSMA_BAND_1_MAX_AIRTIME) { new_cw_band = 1; }
    else {
      int at = airtime_pct + CSMA_BAND_1_MAX_AIRTIME;
      new_cw_band = map(at, CSMA_BAND_1_MAX_AIRTIME, CSMA_BAND_N_MIN_AIRTIME, 2, CSMA_CW_BANDS);
    }

    if (new_cw_band > CSMA_CW_BANDS) { new_cw_band = CSMA_CW_BANDS; }
    if (new_cw_band != cw_band) { 
      cw_band = (uint8_t)(new_cw_band);
      cw_min  = (cw_band-1) * CSMA_CW_PER_BAND_WINDOWS;
      cw_max  = (cw_band) * CSMA_CW_PER_BAND_WINDOWS - 1;
      kiss_indicate_csma_stats();
    }
  }
#endif

void tx_queue_handler() {
  if (!airtime_lock && queue_height > 0) {
    if (csma_cw == -1) {
      csma_cw = random(cw_min, cw_max);
      cw_wait_target = csma_cw * csma_slot_ms;
    }

    if (difs_wait_start == -1) {                                                  // DIFS wait not yet started
      if (medium_free()) { difs_wait_start = millis(); return; }                  // Set DIFS wait start time
      else               { return; } }                                            // Medium not yet free, continue waiting
    
    else {                                                                        // We are waiting for DIFS or CW to pass
      if (!medium_free()) { difs_wait_start = -1; cw_wait_start = -1; return; }   // Medium became occupied while in DIFS wait, restart waiting when free again
      else {                                                                      // Medium is free, so continue waiting
        if (millis() < difs_wait_start+difs_ms) { return; }                       // DIFS has not yet passed, continue waiting
        else {                                                                    // DIFS has passed, and we are now in CW wait
          if (cw_wait_start == -1) { cw_wait_start = millis(); return; }          // If we haven't started counting CW wait time, do it from now
          else {                                                                  // If we are already counting CW wait time, add it to the counter
            cw_wait_passed += millis()-cw_wait_start; cw_wait_start   = millis();
            if (cw_wait_passed < cw_wait_target) { return; }                      // Contention window wait time has not yet passed, continue waiting
            else {                                                                // Wait time has passed, flush the queue
              bool should_flush = !lora_limit_rate && !lora_guard_rate;
              if (should_flush) { CP(CP_TXQ_FLUSH_QUEUE); flush_queue(); } else { CP(CP_TXQ_POP_QUEUE); pop_queue(); }
              cw_wait_passed = 0; csma_cw = -1; difs_wait_start = -1; }
          }
        }
      }
    }
  }
}

void work_while_waiting() { loop(); }

void loop() {
  #if MCU_VARIANT == MCU_ESP32 && HAS_URNS == true
    esp_task_wdt_reset();
  #endif
  CP(CP_LOOP_TOP);
  // housekeeping_task()/kiss_tx_task() (this file) run as their own
  // dedicated FreeRTOS tasks, not folded onto loopTask - see their own
  // comments for why. They briefly were folded here (to recover ~14KB+ of
  // task stack/TCB overhead for BLE's DRAM margin, matching
  // microReticulum_Firmware's zero-extra-task shape) but that reintroduced
  // a real, confirmed hang - the "Serial.write() occasionally never
  // returns" USBCDC quirk these tasks were originally isolated to protect
  // against.
  #if HAS_URNS == true
    if (urns_ready) {
      CP(CP_URNS_RETICULUM_LOOP);
      urns_reticulum.loop();
      CP(CP_URNS_LXMF_LOOP);
      urns_lxmf_loop();
      #if HAS_LXMF == true
        CP(CP_MSNGR_PING);
        messenger_ping_process();
        CP(CP_MSNGR_SEND);
        messenger_send_process();
        CP(CP_MSNGR_SEND_RESULT);
        msngr_send_result_process();
        #if HAS_DEBUG_UART == true
          CP(CP_MSNGR_HEARTBEAT);
          messenger_heartbeat_process();
        #endif
      #endif

      // Tier 2 one-shot smoke test: announce once, a few seconds after
      // boot so it goes out through the normal CSMA gate instead of
      // racing radio/queue bring-up. Not a real application behavior -
      // remove once RX has been confirmed on a second unit.
      static bool urns_announced = false;
      if (!urns_announced && millis() > 8000) {
        urns_announced = true;
        CP(CP_URNS_ANNOUNCE);
        urns_announce();
      }
    }
  #endif

  if (radio_online) {
    #if MCU_VARIANT == MCU_ESP32
      // Drains a DIO0 interrupt flagged by sx126x's onDio0Rise() (see its
      // own comment, sx126x.cpp) - must run every loop() iteration,
      // before the modem_packet_queue dequeue right below, since this is
      // what actually populates that queue via receive_callback(). A
      // no-op on other modems (MODEM != SX1262) - see
      // handleDio0IfPending()'s own comment for why this is scoped to
      // sx126x only for now.
      #if MODEM == SX1262
        CP(CP_DIO0_PENDING);
        LoRa->handleDio0IfPending();
      #endif
      CP(CP_MODEM_QUEUE_DRAIN);
      modem_packet_t *modem_packet = NULL;
      if(modem_packet_queue && xQueueReceive(modem_packet_queue, &modem_packet, 0) == pdTRUE && modem_packet) {
        host_write_len = modem_packet->len;
        last_rssi      = modem_packet->rssi;
        last_snr_raw   = modem_packet->snr_raw;
        memcpy(&pbuf, modem_packet->data, modem_packet->len);

        // Same as kiss_write_packet()'s own call (RNode_Firmware.ino) -
        // kept here, synchronous, on loopTask, since this is a bounded
        // memcpy into a single-slot staging buffer, not a Serial write.
        // See kiss_tx_task()'s own comment for why the actual KISS write
        // is deferred through a queue rather than done inline here.
        #if HAS_URNS == true
          if (urns_ready) { urns_stage_incoming(pbuf, host_write_len); }
        #endif

        // Hand ownership of modem_packet straight to the g_kiss_tx_queue
        // instead of freeing it here and calling kiss_indicate_stat_rssi()/
        // kiss_indicate_stat_snr()/kiss_write_packet() synchronously - see
        // kiss_tx_task()'s own comment (this file) for why.
        if (g_kiss_tx_queue && xQueueSend(g_kiss_tx_queue, &modem_packet, 0) != pdTRUE) {
          free(modem_packet); // queue full (consumer stuck?) - drop rather than leak
        }
        modem_packet = NULL;
      }

      airtime_lock = false;
      if (st_airtime_limit != 0.0 && airtime >= st_airtime_limit) airtime_lock = true;
      if (lt_airtime_limit != 0.0 && longterm_airtime >= lt_airtime_limit) airtime_lock = true;

    #elif MCU_VARIANT == MCU_NRF52
      modem_packet_t *modem_packet = NULL;
      if(modem_packet_queue && xQueueReceive(modem_packet_queue, &modem_packet, 0) == pdTRUE && modem_packet) {
        memcpy(&pbuf, modem_packet->data, modem_packet->len);
        host_write_len = modem_packet->len;
        free(modem_packet);
        modem_packet = NULL;

        portENTER_CRITICAL();
        last_rssi = LoRa->packetRssi();
        last_snr_raw = LoRa->packetSnrRaw();
        portEXIT_CRITICAL();
        kiss_indicate_stat_rssi();
        kiss_indicate_stat_snr();
        kiss_write_packet();
      }

      airtime_lock = false;
      if (st_airtime_limit != 0.0 && airtime >= st_airtime_limit) airtime_lock = true;
      if (lt_airtime_limit != 0.0 && longterm_airtime >= lt_airtime_limit) airtime_lock = true;

    #endif

    CP(CP_TX_QUEUE_HANDLER);
    tx_queue_handler();
    CP(CP_CHECK_MODEM_STATUS);
    check_modem_status();

  } else {
    if (hw_ready) {
      if (console_active) {
        #if HAS_CONSOLE
          console_loop();
        #endif
      } else {
        if (!LED_DISPLAY_BLANKED) {
          CP(CP_LED_STANDBY);
          #if HAS_ESPNOW == true && HAS_NP == true
            // While a host has actually grabbed vport 1 (espnow_ui_active()),
            // show real ESP-NOW RX/TX activity instead of the idle white
            // breathing pulse - mirrors how radio_online being true takes
            // the entirely separate if(radio_online) branch above instead
            // of ever reaching led_indicate_standby() at all.
            if (espnow_ui_active()) { espnow_led_update(); }
            else { led_indicate_standby(); }
          #else
            led_indicate_standby();
          #endif
        }
        #if HAS_NP == true
          else {
            // led_indicate_standby() stops being called once the display
            // blanks, which would otherwise freeze its breathing pulse at
            // whatever brightness it last had instead of turning off -
            // draining the battery on nodes where LED_DISPLAY_BLANKED
            // tracks display_blanked. npset() no-ops once the pixel is
            // already off, so this is cheap to call every loop tick.
            npset(0x00, 0x00, 0x00);
          }
        #endif
      }
    } else {

      led_indicate_not_ready();
      stopRadio();
    }
  }

  CP(CP_SERIAL_BUFFER_POLL);
  #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
      buffer_serial();
      if (!fifo_isempty(&serialFIFO)) serial_poll();
  #else
    if (!fifo_isempty_locked(&serialFIFO)) serial_poll();
  #endif

  #if HAS_DISPLAY
    if (disp_ready && !display_updating) { CP(CP_DISPLAY_UPDATE); update_display(); }
  #endif

  #if HAS_BUZZER == true
    CP(CP_BUZZER_UPDATE);
    buzzer_update();
  #endif

  #if HAS_PMU || IS_ESP32S3
    if (pmu_ready) { CP(CP_PMU_UPDATE); update_pmu(); }
  #endif

  #if HAS_VSENSE == true
    CP(CP_VSENSE_UPDATE);
    update_vsense();
  #endif

  #if HAS_BLUETOOTH || HAS_BLE == true
    if (!console_active && bt_ready) { CP(CP_BT_UPDATE); update_bt(); }
  #endif

  #if HAS_WIFI
    if (wifi_initialized) { CP(CP_WIFI_UPDATE); update_wifi(); }
    if (ws_enabled) update_ws();
  #endif

  #if HAS_OTA == true
    if (!console_active) { CP(CP_OTA_LOOP); ota_loop(); }
  #endif

  #if HAS_ESPNOW == true
    if (espnow_enabled) { CP(CP_ESPNOW_UPDATE); update_espnow(); update_espnow_tx(); }
  #endif

  #if HAS_GPS == true
    CP(CP_GNSS_UPDATE);
    gnss_update();
  #endif

  #if HAS_INPUT
    CP(CP_INPUT_READ);
    input_read();
  #endif

  #if HAS_ENCODER == true
    CP(CP_ENCODER_PROCESS);
    encoder_process();
  #endif
  #if HAS_MENU == true
    CP(CP_MENU_PROCESS);
    menu_button_process();
    menu_timeout_process();
    #if HAS_WIFI == true || HAS_ETHERNET == true
      menu_popup_process();
    #endif
  #endif

  if (memory_low) {
    #if PLATFORM == PLATFORM_ESP32
      if (esp_get_free_heap_size() < 8192) {
        kiss_indicate_error(ERROR_MEMORY_LOW); memory_low = false;
      } else {
        memory_low = false;
      }
    #else
      kiss_indicate_error(ERROR_MEMORY_LOW); memory_low = false;
    #endif
  }
}

void sleep_now() {
  #if HAS_SLEEP == true
    stopRadio(); // TODO: Check this on all platforms
    #if PLATFORM == PLATFORM_ESP32
      #if BOARD_MODEL == BOARD_T3S3 || BOARD_MODEL == BOARD_XIAO_S3
        #if HAS_DISPLAY
          display_intensity = 0;
          update_display(true);
        #endif
      #endif
      #if BOARD_MODEL == BOARD_HELTEC32_V4
          #if LORA_PA_AUTO_DETECT
            if (sx126x_modem.isKCT8103L()) {
              digitalWrite(LORA_PA_CTX, LOW);
            } else {
              digitalWrite(LORA_PA_CPS, LOW);
            }
          #endif
          digitalWrite(LORA_PA_CSD, LOW);
          digitalWrite(LORA_PA_PWR_EN, LOW);
          digitalWrite(Vext, HIGH);
      #endif
      #if PIN_DISP_SLEEP >= 0
        pinMode(PIN_DISP_SLEEP, OUTPUT);
        digitalWrite(PIN_DISP_SLEEP, DISP_SLEEP_LEVEL);
      #endif
      #if HAS_BLUETOOTH
        if (bt_state == BT_STATE_CONNECTED) {
          bt_stop();
          delay(100);
        }
      #endif
      esp_sleep_enable_ext0_wakeup(PIN_WAKEUP, WAKEUP_LEVEL);
      esp_deep_sleep_start();
    #elif PLATFORM == PLATFORM_NRF52
      #if BOARD_MODEL == BOARD_HELTEC_T114
        npset(0,0,0);
        digitalWrite(PIN_VEXT_EN, LOW);
        // set_contrast() (Display.h) drives this pin with analogWrite(),
        // which hands it to one of the nRF52's HardwarePWM peripherals
        // (HwPWMx[], cores/nRF5/HardwarePWM.h) via PSEL.OUT - that
        // peripheral keeps driving the pin's actual output level
        // regardless of what pinMode()/digitalWrite() do afterward
        // (neither touches PWM ownership at all, only the GPIO's own
        // PIN_CNF/OUT registers, which the PWM peripheral overrides at the
        // pin mux level), so a plain digitalWrite(HIGH) alone silently did
        // nothing here - removePin() releases the pin back to plain GPIO
        // control first.
        for (int hwpwm_i = 0; hwpwm_i < HWPWM_MODULE_NUM; hwpwm_i++) {
          if (HwPWMx[hwpwm_i]->checkPin(PIN_T114_TFT_BLGT)) { HwPWMx[hwpwm_i]->removePin(PIN_T114_TFT_BLGT); }
        }
        pinMode(PIN_T114_TFT_BLGT, OUTPUT);
        digitalWrite(PIN_T114_TFT_BLGT, HIGH);
        digitalWrite(PIN_T114_TFT_EN, HIGH);
      #elif BOARD_MODEL == BOARD_HELTEC_T096
        digitalWrite(PIN_T096_TFT_BLGT, HIGH);
        digitalWrite(PIN_T096_TFT_EN, LOW);
      #elif BOARD_MODEL == BOARD_TECHO
        for (uint8_t i = display_intensity; i > 0; i--) { analogWrite(pin_backlight, i-1); delay(1); }
        epd_black(true); delay(300); epd_black(true); delay(300); epd_black(false);
        delay(2000);
        analogWrite(PIN_VEXT_EN, 0);
        delay(100);
      #elif BOARD_MODEL == BOARD_PROMICRO
        #if HAS_DISPLAY
          display_intensity = 0;
          update_display(true);
        #endif
        delay(100);
      #endif
      sd_power_gpregret_set(0, 0x6d);
      // This MCU's attachInterrupt() (WInterrupts.c) is pure GPIOTE - GPIOTE
      // is unpowered in System OFF, so it can never be what wakes the chip,
      // ruling out encoder_init()'s CHANGE interrupts on pin_encoder_up/down
      // as the cause of encoder-rotation wake despite neither of them ever
      // being explicitly SENSE-armed anywhere. The real explanation is the
      // line below: it passes pin_btn_usr1's raw Arduino pin index straight
      // into nrf_gpio_cfg_sense_input(), which expects the SoC's native flat
      // P0.xx/P1.xx encoding instead (pinMode()/digitalWrite() translate
      // this internally via g_ADigitalPinMap[]; this raw call doesn't - same
      // gotcha as the buzzer's nrf_gpio_cfg() call, Utilities.h). So it
      // doesn't arm SENSE on the real button pin at all (which is why
      // pressing it never wakes the device) - it arms SENSE+pullup on
      // whatever pin the untranslated index numerically collides with
      // instead, on PROMICRO an unrelated, unintended pin - and rotating the
      // encoder is apparently enough mechanical/electrical noise near that
      // stray pin to trip it.
      #if BOARD_MODEL == BOARD_PROMICRO
        // Per explicit request: this board should wake only via physical
        // RESET, so no pin gets armed as a SENSE wake source at all here,
        // rather than fixing the translation above only to then have to
        // re-disable a now-correctly-working button-wake.
      #else
        nrf_gpio_cfg_sense_input(pin_btn_usr1, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
      #endif
      NRF_POWER->SYSTEMOFF = 1;
    #endif
  #endif
}

void button_event(uint8_t event, unsigned long duration) {
  #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
    if (display_blanked) {
      display_unblank();
    #if HAS_MENU == true
    } else if (menu_is_open()) {
      // Settings menu owns the screen - don't let the main button's
      // sleep/BT-pairing/console tiers fire while the user is mid-edit.
      // Doubles as an alternate control everywhere except WiFi SSID/PSK
      // text entry, where it stays a dedicated backspace key instead (see
      // menu_main_button_del()) - but only when there's an actual encoder
      // doing the character-wheel/save duty instead. On a button-only
      // board (or an encoder board with the encoder runtime-disabled,
      // encoder_enabled) this button IS the only input, so it has to stay
      // on the normal menu_button_press() dispatch (tap=next char,
      // double-tap=previous char, hold=confirm char/DEL/save - same
      // primitives wheel_move()/menu_confirm_select() already use for the
      // encoder path) or WiFi SSID/PSK entry is otherwise entirely stuck.
      #if HAS_ENCODER == true
        bool wifi_text_edit_has_encoder = encoder_enabled;
      #else
        bool wifi_text_edit_has_encoder = false;
      #endif
      if (menu_state == MENU_STATE_WIFI_TEXT_EDIT && wifi_text_edit_has_encoder) {
        menu_main_button_del();
        display_unblank();
      } else {
        menu_button_press(duration);
      }
    #endif
    } else if (device_init_done) {
      // Every tier below can reach real hardware init (bt_start()'s NimBLE
      // stack, console_start(), sleep_now()) - device_init_done guards the
      // whole chain the same way menu_open_from_closed()/messenger_open_
      // from_closed() already guard themselves, so a press landing before
      // boot fully settles just does nothing instead of reaching into
      // half-initialized subsystems. Confirmed live on hardware: a button
      // press this early can fall into the short-tap BT-toggle fallback
      // below and call bt_start() while WiFi's own init is still settling -
      // nimble_port_init() fails with "rc=-1 Unknown ESP_ERR error" and
      // that failure isn't handled gracefully, crashing with a
      // LoadProhibited exception instead of erroring out cleanly.
      //
      // Each tier's #if guards the whole condition, not just its body -
      // a board missing a given capability (e.g. HAS_CONSOLE, true on
      // very few boards) must fall through to the next lower tier for
      // any duration that would've matched the missing one, not silently
      // do nothing. Guarding only the body (as this used to) left a
      // dead if-branch for that duration range on every board without
      // the capability - e.g. holding past 10s on a HAS_CONSOLE==false
      // board landed in the empty duration>10000 branch and never fell
      // through to trigger sleep, even though HAS_SLEEP was available.
      #if HAS_CONSOLE
        if (duration > 10000) {
          #if HAS_BLUETOOTH || HAS_BLE
            bt_stop();
          #endif
          console_active = true;
          console_start();
        } else
      #endif
      #if HAS_SLEEP
        // Only actually sleeps within SLEEP_HOLD_CANCEL_MS of the
        // threshold (Config.h) - past that, this still claims the
        // duration (no `else` fall-through to Bt Pairing/Settings below,
        // matching button_hold_tier()'s own no-fall-through, Menu.h) but
        // takes no action at all, intentionally: holding well past the
        // Sleep tier and releasing is how the on-screen box's own
        // disappearing tells the user to back out of an accidental hold,
        // without having to time a precise release.
        if (duration > 7000) {
          if (duration <= 7000 + SLEEP_HOLD_CANCEL_MS) sleep_now();
        } else
      #endif
      #if HAS_BLUETOOTH || HAS_BLE
        if (duration > 5000) {
          if (bt_state != BT_STATE_CONNECTED) {
            // bt_enable_pairing() calls bt_start(), which touches NVS flash
            // (BLE bond store + controller init) - mask DIO0 around it so an
            // incoming-packet interrupt can't fire mid-op and hit the same
            // hard FreeRTOS assert as feedback_dio0_isr_vs_flash_io_crash
            // (blocking SPI from true ISR context while the scheduler is
            // suspended for flash I/O).
            #if MODEM == SX1262
              LoRa->maskDio0();
            #endif
            bt_enable_pairing();
            #if MODEM == SX1262
              LoRa->unmaskDio0();
            #endif
          }
        } else
      #endif
      #if HAS_MENU == true
        if (duration > 3000) {
          menu_open_from_closed();
        } else
      #endif
      #if HAS_LXMF == true
        // Dedicated shorter hold for the emergency Messenger app
        // (Messenger.h/BUTTON_HOLD_TIER_MESSENGER) - see that tier's own
        // comment (Menu.h) for why 1500ms sits where it does relative to
        // the Settings tier just above and every board's own short-click
        // action just below.
        if (duration > 1500) {
          messenger_open_from_closed();
        } else
      #endif
      {
        #if HAS_BLUETOOTH || HAS_BLE
        if (bt_state != BT_STATE_CONNECTED) {
          // bt_start()/bt_stop() touch NVS flash (BLE bond store +
          // controller init) - mask DIO0 around them so an incoming-packet
          // interrupt can't fire mid-op and hit the same hard FreeRTOS
          // assert as feedback_dio0_isr_vs_flash_io_crash (blocking SPI from
          // true ISR context while the scheduler is suspended for flash
          // I/O). This is the short-tap default action, so it's the most
          // likely of the BT tiers to land mid-RX.
          #if MODEM == SX1262
            LoRa->maskDio0();
          #endif
          if (bt_state == BT_STATE_OFF) {
            bt_start();
            bt_conf_save(true);
            #if HAS_BUZZER == true
              buzzer_bt_on_melody();
            #endif
          } else {
            bt_stop();
            bt_conf_save(false);
            #if HAS_BUZZER == true
              buzzer_bt_off_melody();
            #endif
          }
          #if MODEM == SX1262
            LoRa->unmaskDio0();
          #endif
        }
        #endif
      }
    }
  #endif
}

volatile bool serial_polling = false;
void serial_poll() {
  serial_polling = true;

  #if MCU_VARIANT != MCU_ESP32 && MCU_VARIANT != MCU_NRF52
  while (!fifo_isempty_locked(&serialFIFO)) {
  #else
  while (!fifo_isempty(&serialFIFO)) {
  #endif
    char sbyte = fifo_pop(&serialFIFO);
    serial_callback(sbyte);
  }

  serial_polling = false;
}

#if MCU_VARIANT != MCU_ESP32
  #define MAX_CYCLES 20
#else
  #define MAX_CYCLES 10
#endif
void buffer_serial() {
  if (!serial_buffering) {
    serial_buffering = true;

    uint8_t c = 0;

    #if HAS_BLUETOOTH || HAS_BLE == true
    while (
      c < MAX_CYCLES &&
      #if HAS_ETHERNET == true
      ( (bt_state != BT_STATE_CONNECTED && Serial.available()) || (bt_state == BT_STATE_CONNECTED && SerialBT.available()) || ((eth_is_connected || wr_state >= WR_STATE_ON) && wifi_remote_available()) || (ws_state >= WS_STATE_ON && ws_remote_available()) )
      #elif HAS_WIFI
      ( (bt_state != BT_STATE_CONNECTED && Serial.available()) || (bt_state == BT_STATE_CONNECTED && SerialBT.available()) || (wr_state >= WR_STATE_ON && wifi_remote_available()) || (ws_state >= WS_STATE_ON && ws_remote_available()) )
      #else
      ( (bt_state != BT_STATE_CONNECTED && Serial.available()) || (bt_state == BT_STATE_CONNECTED && SerialBT.available()) )
      #endif
      )
    #else
    while (c < MAX_CYCLES && Serial.available())
    #endif
    {
      c++;

      #if MCU_VARIANT != MCU_ESP32 && MCU_VARIANT != MCU_NRF52
        if (!fifo_isfull_locked(&serialFIFO)) { fifo_push_locked(&serialFIFO, Serial.read()); }
      #elif HAS_BLUETOOTH || HAS_BLE == true || HAS_WIFI || HAS_ETHERNET == true
        #if HAS_BLUETOOTH || HAS_BLE == true
        if      (bt_state == BT_STATE_CONNECTED) { if (!fifo_isfull(&serialFIFO)) { fifo_push(&serialFIFO, SerialBT.read()); } }
        #if HAS_ETHERNET == true
        else if (eth_is_connected && wifi_host_is_connected())  { if (!fifo_isfull(&serialFIFO)) { fifo_push(&serialFIFO, wifi_remote_read()); } }
        #endif
        #if HAS_WIFI
        else if (wifi_host_is_connected())       { if (!fifo_isfull(&serialFIFO)) { fifo_push(&serialFIFO, wifi_remote_read()); } }
        else if (ws_host_is_connected())         { if (!fifo_isfull(&serialFIFO)) { fifo_push(&serialFIFO, ws_remote_read()); } }
        #endif
        else                                     { if (!fifo_isfull(&serialFIFO)) { fifo_push(&serialFIFO, Serial.read()); } }
        #else
                                                   if (!fifo_isfull(&serialFIFO)) { fifo_push(&serialFIFO, Serial.read()); }
        #endif
      #else
        if (!fifo_isfull(&serialFIFO)) { fifo_push(&serialFIFO, Serial.read()); }
      #endif
    }

    serial_buffering = false;
  }
}

void serial_interrupt_init() {
  #if MCU_VARIANT == MCU_1284P
      TCCR3A = 0;
      TCCR3B = _BV(CS10) |
               _BV(WGM33)|
               _BV(WGM32);

      // Buffer incoming frames every 1ms
      ICR3 = 16000;
      TIMSK3 = _BV(ICIE3);

  #elif MCU_VARIANT == MCU_2560
      // TODO: This should probably be updated for
      // atmega2560 support. Might be source of
      // reported issues from snh.
      TCCR3A = 0;
      TCCR3B = _BV(CS10) |
               _BV(WGM33)|
               _BV(WGM32);

      // Buffer incoming frames every 1ms
      ICR3 = 16000;
      TIMSK3 = _BV(ICIE3);

  #elif MCU_VARIANT == MCU_ESP32
      // No interrupt-based polling on ESP32
  #endif

}

#if MCU_VARIANT == MCU_1284P || MCU_VARIANT == MCU_2560
  ISR(TIMER3_CAPT_vect) { buffer_serial(); }
#endif
