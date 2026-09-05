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

#include "Config.h"
#include <stdarg.h>

// DEBUG_UART_BEGIN()/DEBUG_LOG() - general-purpose debug logging over
// whichever free UART a board declares via HAS_DEBUG_UART (Boards.h) -
// usable from any file included below this point, not tied to any one
// feature. Comment out DEBUG_UART_ENABLED to compile every call out
// entirely (e.g. for a release build) without touching call sites - they
// stay in the source either way, they just become no-ops.
#define DEBUG_UART_ENABLED

#if defined(DEBUG_UART_ENABLED) && HAS_DEBUG_UART == true
  #define DEBUG_UART_BEGIN() Serial0.begin(115200)
  #if MCU_VARIANT == MCU_ESP32
    // Every DEBUG_LOG(...) call in the firmware used to be a plain
    // Serial0.printf() - fine as long as only one FreeRTOS task ever
    // called it at a time, which isn't true here (loopTask and
    // LXStamper's own core-0 proof-of-work task both log through the RNS
    // log callback, see Messenger.h). A first attempt at fixing this
    // wrapped every call in a bounded (50ms) mutex - real improvement,
    // but not sufficient: confirmed live that the actual Serial0.print()
    // call *inside* that guard can itself hang indefinitely (same class
    // of bug that this same isolation was built to fix for the *other*
    // UART's own Serial.write() hang, same symptom). When that happens,
    // whoever holds the mutex at that moment never reaches
    // xSemaphoreGive(), and loopTask itself can be that holder - the
    // mutex approach didn't actually prevent it here.
    //
    // Fixed by queueing instead: nobody calls Serial0.print() directly
    // from wherever DEBUG_LOG() was called - it formats the line (cheap,
    // can't hang) and drops it on a queue; the actual write happens in
    // housekeeping_task() (RNode_Firmware.ino), on its own dedicated task -
    // specifically so a hung write there can't take loopTask down with it.
    // Briefly folded onto loopTask itself (an experiment matching
    // microReticulum_Firmware's zero-extra-task architecture) but that
    // reintroduced exactly this hang (see housekeeping_task()'s own
    // comment) - moved back to its own task. The queue still serializes
    // access either way, so no mutex is needed. Fail-
    // open by design: xQueueSend with a 0 wait just drops the line if the
    // queue's full rather than ever blocking the caller - a missed debug
    // line is harmless, an indefinite hang isn't.
    #define DEBUG_LOG_MSG_LEN 200
    // Was briefly 32 (to absorb TRACE-level boot bursts) - reverted to 16
    // along with RNS::loglevel() going back to INFO (urns_init(), URNS.h -
    // see that revert's own comment for why TRACE was reverted).
    #define DEBUG_LOG_QUEUE_DEPTH 16
    QueueHandle_t g_debug_log_queue = NULL;
    inline void debug_log_guarded(const char* fmt, ...) {
      if (!g_debug_log_queue) return;
      char buf[DEBUG_LOG_MSG_LEN];
      va_list args;
      va_start(args, fmt);
      vsnprintf(buf, sizeof(buf), fmt, args);
      va_end(args);
      xQueueSend(g_debug_log_queue, buf, 0);
    }
    #define DEBUG_LOG(...) debug_log_guarded(__VA_ARGS__)
  #else
    #define DEBUG_LOG(...) Serial0.printf(__VA_ARGS__)
  #endif
#else
  #define DEBUG_UART_BEGIN()
  #define DEBUG_LOG(...)
#endif

#if MCU_VARIANT == MCU_ESP32 && HAS_URNS == true
  // CMD_LOG (0x80, Framing.h) - a real KISS frame carrying RNS log text,
  // consumed by microReticulum_Firmware's own webconsole (its Logs tab
  // already parses exactly this "HH:MM:SS.mmm [LVL] message" format for
  // level filtering - see RNS::getTimeString()/getLevelName()). The
  // opcode existed in Framing.h already but nothing ever sent it. Same
  // queue-based isolation as DEBUG_LOG/g_debug_log_queue just above -
  // serial_write()/escaped_serial_write() write to the same Serial
  // connection the binary KISS protocol itself uses, and the RNS log
  // callback (urns_rns_log_callback(), URNS.h) can be called from other
  // tasks too (LXStamper's own core-0 proof-of-work task, same reasoning
  // as DEBUG_LOG's own comment above) - so formatting happens wherever
  // RNS::log() was called, and only housekeeping_task() (RNode_Firmware.
  // ino, its own dedicated task) ever actually touches Serial for this.
  #define CMD_LOG_MSG_LEN 256
  // Was briefly raised to 32 to absorb TRACE-level bursts, reverted to 8
  // along with RNS::loglevel() going back to INFO (urns_init(), URNS.h) -
  // TRACE reproducibly crashed real hardware (BLE margin collapsed to
  // ~980 bytes, cascading into a NimBLE assert) from the per-call-site
  // stack cost of logging inside already-deep call chains, not from queue
  // sizing - see that revert's own comment for the full story. INFO-level
  // logging is genuinely sparse (~24 call sites in the whole vendored
  // library, no per-packet chatter), so 8 is enough again.
  #define CMD_LOG_QUEUE_DEPTH 8
  QueueHandle_t g_cmd_log_queue = NULL;
  inline void cmd_log_guarded(const char* line) {
    if (!g_cmd_log_queue) return;
    char buf[CMD_LOG_MSG_LEN];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    xQueueSend(g_cmd_log_queue, buf, 0);
  }
#endif

#if HAS_EEPROM
    #include <EEPROM.h>
#elif PLATFORM == PLATFORM_NRF52
		#include <hal/nrf_rng.h>
    #include <Adafruit_LittleFS.h>
    #include <InternalFileSystem.h>
    using namespace Adafruit_LittleFS_Namespace;
    #define EEPROM_FILE "eeprom"
    bool file_exists = false;
    int written_bytes = 4;
    File file(InternalFS);
#endif
#include <stddef.h>

#if MODEM == SX1262
#include "sx126x.h"
sx126x *LoRa = &sx126x_modem;
#elif MODEM == SX1276 || MODEM == SX1278
#include "sx127x.h"
sx127x *LoRa = &sx127x_modem;
#elif MODEM == SX1280
#include "sx128x.h"
sx128x *LoRa = &sx128x_modem;
#endif

#include "ROM.h"
#include "Framing.h"
#include "MD5.h"

#if !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
uint8_t eeprom_read(uint32_t mapped_addr);
#endif

void set_rns_link_state(uint8_t new_state);

#if HAS_WIFI == true || HAS_ETHERNET == true
// Reads a 4-byte IP/netmask from the config region (config_addr(), not
// eeprom_addr() - shared by WiFi's ADDR_CONF_IP/NM and, on MeshPoE-S3,
// wired Ethernet's own ADDR_CONF_ETH_IP/NM, see ROM.h) into out[4] -
// returns false if the stored bytes are all-zero (never configured) or
// all-0xFF (erased EEPROM), either of which means "unset, fall back to
// DHCP". Declared here (not down with the rest of the network code) so
// both Remote.h and Ethernet.h - included further down in this same file -
// can already see it.
bool addr4_read(int addr_base, uint8_t *out) {
  bool all_zero = true;
  bool all_ff = true;
  for (uint8_t i = 0; i < 4; i++) {
    #if HAS_EEPROM
      out[i] = EEPROM.read(config_addr(addr_base+i));
    #elif MCU_VARIANT == MCU_NRF52
      out[i] = eeprom_read(config_addr(addr_base+i));
    #endif
    if (out[i] != 0x00) all_zero = false;
    if (out[i] != 0xFF) all_ff = false;
  }
  return !(all_zero || all_ff);
}
#endif

// Board default (SOUND_ENABLED_DEFAULT, Boards.h) - true everywhere the
// buzzer is a standard always-populated feature (preserving the historical
// always-on behavior for anyone who never touches Sound in the menu), false
// on boards where it's a DIY add-on most builds skip (e.g. PROMICRO).
bool sound_enabled = SOUND_ENABLED_DEFAULT;
#if HAS_BUZZER == true
  // Declared here (not down with the rest of the buzzer_*() functions)
  // because Menu.h - which reads this for the GPIO submenu - is #include'd
  // further down in this same file, before that point is reached.
  uint8_t buzzer_pin = PIN_BUZZER;
#endif
#if HAS_ENCODER == true
  // Same reasoning as buzzer_pin above - declared here, not down with
  // Encoder.h, so Menu.h (included before Encoder.h) can already see them.
  uint8_t pin_encoder_up    = PIN_ENCODER_UP;
  uint8_t pin_encoder_down  = PIN_ENCODER_DOWN;
  uint8_t pin_encoder_press = PIN_ENCODER_PRESS;
  // Whether a physical encoder is actually populated - defaults OFF
  // (unlike sound_enabled, this has no board-specific default: an encoder
  // is always either a PCB-provisioned-but-optional part, e.g.
  // MeshAdventurer-S3, or a DIY add-on, e.g. PROMICRO - never a guaranteed
  // always-there feature). Only affects the menu's on-screen footer hint
  // (turn/press vs tap/hold) - the encoder itself is always serviced
  // regardless of this setting.
  bool encoder_enabled = false;
#endif
#if HAS_NP == true
  // Same reasoning as buzzer_pin/pin_encoder_up above - declared here (not
  // down with the rest of the NeoPixel code) so Menu.h, #include'd further
  // down in this same file, can already see it.
  uint8_t np_intensity = (uint8_t)(NP_M * 255.0f);
  void led_set_intensity(uint8_t intensity);
  void np_int_conf_save(uint8_t p_int);
#endif
void db_conf_save(uint8_t val);
void di_conf_save(uint8_t dint);
int lora_txp_max();
void eeprom_conf_save();
void eeprom_conf_delete();
void snd_conf_save(bool is_enabled);
void wr_conf_save(uint8_t mode);
void drot_conf_save(uint8_t val);
#if HAS_VSENSE == true
  void vsr_conf_save(uint8_t val);
#endif
#if HAS_BATTERY_DIVIDER == true
  void bvs_conf_save(uint8_t val);
#endif
#if HAS_ENCODER == true
  void enc_conf_save(bool is_enabled);
#endif
#if HAS_GPIO_MENU == true
  void gpio_conf_save(uint8_t addr, uint8_t val);
#endif
#if HAS_ETHERNET == true
  void ethspd_conf_save(uint8_t val);
  void ethaddr_conf_save(int addr_base, uint8_t *val);
#endif
#if HAS_ESPNOW == true
  void espnow_conf_save(uint8_t val);
  void espnow_mode_conf_save(uint8_t val);
  void espnow_lr_conf_save(uint8_t val);
#endif
#if HAS_BLUETOOTH == true || HAS_BLE == true
  void bt_conf_save(bool is_enabled);
  // Forward-declared so bt_start()/bt_stop() (Bluetooth.h, included below
  // before these are actually defined) can call them directly, gated on
  // whether bt_state actually transitioned - rather than the caller
  // guessing melody-vs-no-melody from whether it merely *attempted* a
  // toggle, which used to fire the "on" chirp even when bt_start() silently
  // no-op'd (e.g. still within BT_START_MIN_UPTIME_MS right after boot).
  // Real melody or no-op stub either way (Utilities.h, HAS_BUZZER-gated
  // definitions further down) - safe to call unconditionally.
  void buzzer_bt_on_melody();
  void buzzer_bt_off_melody();
#endif
#if MCU_VARIANT == MCU_ESP32 && HAS_BLE == true
  void bt_legacy_pairing_conf_save(bool is_enabled);
  void bt_just_works_conf_save(bool is_enabled);
  void bt_auto_start_conf_save(bool is_enabled);
#endif
#if HAS_RTC == true
  void kiss_indicate_time();
#endif
#if HAS_GPS == true
  void gnss_conf_save(bool is_enabled);
  void gnss_interval_conf_save(uint8_t preset_index);
#endif
#if HAS_RTC == true || HAS_GPS == true
  void tz_conf_save(uint8_t val);
#endif
#if MCU_VARIANT == MCU_ESP32 && HAS_RTC == true && (HAS_WIFI == true || HAS_ETHERNET == true)
  void kiss_indicate_ntp_sync(uint8_t status);
#endif
void eeprom_update(int mapped_addr, uint8_t byte);
void buzzer_encoder_tick_melody();
void buzzer_encoder_click_melody();
void buzzer_lxmf_rx_melody();
void buzzer_wait_for_melody();

#if HAS_GPS == true
  #if HAS_URNS == true && HAS_RTC == false
    // Forward declaration - defined in RNode_Firmware.ino, further down
    // than this #include. Called directly from gnss_update() (GNSS.h)
    // right after it decodes a fresh NMEA sentence, rather than
    // unconditionally every loop() tick regardless of whether any new
    // GNSS data actually arrived that tick.
    void urns_sync_time_from_gnss();
  #endif
  // Must come before Display.h below - the T114 branch of draw_disp_area()
  // (Display.h) reads gnss_enabled/gnss_* accessors directly to alternate
  // the radio-parameters box with a GNSS info page.
  #include "GNSS.h"
#endif

#if HAS_DISPLAY == true
  #include "Display.h"
#else
	void display_unblank() {}
	bool display_blanked = false;
	#define DISPLAY_IS_OLED false
#endif

// Whether an I2C environment sensor (BMP280/BME280, Sensors.h) can be
// auto-detected on this board. Reuses whatever I2C bus display_init()
// (Display.h) already brought up for the board's I2C OLED panel
// (DISPLAY_IS_OLED, set just above) rather than standing up a bus of its
// own - boards with a SPI color display (T096/T114) or no display at all
// have no live I2C bus to probe, and wiring one up for them would mean
// picking new board-specific pins, out of scope for this auto-detect-only
// feature.
#ifndef HAS_SENSORS
  #define HAS_SENSORS (HAS_DISPLAY == true && DISPLAY_IS_OLED == true)
#endif
#if HAS_SENSORS == true
  #include "Sensors.h"
  void kiss_indicate_sensor();
#endif

#if HAS_URNS == true
  // Forward-declared for Provisioning.h - defined later in this file
  // (hard_reset/serial_write/escaped_serial_write) or in Utilities.h's
  // #if HAS_ESPNOW block (kiss_select_interface), same pattern as
  // kiss_indicate_sensor() above.
  void hard_reset(void);
  void serial_write(uint8_t byte);
  void escaped_serial_write(uint8_t byte);
  #if HAS_ESPNOW == true
    void kiss_select_interface(uint8_t vport);
  #endif
  #include "URNS.h"
  #if HAS_LXMF == true
    #include "Messenger.h"
  #endif
  #include "Provisioning.h"
#endif

#if HAS_BLUETOOTH == true || HAS_BLE == true
	void kiss_indicate_btpin();
#endif
#if MCU_VARIANT == MCU_ESP32 || HAS_BLUETOOTH == true || HAS_BLE == true
  #include "Bluetooth.h"
#endif

#if HAS_WIFI == true
  // Forward-declared so Remote.h's wifi_remote_available() (included next)
  // can enforce cross-transport exclusivity against the WS listener, which
  // is only fully defined afterwards, in WebSocketRemote.h.
  bool ws_host_is_connected();
  #include "Remote.h"
  #include "WebSocketRemote.h"
#endif

#if HAS_ESPNOW == true
  #include "ESPNOW.h"
#endif

#if HAS_ETHERNET == true
  #include "Ethernet.h"
#endif

#if HAS_PMU == true || IS_ESP32S3
  #include "Power.h"
#endif

#if HAS_RTC == true
  #include "RTC.h"
#endif

#if HAS_RTC == true || HAS_GPS == true
  // Unified system-time layer - RTC and GNSS.h are both already #include'd
  // by this point (whichever apply), so this can call straight into either.
  // RTC takes priority when present: a real chip is instantly live at boot,
  // no fix/connection needed, unlike GNSS which has to wait for a fix.
  // Whatever's forward-declared for these in Display.h (included earlier
  // than this point) must match - see that file's own comment.
  bool system_time_valid() {
    #if HAS_RTC == true
      if (rtc_time_valid()) return true;
    #endif
    #if HAS_GPS == true
      if (gnss_system_time_valid()) return true;
    #endif
    return false;
  }

  // Returns 0 if no time source is currently valid.
  uint32_t system_time_utc() {
    #if HAS_RTC == true
      if (rtc_time_valid()) return rtc_get_unixtime();
    #endif
    #if HAS_GPS == true
      if (gnss_system_time_valid()) return gnss_system_time_now();
    #endif
    return 0;
  }

  // Display-only UTC offset (set via tz_conf_save() below, from the RTC/
  // GNSS settings pages' shared Timezone field, Menu.h) - whichever clock
  // feeds system_time_utc() above stays strictly UTC; this offset is
  // applied only when rendering time for a human to read (Menu.h's RTC/
  // GNSS pages, Display.h's time banner). Deliberately not a full
  // timezone - no DST rules, no IANA database, just a fixed
  // minutes-from-UTC shift, per [[project note: "simple time display
  // offset" requested over full timezone support]].
  //
  // Used to live in RTC.h, RTC-exclusive - moved here since it's pure
  // EEPROM+math with no actual chip dependency, and GNSS-only boards need
  // the exact same offset. Same EEPROM address (ADDR_CONF_TZ, ROM.h) and
  // byte encoding as before, just no longer gated on HAS_RTC alone.
  //
  // Stored as raw+64 quarter-hours (not a plain signed value) so 0x00/0xFF
  // (unset/erased EEPROM) fall outside the valid range and unambiguously
  // mean "never configured" - same convention as vsr_conf_save()/
  // bvs_conf_save() (Utilities.h).
  #define TZ_OFFSET_QH_MIN    -48  // UTC-12:00
  #define TZ_OFFSET_QH_MAX     56  // UTC+14:00
  #define TZ_OFFSET_RAW_ZERO   64  // raw EEPROM byte encoding UTC+00:00 (qh=0)

  int8_t get_tz_offset_qh() {
    #if HAS_EEPROM
      uint8_t raw = EEPROM.read(eeprom_addr(ADDR_CONF_TZ));
    #elif MCU_VARIANT == MCU_NRF52
      uint8_t raw = eeprom_read(eeprom_addr(ADDR_CONF_TZ));
    #endif
    int16_t qh = (int16_t)raw - TZ_OFFSET_RAW_ZERO;
    if (qh < TZ_OFFSET_QH_MIN || qh > TZ_OFFSET_QH_MAX) return 0; // unset/erased -> UTC
    return (int8_t)qh;
  }

  // Shifts a UTC unix timestamp by the configured display offset - the
  // result is NOT a real unix time (it's "local wall-clock seconds", the
  // same trick RTC.h's own epoch/civil-calendar math already works on
  // regardless) - only ever feed it to a days_from_civil()/civil_from_days()
  // pair for display, never back into rtc_set_unixtime() or over the wire.
  uint32_t apply_tz_offset(uint32_t utc_epoch) {
    return (uint32_t)((int64_t)utc_epoch + (int64_t)get_tz_offset_qh() * 900);
  }

  // No live-apply step needed (unlike e.g. ethspd_conf_save()) - every
  // reader re-derives the offset fresh from EEPROM on each display refresh,
  // nothing to reboot or re-init.
  void tz_conf_save(uint8_t val) {
    eeprom_update(eeprom_addr(ADDR_CONF_TZ), val);
  }
