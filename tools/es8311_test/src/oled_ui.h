#pragma once
// Thin SSD1306 wrapper with the handful of screens this test needs. Reuses
// the real firmware's I2C pins/address (pins.h) and Adafruit_SSD1306 - same
// library the main RNode_Firmware already depends on.

#include <Arduino.h>

// Brings the OLED up (assumes Wire.begin() already happened - shared bus
// with the ES8311, see main.cpp). Returns false if display.begin() fails.
bool oled_init();

// cursor: 0=Record, 1=Play Raw, 2=Play Codec2, 3=Mode (see main.cpp's Mode
// enum / homeCursor). modeName: current Codec2 mode label (e.g. "2400").
void oled_draw_home(int cursor, bool hasRecording, const char *modeName);
// level: 0-100, rough visual gauge of live mic input while recording.
void oled_draw_recording(uint32_t elapsedMs, uint32_t maxMs, int level);
void oled_draw_processing(const char *modeName);
void oled_draw_codec2_result(size_t encodedBytes, float bitrateKbps, uint32_t durationMs);
// label: e.g. "Raw" or "Codec2 2400".
void oled_draw_playing(uint32_t elapsedMs, uint32_t totalMs, const char *label);
