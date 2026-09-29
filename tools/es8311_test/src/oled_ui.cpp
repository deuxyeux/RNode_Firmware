#include "oled_ui.h"
#include "pins.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

static Adafruit_SSD1306 display(DISP_W, DISP_H, &Wire, DISP_RST);

bool oled_init() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, DISP_ADDR)) {
    Serial.println("[OLED] display.begin() FAILED - check DISP_ADDR/wiring");
    return false;
  }
  display.setRotation(0);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.display();
  Serial.println("[OLED] init OK");
  return true;
}

static void drawBar(int x, int y, int w, int h, int percent) {
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  display.drawRect(x, y, w, h, SSD1306_WHITE);
  int fillW = (w - 2) * percent / 100;
  if (fillW > 0) display.fillRect(x + 1, y + 1, fillW, h - 2, SSD1306_WHITE);
}

static void drawMMSS(int x, int y, uint32_t ms) {
  uint32_t totalSec = ms / 1000;
  display.setCursor(x, y);
  display.printf("%02u:%02u", (unsigned)(totalSec / 60), (unsigned)(totalSec % 60));
}

void oled_draw_home(int cursor, bool hasRecording, const char *modeName) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("ES8311 Audio Test");
  display.drawFastHLine(0, 9, DISP_W, SSD1306_WHITE);

  display.setCursor(6, 13);
  display.print(cursor == 0 ? "> Record" : "  Record");

  display.setCursor(6, 24);
  if (hasRecording) {
    display.print(cursor == 1 ? "> Play Raw" : "  Play Raw");
  } else {
    display.print("  Play Raw (empty)");
  }

  display.setCursor(6, 35);
  if (hasRecording) {
    display.print(cursor == 2 ? "> Play Codec2" : "  Play Codec2");
  } else {
    display.print("  Play Codec2 (empty)");
  }

  display.setCursor(6, 46);
  display.print(cursor == 3 ? "> Mode: " : "  Mode: ");
  display.print(modeName);

  display.setCursor(0, 56);
  display.print("Turn=select Click=go/next");
  display.display();
}

void oled_draw_recording(uint32_t elapsedMs, uint32_t maxMs, int level) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Recording...");

  drawMMSS(0, 16, elapsedMs);
  display.print(" / ");
  drawMMSS(48, 16, maxMs);

  display.setCursor(0, 30);
  display.print("Level:");
  drawBar(0, 40, DISP_W, 12, level);

  display.setCursor(0, 56);
  display.print("Click to stop");
  display.display();
}

void oled_draw_processing(const char *modeName) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Codec2 ");
  display.print(modeName);
  display.setCursor(0, 24);
  display.print("Encoding + decoding...");
  display.display();
}

void oled_draw_codec2_result(size_t encodedBytes, float bitrateKbps, uint32_t durationMs) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Codec2 result:");
  display.setCursor(0, 16);
  display.printf("%u bytes", (unsigned)encodedBytes);
  display.setCursor(0, 28);
  display.printf("%.2f kbps", bitrateKbps);
  display.setCursor(0, 40);
  display.print("Duration: ");
  drawMMSS(66, 40, durationMs);
  display.display();
}

void oled_draw_playing(uint32_t elapsedMs, uint32_t totalMs, const char *label) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Playing: ");
  display.print(label);

  drawMMSS(0, 16, elapsedMs);
  display.print(" / ");
  drawMMSS(48, 16, totalMs);

  int percent = totalMs > 0 ? (int)((uint64_t)elapsedMs * 100 / totalMs) : 0;
  drawBar(0, 32, DISP_W, 12, percent);

  display.setCursor(0, 56);
  display.print("Click to stop");
  display.display();
}