#endif

#if HAS_INPUT == true
	#include "Input.h"
#endif

#if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
	#include "Device.h"
#endif

// Network OTA updates (HAS_OTA boards only, currently MeshAdventurer-S3 -
// see OTA.h's own comment) - must come after Device.h
// (dev_firmware_hash_target/device_save_firmware_hash) and after Remote.h/
// Ethernet.h above (wifi_is_connected()/eth_is_connected).
#if HAS_OTA == true
  #include "OTA.h"
#endif

#if MCU_VARIANT == MCU_ESP32
  //https://github.com/espressif/esp-idf/issues/8855
  #if BOARD_MODEL == BOARD_HELTEC32_V3
    #include "hal/wdt_hal.h"
	#elif BOARD_MODEL == BOARD_T3S3
		#include "hal/wdt_hal.h"
  #else
		#include "hal/wdt_hal.h"
	#endif
  #define ISR_VECT IRAM_ATTR
#else
  #define ISR_VECT
#endif

#if MCU_VARIANT == MCU_1284P || MCU_VARIANT == MCU_2560
	#include <avr/wdt.h>
	#include <util/atomic.h>
#endif

// Must come after ISR_VECT is defined above (Encoder.h's ISR uses it).
// Menu.h is the shared settings-menu state machine, usable with just the
// main button (HAS_MENU) or additionally with a rotary encoder
// (HAS_ENCODER) - see Boards.h.
#if HAS_MENU == true
  // Forward declarations - defined in RNode_Firmware.ino, further down than
  // this #include. Needed for the Radio submenu's Start/Stop Radio item.
  bool startRadio();
  void stopRadio();
  #include "Menu.h"
#endif
#if HAS_ENCODER == true
  #include "Encoder.h"
#endif

uint8_t boot_vector = 0x00;

#if MCU_VARIANT == MCU_1284P || MCU_VARIANT == MCU_2560
	uint8_t OPTIBOOT_MCUSR __attribute__ ((section(".noinit")));
	void resetFlagsInit(void) __attribute__ ((naked)) __attribute__ ((used)) __attribute__ ((section (".init0")));
	void resetFlagsInit(void) {
	    __asm__ __volatile__ ("sts %0, r2\n" : "=m" (OPTIBOOT_MCUSR) :);
	}
#elif MCU_VARIANT == MCU_ESP32
	// TODO: Get ESP32 boot flags
#elif MCU_VARIANT == MCU_NRF52
	// TODO: Get NRF52 boot flags
#endif

#if MCU_VARIANT == MCU_NRF52
	unsigned long get_rng_seed() {
		nrf_rng_error_correction_enable(NRF_RNG);
		nrf_rng_shorts_disable(NRF_RNG, NRF_RNG_SHORT_VALRDY_STOP_MASK);
		nrf_rng_task_trigger(NRF_RNG, NRF_RNG_TASK_START);
		while (!nrf_rng_event_check(NRF_RNG, NRF_RNG_EVENT_VALRDY));
		uint8_t rb_a = nrf_rng_random_value_get(NRF_RNG);
		nrf_rng_event_clear(NRF_RNG, NRF_RNG_EVENT_VALRDY);
		while (!nrf_rng_event_check(NRF_RNG, NRF_RNG_EVENT_VALRDY));
		uint8_t rb_b = nrf_rng_random_value_get(NRF_RNG);
		nrf_rng_event_clear(NRF_RNG, NRF_RNG_EVENT_VALRDY);
		while (!nrf_rng_event_check(NRF_RNG, NRF_RNG_EVENT_VALRDY));
		uint8_t rb_c = nrf_rng_random_value_get(NRF_RNG);
		nrf_rng_event_clear(NRF_RNG, NRF_RNG_EVENT_VALRDY);
		while (!nrf_rng_event_check(NRF_RNG, NRF_RNG_EVENT_VALRDY));
		uint8_t rb_d = nrf_rng_random_value_get(NRF_RNG);
		nrf_rng_event_clear(NRF_RNG, NRF_RNG_EVENT_VALRDY);
		nrf_rng_task_trigger(NRF_RNG, NRF_RNG_TASK_STOP);
		return rb_a << 24 | rb_b << 16 | rb_c << 8 | rb_d;
	}
#endif

#if HAS_NP == true
	#include <Adafruit_NeoPixel.h>
	#define NUMPIXELS 1
	Adafruit_NeoPixel pixels(NUMPIXELS, pin_np, NEO_GRB + NEO_KHZ800);

  uint8_t npr = 0;
  uint8_t npg = 0;
  uint8_t npb = 0;
  float npi = NP_M;
  // np_intensity (raw 0-255 form of npi) is declared further up in this
  // file, before Menu.h's #include - see that declaration's comment.
  bool pixels_started = false;

  void led_set_intensity(uint8_t intensity) {
  	npi = (float)intensity/255.0;
  	np_intensity = intensity;
  }

  void led_init() {
  	#if BOARD_MODEL == BOARD_HELTEC_T114
  		// Enable vext power supply to neopixel
  		pinMode(PIN_VEXT_EN, OUTPUT);
  		digitalWrite(PIN_VEXT_EN, HIGH);
  	#endif

    #if MCU_VARIANT == MCU_NRF52
      if (eeprom_read(eeprom_addr(ADDR_CONF_PSET)) == CONF_OK_BYTE) {
        uint8_t int_val = eeprom_read(eeprom_addr(ADDR_CONF_PINT));
        led_set_intensity(int_val);
      }
    #else
    if (EEPROM.read(eeprom_addr(ADDR_CONF_PSET)) == CONF_OK_BYTE) {
        uint8_t int_val = EEPROM.read(eeprom_addr(ADDR_CONF_PINT));
        led_set_intensity(int_val);
    }
    #endif
  }

  void npset(uint8_t r, uint8_t g, uint8_t b) {
  	if (pixels_started != true) {
  		pixels.begin();
  		pixels_started = true;
  	}

  	if (r != npr || g != npg || b != npb) {
  		npr = r; npg = g; npb = b;
  		pixels.setPixelColor(0, pixels.Color(npr*npi, npg*npi, npb*npi));
  		pixels.show();
  	}
  }

  void boot_seq() {
  	uint8_t rs[] = { 0x00, 0x00, 0x00 };
  	uint8_t gs[] = { 0x10, 0x08, 0x00 };
  	uint8_t bs[] = { 0x00, 0x08, 0x10 };
  	for (size_t i = 0; i < sizeof(rs); i++) {
	  	npset(rs[i%sizeof(rs)], gs[i%sizeof(gs)], bs[i%sizeof(bs)]);
	  	delay(33);
	  	npset(0x00, 0x00, 0x00);
	  	delay(66);
  	}
  }
#else
  void boot_seq() { }
#endif

#if HAS_BUZZER == true
  // Avoid tone()'s duration overload: its internal auto-stop timer can
  // race with the noTone() call at the end of each note in a tight loop
  // and crash the LEDC driver, so time each note manually instead.
  void buzzer_play_notes(const uint16_t *notes, uint8_t count, uint16_t note_ms) {
    if (!sound_enabled) return;
    for (uint8_t i = 0; i < count; i++) {
      tone(buzzer_pin, notes[i]);
      delay(note_ms);
      noTone(buzzer_pin);
      // noTone() only queues a TONE_END message for Tone.cpp's own async
      // tone_task (ESP32 Arduino core, a separate higher-priority
      // FreeRTOS task) to process - the actual ledcDetach() doesn't run
      // synchronously here. Touching the pin ourselves before that
      // finishes used to race it: pinMode() calling its own
      // perimanClearPinBus() while the tone_task's detach was still in
      // flight could both end up calling LEDC's detach callback on the
      // same heap-allocated channel handle, double-freeing it (confirmed
      // real crash: heap poisoning "head != NULL", multi_heap_free ->
      // ledcDetachBus -> free()). A couple of ms here is enough for that
      // higher-priority task to actually finish first, which is what
      // makes the pinMode() below safe again.
      delay(2);
      // pinMode() (not just digitalWrite()) is required, not optional -
      // once LEDC releases the pin, the peripheral manager no longer
      // considers it a GPIO at all, and __digitalWrite() (esp32-hal-
      // gpio.c) silently no-ops instead of driving the pin when that's
      // true. Dropping this call for the log-spam workaround it looked
      // like it was fixing didn't just mask a warning - it meant the LOW
      // below was silently never actually landing.
      pinMode(buzzer_pin, OUTPUT);
      digitalWrite(buzzer_pin, LOW); // see buzzer_init()
      delay(8);
    }
  }

  // Simple ascending startup jingle, played once while the boot banner is
  // shown. Called right after buzzer_init() during setup() (RNode_Firmware.
  // ino), before loop() starts - the melody task buzzer_init() just created
  // is already running by this point, but stays idle-blocked on
  // xQueueReceive(portMAX_DELAY) with nothing queued, so it doesn't touch
  // the pin while this blocking player runs. Blocking here is fine either
  // way - nothing else is competing for CPU time yet.
  void buzzer_boot_melody() {
    const uint16_t notes[] = { 1319, 1568, 1976, 2637 };
    buzzer_play_notes(notes, sizeof(notes)/sizeof(notes[0]), 80);
  }

  // Melody player for cues triggered from hot paths (button handling,
  // serial_callback(), the LXMF delivery callback). Used to be a
  // loop()-polled state machine (buzzer_update(), called once per
  // loop() iteration) - but loop() itself blocks synchronously for real
  // stretches of time in more than one place (the radio TX-done poll in
  // sx126x::endPacket(), and flash/LittleFS persistence in
  // urns_lxmf_loop()), and whichever note was mid-flight when that
  // happened just kept sounding in hardware for the whole block, since
  // noTone() only ever got called from the starved buzzer_update(). Now
  // runs as its own dedicated FreeRTOS task with its own timing
  // (xQueueReceive's own timeout), independent of whether loop() ever
  // gets a turn - the actual fix, not a race against where in loop() a
  // blocking call happens to land.
  //
  // Every buzzer_*_melody() wrapper below just posts a request onto a
  // length-1 "mailbox" queue (xQueueOverwrite - never blocks, never
  // fails, and a new request replacing an unstarted one matches this
  // subsystem's original behavior exactly, which always let a new melody
  // clobber whatever was playing). The task is the *only* code that ever
  // touches melody-playback state - no globals shared/polled across
  // tasks, unlike the old buzzer_async_* fields.
  enum class BuzzerMelodyId : uint8_t {
    BT_ON, BT_OFF, RNS_CONNECT, RNS_DISCONNECT,
    ENCODER_TICK, ENCODER_CLICK, LXMF_RX
  };

  struct BuzzerMelodyDef {
    const uint16_t *notes;
    uint8_t count;
    uint16_t note_ms;
  };

  const uint16_t BUZZER_ASYNC_GAP_MS = 10;

  QueueHandle_t g_buzzer_melody_queue = NULL;
  // Written only by buzzer_task(), read only by buzzer_wait_for_melody()
  // (called from loopTask, Messenger.h) - single-writer/single-reader,
  // same pattern already used safely for bt_pending_rns_link_state
  // (Bluetooth.h).
  volatile bool g_buzzer_task_playing = false;

  void buzzer_task(void *pvParameters) {
    static const uint16_t notes_bt_on[]         = { 1568, 1175 };
    static const uint16_t notes_bt_off[]        = { 1568, 2093 };
    static const uint16_t notes_rns_connect[]   = { 1976, 2637 };
    static const uint16_t notes_rns_disconnect[] = { 1319, 988 };
    static const uint16_t notes_encoder_tick[]  = { 500 };
    static const uint16_t notes_encoder_click[] = { 1200 };
    // Three-note alert for an inbound Messenger LXMF message (Messenger.h)
    // - deliberately distinct from every other cue here so it reads as
    // "look at the screen now", matching the app's emergency-messenger
    // purpose.
    static const uint16_t notes_lxmf_rx[]       = { 1568, 1976, 2093 };

    const BuzzerMelodyDef melodies[] = {
      /* BT_ON          */ { notes_bt_on,          2, 60 },
      /* BT_OFF         */ { notes_bt_off,         2, 60 },
      /* RNS_CONNECT    */ { notes_rns_connect,    2, 45 },
      /* RNS_DISCONNECT */ { notes_rns_disconnect, 2, 45 },
      /* ENCODER_TICK   */ { notes_encoder_tick,   1, 12 },
      /* ENCODER_CLICK  */ { notes_encoder_click,  1, 12 },
      /* LXMF_RX         */ { notes_lxmf_rx,        3, 55 },
    };

    const BuzzerMelodyDef *active = NULL;
    uint8_t index = 0;
    bool in_gap = false;

    for (;;) {
      BuzzerMelodyId id;
      TickType_t wait = active ? pdMS_TO_TICKS(in_gap ? BUZZER_ASYNC_GAP_MS : active->note_ms) : portMAX_DELAY;
      if (xQueueReceive(g_buzzer_melody_queue, &id, wait) == pdTRUE) {
        // New (or overriding) request - start its first note immediately,
        // clobbering whatever was mid-flight, matching the old
        // buzzer_start_async_melody()'s unconditional-overwrite behavior.
        active = &melodies[(uint8_t)id];
        index = 0;
        in_gap = false;
        tone(buzzer_pin, active->notes[0]);
        g_buzzer_task_playing = true;
      } else if (active) {
        // Timed out - the current note or gap boundary was reached.
        if (!in_gap) {
          #if MCU_VARIANT == MCU_ESP32
            // Silence via 0Hz (ledcWriteTone(pin,0) -> ledcWrite(pin,0),
            // esp32-hal-ledc.c) instead of noTone()+pinMode() - the latter
            // pair used to run here on every single note-to-note
            // transition, each one a fresh chance to lose the LEDC
            // async-detach race documented on buzzer_play_notes() (this
            // branch's own former vTaskDelay(BUZZER_ASYNC_SETTLE_MS) was a
            // best-effort "hope Tone.cpp's own tone_task gets scheduled in
            // time" heuristic, not a real guarantee) - confirmed live via
            // a debug-UART capture during real BLE pairing traffic:
            // "assert failed: multi_heap_free multi_heap_poisoning.c:279
            // (head != NULL)" from ledcDetachBus() -> free(), immediately
            // preceded by "noTone(): Tone is not running on given pin"
            // (Tone.cpp) - buzzer_task()'s own pinMode() had already
            // detached the LEDC channel out from under Tone.cpp's
            // tone_task before it got to process its own already-queued
            // detach of the same channel, double-freeing its
            // heap-allocated handle. Heavy concurrent NimBLE host-task
            // activity during a live pairing handshake made that
            // scheduling window (previously narrow enough to go
            // unnoticed) wide enough to actually lose. tone(pin,0) writes
            // the same message queue Tone.cpp's own tone() would but
            // never detaches the channel - duty 0 holds the pin actively
            // low (not floating, so no self-oscillation risk either)
            // without ever touching the peripheral manager from this
            // task, so there's no longer a second party to race at all,
            // not just a smaller window. ESP32-only: nRF52's tone()/
            // noTone() (Adafruit core, PWM peripheral, not LEDC) don't
            // share this specific hazard and were never observed to hit
            // it, so that side keeps the original noTone()+pinMode()
            // sequence rather than assuming an unverified freq=0 meaning
            // there too.
            tone(buzzer_pin, 0);
          #else
            noTone(buzzer_pin);
            pinMode(buzzer_pin, OUTPUT);
            digitalWrite(buzzer_pin, LOW);
          #endif
          in_gap = true;
        } else {
          index++;
          if (index >= active->count) {
            active = NULL;
            g_buzzer_task_playing = false;
          } else {
            tone(buzzer_pin, active->notes[index]);
            in_gap = false;
          }
        }
      }
    }
  }

  void buzzer_request_melody(BuzzerMelodyId id) {
    if (!sound_enabled || !g_buzzer_melody_queue) return;
    xQueueOverwrite(g_buzzer_melody_queue, &id);
  }

  // buzzer_pin itself is declared earlier in this file - see the comment
  // there.
  void buzzer_init() {
    pinMode(buzzer_pin, OUTPUT);
    noTone(buzzer_pin);
    // On at least the nRF52 core, noTone() disconnects the PWM peripheral
    // from the pin (PSEL.OUT -> NOT_CONNECTED) but never explicitly drives
    // it LOW afterward - the pin is left wherever the GPIO peripheral's own
    // OUT bit happens to be, not guaranteed low. A passive piezo element
    // left on an undriven/floating gate can self-oscillate from its own
    // vibration-induced feedback voltage (silenced by physically damping
    // the resonance, e.g. tapping it) - force a real, known LOW here so
    // the gate is always actively held off, not just released.
    digitalWrite(buzzer_pin, LOW);

    #if MCU_VARIANT == MCU_NRF52
      // Belt-and-suspenders on top of the digitalWrite(LOW) above: keep the
      // SoC's own weak pull-down enabled on this pin even while it's an
      // output (nRF52's PIN_CNF register allows DIR and PULL to be set
      // independently, unlike MCUs where pull config only applies in INPUT
      // mode) - so the gate has a defined level even during windows
      // nothing is actively driving it at all (e.g. the PWM-disconnect gap
      // above, or before setup() ever reaches this point). Arduino's
      // pinMode()/digitalWrite() translate the sketch-facing pin number to
      // the SoC's native flat P0.xx/P1.xx numbering via g_ADigitalPinMap
      // before touching hardware - the low-level nrf_gpio_cfg() call below
      // doesn't do that translation itself, so it's done explicitly here.
      nrf_gpio_cfg(
        g_ADigitalPinMap[buzzer_pin],
        NRF_GPIO_PIN_DIR_OUTPUT,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        NRF_GPIO_PIN_PULLDOWN,
        NRF_GPIO_PIN_S0S1,
        NRF_GPIO_PIN_NOSENSE
      );
    #endif

    #if BOARD_MODEL == BOARD_HELTEC_T1
      // T1's piezo buzzer sits behind a voltage-doubler circuit gated by
      // two extra pins (Meshtastic's/MeshCore's variant.h:
      // PIN_BUZZER_VOLTAGE_MULTIPLIER_1/2) - both held HIGH permanently to
      // enable the boost, never toggled again afterward (matches
      // MeshCore's own T1Board::begin(), the only reference driver that
      // actually does anything with these two pins).
      pinMode(PIN_T1_BUZZER_MULT1, OUTPUT);
      pinMode(PIN_T1_BUZZER_MULT2, OUTPUT);
      digitalWrite(PIN_T1_BUZZER_MULT1, HIGH);
      digitalWrite(PIN_T1_BUZZER_MULT2, HIGH);
    #endif

    // Queue+task created last, after every pin/hardware quirk above is
    // settled - the task immediately blocks on xQueueReceive(portMAX_DELAY)
    // and touches nothing until the first real request arrives, so this
    // can't race buzzer_boot_melody() (called right after this returns,
    // still fully blocking/synchronous - see its own comment).
    g_buzzer_melody_queue = xQueueCreate(1, sizeof(BuzzerMelodyId));
    // Stack: buzzer_task()'s own call depth is trivial (tone()/noTone()/
    // vTaskDelay()/xQueueReceive() - shallow leaf calls, nothing like the
    // multi-frame dive into WebSockets/lwIP that undersized kiss_tx_task's
    // old 2048B stack) - 3072 is a comfortable margin, not a tight fit.
    // Priority: loopTask's own priority + 1 (1 on both ESP32 and nRF52),
    // so this can preempt a loopTask stuck in a blocking call - still well
    // under ESP32's own tone()/noTone() async worker (priority 10) and
    // nRF52's Bluefruit task (priority 3).
    #if MCU_VARIANT == MCU_ESP32
      // Pinned to core 0, matching the codebase's one other live
      // xTaskCreatePinnedToCore precedent (LXStamper.cpp's stamp_worker_
      // task) - leaves core 1 (loopTask, radio/SPI work) undisturbed.
      xTaskCreatePinnedToCore(buzzer_task, "buzzer", 3072, NULL, 2, NULL, 0);
    #elif MCU_VARIANT == MCU_NRF52
      xTaskCreate(buzzer_task, "buzzer", 3072, NULL, TASK_PRIO_NORMAL, NULL);
    #endif
  }

  // Short two-note cues for Bluetooth toggling via the user button.
  void buzzer_bt_on_melody()  { buzzer_request_melody(BuzzerMelodyId::BT_ON); }
  void buzzer_bt_off_melody() { buzzer_request_melody(BuzzerMelodyId::BT_OFF); }

  // Short chirps for the RNS host (rns_link_state) attaching to / leaving the KISS interface.
  void buzzer_rns_connect_melody()    { buzzer_request_melody(BuzzerMelodyId::RNS_CONNECT); }
  void buzzer_rns_disconnect_melody() { buzzer_request_melody(BuzzerMelodyId::RNS_DISCONNECT); }

  // Single-note, very short ticks for encoder feedback - deliberately
  // shorter than the other cues so rapid rotation doesn't get annoying.
  void buzzer_encoder_tick_melody()  { buzzer_request_melody(BuzzerMelodyId::ENCODER_TICK); }
  void buzzer_encoder_click_melody() { buzzer_request_melody(BuzzerMelodyId::ENCODER_CLICK); }

  void buzzer_lxmf_rx_melody() { buzzer_request_melody(BuzzerMelodyId::LXMF_RX); }

  // Lets whichever melody the encoder/button confirm-click already
  // started (menu_encoder_button()/menu_button_press(), Menu.h - fires
  // unconditionally on every confirm, before the specific action itself
  // runs) finish naturally before a caller goes on to do something
  // blocking of its own (a LoRa TX, a flash read). Bounded with a max
  // wait since this now depends on the buzzer task's own health rather
  // than driving the state to completion itself - the confirm click is
  // only a couple of ~12ms ticks (buzzer_encoder_click_melody()), so
  // 500ms is generous headroom, not a real-world limit.
  void buzzer_wait_for_melody() {
    unsigned long start = millis();
    while (g_buzzer_task_playing && millis() - start < 500) { delay(1); }
  }
