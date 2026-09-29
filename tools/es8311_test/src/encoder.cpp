#include "encoder.h"
#include "pins.h"

namespace {
  constexpr uint8_t STEPS_PER_DETENT = 4;
  constexpr int PRESSED = LOW;
  constexpr int RELEASED = HIGH;

  // Classic 4-state Gray-code quadrature transition table, indexed by
  // (prev_AB<<2)|curr_AB - same table as RNode_Firmware/Encoder.h.
  const int8_t TABLE[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
  };

  volatile uint8_t prevAB = 0;
  volatile int8_t quarter = 0;
  volatile int8_t delta = 0;
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

  void IRAM_ATTR isr() {
    uint8_t curAB = (digitalRead(PIN_ENCODER_UP) << 1) | digitalRead(PIN_ENCODER_DOWN);
    int8_t step = TABLE[(prevAB << 2) | curAB];
    prevAB = curAB;
    if (step != 0) {
      portENTER_CRITICAL_ISR(&mux);
      quarter += step;
      if (quarter >= STEPS_PER_DETENT) { delta = 1; quarter = 0; }
      if (quarter <= -STEPS_PER_DETENT) { delta = -1; quarter = 0; }
      portEXIT_CRITICAL_ISR(&mux);
    }
  }

  int btnState = RELEASED;
  int btnDebounceState = RELEASED;
  unsigned long btnDebounceLast = 0;
  const unsigned long BTN_DEBOUNCE_DELAY = 25;
}  // namespace

void encoder_begin() {
  pinMode(PIN_ENCODER_UP, INPUT_PULLUP);
  pinMode(PIN_ENCODER_DOWN, INPUT_PULLUP);
  pinMode(PIN_ENCODER_PRESS, INPUT_PULLUP);
  prevAB = (digitalRead(PIN_ENCODER_UP) << 1) | digitalRead(PIN_ENCODER_DOWN);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_UP), isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_DOWN), isr, CHANGE);
}

int8_t encoder_read_rotation() {
  int8_t d;
  portENTER_CRITICAL(&mux);
  d = delta;
  delta = 0;
  portEXIT_CRITICAL(&mux);
  return d;
}

bool encoder_read_click() {
  int reading = digitalRead(PIN_ENCODER_PRESS);
  if (reading != btnDebounceState) {
    btnDebounceLast = millis();
    btnDebounceState = reading;
  }
  bool clicked = false;
  if ((millis() - btnDebounceLast) > BTN_DEBOUNCE_DELAY) {
    if (reading != btnState) {
      btnState = reading;
      if (btnState == RELEASED) clicked = true;  // fire on release, not press
    }
  }
  return clicked;
}
