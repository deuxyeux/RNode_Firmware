#pragma once
// Self-contained rotary encoder driver - ISR-based quadrature decode plus
// push-button debounce, adapted from RNode_Firmware/Encoder.h's proven
// approach but stripped of all Menu.h/vault-unlock integration since this
// test only needs a raw rotate delta and click events.

#include <Arduino.h>

void encoder_begin();

// Call every loop() iteration. Returns -1/0/+1 for one detent of rotation
// since the last call (0 most of the time).
int8_t encoder_read_rotation();

// Call every loop() iteration. Returns true exactly once, on the iteration
// a debounced press-then-release completes.
bool encoder_read_click();