#else
  void buzzer_init() { }
  void buzzer_bt_on_melody() { }
  void buzzer_bt_off_melody() { }
  void buzzer_boot_melody() { }
  void buzzer_rns_connect_melody() { }
  void buzzer_rns_disconnect_melody() { }
  void buzzer_encoder_tick_melody() { }
  void buzzer_encoder_click_melody() { }
  void buzzer_lxmf_rx_melody() { }
  void buzzer_wait_for_melody() { }
#endif

// Centralises rns_link_state transitions so the RNS connect/disconnect chirp
// only fires on the actual edge, not on every KISS byte that touches rns_link_state.
void set_rns_link_state(uint8_t new_state) {
  if (new_state != rns_link_state) {
    if (new_state == RNS_LINK_STATE_CONNECTED)         { buzzer_rns_connect_melody(); }
    else if (new_state == RNS_LINK_STATE_DISCONNECTED) { buzzer_rns_disconnect_melody(); }
  }
  rns_link_state = new_state;
}

// TX/RX LEDs should work when display is blanked on externally powered nodes
#if BOARD_MODEL == BOARD_MESHPOE_S3 || BOARD_MODEL == BOARD_MESHADVENTURER_S3
  #define LED_DISPLAY_BLANKED false
#else
  #define LED_DISPLAY_BLANKED display_blanked
#endif

#if MCU_VARIANT == MCU_1284P || MCU_VARIANT == MCU_2560
	void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
	void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
	void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
	void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
	void led_id_on()  { }
	void led_id_off() { }
#elif MCU_VARIANT == MCU_ESP32
	#if HAS_NP == true
		void led_rx_on()  { npset(0, 0xFF, 0); }
		void led_rx_off() {	npset(0, 0, 0); }
		void led_tx_on()  { npset(0, 0, 0xFF); }
		void led_tx_off() { npset(0, 0, 0); }
		void led_id_on()  { npset(0x90, 0, 0x70); }
		void led_id_off() { npset(0, 0, 0); }
	#elif BOARD_MODEL == BOARD_RNODE_NG_20
		void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
		void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
		void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
		void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
		void led_id_on()  { }
		void led_id_off() { }
	#elif BOARD_MODEL == BOARD_RNODE_NG_21
		void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
		void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
		void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
		void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
		void led_id_on()  { }
		void led_id_off() { }
	#elif BOARD_MODEL == BOARD_T3S3
		void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
		void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
		void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
		void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
		void led_id_on()  { }
		void led_id_off() { }
	#elif BOARD_MODEL == BOARD_TBEAM
		void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
		void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
		void led_tx_on()  { digitalWrite(pin_led_tx, LOW); }
		void led_tx_off() { digitalWrite(pin_led_tx, HIGH); }
		void led_id_on()  { }
		void led_id_off() { }
	#elif BOARD_MODEL == BOARD_TDECK
		void led_rx_on()  { }
		void led_rx_off() {	}
		void led_tx_on()  { }
		void led_tx_off() { }
		void led_id_on()  { }
		void led_id_off() { }
	#elif BOARD_MODEL == BOARD_TBEAM_S_V1
		void led_rx_on()  { }
		void led_rx_off() {	}
		void led_tx_on()  { }
		void led_tx_off() { }
		void led_id_on()  { }
		void led_id_off() { }
	#elif BOARD_MODEL == BOARD_TBEAM_S_V3
		void led_rx_on()  { }
		void led_rx_off() {	}
		void led_tx_on()  { }
		void led_tx_off() { }
		void led_id_on()  { }
		void led_id_off() { }
	#elif BOARD_MODEL == BOARD_TBEAM_1W
		void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
		void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
		void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
		void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
		void led_id_on()  { }
		void led_id_off() { }
	#elif BOARD_MODEL == BOARD_LORA32_V1_0
		#if defined(EXTERNAL_LEDS)
			void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
			void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
			void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
			void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
			void led_id_on()  { }
			void led_id_off() { }
		#else
			void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
			void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
			void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
			void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
			void led_id_on()  { }
			void led_id_off() { }
		#endif
	#elif BOARD_MODEL == BOARD_LORA32_V2_0
		#if defined(EXTERNAL_LEDS)
			void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
			void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
			void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
			void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
			void led_id_on()  { }
			void led_id_off() { }
		#else
			void led_rx_on()  { digitalWrite(pin_led_rx, LOW); }
			void led_rx_off() {	digitalWrite(pin_led_rx, HIGH); }
			void led_tx_on()  { digitalWrite(pin_led_tx, LOW); }
			void led_tx_off() { digitalWrite(pin_led_tx, HIGH); }
			void led_id_on()  { }
			void led_id_off() { }
		#endif
	#elif BOARD_MODEL == BOARD_HELTEC32_V2
		#if defined(EXTERNAL_LEDS)
			void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
			void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
			void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
			void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
			void led_id_on()  { }
			void led_id_off() { }
		#else
			void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
			void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
			void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
			void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
			void led_id_on()  { }
			void led_id_off() { }
		#endif
	#elif BOARD_MODEL == BOARD_HELTEC32_V3
			void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
			void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
			void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
			void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
			void led_id_on()  { }
			void led_id_off() { }
	#elif BOARD_MODEL == BOARD_HELTEC32_V4
			void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
			void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
			void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
			void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
			void led_id_on()  { }
			void led_id_off() { }
	#elif BOARD_MODEL == BOARD_LORA32_V2_1
		void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
		void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
		void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
		void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
		void led_id_on()  { }
		void led_id_off() { }
  #elif BOARD_MODEL == BOARD_XIAO_S3
		void led_rx_on()  { digitalWrite(pin_led_rx, LED_ON); }
		void led_rx_off() { digitalWrite(pin_led_rx, LED_OFF); }
		void led_tx_on()  { digitalWrite(pin_led_tx, LED_ON); }
		void led_tx_off() { digitalWrite(pin_led_tx, LED_OFF); }
		void led_id_on()  { }
		void led_id_off() { }
	#elif BOARD_MODEL == BOARD_HUZZAH32
		void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
		void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
		void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
		void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
		void led_id_on()  { }
		void led_id_off() { }
	#elif BOARD_MODEL == BOARD_GENERIC_ESP32
		void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
		void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
		void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
		void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
		void led_id_on()  { }
		void led_id_off() { }
	#elif BOARD_MODEL == BOARD_MESHADVENTURER_S3
		void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
		void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
		void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
		void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
		void led_id_on()  { }
		void led_id_off() { }
	#elif BOARD_MODEL == BOARD_MESHADVENTURER
		void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
		void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
		void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
		void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
		void led_id_on()  { }
		void led_id_off() { }
	#elif BOARD_MODEL == BOARD_DIY_V1
		void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
		void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
		void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
		void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
		void led_id_on()  { }
		void led_id_off() { }
	#elif BOARD_MODEL == BOARD_AETHERNODE
		void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
		void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
		void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
		void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
		void led_id_on()  { }
		void led_id_off() { }
	#elif BOARD_MODEL == BOARD_AETHERNODE_S3
		void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
		void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
		void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
		void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
		void led_id_on()  { }
		void led_id_off() { }
	#elif BOARD_MODEL == BOARD_MESHPOE_S3
		void led_rx_on()  { }
		void led_rx_off() { }
		void led_tx_on()  { }
		void led_tx_off() { }
		void led_id_on()  { }
		void led_id_off() { }
	#elif BOARD_MODEL == BOARD_HELTEC_WTRACKER_V2
		// Same active-HIGH convention as BOARD_HELTEC_T096 (Utilities.h,
		// MCU_NRF52 branch below) - neither Meshtastic's nor MeshCore's
		// reference variant.h for this board states its LED's active level,
		// assumed same as T096 pending real hardware to verify (worst case
		// it's just inverted, not damaging).
		void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
		void led_rx_off() { digitalWrite(pin_led_rx, LOW); }
		void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
		void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
		void led_id_on()  { }
		void led_id_off() { }
	#endif
#elif MCU_VARIANT == MCU_NRF52
    #if HAS_NP == true
      void led_rx_on()  { npset(0, 0xFF, 0); }
      void led_rx_off() {	npset(0, 0, 0); }
      void led_tx_on()  { npset(0, 0, 0xFF); }
      void led_tx_off() { npset(0, 0, 0); }
			void led_id_on()  { npset(0x90, 0, 0x70); }
			void led_id_off() { npset(0, 0, 0); }
    #elif BOARD_MODEL == BOARD_RAK4631 || BOARD_MODEL == BOARD_RAK3401
		void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
		void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
		void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
		void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
		void led_id_on()  { }
		void led_id_off() { }
  #elif BOARD_MODEL == BOARD_PROMICRO
	void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
	void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
	void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
	void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
	void led_id_on()  { }
	void led_id_off() { }
  #elif BOARD_MODEL == BOARD_HELTEC_T114 || BOARD_MODEL == BOARD_HELTEC_T1
    // Heltec T114 pulls pins LOW to turn on - same polarity as T1's LED
    // (both reference variant.h's for T1: LED_STATE_ON=0/LOW), unlike
    // T096's active-HIGH LED below.
    void led_rx_on()  { digitalWrite(pin_led_rx, LOW); }
    void led_rx_off() {	digitalWrite(pin_led_rx, HIGH); }
    void led_tx_on()  { digitalWrite(pin_led_tx, LOW); }
    void led_tx_off() { digitalWrite(pin_led_tx, HIGH); }
		void led_id_on()  { }
		void led_id_off() { }
  #elif BOARD_MODEL == BOARD_HELTEC_T096
    // Heltec T096 pulls pins HIGH to turn on
    void led_rx_on()  { digitalWrite(pin_led_rx, HIGH); }
    void led_rx_off() {	digitalWrite(pin_led_rx, LOW); }
    void led_tx_on()  { digitalWrite(pin_led_tx, HIGH); }
    void led_tx_off() { digitalWrite(pin_led_tx, LOW); }
		void led_id_on()  { }
		void led_id_off() { }
  #elif BOARD_MODEL == BOARD_TECHO
		void led_rx_on()  { digitalWrite(pin_led_rx, LED_ON); }
		void led_rx_off() {	digitalWrite(pin_led_rx, LED_OFF); }
		void led_tx_on()  { digitalWrite(pin_led_tx, LED_ON); }
		void led_tx_off() { digitalWrite(pin_led_tx, LED_OFF); }
		void led_id_on()  { }
		void led_id_off() { }
	#endif
#endif

void hard_reset(void) {
	#if MCU_VARIANT == MCU_1284P || MCU_VARIANT == MCU_2560
		wdt_enable(WDTO_15MS);
		while(true) {
			led_tx_on(); led_rx_off();
		}
	#elif MCU_VARIANT == MCU_ESP32
		ESP.restart();
	#elif MCU_VARIANT == MCU_NRF52
    NVIC_SystemReset();
	#endif
}

