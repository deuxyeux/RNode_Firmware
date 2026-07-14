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
    if (d != 0 && encoder_enabled) menu_encoder_rotate(d);

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
        } else if (encoder_enabled) {
          menu_encoder_button(millis() - enc_btn_down_last);
        }
      }
    }
  }

#endif
