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

#ifndef ENCODER_H
  #define ENCODER_H

  // Common EC11 case: one detent = four quadrature counts. If a physical
  // click moves the menu cursor by two positions instead of one on real
  // hardware, this part detents every half-cycle instead - drop to 2.
  #define ENCODER_STEPS_PER_DETENT 4

  #define ENC_PRESSED  LOW
  #define ENC_RELEASED HIGH

  // Forward declarations - implemented in Menu.h. wrap defaults false here,
  // so the physical encoder always clamps at list ends - only the main
  // button's cycling (menu_button_press()/menu_button_process()) passes
  // wrap=true.
  void menu_encoder_rotate(int8_t dir, bool wrap = false);
  void menu_encoder_button(unsigned long duration);
  // Dispatched instead of menu_encoder_rotate() when a rotation tick
  // arrives while the push-button is physically held down (see
  // encoder_process() below) - only MENU_STATE_MSNGR_TEXT_ENTRY gives the
  // resulting chord any meaning, everywhere else it just falls through to
  // the normal rotate.
  void menu_encoder_chord_rotate(int8_t dir);
  extern bool msngr_kb_chord_used;

  // Forward declarations - implemented in VaultUnlock.h. When the boot-
  // unlock screen (or the Menu.h PIN-enable/disable flow) owns the input,
  // rotation/press dispatch redirects here instead of into Menu.h - see
  // encoder_process()'s own vault_unlock_active check below. menu_state
  // isn't initialized yet during the boot-unlock screen, so this can't
  // just be handled inside menu_encoder_rotate()/menu_encoder_button()
  // themselves.
  //
  // VaultUnlock.h is only included under HAS_URNS == true (Utilities.h) -
  // stub fallbacks here for the false case, same "always false, no vault"
  // effect as RNode_Firmware.ino's button_event() gets from wrapping its
  // own vault_unlock_active checks in #if HAS_URNS == true, but without
  // needing to scatter that guard through encoder_process()'s own
  // interspersed if/else-if logic below (found via a full-fleet release
  // build - promicro/meshadventurer, both non-URNS, failed to link on
  // these exact symbols).
  #if HAS_URNS == true
    extern bool vault_unlock_active;
    extern bool vault_suppress_next_release;
    void vault_unlock_encoder_rotate(int8_t dir);
    void vault_unlock_encoder_button(unsigned long duration);
  #else
    static bool vault_unlock_active = false;
    static bool vault_suppress_next_release = false;
    inline void vault_unlock_encoder_rotate(int8_t dir) {}
    inline void vault_unlock_encoder_button(unsigned long duration) {}
  #endif

  // Classic 4-state Gray-code quadrature transition table, indexed by
  // (prev_AB<<2)|curr_AB. Illegal transitions (both channels changed
  // between reads - noise the board's RC filtering didn't fully catch)
  // yield 0 and are silently discarded.
  static const int8_t ENCODER_TABLE[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
  };

  volatile uint8_t encoder_prev_ab = 0;
  volatile int8_t  encoder_quarter = 0;
  volatile int8_t  encoder_delta   = 0;

  // ESP-IDF's spinlock (portMUX_TYPE) has no nRF52 equivalent - a plain
  // global interrupt gate is all that's needed here anyway, since the
  // critical sections just protect a couple of int8_t's from a single-core
  // MCU's own ISR.
  #if MCU_VARIANT == MCU_ESP32
    portMUX_TYPE encoder_mux = portMUX_INITIALIZER_UNLOCKED;
    #define ENCODER_ENTER_CRITICAL_ISR() portENTER_CRITICAL_ISR(&encoder_mux)
    #define ENCODER_EXIT_CRITICAL_ISR()  portEXIT_CRITICAL_ISR(&encoder_mux)
    #define ENCODER_ENTER_CRITICAL()     portENTER_CRITICAL(&encoder_mux)
    #define ENCODER_EXIT_CRITICAL()      portEXIT_CRITICAL(&encoder_mux)
  #else
    #define ENCODER_ENTER_CRITICAL_ISR() noInterrupts()
    #define ENCODER_EXIT_CRITICAL_ISR()  interrupts()
    #define ENCODER_ENTER_CRITICAL()     noInterrupts()
    #define ENCODER_EXIT_CRITICAL()      interrupts()
  #endif

  void ISR_VECT encoder_isr() {
    uint8_t curr_ab = (digitalRead(pin_encoder_up) << 1) | digitalRead(pin_encoder_down);
    int8_t  step    = ENCODER_TABLE[(encoder_prev_ab << 2) | curr_ab];
    encoder_prev_ab = curr_ab;
    if (step != 0) {
      ENCODER_ENTER_CRITICAL_ISR();
      encoder_quarter += step;
      if (encoder_quarter >= ENCODER_STEPS_PER_DETENT)  { encoder_delta = 1;  encoder_quarter = 0; }
      if (encoder_quarter <= -ENCODER_STEPS_PER_DETENT) { encoder_delta = -1; encoder_quarter = 0; }
      ENCODER_EXIT_CRITICAL_ISR();
    }
  }

  // Encoder push-button debounce state. Self-contained (own PRESSED/
  // RELEASED constants) rather than reusing Input.h's, since this is a
  // separate physical pin with its own state machine.
  int enc_btn_state          = ENC_RELEASED;
  int enc_btn_debounce_state = enc_btn_state;
  unsigned long enc_btn_debounce_last = 0;
  const unsigned long ENC_BTN_DEBOUNCE_DELAY = 25;
  unsigned long enc_btn_down_last = 0;

  // Set false on every new press, latched true once the hold's crossed
  // menu_encoder_button()'s own long-press threshold (Menu.h) - gives a
  // one-shot "you're past the threshold" tick while still held, matching
  // the main button's per-tier ticks (draw_button_hold_overlay(), Menu.h),
  // instead of only finding out what a hold was about to do after already
  // releasing it.
  bool enc_btn_hold_beeped = false;

  void encoder_init() {
    pinMode(pin_encoder_up, INPUT_PULLUP);
    pinMode(pin_encoder_down, INPUT_PULLUP);
    pinMode(pin_encoder_press, INPUT_PULLUP);
    encoder_prev_ab = (digitalRead(pin_encoder_up) << 1) | digitalRead(pin_encoder_down);
    attachInterrupt(digitalPinToInterrupt(pin_encoder_up),   encoder_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(pin_encoder_down), encoder_isr, CHANGE);
  }

  // Called every loop() iteration: drains rotation steps captured by the
  // ISR and runs the press-button debounce, dispatching into Menu.h.
  void encoder_process() {
    int8_t d = 0;
    ENCODER_ENTER_CRITICAL();
    d = encoder_delta;
    encoder_delta = 0;
    ENCODER_EXIT_CRITICAL();
    // Rotation/button state above is still tracked regardless (so nothing's
    // left half-updated if this gets re-enabled later), but only actually
    // reaches the menu if the board's encoder is flagged as populated -
    // see encoder_enabled, MENU_ITEM_ENCODER.
    //
    // Holding the button down while turning is a chord (capital letters on
    // MENU_STATE_MSNGR_TEXT_ENTRY, see menu_encoder_chord_rotate()) rather
    // than a plain rotate - checked against the debounced button state
    // (enc_btn_state), not a raw pin read, so it can't flicker mid-turn on
    // contact bounce.
    // vault_unlock_active bypasses the encoder_enabled preference check -
    // that flag is the user's menu-navigation input preference (defaults
    // OFF even on encoder-equipped boards, RNode_Firmware.ino's own
    // comment on ADDR_CONF_ENA), a different thing from "does this board
    // physically have an encoder." The boot-unlock screen runs before that
    // preference is even read from EEPROM (urns_init() runs earlier than
    // ADDR_CONF_ENA's read in setup()), and a locked-out user should be
    // able to use working hardware regardless of an unrelated nav setting.
    if (d != 0 && (encoder_enabled || vault_unlock_active)) {
      if (vault_unlock_active) vault_unlock_encoder_rotate(d);
      else if (enc_btn_state == ENC_PRESSED) menu_encoder_chord_rotate(d);
      else menu_encoder_rotate(d);
    }

    int reading = digitalRead(pin_encoder_press);
    if (reading != enc_btn_debounce_state) {
      enc_btn_debounce_last = millis();
      enc_btn_debounce_state = reading;
    }

    if ((millis() - enc_btn_debounce_last) > ENC_BTN_DEBOUNCE_DELAY) {
      if (reading != enc_btn_state) {
        enc_btn_state = reading;
        if (enc_btn_state == ENC_PRESSED) {
          enc_btn_down_last = millis();
          enc_btn_hold_beeped = false;
          msngr_kb_chord_used = false; // fresh press - no chord performed with it yet
          #if HAS_LXMF == true || HAS_WIFI == true
            msngr_kb_lang_hold_fired_enc = false;  // fresh press - EN/RU switch hasn't fired yet either
            msngr_kb_alt_hold_fired_enc = false; // ...nor has the punctuation/letter-alternate
            msngr_kb_del_hold_fired_enc = false;   // ...nor has DEL-repeat
            msngr_kb_del_repeat_last_enc = 0;
          #endif
        } else if (encoder_enabled || vault_unlock_active) {
          if (vault_unlock_active) vault_unlock_encoder_button(millis() - enc_btn_down_last);
          else if (vault_suppress_next_release) vault_suppress_next_release = false;
          else menu_encoder_button(millis() - enc_btn_down_last);
        }
      }
    }

    // Same threshold logic as menu_encoder_button() (Menu.h) - announces
    // "release now" whether that release is about to open Settings from
    // closed, commit-and-exit an already-open menu/submenu, or (composing
    // a message) leave text entry. Skipped for MENU_STATE_STATUS_POPUP,
    // which has no long-press-specific meaning of its own - any release
    // there just dismisses the popup, short or long alike.
    unsigned long hold_beep_threshold = 700;
    #if HAS_LXMF == true || HAS_WIFI == true
      if (menu_state == MENU_STATE_MSNGR_TEXT_ENTRY) hold_beep_threshold = 3000;
    #endif
    // Suppressed during the vault unlock/enroll screen - this tone's
    // meaning ("hold longer and release to trigger the current context's
    // long-press action") is specifically wrong there: it's inherited from
    // Settings' own long-press-opens-Settings semantics, and hearing it
    // mid-PIN-entry told the user "release now and this will open
    // Settings," which isn't what a long hold does here at all
    // (backspace/submit - see vault_unlock_encoder_button(), VaultUnlock.h).
    if (encoder_enabled && enc_btn_state == ENC_PRESSED && !enc_btn_hold_beeped &&
        menu_state != MENU_STATE_STATUS_POPUP && !vault_unlock_active &&
        !vault_suppress_next_release &&
        (millis() - enc_btn_down_last) > hold_beep_threshold) {
      buzzer_encoder_tick_melody();
      enc_btn_hold_beeped = true;
    }

    // Live EN/RU switch, punctuation/letter-alternate, and DEL-repeat on
    // a held encoder button - see Menu.h's msngr_kb_lang_hold_try()/
    // msngr_kb_alt_hold_try()/msngr_kb_del_hold_try() for the shared
    // logic (this control's own separate fired-flags are needed because
    // enc_btn_state is a different debounce state machine than the
    // main button's, Input.h).
    #if HAS_LXMF == true || HAS_WIFI == true
      if (encoder_enabled && enc_btn_state == ENC_PRESSED) {
        unsigned long held_ms = millis() - enc_btn_down_last;
        msngr_kb_lang_hold_try(held_ms, msngr_kb_lang_hold_fired_enc);
        msngr_kb_alt_hold_try(held_ms, msngr_kb_alt_hold_fired_enc);
        msngr_kb_del_hold_try(held_ms, msngr_kb_del_hold_fired_enc, msngr_kb_del_repeat_last_enc);
      } else {
        msngr_kb_lang_hold_fired_enc = false;
        msngr_kb_alt_hold_fired_enc = false;
        msngr_kb_del_hold_fired_enc = false;
        msngr_kb_del_repeat_last_enc = 0;
      }
    #endif
  }

#endif