// LED Indication: Error
void led_indicate_error(int cycles) {
	#if HAS_NP == true
		bool forever = (cycles == 0) ? true : false;
		cycles = forever ? 1 : cycles;
		while(cycles > 0) {
			npset(0xFF, 0x00, 0x00);
			delay(100);
			npset(0xFF, 0x50, 0x00);
			delay(100);
			if (!forever) cycles--;
		}
		npset(0,0,0);
	#else
		bool forever = (cycles == 0) ? true : false;
		cycles = forever ? 1 : cycles;
		while(cycles > 0) {
	        digitalWrite(pin_led_rx, HIGH);
	        digitalWrite(pin_led_tx, LOW);
	        delay(100);
	        digitalWrite(pin_led_rx, LOW);
	        digitalWrite(pin_led_tx, HIGH);
	        delay(100);
	        if (!forever) cycles--;
	    }
	    led_rx_off();
	    led_tx_off();
	#endif
}

// LED Indication: Airtime Lock
void led_indicate_airtime_lock() {
	#if HAS_NP == true
		npset(32,0,2);
	#endif
}

// LED Indication: Boot Error
void led_indicate_boot_error() {
	#if HAS_NP == true
		while(true) {
			npset(0xFF, 0xFF, 0xFF);
		}
	#else
		while (true) {
		    led_tx_on();
		    led_rx_off();
		    delay(10);
		    led_rx_on();
		    led_tx_off();
		    delay(5);
		}
	#endif
}

// LED Indication: Warning
void led_indicate_warning(int cycles) {
	#if HAS_NP == true
		bool forever = (cycles == 0) ? true : false;
		cycles = forever ? 1 : cycles;
		while(cycles > 0) {
			npset(0xFF, 0x50, 0x00);
			delay(100);
			npset(0x00, 0x00, 0x00);
			delay(100);
			if (!forever) cycles--;
		}
		npset(0,0,0);
	#else
		bool forever = (cycles == 0) ? true : false;
		cycles = forever ? 1 : cycles;
		digitalWrite(pin_led_tx, HIGH);
		while(cycles > 0) {
      led_tx_off();
      delay(100);
      led_tx_on();
      delay(100);
      if (!forever) cycles--;
    }
    led_tx_off();
	#endif
}

// LED Indication: Info
#if MCU_VARIANT == MCU_1284P || MCU_VARIANT == MCU_2560
	void led_indicate_info(int cycles) {
		bool forever = (cycles == 0) ? true : false;
		cycles = forever ? 1 : cycles;
		while(cycles > 0) {
	    led_rx_off();
	    delay(100);
	    led_rx_on();
	    delay(100);
	    if (!forever) cycles--;
	  }
	  led_rx_off();
	}
#elif MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
	#if HAS_NP == true
		void led_indicate_info(int cycles) {
			bool forever = (cycles == 0) ? true : false;
			cycles = forever ? 1 : cycles;
			while(cycles > 0) {
		    npset(0x00, 0x00, 0xFF);
  			delay(100);
  			npset(0x00, 0x00, 0x00);
  			delay(100);
  			if (!forever) cycles--;
		  }
		  npset(0,0,0);
		}
	#elif BOARD_MODEL == BOARD_LORA32_V2_1
		void led_indicate_info(int cycles) {
			bool forever = (cycles == 0) ? true : false;
			cycles = forever ? 1 : cycles;
			while(cycles > 0) {
		    led_rx_off();
		    delay(100);
		    led_rx_on();
		    delay(100);
		    if (!forever) cycles--;
		  }
		  led_rx_off();
		}
	#elif BOARD_MODEL == BOARD_LORA32_V2_0
		void led_indicate_info(int cycles) {
			bool forever = (cycles == 0) ? true : false;
			cycles = forever ? 1 : cycles;
			while(cycles > 0) {
		    led_rx_off();
		    delay(100);
		    led_rx_on();
		    delay(100);
		    if (!forever) cycles--;
		  }
		  led_rx_off();
		}
	#elif BOARD_MODEL == BOARD_TECHO
		void led_indicate_info(int cycles) {
			bool forever = (cycles == 0) ? true : false;
			cycles = forever ? 1 : cycles;
			while(cycles > 0) {
		    led_rx_off();
		    delay(100);
		    led_rx_on();
		    delay(100);
		    if (!forever) cycles--;
		  }
		  led_rx_off();
		}
	#else
		void led_indicate_info(int cycles) {
			bool forever = (cycles == 0) ? true : false;
			cycles = forever ? 1 : cycles;
			while(cycles > 0) {
		    led_tx_off();
		    delay(100);
		    led_tx_on();
		    delay(100);
		    if (!forever) cycles--;
		  }
		  led_tx_off();
		}
	#endif
#endif


unsigned long led_standby_ticks = 0;
#if MCU_VARIANT == MCU_1284P || MCU_VARIANT == MCU_2560
	uint8_t led_standby_min = 1;
	uint8_t led_standby_max = 40;
	unsigned long led_standby_wait = 11000;

#elif MCU_VARIANT == MCU_ESP32

	#if HAS_NP == true
		int led_standby_lng = 200;
		// Fade width in raw units - since led_standby_step is already at
		// its floor of 1 (Utilities.h, led_indicate_standby()), this is
		// also exactly the number of distinct brightness levels crossed
		// during the fade (and, since it feeds the NeoPixel channels
		// directly via intensity/3 below with no separate normalization,
		// also the peak brightness) - back at 100, not widened, so peak
		// brightness stays as it originally was. Slowdowns from here go
		// through led_standby_wait instead, which only affects timing.
		int led_standby_cut = 100;
		// Hold time at each extreme (fully off below led_standby_lng, fully
		// bright above led_standby_lng+led_standby_cut) is however far
		// min/max sit outside that 100-unit fade window - was 200 units off
		// + 275 units on, versus only 100 units for the actual fade, i.e.
		// the LED spent most of each cycle sitting at the extremes rather
		// than visibly transitioning. Narrowed to a short 30-unit hold on
		// each side instead, fade width/speed (led_standby_cut, the step
		// size below) unchanged.
		#define LED_STANDBY_HOLD 30
		int led_standby_min = led_standby_lng - LED_STANDBY_HOLD;
		int led_standby_max = led_standby_lng + led_standby_cut + LED_STANDBY_HOLD;
		int led_notready_min = 0;
		int led_notready_max = led_standby_max;
		int led_notready_value = led_notready_min;
		int8_t  led_notready_direction = 0;
		unsigned long led_notready_ticks = 0;
		// Ramp advances led_standby_step units every led_standby_wait loop()
		// calls - only a 100-unit window in the middle of the full
		// 0..led_standby_max range actually changes visible intensity (the
		// led_standby_lng/led_standby_cut clamp below), the rest is a
		// deliberate hold at fully-off/fully-on, so both the tick rate and
		// the step size need to be large enough that crossing the full
		// range (hold + fade + hold) doesn't take minutes at whatever rate
		// loop() actually iterates at (observed ~30Hz on real hardware -
		// confirmed via wait=60 alone measuring out to ~2s/step, far slower
		// than intended) - but too large a step (confirmed live: step=8
		// gave only ~12 distinct brightness levels across the 100-unit fade
		// window, visibly choppy/near-binary) trades away the smoothness of
		// the fade itself. wait=1 (tick every loop() call, the fastest this
		// can go without changing loop()'s own rate) + a small step keeps
		// the fade at a fine 100 distinct levels (wait is already at its
		// floor of 1 - one tick per loop() call - so step is the only knob
		// left to slow this down further without reducing granularity).
		// step is now also at its floor (1 - can't go lower without
		// skipping levels), so further slowdowns go back to wait instead -
		// doesn't cost any granularity either, just holds each of the same
		// 100 distinct levels for longer (no brightness-ceiling side effect
		// either, unlike widening led_standby_cut). 7 is ~20% slower than 6.
		unsigned long led_standby_wait = 7;
		int led_standby_step = 1;
		unsigned long led_console_wait = 1;
		unsigned long led_notready_wait = 200;
	
	#else
		uint8_t led_standby_min = 200;
		uint8_t led_standby_max = 255;
		uint8_t led_notready_min = 0;
		uint8_t led_notready_max = 255;
		uint8_t led_notready_value = led_notready_min;
		int8_t  led_notready_direction = 0;
		unsigned long led_notready_ticks = 0;
		unsigned long led_standby_wait = 1768;
		unsigned long led_notready_wait = 150;
	#endif

#elif MCU_VARIANT == MCU_NRF52
        int led_standby_lng = 200;
        int led_standby_cut = 100;
		uint8_t led_standby_min = 200;
		uint8_t led_standby_max = 255;
		uint8_t led_notready_min = 0;
		uint8_t led_notready_max = 255;
		uint8_t led_notready_value = led_notready_min;
		int8_t  led_notready_direction = 0;
		unsigned long led_notready_ticks = 0;
		unsigned long led_standby_wait = 1768;
		// Referenced by the shared led_indicate_standby() body below
		// (MCU_ESP32 || MCU_NRF52) - step=1 matches this board's original,
		// untuned-tonight behavior (single-unit increments, same as before
		// led_standby_step existed at all).
		int led_standby_step = 1;
		unsigned long led_notready_wait = 150;
#endif

unsigned long led_standby_value = led_standby_min;
int8_t  led_standby_direction = 0;

#if MCU_VARIANT == MCU_1284P || MCU_VARIANT == MCU_2560
	void led_indicate_standby() {
		led_standby_ticks++;
		if (led_standby_ticks > led_standby_wait) {
			led_standby_ticks = 0;
			if (led_standby_value <= led_standby_min) {
				led_standby_direction = 1;
			} else if (led_standby_value >= led_standby_max) {
				led_standby_direction = -1;
			}
			led_standby_value += led_standby_direction;
			analogWrite(pin_led_rx, led_standby_value);
			led_tx_off();
		}
	}

#elif MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
	#if HAS_NP == true
		void led_indicate_standby() {
			led_standby_ticks++;

			if (led_standby_ticks > led_standby_wait) {
				led_standby_ticks = 0;

				if (led_standby_value <= led_standby_min) {
					led_standby_direction = 1;
				} else if (led_standby_value >= led_standby_max) {
					led_standby_direction = -1;
				}

				uint8_t led_standby_intensity;
				led_standby_value += led_standby_direction * led_standby_step;
				int led_standby_ti = led_standby_value - led_standby_lng;

				if (led_standby_ti < 0) {
					led_standby_intensity = 0;
				} else if (led_standby_ti > led_standby_cut) {
					led_standby_intensity = led_standby_cut;
				} else {
					led_standby_intensity = led_standby_ti;
				}
  			npset(led_standby_intensity/3, led_standby_intensity/3, led_standby_intensity/3);
			}
		}

		void led_indicate_console() {
			npset(0x60, 0x00, 0x60);
			// led_standby_ticks++;

			// if (led_standby_ticks > led_console_wait) {
			// 	led_standby_ticks = 0;
				
			// 	if (led_standby_value <= led_standby_min) {
			// 		led_standby_direction = 1;
			// 	} else if (led_standby_value >= led_standby_max) {
			// 		led_standby_direction = -1;
			// 	}

			// 	uint8_t led_standby_intensity;
			// 	led_standby_value += led_standby_direction;
			// 	int led_standby_ti = led_standby_value - led_standby_lng;

			// 	if (led_standby_ti < 0) {
			// 		led_standby_intensity = 0;
			// 	} else if (led_standby_ti > led_standby_cut) {
			// 		led_standby_intensity = led_standby_cut;
			// 	} else {
			// 		led_standby_intensity = led_standby_ti;
			// 	}
  	// 		npset(led_standby_intensity, 0x00, led_standby_intensity);
			// }
		}

	#else
		void led_indicate_standby() {
			led_standby_ticks++;
			if (led_standby_ticks > led_standby_wait) {
				led_standby_ticks = 0;
				if (led_standby_value <= led_standby_min) {
					led_standby_direction = 1;
				} else if (led_standby_value >= led_standby_max) {
					led_standby_direction = -1;
				}
				led_standby_value += led_standby_direction;
				if (led_standby_value > 253) {
					#if BOARD_MODEL == BOARD_TECHO
						led_rx_on();
					#else
						led_tx_on();
					#endif
				} else {
					#if BOARD_MODEL == BOARD_TECHO
						led_rx_off();
					#else
						led_tx_off();
					#endif
				}
				#if BOARD_MODEL == BOARD_LORA32_V2_1
					#if defined(EXTERNAL_LEDS)
						led_rx_off();
					#endif
				#elif BOARD_MODEL == BOARD_LORA32_V2_0
					#if defined(EXTERNAL_LEDS)
						led_rx_off();
					#endif
				#else
					led_rx_off();
				#endif
			}
		}

		void led_indicate_console() {
			led_indicate_standby();
		}
  #endif
#endif

#if MCU_VARIANT == MCU_1284P || MCU_VARIANT == MCU_2560
	void led_indicate_not_ready() {
		led_standby_ticks++;
		if (led_standby_ticks > led_standby_wait) {
			led_standby_ticks = 0;
			if (led_standby_value <= led_standby_min) {
				led_standby_direction = 1;
			} else if (led_standby_value >= led_standby_max) {
				led_standby_direction = -1;
			}
			led_standby_value += led_standby_direction;
			analogWrite(pin_led_tx, led_standby_value);
			led_rx_off();
		}
	}
#elif MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
	#if HAS_NP == true
    void led_indicate_not_ready() {
    	led_standby_ticks++;

			if (led_standby_ticks > led_notready_wait) {
				led_standby_ticks = 0;
				
				if (led_standby_value <= led_standby_min) {
					led_standby_direction = 1;
				} else if (led_standby_value >= led_standby_max) {
					led_standby_direction = -1;
				}

				uint8_t led_standby_intensity;
				led_standby_value += led_standby_direction;
				int led_standby_ti = led_standby_value - led_standby_lng;

				if (led_standby_ti < 0) {
					led_standby_intensity = 0;
				} else if (led_standby_ti > led_standby_cut) {
					led_standby_intensity = led_standby_cut;
				} else {
					led_standby_intensity = led_standby_ti;
				}

  			npset(led_standby_intensity, 0x00, 0x00);
			}
		}
	#else
		void led_indicate_not_ready() {
			led_notready_ticks++;
			if (led_notready_ticks > led_notready_wait) {
				led_notready_ticks = 0;
				if (led_notready_value <= led_notready_min) {
					led_notready_direction = 1;
				} else if (led_notready_value >= led_notready_max) {
					led_notready_direction = -1;
				}
				led_notready_value += led_notready_direction;
				if (led_notready_value > 128) {
					led_tx_on();
				} else {
					led_tx_off();
				}
				#if BOARD_MODEL == BOARD_LORA32_V2_1
					#if defined(EXTERNAL_LEDS)
						led_rx_off();
					#endif
				#elif BOARD_MODEL == BOARD_LORA32_V2_0
					#if defined(EXTERNAL_LEDS)
						led_rx_off();
					#endif
				#else
					led_rx_off();
				#endif
			}
		}
	#endif
#endif

// Non-blocking host write to the native USB-CDC KISS port. Root cause of the
// MeshPoE-S3 global tick-death (2026-08-17): the native USB-CDC Serial.write()
// blocks indefinitely when its TX FIFO is full and no host is draining the
// port. Since kiss_tx now runs on loopTask (RNode_Firmware.ino, folded off its
// old dedicated task), that hang stops loopTask feeding the FreeRTOS tick,
// killing scheduling device-wide (INT_WDT/TASK_WDT reset at a random
// checkpoint). Worst on ARDUINO_USB_MODE=1 (hardware USB-Serial/JTAG CDC)
// boards like MeshPoE-S3 - which is exactly why it crashes there but not on
// MeshAdventurer-S3 (TinyUSB CDC, times out instead of blocking) despite
// identical code. Guard: write only when the FIFO has room; if it stays full
// past a short bound, latch "host gone" and drop bytes instantly until it
// drains again. A standalone node with nothing reading its KISS port thus
// never hangs, while an actively-reading host still gets whole frames.
#if MCU_VARIANT == MCU_ESP32
static bool kiss_host_tx_stalled = false;
#endif
static inline void kiss_serial_put(uint8_t byte) {
	#if MCU_VARIANT == MCU_ESP32
		if (Serial.availableForWrite() < 1) {
			if (kiss_host_tx_stalled) { return; }
			uint32_t start = millis();
			while (Serial.availableForWrite() < 1) {
				if ((millis() - start) >= 5) { kiss_host_tx_stalled = true; return; }
			}
		}
		kiss_host_tx_stalled = false;
	#endif
	Serial.write(byte);
}

