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
void db_conf_save(uint8_t val);
void di_conf_save(uint8_t dint);
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
void eeprom_update(int mapped_addr, uint8_t byte);
void buzzer_encoder_tick_melody();
void buzzer_encoder_click_melody();

#if HAS_DISPLAY == true
  #include "Display.h"
#else
	void display_unblank() {}
	bool display_blanked = false;
	#define DISPLAY_IS_OLED false
#endif

#if HAS_BLUETOOTH == true || HAS_BLE == true
	void kiss_indicate_btpin();
#endif
#if MCU_VARIANT == MCU_ESP32 || HAS_BLUETOOTH == true || HAS_BLE == true
  #include "Bluetooth.h"
#endif

#if HAS_WIFI == true
  #include "Remote.h"
#endif

#if HAS_ETHERNET == true
  #include "Ethernet.h"
#endif

#if HAS_PMU == true || IS_ESP32S3
  #include "Power.h"
#endif

#if HAS_INPUT == true
	#include "Input.h"
#endif

#if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
	#include "Device.h"
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
  bool pixels_started = false;

  void led_set_intensity(uint8_t intensity) {
  	npi = (float)intensity/255.0;
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
  	for (int i = 0; i < 1*sizeof(rs); i++) {
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
  }

  // Avoid tone()'s duration overload: its internal auto-stop timer can
  // race with the noTone() call at the end of each note in a tight loop
  // and crash the LEDC driver, so time each note manually instead.
  void buzzer_play_notes(const uint16_t *notes, uint8_t count, uint16_t note_ms) {
    if (!sound_enabled) return;
    for (uint8_t i = 0; i < count; i++) {
      tone(buzzer_pin, notes[i]);
      delay(note_ms);
      noTone(buzzer_pin);
      digitalWrite(buzzer_pin, LOW); // see buzzer_init()
      delay(10);
    }
  }

  // Simple ascending startup jingle, played once while the boot banner is
  // shown. Runs during setup(), before loop() starts, so blocking is fine
  // here - nothing else is competing for CPU time yet.
  void buzzer_boot_melody() {
    const uint16_t notes[] = { 1319, 1568, 1976, 2637 };
    buzzer_play_notes(notes, sizeof(notes)/sizeof(notes[0]), 80);
  }

  // Non-blocking melody player for cues triggered from hot paths (button
  // handling, serial_callback()). Blocking here would stall packet queue
  // processing and display updates for the duration of the melody, which
  // was observed to disrupt the waterfall right as an RNS host attaches.
  // buzzer_update() must be called every loop() iteration to advance it.
  const uint16_t *buzzer_async_notes = NULL;
  uint8_t buzzer_async_count = 0;
  uint8_t buzzer_async_index = 0;
  uint16_t buzzer_async_note_ms = 0;
  bool buzzer_async_in_gap = false;
  bool buzzer_async_playing = false;
  unsigned long buzzer_async_phase_started = 0;
  const uint16_t BUZZER_ASYNC_GAP_MS = 10;

  void buzzer_start_async_melody(const uint16_t *notes, uint8_t count, uint16_t note_ms) {
    if (!sound_enabled) return;
    buzzer_async_notes = notes;
    buzzer_async_count = count;
    buzzer_async_note_ms = note_ms;
    buzzer_async_index = 0;
    buzzer_async_in_gap = false;
    buzzer_async_playing = true;
    tone(buzzer_pin, notes[0]);
    buzzer_async_phase_started = millis();
  }

  void buzzer_update() {
    if (!buzzer_async_playing) return;
    unsigned long now = millis();
    if (!buzzer_async_in_gap) {
      if (now - buzzer_async_phase_started >= buzzer_async_note_ms) {
        noTone(buzzer_pin);
        digitalWrite(buzzer_pin, LOW); // see buzzer_init()
        buzzer_async_in_gap = true;
        buzzer_async_phase_started = now;
      }
    } else if (now - buzzer_async_phase_started >= BUZZER_ASYNC_GAP_MS) {
      buzzer_async_index++;
      if (buzzer_async_index >= buzzer_async_count) {
        buzzer_async_playing = false;
      } else {
        tone(buzzer_pin, buzzer_async_notes[buzzer_async_index]);
        buzzer_async_in_gap = false;
        buzzer_async_phase_started = now;
      }
    }
  }

  // Short two-note cues for Bluetooth toggling via the user button.
  void buzzer_bt_on_melody() {
    static const uint16_t notes[] = { 1568, 1175 };
    buzzer_start_async_melody(notes, sizeof(notes)/sizeof(notes[0]), 60);
  }

  void buzzer_bt_off_melody() {
    static const uint16_t notes[] = { 1568, 2093 };
    buzzer_start_async_melody(notes, sizeof(notes)/sizeof(notes[0]), 60);
  }

  // Short chirps for the RNS host (rns_link_state) attaching to / leaving the KISS interface.
  void buzzer_rns_connect_melody() {
    static const uint16_t notes[] = { 1976, 2637 };
    buzzer_start_async_melody(notes, sizeof(notes)/sizeof(notes[0]), 45);
  }

  void buzzer_rns_disconnect_melody() {
    static const uint16_t notes[] = { 1319, 988 };
    buzzer_start_async_melody(notes, sizeof(notes)/sizeof(notes[0]), 45);
  }

  // Single-note, very short ticks for encoder feedback - deliberately
  // shorter than the other cues so rapid rotation doesn't get annoying.
  void buzzer_encoder_tick_melody() {
    static const uint16_t notes[] = { 500 };
    buzzer_start_async_melody(notes, 1, 12);
  }

  void buzzer_encoder_click_melody() {
    static const uint16_t notes[] = { 1200 };
    buzzer_start_async_melody(notes, 1, 12);
  }
#else
  void buzzer_init() { }
  void buzzer_update() { }
  void buzzer_bt_on_melody() { }
  void buzzer_bt_off_melody() { }
  void buzzer_boot_melody() { }
  void buzzer_rns_connect_melody() { }
  void buzzer_rns_disconnect_melody() { }
  void buzzer_encoder_tick_melody() { }
  void buzzer_encoder_click_melody() { }
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
	#endif
#elif MCU_VARIANT == MCU_NRF52
    #if HAS_NP == true
      void led_rx_on()  { npset(0, 0, 0xFF); }
      void led_rx_off() {	npset(0, 0, 0); }
      void led_tx_on()  { npset(0xFF, 0x50, 0x00); }
      void led_tx_off() { npset(0, 0, 0); }
			void led_id_on()  { npset(0x90, 0, 0x70); }
			void led_id_off() { npset(0, 0, 0); }
    #elif BOARD_MODEL == BOARD_RAK4631
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
  #elif BOARD_MODEL == BOARD_HELTEC_T114
    // Heltec T114 pulls pins LOW to turn on
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
		int led_standby_cut = 100;
		int led_standby_min = 0;
		int led_standby_max = 375+led_standby_lng;
		int led_notready_min = 0;
		int led_notready_max = led_standby_max;
		int led_notready_value = led_notready_min;
		int8_t  led_notready_direction = 0;
		unsigned long led_notready_ticks = 0;
		unsigned long led_standby_wait = 350;
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
				led_standby_value += led_standby_direction;
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

void serial_write(uint8_t byte) {
	#if HAS_BLUETOOTH || HAS_BLE == true
		if (bt_state != BT_STATE_CONNECTED) {
			#if HAS_ETHERNET
				if (eth_is_connected && wifi_host_is_connected()) { connection.write(byte); }
				#if HAS_WIFI
				else if (wifi_host_is_connected()) { wifi_remote_write(byte); }
				#endif
				else                               { Serial.write(byte); }
			#elif HAS_WIFI
				if (wifi_host_is_connected()) { wifi_remote_write(byte); }
				else                          { Serial.write(byte); }
			#else
				Serial.write(byte);
			#endif
		} else {
			SerialBT.write(byte);
      #if MCU_VARIANT == MCU_NRF52 && HAS_BLE
	      // This ensures that the TX buffer is flushed after a frame is queued in serial.
	      // serial_in_frame is used to ensure that the flush only happens at the end of the frame
	      if (serial_in_frame && byte == FEND) { SerialBT.flushTXD(); serial_in_frame = false; }
	      else if (!serial_in_frame && byte == FEND) { serial_in_frame = true; }
      #endif
		}
	#else
		Serial.write(byte);
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

void kiss_indicate_radiostate() {
	serial_write(FEND);
	serial_write(CMD_RADIO_STATE);
	serial_write(radio_online);
	serial_write(FEND);
}

void kiss_indicate_stat_rx() {
	serial_write(FEND);
	serial_write(CMD_STAT_RX);
	escaped_serial_write(stat_rx>>24);
	escaped_serial_write(stat_rx>>16);
	escaped_serial_write(stat_rx>>8);
	escaped_serial_write(stat_rx);
	serial_write(FEND);
}

void kiss_indicate_stat_tx() {
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
	serial_write(FEND);
	serial_write(CMD_STAT_RSSI);
	escaped_serial_write(packet_rssi_val);
	serial_write(FEND);
}

void kiss_indicate_stat_snr() {
	serial_write(FEND);
	serial_write(CMD_STAT_SNR);
	escaped_serial_write(last_snr_raw);
	serial_write(FEND);
}

void kiss_indicate_radio_lock() {
	serial_write(FEND);
	serial_write(CMD_RADIO_LOCK);
	serial_write(radio_locked);
	serial_write(FEND);
}

void kiss_indicate_spreadingfactor() {
	serial_write(FEND);
	serial_write(CMD_SF);
	serial_write((uint8_t)lora_sf);
	serial_write(FEND);
}

void kiss_indicate_codingrate() {
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
	serial_write(FEND);
	serial_write(CMD_BANDWIDTH);
	escaped_serial_write(lora_bw>>24);
	escaped_serial_write(lora_bw>>16);
	escaped_serial_write(lora_bw>>8);
	escaped_serial_write(lora_bw);
	serial_write(FEND);
}

void kiss_indicate_frequency() {
	serial_write(FEND);
	serial_write(CMD_FREQUENCY);
	escaped_serial_write(lora_freq>>24);
	escaped_serial_write(lora_freq>>16);
	escaped_serial_write(lora_freq>>8);
	escaped_serial_write(lora_freq);
	serial_write(FEND);
}

void kiss_indicate_st_alock() {
	uint16_t at = (uint16_t)(st_airtime_limit*100*100);
	serial_write(FEND);
	serial_write(CMD_ST_ALOCK);
	escaped_serial_write(at>>8);
	escaped_serial_write(at);
	serial_write(FEND);
}

void kiss_indicate_lt_alock() {
	uint16_t at = (uint16_t)(lt_airtime_limit*100*100);
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
			float pmu_temp = pmu_temperature+PMU_TEMP_OFFSET;
			uint8_t temp = (uint8_t)pmu_temp;
			serial_write(FEND);
			serial_write(CMD_STAT_TEMP);
			escaped_serial_write(pmu_temp);
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
		#if HAS_MENU == true
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
			escaped_serial_write(0x00);
			uint8_t *da = disp_area.getBuffer();
			uint8_t *sa = stat_area.getBuffer();
			size_t da_len = ((disp_area.width()+7)/8)*disp_area.height();
			size_t sa_len = ((stat_area.width()+7)/8)*stat_area.height();
			for (size_t i = 0; i < da_len; i++) { escaped_serial_write(da[i]); }
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
    #elif BOARD_MODEL == BOARD_HELTEC_T096
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
	#elif BOARD_MODEL == BOARD_HELTEC_T096
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
	if (rval == PRODUCT_RNODE || rval == BOARD_RNODE_NG_20 || rval == BOARD_RNODE_NG_21 || rval == PRODUCT_HMBRW || rval == PRODUCT_TBEAM || rval == PRODUCT_T32_10 || rval == PRODUCT_T32_20 || rval == PRODUCT_T32_21 || rval == PRODUCT_H32_V2 || rval == PRODUCT_H32_V3 || rval == PRODUCT_H32_V4 || rval == PRODUCT_TDECK_V1 || rval == PRODUCT_TBEAM_S_V1 || rval == PRODUCT_TBEAM_S_V3 || rval == PRODUCT_XIAO_S3) {
	#elif PLATFORM == PLATFORM_NRF52
	if (rval == PRODUCT_RAK4631 || rval == PRODUCT_HELTEC_T114 || rval == PRODUCT_HELTEC_T096 || rval == PRODUCT_TECHO || rval == PRODUCT_HMBRW) {
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
	if (model == MODEL_FF || model == MODEL_FE || model = MODEL_FD) {
	#elif BOARD_MODEL == BOARD_TBEAM
	if (model == MODEL_E4 || model == MODEL_E9 || model == MODEL_E3 || model == MODEL_E8) {
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
  #elif BOARD_MODEL == BOARD_RAK4631
  if (model == MODEL_11 || model == MODEL_12) {
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
    f->tail++;
  }
}

inline unsigned char fifo_pop(FIFOBuffer *f) {
  if(f->head == f->end) {
    f->head = f->begin;
    return *(f->end);
  } else {
    return *(f->head++);
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
  f->head = f->tail = f->begin = buffer;
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
    f->tail++;
  }
}

inline uint16_t fifo16_pop(FIFOBuffer16 *f) {
  if(f->head == f->end) {
    f->head = f->begin;
    return *(f->end);
  } else {
    return *(f->head++);
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
  f->head = f->tail = f->begin = buffer;
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