void serial_write(uint8_t byte) {
	#if HAS_BLUETOOTH || HAS_BLE == true
		if (bt_state != BT_STATE_CONNECTED) {
			#if HAS_ETHERNET
				if (eth_is_connected && wifi_host_is_connected()) { connection.write(byte); }
				#if HAS_WIFI
				else if (wifi_host_is_connected()) { wifi_remote_write(byte); }
				else if (ws_host_is_connected())   { ws_remote_write(byte); }
				#endif
				else                               { kiss_serial_put(byte); }
			#elif HAS_WIFI
				if (wifi_host_is_connected())      { wifi_remote_write(byte); }
				else if (ws_host_is_connected())   { ws_remote_write(byte); }
				else                               { kiss_serial_put(byte); }
			#else
				kiss_serial_put(byte);
			#endif
		} else {
			SerialBT.write(byte);
      #if MCU_VARIANT == MCU_NRF52 && HAS_BLE
	      // This ensures that the TX buffer is flushed after a frame is queued in serial.
	      // serial_in_frame is used to ensure that the flush only happens at the end of the frame
	      if (serial_in_frame && byte == FEND) { SerialBT.flushTXD(); serial_in_frame = false; }
	      else if (!serial_in_frame && byte == FEND) { serial_in_frame = true; }
      #elif MCU_VARIANT == MCU_ESP32 && HAS_BLE
	      // Same reasoning as the NRF52/Bluefruit case above, and just as
	      // necessary here: without this, multiple KISS replies produced in
	      // quick succession (faster than BLE_FLUSH_TIMEOUT's periodic
	      // update_bt() flush) get buffered together into a single BLE
	      // notification. Windows never negotiates the BLE MTU above its
	      // 23-byte default over this stack (confirmed live - bleak's WinRT
	      // backend has no client-side way to request a larger one either),
	      // leaving only 20 usable payload bytes per notification. A host
	      // sending several small commands back-to-back (e.g. Reticulum's
	      // RNodeInterface combined CMD_DETECT+CMD_FW_VERSION+CMD_PLATFORM+
	      // CMD_MCU probe) can easily produce a combined reply at or over
	      // that limit, silently failing the whole notify() - including the
	      // CMD_DETECT reply that would have fit fine on its own. Flushing
	      // per-frame instead keeps every single BLE notification within one
	      // KISS frame's own (much smaller) size.
	      if (serial_in_frame && byte == FEND) { SerialBT.flush(); serial_in_frame = false; }
	      else if (!serial_in_frame && byte == FEND) { serial_in_frame = true; }
      #endif
		}
	#else
		kiss_serial_put(byte);
	#endif
}

void escaped_serial_write(uint8_t byte) {
	if (byte == FEND) { serial_write(FESC); byte = TFEND; }
    if (byte == FESC) { serial_write(FESC); byte = TFESC; }
    serial_write(byte);
}

void kiss_indicate_reset() {
	serial_write(FEND);
	serial_write(CMD_RESET);
	serial_write(CMD_RESET_BYTE);
	serial_write(FEND);
}

void kiss_indicate_error(uint8_t error_code) {
	serial_write(FEND);
	serial_write(CMD_ERROR);
	serial_write(error_code);
	serial_write(FEND);
}

#if HAS_ESPNOW == true
// Bracket for the RNode Multi-Interface protocol: the host's notion of
// which vport an incoming frame belongs to is driven entirely by the last
// CMD_SEL_INT byte it received from the device, so this must be emitted
// immediately before every radio-specific frame this device sends, for
// both vports.
void kiss_select_interface(uint8_t vport) {
	serial_write(FEND);
	serial_write(CMD_SEL_INT);
	serial_write(vport);
	serial_write(FEND);
}
#endif

void kiss_indicate_radiostate() {
	#if HAS_ESPNOW == true
	  kiss_select_interface(0);
	#endif
	serial_write(FEND);
	serial_write(CMD_RADIO_STATE);
	serial_write(radio_online);
	serial_write(FEND);
}

void kiss_indicate_stat_rx() {
	#if HAS_ESPNOW == true
	  kiss_select_interface(0);
	#endif
	serial_write(FEND);
	serial_write(CMD_STAT_RX);
	escaped_serial_write(stat_rx>>24);
	escaped_serial_write(stat_rx>>16);
	escaped_serial_write(stat_rx>>8);
	escaped_serial_write(stat_rx);
	serial_write(FEND);
}

void kiss_indicate_stat_tx() {
	#if HAS_ESPNOW == true
	  kiss_select_interface(0);
	#endif
	serial_write(FEND);
	serial_write(CMD_STAT_TX);
	escaped_serial_write(stat_tx>>24);
	escaped_serial_write(stat_tx>>16);
	escaped_serial_write(stat_tx>>8);
	escaped_serial_write(stat_tx);
	serial_write(FEND);
}

void kiss_indicate_stat_rssi() {
  uint8_t packet_rssi_val = (uint8_t)(last_rssi+rssi_offset);
	#if HAS_ESPNOW == true
	  kiss_select_interface(0);
	#endif
	serial_write(FEND);
	serial_write(CMD_STAT_RSSI);
	escaped_serial_write(packet_rssi_val);
	serial_write(FEND);
}

void kiss_indicate_stat_snr() {
	#if HAS_ESPNOW == true
	  kiss_select_interface(0);
	#endif
	serial_write(FEND);
	serial_write(CMD_STAT_SNR);
	escaped_serial_write(last_snr_raw);
	serial_write(FEND);
}

void kiss_indicate_radio_lock() {
	#if HAS_ESPNOW == true
	  kiss_select_interface(0);
	#endif
	serial_write(FEND);
	serial_write(CMD_RADIO_LOCK);
	serial_write(radio_locked);
	serial_write(FEND);
}

void kiss_indicate_spreadingfactor() {
	#if HAS_ESPNOW == true
	  kiss_select_interface(0);
	#endif
	serial_write(FEND);
	serial_write(CMD_SF);
	serial_write((uint8_t)lora_sf);
	serial_write(FEND);
}

void kiss_indicate_syncword() {
	#if HAS_ESPNOW == true
	  kiss_select_interface(0);
	#endif
	serial_write(FEND);
	serial_write(CMD_SYNC_WORD);
	serial_write((uint8_t)lora_sw);
	serial_write(FEND);
}

void kiss_indicate_codingrate() {
	#if HAS_ESPNOW == true
	  kiss_select_interface(0);
	#endif
	serial_write(FEND);
	serial_write(CMD_CR);
	serial_write((uint8_t)lora_cr);
	serial_write(FEND);
}

void kiss_indicate_implicit_length() {
	serial_write(FEND);
	serial_write(CMD_IMPLICIT);
	serial_write(implicit_l);
	serial_write(FEND);
}

void kiss_indicate_txpower() {
	#if HAS_ESPNOW == true
	  kiss_select_interface(0);
	#endif
	serial_write(FEND);
	serial_write(CMD_TXPOWER);
	serial_write((uint8_t)lora_txp);
	serial_write(FEND);
}

#if HAS_VSENSE == true
void kiss_indicate_vsense_div() {
	serial_write(FEND);
	serial_write(CMD_VSENSE_DIV);
	escaped_serial_write((uint8_t)(vsense_divider_ratio*10.0));
	serial_write(FEND);
}
#endif

void kiss_indicate_bandwidth() {
	#if HAS_ESPNOW == true
	  kiss_select_interface(0);
	#endif
	serial_write(FEND);
	serial_write(CMD_BANDWIDTH);
	escaped_serial_write(lora_bw>>24);
	escaped_serial_write(lora_bw>>16);
	escaped_serial_write(lora_bw>>8);
	escaped_serial_write(lora_bw);
	serial_write(FEND);
}

void kiss_indicate_frequency() {
	#if HAS_ESPNOW == true
	  kiss_select_interface(0);
	#endif
	serial_write(FEND);
	serial_write(CMD_FREQUENCY);
	escaped_serial_write(lora_freq>>24);
	escaped_serial_write(lora_freq>>16);
	escaped_serial_write(lora_freq>>8);
	escaped_serial_write(lora_freq);
	serial_write(FEND);
}

#if HAS_RTC == true
void kiss_indicate_time() {
	uint32_t epoch = rtc_get_unixtime();
	serial_write(FEND);
	serial_write(CMD_TIME);
	escaped_serial_write(epoch>>24);
	escaped_serial_write(epoch>>16);
	escaped_serial_write(epoch>>8);
	escaped_serial_write(epoch);
	serial_write(FEND);
}
#endif

#if MCU_VARIANT == MCU_ESP32 && HAS_RTC == true && (HAS_WIFI == true || HAS_ETHERNET == true)
// Replies under CMD_NTP_SYNC's own command byte (not CMD_TIME) - every
// other KISS command that echoes a result (CMD_TIME, CMD_VSENSE_DIV,
// CMD_FREQUENCY, ...) replies under its own byte, so a host waiting on a
// CMD_NTP_SYNC response needs one here too, not a CMD_TIME frame instead.
// status is rtc_sync_ntp()'s reason code (NTP_SYNC_OK/NTP_SYNC_ERR_*,
// RTC.h) - 0 means success, matching C convention; epoch is whatever the
// RTC now reads (the freshly-synced time on success, unchanged on failure).
void kiss_indicate_ntp_sync(uint8_t status) {
	uint32_t epoch = rtc_get_unixtime();
	serial_write(FEND);
	serial_write(CMD_NTP_SYNC);
	escaped_serial_write(status);
	escaped_serial_write(epoch>>24);
	escaped_serial_write(epoch>>16);
	escaped_serial_write(epoch>>8);
	escaped_serial_write(epoch);
	serial_write(FEND);
}
#endif

#if HAS_SENSORS == true
// Read-only query, same "command byte alone triggers an immediate reply"
// pattern as kiss_indicate_stat_rx()/_tx()/_rssi() above - no set side to
// this command, unlike CMD_TIME. Temperature/humidity are scaled by 100
// (hundredths of a degree C / percent) into signed/unsigned 16-bit fields;
// pressure is sent as raw Pascals (uint32) - the unit sensor_pressure_pa()
// (Sensors.h) already returns, so no precision is lost converting it.
// Humidity's field reads as 0xFFFF when there's no BME280 present (a
// BMP280 has no humidity element at all) - present/model let the host
// tell "no sensor" apart from "sensor present, this field just doesn't
// apply", same reasoning as GNSS's own N/A fields when there's no fix.
void kiss_indicate_sensor() {
	#if HAS_ESPNOW == true
	  kiss_select_interface(0);
	#endif

	int16_t  temp_cc  = sensor_present ? (int16_t)(sensor_temperature_c()*100.0f) : 0;
	uint16_t humid_cc = (sensor_present && sensor_model == SENSOR_MODEL_BME280) ? (uint16_t)(sensor_humidity_percent()*100.0f) : 0xFFFF;
	uint32_t press_pa = sensor_present ? (uint32_t)sensor_pressure_pa() : 0;

	serial_write(FEND);
	serial_write(CMD_SENSOR);
	escaped_serial_write(sensor_present ? 0x01 : 0x00);
	escaped_serial_write(sensor_model);
	escaped_serial_write(temp_cc>>8);
	escaped_serial_write(temp_cc);
	escaped_serial_write(humid_cc>>8);
	escaped_serial_write(humid_cc);
	escaped_serial_write(press_pa>>24);
	escaped_serial_write(press_pa>>16);
	escaped_serial_write(press_pa>>8);
	escaped_serial_write(press_pa);
	serial_write(FEND);
}
#endif

void kiss_indicate_st_alock() {
	uint16_t at = (uint16_t)(st_airtime_limit*100*100);
	#if HAS_ESPNOW == true
	  kiss_select_interface(0);
	#endif
	serial_write(FEND);
	serial_write(CMD_ST_ALOCK);
	escaped_serial_write(at>>8);
	escaped_serial_write(at);
	serial_write(FEND);
}

void kiss_indicate_lt_alock() {
	uint16_t at = (uint16_t)(lt_airtime_limit*100*100);
	#if HAS_ESPNOW == true
	  kiss_select_interface(0);
	#endif
	serial_write(FEND);
	serial_write(CMD_LT_ALOCK);
	escaped_serial_write(at>>8);
	escaped_serial_write(at);
	serial_write(FEND);
}

void kiss_indicate_channel_stats() {
	#if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
		uint16_t ats = (uint16_t)(airtime*100*100);
		uint16_t atl = (uint16_t)(longterm_airtime*100*100);
		uint16_t cls = (uint16_t)(total_channel_util*100*100);
		uint16_t cll = (uint16_t)(longterm_channel_util*100*100);
		uint8_t  crs = (uint8_t)(current_rssi+rssi_offset);
		uint8_t  nfl = (uint8_t)(noise_floor+rssi_offset);
		uint8_t  ntf = 0xFF; if (interference_detected) { ntf = (uint8_t)(current_rssi+rssi_offset); }
		serial_write(FEND);
		serial_write(CMD_STAT_CHTM);
		escaped_serial_write(ats>>8);
		escaped_serial_write(ats);
		escaped_serial_write(atl>>8);
		escaped_serial_write(atl);
		escaped_serial_write(cls>>8);
		escaped_serial_write(cls);
		escaped_serial_write(cll>>8);
		escaped_serial_write(cll);
		escaped_serial_write(crs);
		escaped_serial_write(nfl);
		escaped_serial_write(ntf);
		serial_write(FEND);
	#endif
}

void kiss_indicate_csma_stats() {
	#if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
		serial_write(FEND);
		serial_write(CMD_STAT_CSMA);
		escaped_serial_write(cw_band);
		escaped_serial_write(cw_min);
		escaped_serial_write(cw_max);
		serial_write(FEND);
	#endif
}

void kiss_indicate_phy_stats() {
	#if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
		uint16_t lst = (uint16_t)(lora_symbol_time_ms*1000);
		uint16_t lsr = (uint16_t)(lora_symbol_rate);
		uint16_t prs = (uint16_t)(lora_preamble_symbols);
		uint16_t prt = (uint16_t)(lora_preamble_time_ms);
		uint16_t cst = (uint16_t)(csma_slot_ms);
		uint16_t dft = (uint16_t)(difs_ms);
		serial_write(FEND);
		serial_write(CMD_STAT_PHYPRM);
		escaped_serial_write(lst>>8);	escaped_serial_write(lst);
		escaped_serial_write(lsr>>8);	escaped_serial_write(lsr);
		escaped_serial_write(prs>>8);	escaped_serial_write(prs);
		escaped_serial_write(prt>>8);	escaped_serial_write(prt);
		escaped_serial_write(cst>>8);	escaped_serial_write(cst);
		escaped_serial_write(dft>>8); escaped_serial_write(dft);
		serial_write(FEND);
	#endif
}

void kiss_indicate_battery() {
	#if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
		serial_write(FEND);
		serial_write(CMD_STAT_BAT);
		escaped_serial_write(battery_state);
		escaped_serial_write((uint8_t)int(battery_percent));
		serial_write(FEND);
	#endif
}

void kiss_indicate_temperature() {
	#if HAS_PMU || IS_ESP32S3
		#if MCU_VARIANT == MCU_ESP32
			uint8_t temp = (uint8_t)roundf(pmu_temperature+PMU_TEMP_OFFSET);
			serial_write(FEND);
			serial_write(CMD_STAT_TEMP);
			escaped_serial_write(temp);
			serial_write(FEND);
		#endif
	#endif
}

void kiss_indicate_btpin() {
	#if HAS_BLUETOOTH || HAS_BLE == true
		serial_write(FEND);
		serial_write(CMD_BT_PIN);
		escaped_serial_write(bt_ssp_pin>>24);
		escaped_serial_write(bt_ssp_pin>>16);
		escaped_serial_write(bt_ssp_pin>>8);
		escaped_serial_write(bt_ssp_pin);
		serial_write(FEND);
	#endif
}

void kiss_indicate_random(uint8_t byte) {
	serial_write(FEND);
	serial_write(CMD_RANDOM);
	serial_write(byte);
	serial_write(FEND);
}

void kiss_indicate_fbstate() {
	serial_write(FEND);
	serial_write(CMD_FB_EXT);
	#if HAS_DISPLAY
		if (disp_ext_fb) {
			serial_write(0x01);
		} else {
			serial_write(0x00);
		}
	#else
		serial_write(0xFF);
	#endif
	serial_write(FEND);
}

#if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
	void kiss_indicate_device_hash() {
	  serial_write(FEND);
	  serial_write(CMD_DEV_HASH);
	  for (int i = 0; i < DEV_HASH_LEN; i++) {
	    uint8_t byte = dev_hash[i];
	 		escaped_serial_write(byte);
	  }
	  serial_write(FEND);
	}

	void kiss_indicate_target_fw_hash() {
	  serial_write(FEND);
	  serial_write(CMD_HASHES);
	  serial_write(0x01);
	  for (int i = 0; i < DEV_HASH_LEN; i++) {
	    uint8_t byte = dev_firmware_hash_target[i];
	 		escaped_serial_write(byte);
	  }
	  serial_write(FEND);
	}

	void kiss_indicate_fw_hash() {
	  serial_write(FEND);
	  serial_write(CMD_HASHES);
	  serial_write(0x02);
	  for (int i = 0; i < DEV_HASH_LEN; i++) {
	    uint8_t byte = dev_firmware_hash[i];
	 		escaped_serial_write(byte);
	  }
	  serial_write(FEND);
	}

	void kiss_indicate_bootloader_hash() {
	  serial_write(FEND);
	  serial_write(CMD_HASHES);
	  serial_write(0x03);
	  for (int i = 0; i < DEV_HASH_LEN; i++) {
	    uint8_t byte = dev_bootloader_hash[i];
	 		escaped_serial_write(byte);
	  }
	  serial_write(FEND);
	}

	void kiss_indicate_partition_table_hash() {
	  serial_write(FEND);
	  serial_write(CMD_HASHES);
	  serial_write(0x04);
	  for (int i = 0; i < DEV_HASH_LEN; i++) {
	    uint8_t byte = dev_partition_table_hash[i];
	 		escaped_serial_write(byte);
	  }
	  serial_write(FEND);
	}
#endif

void kiss_indicate_fb() {
	serial_write(FEND);
	serial_write(CMD_FB_READ);
	#if HAS_DISPLAY
		for (int i = 0; i < 512; i++) {
			uint8_t byte = fb[i];
			escaped_serial_write(byte);
		}
	#else
		serial_write(0xFF);
	#endif
	serial_write(FEND);
}

void kiss_indicate_disp() {
	serial_write(FEND);
	serial_write(CMD_DISP_READ);
	#if HAS_DISPLAY
		// Leading format byte, since the two payloads below aren't otherwise
		// distinguishable by length alone (the disp_area/stat_area split's
		// default geometry and a raw DISP_W x DISP_H buffer both happen to
		// total the same byte count on boards that have a Settings menu).
		#if HAS_MENU == true && BOARD_MODEL != BOARD_HELTEC_T096 && BOARD_MODEL != BOARD_HELTEC_WTRACKER_V2 && BOARD_MODEL != BOARD_HELTEC_T1 && BOARD_MODEL != BOARD_HELTEC_T114
			// The menu draws straight to display's own buffer instead of
			// disp_area/stat_area (which it never touches), so reading those
			// while the menu is open would return stale main-screen content
			// instead of what's actually visible. Read display's buffer
			// directly instead - whatever was last drawn there (main content
			// or the menu) is always what's currently on the physical panel.
			escaped_serial_write(0x01);
			uint8_t *fb = display.getBuffer();
			size_t fb_len = ((DISP_W+7)/8)*DISP_H;
			for (size_t i = 0; i < fb_len; i++) { escaped_serial_write(fb[i]); }
		#else
			// T096/T114's TFTs have no framebuffer of their own to point at
			// (see the branch above) and their menu renders into a separate
			// canvas (menu_canvas, Menu.h/Display.h) rather than disp_area/
			// stat_area - this channel just doesn't reflect the menu's
			// content on these boards, only whatever the normal operational
			// screen last drew.
			escaped_serial_write(0x00);
			uint8_t *da = disp_area.getBuffer();
			size_t da_len = ((disp_area.width()+7)/8)*disp_area.height();
			for (size_t i = 0; i < da_len; i++) { escaped_serial_write(da[i]); }
			#if BOARD_MODEL == BOARD_HELTEC_T114
				// Landscape draws into its own, differently-sized stat_area_land
				// instead of stat_area (see draw_stat_area()'s landscape branch,
				// Display.h) - report whichever one's actually active, or this
				// channel would keep sending stat_area's buffer, which landscape
				// never touches and would read back all zeros.
				uint8_t *sa = (disp_mode == DISP_MODE_LANDSCAPE) ? stat_area_land.getBuffer() : stat_area.getBuffer();
				size_t sa_len = (disp_mode == DISP_MODE_LANDSCAPE) ? ((stat_area_land.width()+7)/8)*stat_area_land.height() : ((stat_area.width()+7)/8)*stat_area.height();
			#else
				uint8_t *sa = stat_area.getBuffer();
				size_t sa_len = ((stat_area.width()+7)/8)*stat_area.height();
			#endif
			for (size_t i = 0; i < sa_len; i++) { escaped_serial_write(sa[i]); }
		#endif
	#else
		serial_write(0xFF);
	#endif
	serial_write(FEND);
}

void kiss_indicate_ready() {
	serial_write(FEND);
	serial_write(CMD_READY);
	serial_write(0x01);
	serial_write(FEND);
}

void kiss_indicate_not_ready() {
	serial_write(FEND);
	serial_write(CMD_READY);
	serial_write(0x00);
	serial_write(FEND);
}

void kiss_indicate_promisc() {
	serial_write(FEND);
	serial_write(CMD_PROMISC);
	if (promisc) {
		serial_write(0x01);
	} else {
		serial_write(0x00);
	}
	serial_write(FEND);
}

void kiss_indicate_detect() {
	serial_write(FEND);
	serial_write(CMD_DETECT);
	serial_write(DETECT_RESP);
	serial_write(FEND);
}

void kiss_indicate_version() {
	serial_write(FEND);
	serial_write(CMD_FW_VERSION);
	serial_write(MAJ_VERS);
	serial_write(MIN_VERS);
	// Appended after the original 2-byte MAJ_VERS/MIN_VERS payload, not
	// interleaved - old clients that only read those two bytes are
	// unaffected. BUILD_NUMBER (git commit count) can already exceed a
	// single byte's range, so it's sent as 4 big-endian bytes, same
	// escaped-multi-byte pattern as kiss_indicate_frequency()/_time().
	uint32_t build_number = (uint32_t)BUILD_NUMBER;
	escaped_serial_write(build_number>>24);
	escaped_serial_write(build_number>>16);
	escaped_serial_write(build_number>>8);
	escaped_serial_write(build_number);
	serial_write(FEND);
}

void kiss_indicate_platform() {
	serial_write(FEND);
	serial_write(CMD_PLATFORM);
	serial_write(PLATFORM);
	serial_write(FEND);
}

void kiss_indicate_board() {
	serial_write(FEND);
	serial_write(CMD_BOARD);
	serial_write(BOARD_MODEL);
	serial_write(FEND);
}

void kiss_indicate_mcu() {
	serial_write(FEND);
	serial_write(CMD_MCU);
	serial_write(MCU_VARIANT);
	serial_write(FEND);
}

inline bool isSplitPacket(uint8_t header) {
	return (header & FLAG_SPLIT);
}

inline uint8_t packetSequence(uint8_t header) {
	return header >> 4;
}

void setPreamble() {
	if (radio_online) LoRa->setPreambleLength(lora_preamble_symbols);
	kiss_indicate_phy_stats();
}

void updateBitrate() {
	#if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
		if (!radio_online) { lora_bitrate = 0; }
		else {
			lora_symbol_rate = (float)lora_bw/(float)(pow(2, lora_sf));
			lora_symbol_time_ms = (1.0/lora_symbol_rate)*1000.0;
			lora_bitrate = (uint32_t)(lora_sf * ( (4.0/(float)lora_cr) / ((float)(pow(2, lora_sf))/((float)lora_bw/1000.0)) ) * 1000.0);
			lora_us_per_byte = 1000000.0/((float)lora_bitrate/8.0);
			
			bool fast_rate   = lora_bitrate > LORA_FAST_THRESHOLD_BPS;
			lora_limit_rate  = lora_bitrate > LORA_LIMIT_THRESHOLD_BPS;
			lora_guard_rate  = (!lora_limit_rate && lora_bitrate > LORA_GUARD_THRESHOLD_BPS);

			int csma_slot_min_ms = CSMA_SLOT_MIN_MS;
			float lora_preamble_target_ms = LORA_PREAMBLE_TARGET_MS;
			if (fast_rate) { csma_slot_min_ms        -= CSMA_SLOT_MIN_FAST_DELTA;
											 lora_preamble_target_ms -= LORA_PREAMBLE_FAST_DELTA; }
			
			csma_slot_ms = lora_symbol_time_ms*CSMA_SLOT_SYMBOLS;
			if (csma_slot_ms > CSMA_SLOT_MAX_MS) { csma_slot_ms = CSMA_SLOT_MAX_MS; }
			if (csma_slot_ms < CSMA_SLOT_MIN_MS) { csma_slot_ms = csma_slot_min_ms; }
			difs_ms = CSMA_SIFS_MS + 2*csma_slot_ms;
			
			float target_preamble_symbols = lora_preamble_target_ms/lora_symbol_time_ms;
			if (target_preamble_symbols < LORA_PREAMBLE_SYMBOLS_MIN) { target_preamble_symbols = LORA_PREAMBLE_SYMBOLS_MIN; }
			else { target_preamble_symbols = (ceil)(target_preamble_symbols); }
			
			lora_preamble_symbols = (long)target_preamble_symbols; setPreamble();
			lora_preamble_time_ms = (ceil)(lora_preamble_symbols * lora_symbol_time_ms);
			lora_header_time_ms   = (ceil)(PHY_HEADER_LORA_SYMBOLS * lora_symbol_time_ms);
		}
	#endif
}

void setSpreadingFactor() {
	if (radio_online) LoRa->setSpreadingFactor(lora_sf);
	updateBitrate();
}

void setCodingRate() {
	if (radio_online) LoRa->setCodingRate4(lora_cr);
	updateBitrate();
}

void setSyncWord() {
	if (radio_online) LoRa->setSyncWord(lora_sw);
}

void set_implicit_length(uint8_t len) {
	implicit_l = len;
	if (implicit_l != 0) {
		implicit = true;
	} else {
		implicit = false;
	}
}

int getTxPower() {
	uint8_t txp = LoRa->getTxPower();
	return (int)txp;
}

#if HAS_LORA_PA
    #if BOARD_MODEL == BOARD_HELTEC32_V4
	bool pa_values_determined = false;
	int tx_gain[PA_GAIN_POINTS] = {100};
    #elif BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_WTRACKER_V2
	bool pa_values_determined = false;
	int tx_gain[PA_GAIN_POINTS] = {100};
    #else
	bool pa_values_determined = true;
	const int tx_gain[PA_GAIN_POINTS] = {PA_GAIN_VALUES};
    #endif
#endif

extern uint8_t lora_pa_model;
void determine_pa_values() {
	#if BOARD_MODEL == BOARD_HELTEC32_V4
		if (lora_pa_model == LORA_PA_GC1109) {
			for (int i=0; i < PA_GAIN_POINTS; i++) { tx_gain[i] = PA_GC1109_VALUES[i]; }
			pa_values_determined = true;
			for (int i=0; i < PA_GAIN_POINTS; i++) { Serial.print(" "); Serial.printf("%d", tx_gain[i]); }
		} else if (lora_pa_model == LORA_PA_KCT8103L) {
			for (int i=0; i < PA_GAIN_POINTS; i++) { tx_gain[i] = PA_KCT8103L_VALUES[i]; }
			pa_values_determined = true;
			for (int i=0; i < PA_GAIN_POINTS; i++) { Serial.print(" "); Serial.printf("%d", tx_gain[i]); }
		}
	#elif BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_WTRACKER_V2
		if (lora_pa_model == LORA_PA_KCT8103L) {
			for (int i=0; i < PA_GAIN_POINTS; i++) { tx_gain[i] = PA_KCT8103L_VALUES[i]; }
			pa_values_determined = true;
		}
	#endif
}

int map_target_power_to_modem_output(int target_tx_power) {
	#if HAS_LORA_PA
		if (!pa_values_determined) { determine_pa_values(); }
		int modem_output_dbm = -9;
		for (int i = 0; i < PA_GAIN_POINTS; i++) {
			int gain = tx_gain[i];
			int effective_output_dbm = i + gain;
			if (effective_output_dbm > target_tx_power) {
				int diff = effective_output_dbm - target_tx_power;
				modem_output_dbm = -1*diff;
				break;
			} else if (effective_output_dbm == target_tx_power) {
				modem_output_dbm = i; break;
			} else if (i == PA_GAIN_POINTS-1) {
				int diff = target_tx_power - effective_output_dbm;
				modem_output_dbm = i+diff; break;
			}
		}
	#else
		int modem_output_dbm = target_tx_power;
	#endif
	
	return modem_output_dbm;
}

int map_modem_output_to_target_power(int modem_output_dbm) {
	#if HAS_LORA_PA
		if (modem_output_dbm < 0)               { modem_output_dbm = 0; }
		if (modem_output_dbm >= PA_GAIN_POINTS) { modem_output_dbm = PA_GAIN_POINTS-1; }
		int gain = tx_gain[modem_output_dbm];
		int target_tx_power = modem_output_dbm+gain;
	#else
		int target_tx_power = modem_output_dbm;
	#endif

	return target_tx_power;
}

// Board/modem TX power ceiling - same bounds CMD_TXPOWER's own KISS handler
// (RNode_Firmware.ino) clamps against, extracted here so the URNS Radio
// settings menu (Menu.h) can't drift from what a connected host is allowed
// to set.
int lora_txp_max() {
	#if MODEM == SX1262
		#if HAS_LORA_PA
			return PA_MAX_OUTPUT;
		#else
			return 22;
		#endif
	#elif MODEM == SX1280
		#if HAS_PA
			return 20;
		#else
			return 13;
		#endif
	#else
		return 20;
	#endif
}

void setTXPower() {
	if (radio_online) {
		int mapped_lora_txp = map_target_power_to_modem_output(lora_txp);
		
		#if HAS_LORA_PA
			int real_lora_txp = map_modem_output_to_target_power(mapped_lora_txp);
			lora_txp = real_lora_txp;
		#endif

		if (model == MODEL_FD && mapped_lora_txp > 8) {
			mapped_lora_txp = 8;
		}

		if (model == MODEL_11) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_RFO_PIN);
		if (model == MODEL_12) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_RFO_PIN);
		if (model == MODEL_13) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_RFO_PIN);

		if (model == MODEL_C6) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_RFO_PIN);
		if (model == MODEL_C7) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_RFO_PIN);

		if (model == MODEL_A1) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_A2) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_A3) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_RFO_PIN);
		if (model == MODEL_A4) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_RFO_PIN);
		if (model == MODEL_A5) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_A6) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_A7) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_A8) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_A9) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_AA) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_AC) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);

		if (model == MODEL_BA) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_BB) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_B3) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_B4) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_B8) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_B9) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);

		if (model == MODEL_C4) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_C9) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_C5) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_CA) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_C8) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);

		if (model == MODEL_D3) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_D5) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);

		if (model == MODEL_D4) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_D9) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);

		if (model == MODEL_DB) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_DC) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);

		if (model == MODEL_DD) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_DE) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);

		if (model == MODEL_E4) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_E9) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_E3) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_E8) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_E5) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_E6) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);

		if (model == MODEL_FD) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_FE) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_PA_BOOST_PIN);
		if (model == MODEL_FF) LoRa->setTxPower(mapped_lora_txp, PA_OUTPUT_RFO_PIN);
	}
}


void getBandwidth() {
	if (radio_online) {
			lora_bw = LoRa->getSignalBandwidth();
	}
	updateBitrate();
}

void setBandwidth() {
	if (radio_online) {
		LoRa->setSignalBandwidth(lora_bw);
		getBandwidth();
	}
}

void getFrequency() {
	if (radio_online) {
		lora_freq = LoRa->getFrequency();
	}
}

void setFrequency() {
	if (radio_online) {
		LoRa->setFrequency(lora_freq);
		getFrequency();
	}
}

uint8_t getRandom() { return random(0xFF); }

void promisc_enable() {
	promisc = true;
}

void promisc_disable() {
	promisc = false;
}

#if !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
  bool eeprom_begin() {
    InternalFS.begin();

    file.open(EEPROM_FILE, FILE_O_READ);
    if (!file) {
      if (file.open(EEPROM_FILE, FILE_O_WRITE)) {
      	for (uint32_t mapped_addr = 0; mapped_addr < EEPROM_SIZE; mapped_addr++) { file.seek(mapped_addr); file.write(0xFF); }
        eeprom_flush();
        return true;
      } else {
        return false;
      }
    } else {
      file.close();
      file.open(EEPROM_FILE, FILE_O_WRITE);
      return true;
    }
  }

  uint8_t eeprom_read(uint32_t mapped_addr) {
      uint8_t byte;
      void* byte_ptr = &byte;
      file.seek(mapped_addr);
      file.read(byte_ptr, 1);
      return byte;
  }
#endif

bool eeprom_info_locked() {
  #if HAS_EEPROM
    uint8_t lock_byte = EEPROM.read(eeprom_addr(ADDR_INFO_LOCK));
  #elif MCU_VARIANT == MCU_NRF52
    uint8_t lock_byte = eeprom_read(eeprom_addr(ADDR_INFO_LOCK));
  #endif
	if (lock_byte == INFO_LOCK_BYTE) {
		return true;
	} else {
		return false;
	}
}

void eeprom_dump_info() {
	for (int addr = ADDR_PRODUCT; addr <= ADDR_INFO_LOCK; addr++) {
        #if HAS_EEPROM
            uint8_t byte = EEPROM.read(eeprom_addr(addr));
        #elif MCU_VARIANT == MCU_NRF52
            uint8_t byte = eeprom_read(eeprom_addr(addr));
        #endif
		escaped_serial_write(byte);
	}
}

void eeprom_dump_config() {
	for (int addr = ADDR_CONF_SF; addr <= ADDR_CONF_OK; addr++) {
        #if HAS_EEPROM
            uint8_t byte = EEPROM.read(eeprom_addr(addr));
        #elif MCU_VARIANT == MCU_NRF52
            uint8_t byte = eeprom_read(eeprom_addr(addr));
        #endif
		escaped_serial_write(byte);
	}
}

void eeprom_dump_all() {
	for (int addr = 0; addr < EEPROM_RESERVED; addr++) {
        #if HAS_EEPROM
            uint8_t byte = EEPROM.read(eeprom_addr(addr));
        #elif MCU_VARIANT == MCU_NRF52
            uint8_t byte = eeprom_read(eeprom_addr(addr));
        #endif
		escaped_serial_write(byte);
	}
}

void eeprom_config_dump_all() {
	#if MCU_VARIANT == MCU_ESP32
		for (int addr = 0; addr < CONFIG_SIZE; addr++) {
	    uint8_t byte = EEPROM.read(config_addr(addr));
			escaped_serial_write(byte);
		}
	#endif
}

void kiss_dump_eeprom() {
	serial_write(FEND);
	serial_write(CMD_ROM_READ);
	eeprom_dump_all();
	serial_write(FEND);
}

void kiss_dump_config() {
	serial_write(FEND);
	serial_write(CMD_CFG_READ);
	eeprom_config_dump_all();
	serial_write(FEND);
}

#if !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
void eeprom_flush() {
    file.close();
    // file.open()'s return value was previously discarded - if this ever
    // failed (LittleFS busy/reallocating right after the close, timing,
    // etc.), file's internal handle stays null, and the next eeprom_read()/
    // eeprom_update() call dereferences that null pointer inside the
    // library's read()/write()/seek() (none of which null-check it),
    // hard-faulting the MCU. Since this runs on every single byte written,
    // a burst of several conf_save calls (e.g. the flasher's Display
    // "Apply") multiplies the chance of hitting one bad reopen. Retry
    // rather than proceeding with a broken handle.
    for (uint8_t attempt = 0; attempt < 5; attempt++) {
        if (file.open(EEPROM_FILE, FILE_O_WRITE)) break;
        delay(5);
    }
    written_bytes = 0;
}
#endif

void eeprom_update(int mapped_addr, uint8_t byte) {
	#if MCU_VARIANT == MCU_1284P || MCU_VARIANT == MCU_2560
		EEPROM.update(mapped_addr, byte);
	#elif MCU_VARIANT == MCU_ESP32
		if (EEPROM.read(mapped_addr) != byte) {
			EEPROM.write(mapped_addr, byte);
			EEPROM.commit();
		}
  #elif !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
    // todo: clean up this implementation, writing one byte and syncing
    // each time is really slow, but this is also suboptimal
    uint8_t read_byte;
    void* read_byte_ptr = &read_byte;
    file.seek(mapped_addr);
    file.read(read_byte_ptr, 1);
    file.seek(mapped_addr);
    if (read_byte != byte) {
      file.write(byte);
    }
    written_bytes++;
    eeprom_flush();
	#endif
}

void eeprom_write(uint8_t addr, uint8_t byte) {
	if (!eeprom_info_locked() && addr >= 0 && addr < EEPROM_RESERVED) {
		eeprom_update(eeprom_addr(addr), byte);
	} else {
		kiss_indicate_error(ERROR_EEPROM_LOCKED);
	}
}

void eeprom_erase() {
	#if !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
		InternalFS.format();
	#else
		for (int addr = 0; addr < EEPROM_RESERVED; addr++) {
			eeprom_update(eeprom_addr(addr), 0xFF);
		}
	#endif
	hard_reset();
}

bool eeprom_lock_set() {
    #if HAS_EEPROM
	    if (EEPROM.read(eeprom_addr(ADDR_INFO_LOCK)) == INFO_LOCK_BYTE) {
    #elif MCU_VARIANT == MCU_NRF52
        if (eeprom_read(eeprom_addr(ADDR_INFO_LOCK)) == INFO_LOCK_BYTE) {
    #endif
		return true;
	} else {
		return false;
	}
}

bool eeprom_product_valid() {
  #if HAS_EEPROM
    uint8_t rval = EEPROM.read(eeprom_addr(ADDR_PRODUCT));
  #elif MCU_VARIANT == MCU_NRF52
    uint8_t rval = eeprom_read(eeprom_addr(ADDR_PRODUCT));
  #endif

	#if PLATFORM == PLATFORM_AVR
	if (rval == PRODUCT_RNODE || rval == PRODUCT_HMBRW) {
	#elif PLATFORM == PLATFORM_ESP32
	if (rval == PRODUCT_RNODE || rval == BOARD_RNODE_NG_20 || rval == BOARD_RNODE_NG_21 || rval == PRODUCT_HMBRW || rval == PRODUCT_TBEAM || rval == PRODUCT_T32_10 || rval == PRODUCT_T32_20 || rval == PRODUCT_T32_21 || rval == PRODUCT_H32_V2 || rval == PRODUCT_H32_V3 || rval == PRODUCT_H32_V4 || rval == PRODUCT_TDECK_V1 || rval == PRODUCT_TBEAM_S_V1 || rval == PRODUCT_TBEAM_S_V3 || rval == PRODUCT_XIAO_S3 || rval == PRODUCT_TBEAM_1W || rval == PRODUCT_HELTEC_WTRACKER_V2) {
	#elif PLATFORM == PLATFORM_NRF52
	if (rval == PRODUCT_RAK4631 || rval == PRODUCT_HELTEC_T114 || rval == PRODUCT_HELTEC_T096 || rval == PRODUCT_HELTEC_T1 || rval == PRODUCT_TECHO || rval == PRODUCT_HMBRW) {
	#else
	if (false) {
	#endif
		return true;
	} else {
		return false;
	}
}

bool eeprom_model_valid() {
    #if HAS_EEPROM
        model = EEPROM.read(eeprom_addr(ADDR_MODEL));
    #elif MCU_VARIANT == MCU_NRF52
        model = eeprom_read(eeprom_addr(ADDR_MODEL));
    #endif
	#if BOARD_MODEL == BOARD_RNODE
	if (model == MODEL_A4 || model == MODEL_A9 || model == MODEL_FF || model == MODEL_FE) {
	#elif BOARD_MODEL == BOARD_RNODE_NG_20
	if (model == MODEL_A3 || model == MODEL_A8) {
	#elif BOARD_MODEL == BOARD_RNODE_NG_21
	if (model == MODEL_A2 || model == MODEL_A7) {
	#elif BOARD_MODEL == BOARD_T3S3
	if (model == MODEL_A1 || model == MODEL_A6 || model == MODEL_A5 || model == MODEL_AA || model == MODEL_AC) {
	#elif BOARD_MODEL == BOARD_HMBRW
	if (model == MODEL_FF || model == MODEL_FE || model == MODEL_FD) {
	#elif BOARD_MODEL == BOARD_TBEAM
	if (model == MODEL_E4 || model == MODEL_E9 || model == MODEL_E3 || model == MODEL_E8) {
	#elif BOARD_MODEL == BOARD_TBEAM_1W
	if (model == MODEL_E5 || model == MODEL_E6) {
	#elif BOARD_MODEL == BOARD_TDECK
	if (model == MODEL_D4 || model == MODEL_D9) {
	#elif BOARD_MODEL == BOARD_TECHO
	if (model == MODEL_16 || model == MODEL_17) {
	#elif BOARD_MODEL == BOARD_TBEAM_S_V1 || BOARD_MODEL == BOARD_TBEAM_S_V3
	if (model == MODEL_DB || model == MODEL_DC) {
	#elif BOARD_MODEL == BOARD_XIAO_S3
	if (model == MODEL_DD || model == MODEL_DE) {
	#elif BOARD_MODEL == BOARD_LORA32_V1_0
	if (model == MODEL_BA || model == MODEL_BB) {
	#elif BOARD_MODEL == BOARD_LORA32_V2_0
	if (model == MODEL_B3 || model == MODEL_B8) {
	#elif BOARD_MODEL == BOARD_LORA32_V2_1
	if (model == MODEL_B4 || model == MODEL_B9) {
	#elif BOARD_MODEL == BOARD_HELTEC32_V2
	if (model == MODEL_C4 || model == MODEL_C9) {
	#elif BOARD_MODEL == BOARD_HELTEC32_V3
	if (model == MODEL_C5 || model == MODEL_CA) {
	#elif BOARD_MODEL == BOARD_HELTEC32_V4
	if (model == MODEL_C8) {
  #elif BOARD_MODEL == BOARD_HELTEC_T114
  if (model == MODEL_C6 || model == MODEL_C7) {
  #elif BOARD_MODEL == BOARD_HELTEC_T096
  if (model == MODEL_D3 || model == MODEL_D5) {
  #elif BOARD_MODEL == BOARD_HELTEC_WTRACKER_V2
  if (model == MODEL_D6 || model == MODEL_D7) {
  #elif BOARD_MODEL == BOARD_HELTEC_T1
  if (model == MODEL_DF || model == MODEL_E2) {
  #elif BOARD_MODEL == BOARD_RAK4631
  if (model == MODEL_11 || model == MODEL_12) {
  #elif BOARD_MODEL == BOARD_RAK3401
  if (model == MODEL_13) {
	#elif BOARD_MODEL == BOARD_HUZZAH32
	if (model == MODEL_FF) {
	#elif BOARD_MODEL == BOARD_GENERIC_ESP32
	if (model == MODEL_FF || model == MODEL_FE) {
	#elif BOARD_MODEL == BOARD_MESHPOE_S3
	if (model == MODEL_FF || model == MODEL_FE || model == MODEL_FD) {
	#elif BOARD_MODEL == BOARD_MESHADVENTURER_S3
	if (model == MODEL_FF || model == MODEL_FE || model == MODEL_FD) {
	#elif BOARD_MODEL == BOARD_MESHADVENTURER
	if (model == MODEL_FF || model == MODEL_FE || model == MODEL_FD) {
	#elif BOARD_MODEL == BOARD_DIY_V1
	if (model == MODEL_FF || model == MODEL_FE || model == MODEL_FD) {
	#elif BOARD_MODEL == BOARD_AETHERNODE
	if (model == MODEL_FF || model == MODEL_FE || model == MODEL_FD) {
	#elif BOARD_MODEL == BOARD_AETHERNODE_S3
	if (model == MODEL_FF || model == MODEL_FE || model == MODEL_FD) {
	#elif BOARD_MODEL == BOARD_PROMICRO
	if (model == MODEL_FF || model == MODEL_FE) {
	#else
	if (false) {
	#endif
		return true;
	} else {
		return false;
	}
}

bool eeprom_hwrev_valid() {
    #if HAS_EEPROM
        hwrev = EEPROM.read(eeprom_addr(ADDR_HW_REV));
    #elif MCU_VARIANT == MCU_NRF52
        hwrev = eeprom_read(eeprom_addr(ADDR_HW_REV));
    #endif
	if (hwrev != 0x00 && hwrev != 0xFF) {
		return true;
	} else {
		return false;
	}
}

bool eeprom_checksum_valid() {
	char *data = (char*)malloc(CHECKSUMMED_SIZE);
	for (uint8_t  i = 0; i < CHECKSUMMED_SIZE; i++) {
        #if HAS_EEPROM
            char byte = EEPROM.read(eeprom_addr(i));
        #elif MCU_VARIANT == MCU_NRF52
            char byte = eeprom_read(eeprom_addr(i));
        #endif
		data[i] = byte;
	}
	
	unsigned char *hash = MD5::make_hash(data, CHECKSUMMED_SIZE);
	bool checksum_valid = true;
	for (uint8_t i = 0; i < 16; i++) {
        #if HAS_EEPROM
            uint8_t stored_chk_byte = EEPROM.read(eeprom_addr(ADDR_CHKSUM+i));
        #elif MCU_VARIANT == MCU_NRF52
            uint8_t stored_chk_byte = eeprom_read(eeprom_addr(ADDR_CHKSUM+i));
        #endif
		uint8_t calced_chk_byte = (uint8_t)hash[i];
		if (stored_chk_byte != calced_chk_byte) {
			checksum_valid = false;
		}
	}

	free(hash);
	free(data);
	return checksum_valid;
}

void wr_conf_save(uint8_t mode) {
	eeprom_update(eeprom_addr(ADDR_CONF_WIFI), mode);
  #if !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
    // have to do a flush because we're only writing 1 byte and it syncs after 8
    eeprom_flush();
  #endif
}

void bt_conf_save(bool is_enabled) {
	if (is_enabled) {
		eeprom_update(eeprom_addr(ADDR_CONF_BT), BT_ENABLE_BYTE);
      #if !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
        // have to do a flush because we're only writing 1 byte and it syncs after 8
        eeprom_flush();
      #endif
	} else {
		eeprom_update(eeprom_addr(ADDR_CONF_BT), 0x00);
    #if !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
      // have to do a flush because we're only writing 1 byte and it syncs after 8
      eeprom_flush();
    #endif
	}
}

#if MCU_VARIANT == MCU_ESP32 && HAS_BLE == true
void bt_legacy_pairing_conf_save(bool is_enabled) {
  bt_legacy_pairing_enabled = is_enabled;
  eeprom_update(ADDR_CONF_BT_LEGACY_PAIRING, is_enabled ? BT_LEGACY_PAIRING_ENABLE_BYTE : BT_LEGACY_PAIRING_DISABLE_BYTE);
  if (bt_ready) bt_security_setup();
}

void bt_just_works_conf_save(bool is_enabled) {
  bt_just_works_enabled = is_enabled;
  eeprom_update(ADDR_CONF_BT_JUST_WORKS, is_enabled ? BT_JUST_WORKS_ENABLE_BYTE : BT_JUST_WORKS_DISABLE_BYTE);
  // Unlike bt_legacy_pairing_conf_save() above, a plain bt_security_setup()
  // isn't enough - this setting changes the RX/TX GATT characteristics'
  // own permission bits (BLESerial.cpp, SetupSerialService()), which
  // NimBLE bakes in at creation time, not something re-checked per
  // connection. Cycling the whole BLE stack rebuilds the GATT service
  // under the new permission level - same as the user toggling Bluetooth
  // off and back on themselves (CMD_BT_CTRL 0x00/0x01).
  if (bt_ready && bt_state != BT_STATE_OFF) {
    bt_stop();
    bt_start();
  }
}

void bt_auto_start_conf_save(bool is_enabled) {
  bt_auto_start_enabled = is_enabled;
  eeprom_update(ADDR_CONF_BT_AUTO_START, is_enabled ? BT_AUTO_START_ENABLE_BYTE : BT_AUTO_START_DISABLE_BYTE);
  // No live effect here (unlike Legacy Pairing/Just Works) - this only
  // governs what happens at the NEXT boot's one-shot check
  // (RNode_Firmware.ino loop()), not current session behavior. Turning
  // it on doesn't start BLE right now if it isn't already running.
}
#endif

void snd_conf_save(bool is_enabled) {
	sound_enabled = is_enabled;
	if (is_enabled) {
		eeprom_update(eeprom_addr(ADDR_CONF_SND), SND_ENABLE_BYTE);
	} else {
		eeprom_update(eeprom_addr(ADDR_CONF_SND), SND_DISABLE_BYTE);
	}
  #if !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
    // have to do a flush because we're only writing 1 byte and it syncs after 8
    eeprom_flush();
  #endif
}

#if HAS_ENCODER == true
void enc_conf_save(bool is_enabled) {
	encoder_enabled = is_enabled;
	if (is_enabled) {
		eeprom_update(eeprom_addr(ADDR_CONF_ENA), ENC_ENABLE_BYTE);
	} else {
		eeprom_update(eeprom_addr(ADDR_CONF_ENA), ENC_DISABLE_BYTE);
	}
  #if !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
    eeprom_flush();
  #endif
}
#endif

#if HAS_GPS == true
// ADDR_CONF_GNSS is offset via eeprom_addr() like every other ADDR_CONF_*
// setting - see its own comment, ROM.h, for why this used to be a raw
// physical byte and why that collided with Device.h's firmware-hash
// storage on nRF52.
void gnss_conf_save(bool is_enabled) {
	if (is_enabled) {
		eeprom_update(eeprom_addr(ADDR_CONF_GNSS), GNSS_ENABLE_BYTE);
	} else {
		eeprom_update(eeprom_addr(ADDR_CONF_GNSS), GNSS_DISABLE_BYTE);
	}
  #if !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
    eeprom_flush();
  #endif
}

// Same eeprom_addr() convention as gnss_conf_save() above. Stores the
// selected index into gnss_update_interval_presets_s (GNSS.h), not the
// raw seconds value - keeps this a single byte regardless of how large
// the preset list gets. 0xFF (erased/never touched) is read back as "use
// the board default" (GNSS_UPDATE_INTERVAL_DEFAULT, always index 0/
// Continuous today) - same unset convention as ADDR_CONF_GNSS's own
// enable byte.
void gnss_interval_conf_save(uint8_t preset_index) {
	eeprom_update(eeprom_addr(ADDR_CONF_GNSS_INTERVAL), preset_index);
  #if !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
    eeprom_flush();
  #endif
}
#endif

#if HAS_VSENSE == true
// Stored as ratio*10 (one decimal place is enough for divider tolerances).
// 0x00 and 0xFF (erased EEPROM) both mean "unset" - fall back to the
// board's VSENSE_DIVIDER_RATIO_DEFAULT instead of using them as-is.
void vsr_conf_save(uint8_t val) {
	eeprom_update(eeprom_addr(ADDR_CONF_VSR), val);
  #if !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
    eeprom_flush();
  #endif
	if (val != 0x00 && val != 0xFF) { vsense_divider_ratio = (float)val / 10.0; }
	else { vsense_divider_ratio = VSENSE_DIVIDER_RATIO_DEFAULT; }
}
#endif

#if HAS_BATTERY_DIVIDER == true
// Stored as a %/of-default correction against BATTERY_V_SCALE_DEFAULT, not
// a divider ratio like VSENSE - measure_battery()'s pin_vbat constant
// already bakes the physical divider and this MCU's ADC characteristics
// together, so there's no clean way to isolate "the divider" alone; this
// recalibrates the whole scale instead, nudged against a multimeter
// reading. 0x00 and 0xFF (erased EEPROM) both mean "unset" - fall back to
// the board's BATTERY_V_SCALE_DEFAULT (100%) instead of using them as-is.
void bvs_conf_save(uint8_t val) {
	eeprom_update(eeprom_addr(ADDR_CONF_BVS), val);
  #if !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
    eeprom_flush();
  #endif
	if (val != 0x00 && val != 0xFF) { battery_v_scale = BATTERY_V_SCALE_DEFAULT * ((float)val / 100.0); }
	else { battery_v_scale = BATTERY_V_SCALE_DEFAULT; }
}
#endif

// tz_conf_save() used to live here, RTC-exclusive - moved further down
// (system_time_utc()'s own section) alongside get_tz_offset_qh()/
// apply_tz_offset(), since GNSS-only boards need it too. Same EEPROM
// address/encoding, just no longer gated on HAS_RTC.

#if HAS_GPIO_MENU == true
// Persists a physical peripheral-pin reassignment - currently the buzzer
// (ADDR_CONF_BUZ) and, if HAS_ENCODER, the optional encoder's Up/Down/Press
// pins (ADDR_CONF_EUP/EDN/EPR) - picked from a short curated list of
// genuinely free GPIOs (Boards.h) via the RNode Settings menu's Hardware >
// GPIO submenu, not entered freely, so val is always one of those
// candidates. Reassigning a pin only takes effect at boot (buzzer_init()/
// encoder_init() each read their own pin variable once, in setup()), so
// this reboots immediately when the value actually changes - same pattern
// as drot_conf_save()'s display-rotation reboot.
void gpio_conf_save(uint8_t addr, uint8_t val) {
  #if HAS_EEPROM
    uint8_t stored = EEPROM.read(eeprom_addr(addr));
  #elif MCU_VARIANT == MCU_NRF52
    uint8_t stored = eeprom_read(eeprom_addr(addr));
  #endif
	eeprom_update(eeprom_addr(addr), val);
  #if !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
    eeprom_flush();
  #endif
	if (stored != val) { hard_reset(); }
}
#endif

#if HAS_ESPNOW == true
// Persists whether the ESP-NOW virtual interface (vport 1, ESPNOW.h) is
// allowed to run (ADDR_CONF_ESPNOW, ROM.h). Only takes effect at boot -
// espnow_init() (ESPNOW.h) has no double-init guard or teardown/deinit
// counterpart, since it was never designed to run more than once per boot -
// so, like gpio_conf_save()/ethspd_conf_save(), this reboots immediately
// when the value actually changed rather than trying to start/stop ESP-NOW
// live.
void espnow_conf_save(uint8_t val) {
  #if HAS_EEPROM
    uint8_t stored = EEPROM.read(eeprom_addr(ADDR_CONF_ESPNOW));
  #elif MCU_VARIANT == MCU_NRF52
    uint8_t stored = eeprom_read(eeprom_addr(ADDR_CONF_ESPNOW));
  #endif
	eeprom_update(eeprom_addr(ADDR_CONF_ESPNOW), val);
  #if !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
    eeprom_flush();
  #endif
	if (stored != val) { hard_reset(); }
}

// Persists the ESP-NOW wire-format mode (ADDR_CONF_ESPNOW_MODE, ROM.h - a
// raw physical byte, not through eeprom_addr(), same convention as
// ADDR_CONF_GNSS). Deliberately has no self-reboot logic of its own, unlike
// espnow_conf_save() above - it's only ever called from
// menu_commit_and_exit() (Menu.h), which may also be committing the
// Enabled/LR fields in the same SAVE & EXIT and needs a single combined
// reboot after all raw bytes are safely written - see that call site's
// own comment for why independently-self-rebooting savers here would be
// unsafe.
void espnow_mode_conf_save(uint8_t val) {
  eeprom_update(ADDR_CONF_ESPNOW_MODE, val);
}

// Persists whether 802.11 LR mode is active (ADDR_CONF_ESPNOW_LR, ROM.h -
// a separate raw physical byte from ADDR_CONF_ESPNOW_MODE above, since
// wire framing and PHY rate are independent axes). Same no-self-reboot
// reasoning as espnow_mode_conf_save() - only ever called from
// menu_commit_and_exit().
void espnow_lr_conf_save(uint8_t val) {
  eeprom_update(ADDR_CONF_ESPNOW_LR, val);
}
#endif

#if HAS_ETHERNET == true
// Persists the forced link speed/duplex (ETH_SPEED_*, Config.h) picked from
// the RNode Settings menu's Ethernet page. Only takes effect at boot -
// ETH.setAutoNegotiation()/setLinkSpeed()/setFullDuplex() (esp32 core)
// refuse once ETH.begin() has already run (see init_ethernet(),
// Ethernet.h) - so this reboots immediately when the value actually
// changes, same pattern as gpio_conf_save()'s pin-reassignment reboot.
void ethspd_conf_save(uint8_t val) {
  #if HAS_EEPROM
    uint8_t stored = EEPROM.read(eeprom_addr(ADDR_CONF_ETHSPD));
  #elif MCU_VARIANT == MCU_NRF52
    uint8_t stored = eeprom_read(eeprom_addr(ADDR_CONF_ETHSPD));
  #endif
	eeprom_update(eeprom_addr(ADDR_CONF_ETHSPD), val);
  #if !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
    eeprom_flush();
  #endif
	if (stored != val) { hard_reset(); }
}

// Persists a static IP/netmask (ADDR_CONF_ETH_IP/NM, ROM.h) - or, called
// with an all-zero val, clears it back to "unset" (DHCP) - in the config
// region (config_addr(), not eeprom_addr() - same region wifi_remote_
// start_sta()'s ADDR_CONF_IP/NM live in, see ROM.h). Unlike ethspd_conf_
// save() above, this never reboots: NetworkInterface::config() (esp32 core)
// can reconfigure the interface, or hand it back to DHCP, at any time after
// boot - the caller applies the change live via eth_apply_addr_config()
// (Ethernet.h) after calling this.
void ethaddr_conf_save(int addr_base, uint8_t *val) {
	for (uint8_t i = 0; i < 4; i++) {
		eeprom_update(config_addr(addr_base+i), val[i]);
	}
}
#endif

void di_conf_save(uint8_t dint) {
	eeprom_update(eeprom_addr(ADDR_CONF_DINT), dint);
}

void da_conf_save(uint8_t dadr) {
	eeprom_update(eeprom_addr(ADDR_CONF_DADR), dadr);
}

void db_conf_save(uint8_t val) {
	#if HAS_DISPLAY
		if (val == 0x00) {
			display_blanking_enabled = false;
		} else {
			display_blanking_enabled = true;
			display_blanking_timeout = val*1000;
		}
		eeprom_update(eeprom_addr(ADDR_CONF_BSET), CONF_OK_BYTE);
		eeprom_update(eeprom_addr(ADDR_CONF_DBLK), val);
	#endif
}

void drot_conf_save(uint8_t val) {
	#if HAS_DISPLAY
		if (val >= 0x00 and val <= 0x03) {
			// Only reboot if the rotation is actually changing - hosts (e.g. the
			// flasher's "Apply" button) resend all display fields together, so
			// an unrelated brightness/timeout change would otherwise also
			// reboot the device via this unconditionally-resent rotation value.
			//
			// Compared against a fresh read each call (not cached in RAM) -
			// a RAM cache was tried here to work around a flaky nRF52 flash
			// read under a write-burst, but that turned out to be a red
			// herring: the actual bug was eeprom_flush() not checking
			// whether its file reopen succeeded (see eeprom_flush() below),
			// and the cache went on to cause its own bugs (a 0xFF sentinel
			// collision with "rotation never configured", plus a
			// still-unexplained false positive on ESP32/MeshAdventurer-S3).
			// With the real bug fixed at the storage layer, a plain fresh
			// read is simpler and was already proven reliable pre-cache.
			#if HAS_EEPROM
				uint8_t stored = EEPROM.read(eeprom_addr(ADDR_CONF_DROT));
			#elif MCU_VARIANT == MCU_NRF52
				uint8_t stored = eeprom_read(eeprom_addr(ADDR_CONF_DROT));
			#endif
			// Match the client's own normalization (an erased/unset byte is
			// treated as rotation 0) so a board that's never had rotation
			// configured doesn't reboot on its first apply either.
			if (stored > 0x03) stored = 0x00;
			bool rotation_changed = stored != val;
			eeprom_update(eeprom_addr(ADDR_CONF_DROT), val);
			if (rotation_changed) { hard_reset(); }
		}
	#endif
}

void dia_conf_save(uint8_t val) {
	if (val > 0x00)  { eeprom_update(eeprom_addr(ADDR_CONF_DIA), 0x01); }
	else             { eeprom_update(eeprom_addr(ADDR_CONF_DIA), 0x00); }
	hard_reset();
}

void np_int_conf_save(uint8_t p_int) {
	eeprom_update(eeprom_addr(ADDR_CONF_PSET), CONF_OK_BYTE);
	eeprom_update(eeprom_addr(ADDR_CONF_PINT), p_int);
}


bool eeprom_have_conf() {
    #if HAS_EEPROM
	    if (EEPROM.read(eeprom_addr(ADDR_CONF_OK)) == CONF_OK_BYTE) {
    #elif MCU_VARIANT == MCU_NRF52
        if (eeprom_read(eeprom_addr(ADDR_CONF_OK)) == CONF_OK_BYTE) {
    #endif
		return true;
	} else {
		return false;
	}
}

void eeprom_conf_load() {
	if (eeprom_have_conf()) {
        #if HAS_EEPROM
            lora_sf = EEPROM.read(eeprom_addr(ADDR_CONF_SF));
            lora_cr = EEPROM.read(eeprom_addr(ADDR_CONF_CR));
            lora_txp = EEPROM.read(eeprom_addr(ADDR_CONF_TXP));
            lora_freq = (uint32_t)EEPROM.read(eeprom_addr(ADDR_CONF_FREQ)+0x00) << 24 | (uint32_t)EEPROM.read(eeprom_addr(ADDR_CONF_FREQ)+0x01) << 16 | (uint32_t)EEPROM.read(eeprom_addr(ADDR_CONF_FREQ)+0x02) << 8 | (uint32_t)EEPROM.read(eeprom_addr(ADDR_CONF_FREQ)+0x03);
            lora_bw = (uint32_t)EEPROM.read(eeprom_addr(ADDR_CONF_BW)+0x00) << 24 | (uint32_t)EEPROM.read(eeprom_addr(ADDR_CONF_BW)+0x01) << 16 | (uint32_t)EEPROM.read(eeprom_addr(ADDR_CONF_BW)+0x02) << 8 | (uint32_t)EEPROM.read(eeprom_addr(ADDR_CONF_BW)+0x03);
        #elif MCU_VARIANT == MCU_NRF52
            lora_sf = eeprom_read(eeprom_addr(ADDR_CONF_SF));
            lora_cr = eeprom_read(eeprom_addr(ADDR_CONF_CR));
            lora_txp = eeprom_read(eeprom_addr(ADDR_CONF_TXP));
            lora_freq = (uint32_t)eeprom_read(eeprom_addr(ADDR_CONF_FREQ)+0x00) << 24 | (uint32_t)eeprom_read(eeprom_addr(ADDR_CONF_FREQ)+0x01) << 16 | (uint32_t)eeprom_read(eeprom_addr(ADDR_CONF_FREQ)+0x02) << 8 | (uint32_t)eeprom_read(eeprom_addr(ADDR_CONF_FREQ)+0x03);
            lora_bw = (uint32_t)eeprom_read(eeprom_addr(ADDR_CONF_BW)+0x00) << 24 | (uint32_t)eeprom_read(eeprom_addr(ADDR_CONF_BW)+0x01) << 16 | (uint32_t)eeprom_read(eeprom_addr(ADDR_CONF_BW)+0x02) << 8 | (uint32_t)eeprom_read(eeprom_addr(ADDR_CONF_BW)+0x03);
        #endif
	}
}

void eeprom_conf_save() {
	if (hw_ready && radio_online) {
		eeprom_update(eeprom_addr(ADDR_CONF_SF), lora_sf);
		eeprom_update(eeprom_addr(ADDR_CONF_CR), lora_cr);
		eeprom_update(eeprom_addr(ADDR_CONF_TXP), lora_txp);

		eeprom_update(eeprom_addr(ADDR_CONF_BW)+0x00, lora_bw>>24);
		eeprom_update(eeprom_addr(ADDR_CONF_BW)+0x01, lora_bw>>16);
		eeprom_update(eeprom_addr(ADDR_CONF_BW)+0x02, lora_bw>>8);
		eeprom_update(eeprom_addr(ADDR_CONF_BW)+0x03, lora_bw);

		eeprom_update(eeprom_addr(ADDR_CONF_FREQ)+0x00, lora_freq>>24);
		eeprom_update(eeprom_addr(ADDR_CONF_FREQ)+0x01, lora_freq>>16);
		eeprom_update(eeprom_addr(ADDR_CONF_FREQ)+0x02, lora_freq>>8);
		eeprom_update(eeprom_addr(ADDR_CONF_FREQ)+0x03, lora_freq);

		eeprom_update(eeprom_addr(ADDR_CONF_OK), CONF_OK_BYTE);
		led_indicate_info(10);
	} else {
		led_indicate_warning(10);
	}
}

void eeprom_conf_delete() {
	eeprom_update(eeprom_addr(ADDR_CONF_OK), 0x00);
}

void unlock_rom() {
	led_indicate_error(50);
	eeprom_erase();
}

void init_channel_stats() {
	#if MCU_VARIANT == MCU_ESP32
		for (uint16_t ai = 0; ai < DCD_SAMPLES; ai++) { util_samples[ai] = false; }
		for (uint16_t ai = 0; ai < AIRTIME_BINS; ai++) { airtime_bins[ai] = 0; }
		for (uint16_t ai = 0; ai < AIRTIME_BINS; ai++) { longterm_bins[ai] = 0.0; }
		local_channel_util = 0.0;
		total_channel_util = 0.0;
		airtime = 0.0;
		longterm_airtime = 0.0;
	#endif
}

typedef struct FIFOBuffer
{
  unsigned char *begin;
  unsigned char *end;
  unsigned char * volatile head;
  unsigned char * volatile tail;
} FIFOBuffer;

inline bool fifo_isempty(const FIFOBuffer *f) {
  return f->head == f->tail;
}

inline bool fifo_isfull(const FIFOBuffer *f) {
  return ((f->head == f->begin) && (f->tail == f->end)) || (f->tail == f->head - 1);
}

inline void fifo_push(FIFOBuffer *f, unsigned char c) {
  *(f->tail) = c;
  
  if (f->tail == f->end) {
    f->tail = f->begin;
  } else {
    f->tail = f->tail + 1;
  }
}

inline unsigned char fifo_pop(FIFOBuffer *f) {
  if(f->head == f->end) {
    f->head = f->begin;
    return *(f->end);
  } else {
    unsigned char *p = f->head;
    f->head = f->head + 1;
    return *p;
  }
}

inline void fifo_flush(FIFOBuffer *f) {
  f->head = f->tail;
}

#if MCU_VARIANT != MCU_ESP32 && MCU_VARIANT != MCU_NRF52
	static inline bool fifo_isempty_locked(const FIFOBuffer *f) {
	  bool result;
	  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
	    result = fifo_isempty(f);
	  }
	  return result;
	}

	static inline bool fifo_isfull_locked(const FIFOBuffer *f) {
	  bool result;
	  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
	    result = fifo_isfull(f);
	  }
	  return result;
	}

	static inline void fifo_push_locked(FIFOBuffer *f, unsigned char c) {
	  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
	    fifo_push(f, c);
	  }
	}
#endif

/*
static inline unsigned char fifo_pop_locked(FIFOBuffer *f) {
  unsigned char c;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    c = fifo_pop(f);
  }
  return c;
}
*/

inline void fifo_init(FIFOBuffer *f, unsigned char *buffer, size_t size) {
  f->begin = buffer;
  f->tail = buffer;
  f->head = buffer;
  f->end = buffer + size;
}

inline size_t fifo_len(FIFOBuffer *f) {
  return f->end - f->begin;
}

typedef struct FIFOBuffer16
{
  uint16_t *begin;
  uint16_t *end;
  uint16_t * volatile head;
  uint16_t * volatile tail;
} FIFOBuffer16;

inline bool fifo16_isempty(const FIFOBuffer16 *f) {
  return f->head == f->tail;
}

inline bool fifo16_isfull(const FIFOBuffer16 *f) {
  return ((f->head == f->begin) && (f->tail == f->end)) || (f->tail == f->head - 1);
}

inline void fifo16_push(FIFOBuffer16 *f, uint16_t c) {
  *(f->tail) = c;

  if (f->tail == f->end) {
    f->tail = f->begin;
  } else {
    f->tail = f->tail + 1;
  }
}

inline uint16_t fifo16_pop(FIFOBuffer16 *f) {
  if(f->head == f->end) {
    f->head = f->begin;
    return *(f->end);
  } else {
    uint16_t *p = f->head;
    f->head = f->head + 1;
    return *p;
  }
}

inline void fifo16_flush(FIFOBuffer16 *f) {
  f->head = f->tail;
}

#if MCU_VARIANT != MCU_ESP32 && MCU_VARIANT != MCU_NRF52
	static inline bool fifo16_isempty_locked(const FIFOBuffer16 *f) {
	  bool result;
	  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
	    result = fifo16_isempty(f);
	  }

	  return result;
	}
#endif

/*
static inline bool fifo16_isfull_locked(const FIFOBuffer16 *f) {
  bool result;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    result = fifo16_isfull(f);
  }
  return result;
}


static inline void fifo16_push_locked(FIFOBuffer16 *f, uint16_t c) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    fifo16_push(f, c);
  }
}

static inline size_t fifo16_pop_locked(FIFOBuffer16 *f) {
  size_t c;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    c = fifo16_pop(f);
  }
  return c;
}
*/

inline void fifo16_init(FIFOBuffer16 *f, uint16_t *buffer, uint16_t size) {
  f->begin = buffer;
  f->tail = buffer;
  f->head = buffer;
  f->end = buffer + size;
}

inline uint16_t fifo16_len(FIFOBuffer16 *f) {
  return (f->end - f->begin);
}

extern void stopRadio();
void host_disconnected() {
	stopRadio();
	set_rns_link_state(RNS_LINK_STATE_DISCONNECTED);
	current_rssi  = -292;
	last_rssi     = -292;
	last_rssi_raw = 0x00;
	last_snr_raw  = 0x80;
}