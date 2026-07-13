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

#include "Graphics.h"
#include <Adafruit_GFX.h>

#if BOARD_MODEL != BOARD_TECHO
  #if BOARD_MODEL == BOARD_TDECK
    #include <Adafruit_ST7789.h>
  #elif BOARD_MODEL == BOARD_HELTEC_T114
    #include "ST7789.h"
    #define COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))
  #elif BOARD_MODEL == BOARD_HELTEC_T096
    #include <Adafruit_ST7735.h>
    // The T096 panel is wired BGR: the high field drives blue, so red and
    // blue swap places compared to standard RGB565
    #define COLOR565(r, g, b) (((b & 0xF8) << 8) | ((g & 0xFC) << 3) | ((r & 0xF8) >> 3))
  #elif BOARD_MODEL == BOARD_TBEAM_S_V1 || BOARD_MODEL == BOARD_TBEAM_S_V3
    #include <Adafruit_SH110X.h>
  #else
    #include <Wire.h>
    #include <Adafruit_SSD1306.h>
  #endif

#else
  void (*display_callback)();
  void display_add_callback(void (*callback)()) { display_callback = callback; }
  void busyCallback(const void* p) { display_callback(); }
  #define SSD1306_BLACK GxEPD_BLACK
  #define SSD1306_WHITE GxEPD_WHITE
  #include <GxEPD2_BW.h>
  #include <SPI.h>
#endif

#include "Fonts/Org_01.h"
#define DISP_W 128
#define DISP_H 64

#if BOARD_MODEL == BOARD_RNODE_NG_20 || BOARD_MODEL == BOARD_LORA32_V2_0
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
#elif BOARD_MODEL == BOARD_TBEAM
  #define DISP_RST 13
  #define DISP_ADDR 0x3C
  #define DISP_CUSTOM_ADDR true
#elif BOARD_MODEL == BOARD_HELTEC32_V2 || BOARD_MODEL == BOARD_LORA32_V1_0
  #define DISP_RST 16
  #define DISP_ADDR 0x3C
  #define SCL_OLED 15
  #define SDA_OLED 4
#elif BOARD_MODEL == BOARD_HELTEC32_V3
  #define DISP_RST 21
  #define DISP_ADDR 0x3C
  #define SCL_OLED 18
  #define SDA_OLED 17
#elif BOARD_MODEL == BOARD_HELTEC32_V4
  #define DISP_RST 21
  #define DISP_ADDR 0x3C
  #define SCL_OLED 18
  #define SDA_OLED 17
#elif BOARD_MODEL == BOARD_GENERIC_ESP32
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define SCL_OLED 22
  #define SDA_OLED 11
#elif BOARD_MODEL == BOARD_MESHPOE_S3
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define SCL_OLED 48
  #define SDA_OLED 47
#elif BOARD_MODEL == BOARD_MESHADVENTURER_S3
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define SCL_OLED 42
  #define SDA_OLED 41
#elif BOARD_MODEL == BOARD_MESHADVENTURER
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define SCL_OLED 22
  #define SDA_OLED 21
#elif BOARD_MODEL == BOARD_DIY_V1
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define SCL_OLED 22
  #define SDA_OLED 21
#elif BOARD_MODEL == BOARD_AETHERNODE
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define SCL_OLED 22
  #define SDA_OLED 21
#elif BOARD_MODEL == BOARD_AETHERNODE_S3
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define SCL_OLED 9
  #define SDA_OLED 8
#elif BOARD_MODEL == BOARD_PROMICRO
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define SCL_OLED 7
  #define SDA_OLED 8
#elif BOARD_MODEL == BOARD_RAK4631
  // RAK1921/SSD1306
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define SCL_OLED 14
  #define SDA_OLED 13
#elif BOARD_MODEL == BOARD_RNODE_NG_21
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
#elif BOARD_MODEL == BOARD_T3S3
  #define DISP_RST 21
  #define DISP_ADDR 0x3C
  #define SCL_OLED 17
  #define SDA_OLED 18
#elif BOARD_MODEL == BOARD_TECHO
  SPIClass displaySPI = SPIClass(NRF_SPIM0, pin_disp_miso, pin_disp_sck, pin_disp_mosi);
  #define DISP_W 128
  #define DISP_H 64
  #define DISP_ADDR -1
#elif BOARD_MODEL == BOARD_TBEAM_S_V1
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define SCL_OLED 18
  #define SDA_OLED 17
  #define DISP_CUSTOM_ADDR false
#elif BOARD_MODEL == BOARD_TBEAM_S_V3
  #define DISP_RST -1
  #define DISP_ADDR 0x3D
  #define SCL_OLED 18
  #define SDA_OLED 17
  #define DISP_CUSTOM_ADDR false
#elif BOARD_MODEL == BOARD_XIAO_S3
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define SCL_OLED 6
  #define SDA_OLED 5
  #define DISP_CUSTOM_ADDR true
#else
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define DISP_CUSTOM_ADDR true
#endif

#define SMALL_FONT &Org_01

#if BOARD_MODEL == BOARD_TDECK
  Adafruit_ST7789 display = Adafruit_ST7789(DISPLAY_CS, DISPLAY_DC, -1);
  #define SSD1306_WHITE ST77XX_WHITE
  #define SSD1306_BLACK ST77XX_BLACK
  #define DISPLAY_IS_OLED false
#elif BOARD_MODEL == BOARD_HELTEC_T096
  Adafruit_ST7735 display = Adafruit_ST7735(&SPI1, DISPLAY_CS, DISPLAY_DC, DISPLAY_RST);
  #define SSD1306_WHITE ST77XX_WHITE
  #define SSD1306_BLACK ST77XX_BLACK
  #define DISPLAY_IS_OLED false
#elif BOARD_MODEL == BOARD_HELTEC_T114
  ST7789Spi display(&SPI1, DISPLAY_RST, DISPLAY_DC, DISPLAY_CS);
  #define SSD1306_WHITE ST77XX_WHITE
  #define SSD1306_BLACK ST77XX_BLACK
  #define DISPLAY_IS_OLED false
#elif BOARD_MODEL == BOARD_TBEAM_S_V1 || BOARD_MODEL == BOARD_TBEAM_S_V3
  Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);
  #define SSD1306_WHITE SH110X_WHITE
  #define SSD1306_BLACK SH110X_BLACK
  #define DISPLAY_IS_OLED true
#elif BOARD_MODEL == BOARD_TECHO
  GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(pin_disp_cs, pin_disp_dc, pin_disp_reset, pin_disp_busy));
  uint32_t last_epd_refresh = 0;
  uint32_t last_epd_full_refresh = 0;
  #define REFRESH_PERIOD 300000
  #define DISPLAY_IS_OLED false
#else
  Adafruit_SSD1306 display(DISP_W, DISP_H, &Wire, DISP_RST);
  #define DISPLAY_IS_OLED true
#endif

float disp_target_fps = 7;
float epd_update_fps  = 0.5;

#define DISP_MODE_UNKNOWN   0x00
#define DISP_MODE_LANDSCAPE 0x01
#define DISP_MODE_PORTRAIT  0x02
#define DISP_PIN_SIZE   6
#define DISPLAY_BLANKING_TIMEOUT 15*1000
uint8_t disp_mode = DISP_MODE_UNKNOWN;
// The raw 0-3 rotation value actually applied at boot (display_rotation
// itself is only a local var inside display_init()) - used by the settings
// menu (Menu.h) to force its own fixed landscape orientation regardless of
// the main content's rotation setting. See update_display()'s menu dispatch.
uint8_t active_display_rotation = 0;
uint8_t disp_ext_fb = false;
unsigned char fb[512];
uint32_t last_disp_update = 0;
uint32_t last_unblank_event = 0;
uint32_t display_blanking_timeout = DISPLAY_BLANKING_TIMEOUT;
uint8_t display_unblank_intensity = display_intensity;
bool display_blanked = false;
bool display_tx = false;
bool recondition_display = false;
int disp_update_interval = 1000/disp_target_fps;
int epd_update_interval = 1000/disp_target_fps;
uint32_t last_page_flip = 0;
int page_interval = 4000;
bool device_signatures_ok();
bool device_firmware_ok();

#if HAS_MENU == true
  // MENU_STATE_* is #define'd in Menu.h, included after Display.h, so it
  // can't be referenced by name here - go through this bool wrapper instead.
  bool menu_is_open();
  void draw_settings_menu_disp();
#endif

#if BOARD_MODEL == BOARD_HELTEC_T096
  // The 80x160 panel gets a redesigned layout: 80x64 device area on top of
  // an 80x96 status area with a wider and taller waterfall. Bitmap art
  // stays 64px wide and is centered with a DISP_BM_X offset.
  #define WATERFALL_SIZE 78
  #define STAT_AREA_W 80
  #define STAT_AREA_H 96
  #define DISP_AREA_W 80
  #define DISP_AREA_H 64
  #define DISP_BM_X 8
  #define DIAG_COL2 42
  // Waterfall position within the status area
  #define WF_POS_X 27
  #define WF_POS_Y 4
#else
  #define WATERFALL_SIZE 46
  #define STAT_AREA_W 64
  #define STAT_AREA_H 64
  #define DISP_AREA_W 64
  #define DISP_AREA_H 64
  #define DISP_BM_X 0
  #define DIAG_COL2 32
#endif
#define DISP_BM_W 64
int waterfall[WATERFALL_SIZE];
int waterfall_meta[WATERFALL_SIZE];
int waterfall_head = 0;

#if MODEM == SX1280
  #define WF_TX_SIZE 5
#else
  #define WF_TX_SIZE 5
#endif
#if BOARD_MODEL == BOARD_HELTEC_T096
  // The KCT8103L LNA raises the idle noise reading; -120 keeps the graph
  // near zero at ambient instead of idling a fifth up the scale. The wider
  // 26-pixel waterfall gets headroom up to -40 before pegging full.
  #define WF_RSSI_MAX -40
  #define WF_RSSI_MIN -120
#else
  #define WF_RSSI_MAX -60
  #define WF_RSSI_MIN -135
#endif
#define WF_RSSI_SPAN (WF_RSSI_MAX-WF_RSSI_MIN)
#if BOARD_MODEL == BOARD_HELTEC_T096
  #define WF_PIXEL_WIDTH 26
#else
  #define WF_PIXEL_WIDTH 10
#endif
#define WF_M_RX   0x00
#define WF_M_TX   0x01
#define WF_M_NTFR 0x02
#define WF_M_RX_PKT 0x03 // sampled while carrier was detected

int p_ad_x = 0;
int p_ad_y = 0;
int p_as_x = 0;
int p_as_y = 0;

#if BOARD_MODEL == BOARD_HELTEC_T096
  // In landscape the status area is pushed as two regions: the icon and
  // waterfall cluster (stat rows 0..79) on the right half, and the status
  // strip (stat rows 80..95) at p_ss on the left half under the banner.
  int p_ss_x = 0;
  int p_ss_y = 0;
  // Stat-canvas positions that differ between the two orientations
  int16_t st_box_y0 = 8;   // first icon row (cable / lora)
  int16_t st_box_y1 = 36;  // second icon row (bt / 2.4G)
  int16_t st_box_y2 = 64;  // lamp row (rx / tx)
  int16_t wf_y = WF_POS_Y; // waterfall content top
  // Set around stat-area pushes so the colourizer in drawBitmap can map
  // bitmap rows back to stat-canvas rows
  bool push_is_stat = false;
  int16_t push_stat_dy = 0;
#endif

GFXcanvas1 stat_area(STAT_AREA_W, STAT_AREA_H);
GFXcanvas1 disp_area(DISP_AREA_W, DISP_AREA_H);

static const uint8_t one_counts[256] = {
  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  1,  1,  1,  1,
  1,  1,  1,  1,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,
  0,  0,  0,  0,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  2,  3,
  2,  2,  2,  2,  2,  2,  2,  2,  1,  2,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,
  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  0,  0,  0,  0,  0,  0,
  0,  0,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0
};

void fillRect(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t colour);

void update_area_positions() {
  #if BOARD_MODEL == BOARD_HELTEC_T114
    if (disp_mode == DISP_MODE_PORTRAIT) {
      p_ad_x = 16;
      p_ad_y = 64;
      p_as_x = 16;
      p_as_y = p_ad_y+126;
    } else if (disp_mode == DISP_MODE_LANDSCAPE) {
      p_ad_x = 0;
      p_ad_y = 96;
      p_as_x = 126;
      p_as_y = p_ad_y;
    }
  #elif BOARD_MODEL == BOARD_HELTEC_T096
    if (disp_mode == DISP_MODE_LANDSCAPE) {
      // Device area on the left half with the status strip under the
      // banner; icon/waterfall cluster fills the right half
      p_ad_x = 0;
      p_ad_y = 0;
      p_as_x = 80;
      p_as_y = 0;
      p_ss_x = 0;
      p_ss_y = 64;
      st_box_y0 = 6;
      st_box_y1 = 31;
      st_box_y2 = 56;
      wf_y = 1;
    } else if (disp_mode == DISP_MODE_PORTRAIT) {
      p_ad_x = 0;
      p_ad_y = 0;
      p_as_x = 0;
      p_as_y = 64;
      p_ss_x = 0;
      p_ss_y = 144; // unused: the portrait stat push covers the strip
      st_box_y0 = 8;
      st_box_y1 = 36;
      st_box_y2 = 64;
      wf_y = WF_POS_Y;
    }
  #elif BOARD_MODEL == BOARD_TECHO
    if (disp_mode == DISP_MODE_PORTRAIT) {
      p_ad_x = 61;
      p_ad_y = 36;
      p_as_x = 64;
      p_as_y = 64+36;
    } else if (disp_mode == DISP_MODE_LANDSCAPE) {
      p_ad_x = 0;
      p_ad_y = 0;
      p_as_x = 64;
      p_as_y = 0;
    }
  #else
    if (disp_mode == DISP_MODE_PORTRAIT) {
      p_ad_x = 0 * DISPLAY_SCALE;
      p_ad_y = 0 * DISPLAY_SCALE;
      p_as_x = 0 * DISPLAY_SCALE;
      p_as_y = 64 * DISPLAY_SCALE;
    } else if (disp_mode == DISP_MODE_LANDSCAPE) {
      p_ad_x = 0 * DISPLAY_SCALE;
      p_ad_y = 0 * DISPLAY_SCALE;
      p_as_x = 64 * DISPLAY_SCALE;
      p_as_y = 0 * DISPLAY_SCALE;
    }
  #endif
}

uint8_t display_contrast = 0x00;
#if BOARD_MODEL == BOARD_TBEAM_S_V1 || BOARD_MODEL == BOARD_TBEAM_S_V3
  void set_contrast(Adafruit_SH1106G *display, uint8_t value) {
  }
#elif BOARD_MODEL == BOARD_HELTEC_T114
  void set_contrast(ST7789Spi *display, uint8_t value) { }
#elif BOARD_MODEL == BOARD_HELTEC_T096
  // Perceived brightness follows duty^(1/gamma), not duty linearly, so a
  // linear duty cycle looks nearly full-bright until well below half scale
  // and then drops off fast. This table gamma-corrects (gamma 2.8) the
  // intensity value into duty space so it dims perceptually linearly across
  // the full range. Raw PWM values below 7 don't light the backlight at all,
  // so nonzero entries are also scaled onto the usable range (7-255).
  static const uint8_t t096_backlight_gamma[256] = {
    0, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9,
    9, 9, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 12, 12, 12,
    12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 16, 16, 16,
    17, 17, 17, 18, 18, 18, 19, 19, 20, 20, 20, 21, 21, 22, 22, 23,
    23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 31, 31,
    32, 32, 33, 34, 34, 35, 36, 36, 37, 38, 38, 39, 40, 41, 41, 42,
    43, 44, 45, 45, 46, 47, 48, 49, 50, 51, 51, 52, 53, 54, 55, 56,
    57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 69, 70, 71, 72, 73,
    74, 75, 77, 78, 79, 80, 82, 83, 84, 85, 87, 88, 89, 91, 92, 93,
    95, 96, 98, 99, 101, 102, 103, 105, 106, 108, 110, 111, 113, 114, 116, 117,
    119, 121, 122, 124, 126, 127, 129, 131, 133, 134, 136, 138, 140, 142, 143, 145,
    147, 149, 151, 153, 155, 157, 159, 161, 163, 165, 167, 169, 171, 173, 175, 177,
    180, 182, 184, 186, 188, 191, 193, 195, 197, 200, 202, 204, 207, 209, 211, 214,
    216, 219, 221, 224, 226, 229, 231, 234, 236, 239, 242, 244, 247, 250, 252, 255,
  };

  void set_contrast(Adafruit_ST7735 *display, uint8_t value) {
    // Backlight is active-low, so duty cycle is inverted.
    uint8_t pwm = t096_backlight_gamma[value];
    analogWrite(PIN_T096_TFT_BLGT, 255-pwm);
  }
#elif BOARD_MODEL == BOARD_TECHO
  void set_contrast(void *display, uint8_t value) {
    if (value == 0) { analogWrite(pin_backlight, 0); }
    else            { analogWrite(pin_backlight, value); }
  }
#elif BOARD_MODEL == BOARD_TDECK
  void set_contrast(Adafruit_ST7789 *display, uint8_t value) {
    static uint8_t level = 0;
    static uint8_t steps = 16;
    if (value > 15) value = 15;
    if (value == 0) {
        digitalWrite(DISPLAY_BL_PIN, 0);
        delay(3);
        level = 0;
        return;
    }
    if (level == 0) {
        digitalWrite(DISPLAY_BL_PIN, 1);
        level = steps;
        delayMicroseconds(30);
    }
    int from = steps - level;
    int to = steps - value;
    int num = (steps + to - from) % steps;
    for (int i = 0; i < num; i++) {
        digitalWrite(DISPLAY_BL_PIN, 0);
        digitalWrite(DISPLAY_BL_PIN, 1);
    }
    level = value;
  }
#else
  void set_contrast(Adafruit_SSD1306 *display, uint8_t contrast) {
    // SETCONTRAST alone only adjusts segment drive current - the panel
    // stays lit even at 0, it doesn't "turn off". Actually powering the
    // OLED matrix off/on is a separate command (DISPLAYOFF/DISPLAYON,
    // 0xAE/0xAF), which we use here at the 0 boundary for a real, visible
    // effect instead of relying on contrast's often-marginal range.
    //
    // Bundled into one I2C transaction (0x00 control byte = command
    // stream, then all command bytes) rather than one ssd1306_command()
    // call per byte - some SSD1306-compatible controllers only accept a
    // command's parameter byte if it arrives in the same transaction as
    // the opcode. ssd1306_commandList() does this the same way internally
    // but is a protected library method, so replicate its wire protocol
    // directly instead.
    Wire.beginTransmission(DISP_ADDR);
    Wire.write((uint8_t)0x00);
    if (contrast == 0) {
      Wire.write(SSD1306_DISPLAYOFF);
    } else {
      Wire.write(SSD1306_DISPLAYON);
      Wire.write(SSD1306_SETCONTRAST);
      Wire.write(contrast);
    }
    Wire.endTransmission();
  }
#endif

bool display_init() {
  #if HAS_DISPLAY
    #if BOARD_MODEL == BOARD_RNODE_NG_20 || BOARD_MODEL == BOARD_LORA32_V2_0
      int pin_display_en = 16;
      digitalWrite(pin_display_en, LOW);
      delay(50);
      digitalWrite(pin_display_en, HIGH);
    #elif BOARD_MODEL == BOARD_T3S3
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_HELTEC32_V2
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_HELTEC32_V3
      // enable vext / pin 36
      pinMode(Vext, OUTPUT);
      digitalWrite(Vext, LOW);
      delay(50);
      int pin_display_en = 21;
      pinMode(pin_display_en, OUTPUT);
      digitalWrite(pin_display_en, LOW);
      delay(50);
      digitalWrite(pin_display_en, HIGH);
      delay(50);
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_HELTEC32_V4
      // enable vext / pin 36
      pinMode(Vext, OUTPUT);
      digitalWrite(Vext, LOW);
      delay(50);
      int pin_display_en = 21;
      pinMode(pin_display_en, OUTPUT);
      digitalWrite(pin_display_en, LOW);
      delay(50);
      digitalWrite(pin_display_en, HIGH);
      delay(50);
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_LORA32_V1_0
      int pin_display_en = 16;
      digitalWrite(pin_display_en, LOW);
      delay(50);
      digitalWrite(pin_display_en, HIGH);
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_HELTEC_T114
      pinMode(PIN_T114_TFT_EN, OUTPUT);
      digitalWrite(PIN_T114_TFT_EN, LOW);
    #elif BOARD_MODEL == BOARD_HELTEC_T096
      pinMode(PIN_T096_TFT_EN, OUTPUT);
      digitalWrite(PIN_T096_TFT_EN, HIGH);
    #elif BOARD_MODEL == BOARD_MESHADVENTURER_S3 || BOARD_MODEL == BOARD_MESHPOE_S3
      Wire.setPins(SDA_OLED, SCL_OLED);
      Wire.begin();
    #elif BOARD_MODEL == BOARD_AETHERNODE || BOARD_MODEL == AETHERNODE_S3
      Wire.setPins(SDA_OLED, SCL_OLED);
      Wire.begin();
    #elif BOARD_MODEL == BOARD_PROMICRO
      Wire.setPins(SDA_OLED, SCL_OLED);
      Wire.begin();
    #elif BOARD_MODEL == BOARD_TECHO
      display.init(0, true, 10, false, displaySPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
      display.setPartialWindow(0, 0, DISP_W, DISP_H);
      display.epd2.setBusyCallback(busyCallback);
      #if HAS_BACKLIGHT
        pinMode(pin_backlight, OUTPUT);
        analogWrite(pin_backlight, 0);
      #endif
    #elif BOARD_MODEL == BOARD_TBEAM_S_V1 || BOARD_MODEL == BOARD_TBEAM_S_V3
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_XIAO_S3
      Wire.begin(SDA_OLED, SCL_OLED);
    #endif

    #if HAS_EEPROM
      uint8_t display_rotation = EEPROM.read(eeprom_addr(ADDR_CONF_DROT));
    #elif MCU_VARIANT == MCU_NRF52
      uint8_t display_rotation = eeprom_read(eeprom_addr(ADDR_CONF_DROT));
    #endif
    if (display_rotation < 0 or display_rotation > 3) display_rotation = 0xFF;

    #if DISP_CUSTOM_ADDR == true
      #if HAS_EEPROM
        uint8_t display_address = EEPROM.read(eeprom_addr(ADDR_CONF_DADR));
      #elif MCU_VARIANT == MCU_NRF52
        uint8_t display_address = eeprom_read(eeprom_addr(ADDR_CONF_DADR));
      #endif
      if (display_address == 0xFF) display_address = DISP_ADDR;
    #else
      uint8_t display_address = DISP_ADDR;
    #endif

    #if HAS_EEPROM
      if (EEPROM.read(eeprom_addr(ADDR_CONF_BSET)) == CONF_OK_BYTE) {
        uint8_t db_timeout = EEPROM.read(eeprom_addr(ADDR_CONF_DBLK));
        if (db_timeout == 0x00) {
          display_blanking_enabled = false;
        } else {
          display_blanking_enabled = true;
          display_blanking_timeout = db_timeout*1000;
        }
      }
    #elif MCU_VARIANT == MCU_NRF52
      if (eeprom_read(eeprom_addr(ADDR_CONF_BSET)) == CONF_OK_BYTE) {
        uint8_t db_timeout = eeprom_read(eeprom_addr(ADDR_CONF_DBLK));
        if (db_timeout == 0x00) {
          display_blanking_enabled = false;
        } else {
          display_blanking_enabled = true;
          display_blanking_timeout = db_timeout*1000;
        }
      }
    #endif
    
    #if BOARD_MODEL == BOARD_TECHO
    // Don't check if display is actually connected
    if(false) {
    #elif BOARD_MODEL == BOARD_TDECK
    display.init(240, 320);
    display.setSPISpeed(80e6);
    #elif BOARD_MODEL == BOARD_HELTEC_T114
    display.init();
    // set white as default pixel colour for Heltec T114
    display.setRGB(COLOR565(0xFF, 0xFF, 0xFF));
    if (false) {
    #elif BOARD_MODEL == BOARD_HELTEC_T096
    display.initR(INITR_MINI160x80);
    if (false) {
    #elif BOARD_MODEL == BOARD_TBEAM_S_V1 || BOARD_MODEL == BOARD_TBEAM_S_V3
    if (!display.begin(display_address, true)) {
    #else
    if (!display.begin(SSD1306_SWITCHCAPVCC, display_address)) {
    #endif
      return false;
    } else {
      set_contrast(&display, display_contrast);
      if (display_rotation != 0xFF) {
        #if BOARD_MODEL == BOARD_HELTEC_T096
          // MINI160x80 native orientation is portrait (80x160), so rotations
          // 1 and 3 yield landscape (160x80) and 0/2 yield portrait (80x160).
          if (display_rotation == 1 || display_rotation == 3) {
            disp_mode = DISP_MODE_LANDSCAPE;
          } else {
            disp_mode = DISP_MODE_PORTRAIT;
          }
        #else
          if (display_rotation == 0 || display_rotation == 2) {
            disp_mode = DISP_MODE_LANDSCAPE;
          } else {
            disp_mode = DISP_MODE_PORTRAIT;
          }
        #endif
        display.setRotation(display_rotation);
      } else {
        #if BOARD_MODEL == BOARD_RNODE_NG_20
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #elif BOARD_MODEL == BOARD_RNODE_NG_21
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #elif BOARD_MODEL == BOARD_LORA32_V1_0
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #elif BOARD_MODEL == BOARD_LORA32_V2_0
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #elif BOARD_MODEL == BOARD_LORA32_V2_1
          disp_mode = DISP_MODE_LANDSCAPE;
          display.setRotation(0);
        #elif BOARD_MODEL == BOARD_TBEAM
          disp_mode = DISP_MODE_LANDSCAPE;
          display.setRotation(0);
        #elif BOARD_MODEL == BOARD_TBEAM_S_V1 || BOARD_MODEL == BOARD_TBEAM_S_V3
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(1);
        #elif BOARD_MODEL == BOARD_HELTEC32_V2
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(1);
        #elif BOARD_MODEL == BOARD_HELTEC32_V3
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(1);
        #elif BOARD_MODEL == BOARD_HELTEC32_V4
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(1);
        #elif BOARD_MODEL == BOARD_HELTEC_T114
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(1);
        #elif BOARD_MODEL == BOARD_HELTEC_T096
          disp_mode = DISP_MODE_LANDSCAPE;
          display.setRotation(1);
        #elif BOARD_MODEL == BOARD_RAK4631
          disp_mode = DISP_MODE_LANDSCAPE;
          display.setRotation(0);
        #elif BOARD_MODEL == BOARD_TDECK
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #elif BOARD_MODEL == BOARD_TECHO
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #elif BOARD_MODEL == BOARD_MESHADVENTURER_S3 || BOARD_MODEL == BOARD_MESHPOE_S3
          disp_mode = DISP_MODE_LANDSCAPE;
          display.setRotation(0);
        #elif BOARD_MODEL == BOARD_MESHADVENTURER
          disp_mode = DISP_MODE_LANDSCAPE;
          display.setRotation(0);
        #elif BOARD_MODEL == BOARD_DIY_V1
          disp_mode = DISP_MODE_LANDSCAPE;
          display.setRotation(0);
        #elif BOARD_MODEL == BOARD_AETHERNODE || BOARD_MODEL == BOARD_AETHERNODE_S3
          disp_mode = DISP_MODE_LANDSCAPE;
          display.setRotation(0);
        #elif BOARD_MODEL == BOARD_PROMICRO
          disp_mode = DISP_MODE_LANDSCAPE;
          display.setRotation(0);
        #else
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #endif
      }

      active_display_rotation = display.getRotation();

      update_area_positions();

      for (int i = 0; i < WATERFALL_SIZE; i++) { waterfall[i] = 0; }

      last_page_flip = millis();

      stat_area.cp437(true);
      disp_area.cp437(true);

      #if BOARD_MODEL != BOARD_HELTEC_T114
      display.cp437(true);
      #endif

      #if HAS_EEPROM
        display_intensity = EEPROM.read(eeprom_addr(ADDR_CONF_DINT));
      #elif MCU_VARIANT == MCU_NRF52
        display_intensity = eeprom_read(eeprom_addr(ADDR_CONF_DINT));
      #endif
      display_unblank_intensity = display_intensity;

      #if BOARD_MODEL == BOARD_TECHO
        #if HAS_BACKLIGHT
          if (display_intensity == 0) { analogWrite(pin_backlight, 0); }
          else                        { analogWrite(pin_backlight, display_intensity); }
        #endif
      #endif

      #if BOARD_MODEL == BOARD_TDECK
        display.fillScreen(SSD1306_BLACK);
      #endif

      #if BOARD_MODEL == BOARD_HELTEC_T114
        // Enable backlight led (display is always black without this)
        fillRect(p_ad_x, p_ad_y, 128, 128, SSD1306_BLACK);
        fillRect(p_as_x, p_as_y, 128, 128, SSD1306_BLACK);
        pinMode(PIN_T114_TFT_BLGT, OUTPUT);
        digitalWrite(PIN_T114_TFT_BLGT, LOW);
      #elif BOARD_MODEL == BOARD_HELTEC_T096
        display.fillScreen(SSD1306_BLACK);
        pinMode(PIN_T096_TFT_BLGT, OUTPUT);
        set_contrast(&display, display_intensity);
      #endif

      return true;
    }
  #else
    return false;
  #endif
}

// Draws a line on the screen
void drawLine(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t colour) {
  #if BOARD_MODEL == BOARD_HELTEC_T114
  if(colour == SSD1306_WHITE){
    display.setColor(WHITE);
  } else if(colour == SSD1306_BLACK) {
    display.setColor(BLACK);
  }
  display.drawLine(x, y, width, height);
  #else
  display.drawLine(x, y, width, height, colour);
  #endif
}

// Draws a filled rectangle on the screen
void fillRect(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t colour) {
  #if BOARD_MODEL == BOARD_HELTEC_T114
  if(colour == SSD1306_WHITE){
    display.setColor(WHITE);
  } else if(colour == SSD1306_BLACK) {
    display.setColor(BLACK);
  }
  display.fillRect(x, y, width, height);
  #else
  display.fillRect(x, y, width, height, colour);
  #endif
}

#if BOARD_MODEL == BOARD_HELTEC_T096
  // The T114 avoids flicker by keeping a back buffer in its ST7789 driver
  // and only pushing changed pixels to the panel (see ST7789.h). The
  // Adafruit ST7735 driver used here has no framebuffer, so keep a copy of
  // the last bitmap pushed to each screen region and only write the
  // bounding box of changed pixels.
  #define REGION_CACHE_SLOTS 4
  #define REGION_CACHE_BYTES 960 // enough for an 80x96 mono bitmap
  #if USE_COLOR_DISPLAY == true
    #define COLOR_LAMP_RX COLOR565(0x3E, 0xD8, 0x60)
    #define COLOR_LAMP_TX COLOR565(0x48, 0x96, 0xFF)
    #define COLOR_BAT_LOW COLOR565(0xEB, 0x4C, 0x42)
    #define COLOR_BANNER_OK COLOR565(0x28, 0x90, 0x40)    // darker green for status banners
    #define COLOR_BANNER_ALERT COLOR565(0xFF, 0xA0, 0x20)  // amber for warning banners
    #define COLOR_BT_ON COLOR565(0x28, 0x60, 0xC0)         // darker blue bluetooth box fill
  #endif
  // Background tint of the currently displayed status banner, 0 = none
  uint16_t disp_banner_fg = 0;
  // RX/TX indicator lamps and low-battery warning; states set in
  // draw_stat_area, read by the push-time colourizer in drawBitmap
  #define LAMP_HOLD_MS 300
  // Voltage readout turns red when approaching the critical voltage
  // (BAT_V_MIN, 3.15V on this board; defined later in Power.h)
  #define BAT_V_ALERT 3.30
  bool lamp_rx_lit = false;
  bool lamp_tx_lit = false;
  bool battery_low_lit = false;
  bool battery_volt_low_lit = false;
  bool bt_enabled_lit = false;
  uint8_t bt_icon_i = 0; // icon variant shown in the bluetooth box
  uint32_t lamp_rx_until = 0;
  uint32_t lamp_tx_until = 0;
  struct RegionCache {
    int16_t x = -1; int16_t y = -1; int16_t w = 0; int16_t h = 0;
    uint16_t fg = 0; uint16_t bg = 0;
    uint8_t back[REGION_CACHE_BYTES];
  };
  RegionCache region_cache[REGION_CACHE_SLOTS];
  uint8_t region_cache_next = 0;
  // Colour-only changes leave the mono canvas identical, so the diff in
  // drawBitmap would skip them. Flushing the whole cache would force a
  // full-panel repaint - a visible flicker sweep - so instead the affected
  // panel-space rectangles are queued here and folded into the bounds of
  // the next push that covers them.
  #if USE_COLOR_DISPLAY == true
    #define CDIRTY_SLOTS 4
    struct CDirtyRect { int16_t x; int16_t y; int16_t w; int16_t h; };
    CDirtyRect cdirty[CDIRTY_SLOTS];
    uint8_t cdirty_count = 0;
    void colour_mark_dirty(int16_t x, int16_t y, int16_t w, int16_t h) {
      if (cdirty_count < CDIRTY_SLOTS) {
        cdirty[cdirty_count].x = x; cdirty[cdirty_count].y = y;
        cdirty[cdirty_count].w = w; cdirty[cdirty_count].h = h;
        cdirty_count++;
      } else {
        // Queue full; widen the last rect to cover the new one
        CDirtyRect *r = &cdirty[CDIRTY_SLOTS-1];
        int16_t x1 = r->x+r->w; if (x+w > x1) x1 = x+w;
        int16_t y1 = r->y+r->h; if (y+h > y1) y1 = y+h;
        if (x < r->x) r->x = x;
        if (y < r->y) r->y = y;
        r->w = x1-r->x; r->h = y1-r->y;
      }
    }
  #else
    void colour_mark_dirty(int16_t x, int16_t y, int16_t w, int16_t h) {}
  #endif
  // Maps a stat-canvas rectangle to its position on the panel; in landscape
  // the strip rows (80+) are pushed separately at p_ss on the left half
  void stat_mark_dirty(int16_t sx, int16_t sy, int16_t w, int16_t h) {
    if (disp_mode == DISP_MODE_LANDSCAPE && sy >= 80) {
      colour_mark_dirty(p_ss_x+sx, p_ss_y+sy-80, w, h);
    } else {
      colour_mark_dirty(p_as_x+sx, p_as_y+sy, w, h);
    }
  }
#endif

// Draws a bitmap to the display and auto scales it based on the boards configured DISPLAY_SCALE
void drawBitmap(int16_t startX, int16_t startY, const uint8_t* bitmap, int16_t bitmapWidth, int16_t bitmapHeight, uint16_t foregroundColour, uint16_t backgroundColour) {
  #if BOARD_MODEL == BOARD_HELTEC_T096
    {
      // The whole changed rect is assembled here and sent as a single DMA
      // transfer. Splitting the push into per-row writePixels calls (one
      // DMA setup each) stretches the write burst several-fold, and the
      // panel visibly dips in brightness for the duration of a burst.
      static uint16_t pushbuf[STAT_AREA_W*STAT_AREA_H];
      int16_t byteWidth = (bitmapWidth + 7) / 8;
      int32_t bitmapBytes = (int32_t)byteWidth * bitmapHeight;
      bool cacheable = bitmapBytes <= REGION_CACHE_BYTES && bitmapWidth <= 80;

      RegionCache *reg = NULL;
      if (cacheable) {
        for (uint8_t i = 0; i < REGION_CACHE_SLOTS; i++) {
          RegionCache *c = &region_cache[i];
          if (c->x == startX && c->y == startY && c->w == bitmapWidth && c->h == bitmapHeight &&
              c->fg == foregroundColour && c->bg == backgroundColour) {
            reg = c; break;
          }
        }
      }

      int16_t minX = 0, minY = 0, maxX = bitmapWidth-1, maxY = bitmapHeight-1;
      if (reg == NULL) {
        if (cacheable) {
          // Cached regions overlapping this one on the panel no longer
          // reflect what is displayed there
          for (uint8_t i = 0; i < REGION_CACHE_SLOTS; i++) {
            RegionCache *c = &region_cache[i];
            if (c->x >= 0 && startX < c->x + c->w && c->x < startX + bitmapWidth &&
                startY < c->y + c->h && c->y < startY + bitmapHeight) {
              c->x = -1;
            }
          }
          reg = &region_cache[region_cache_next];
          region_cache_next = (region_cache_next+1) % REGION_CACHE_SLOTS;
          reg->x = startX; reg->y = startY; reg->w = bitmapWidth; reg->h = bitmapHeight;
          reg->fg = foregroundColour; reg->bg = backgroundColour;
          memcpy(reg->back, bitmap, bitmapBytes);
        }
      } else {
        minX = bitmapWidth; minY = bitmapHeight; maxX = -1; maxY = -1;
        for (int16_t row = 0; row < bitmapHeight; row++) {
          for (int16_t bc = 0; bc < byteWidth; bc++) {
            int32_t idx = (int32_t)row * byteWidth + bc;
            if (bitmap[idx] != reg->back[idx]) {
              uint8_t diff = bitmap[idx] ^ reg->back[idx];
              if (row < minY) minY = row;
              if (row > maxY) maxY = row;
              // Track changed columns per-pixel, so static pixels sharing
              // a byte with changing ones don't get rewritten
              for (uint8_t b = 0; b < 8; b++) {
                if (diff & (0x80 >> b)) {
                  if (bc*8+b < minX) minX = bc*8+b;
                  if (bc*8+b > maxX) maxX = bc*8+b;
                }
              }
              reg->back[idx] = bitmap[idx];
            }
          }
        }
        if (maxX > bitmapWidth-1) maxX = bitmapWidth-1;
      }

      #if USE_COLOR_DISPLAY == true
        // Fold queued colour-only dirty rects overlapping this region into
        // the push bounds, then retire them
        if (cacheable) {
          for (uint8_t i = 0; i < cdirty_count; ) {
            int16_t ix0 = cdirty[i].x-startX;            if (ix0 < 0) ix0 = 0;
            int16_t iy0 = cdirty[i].y-startY;            if (iy0 < 0) iy0 = 0;
            int16_t ix1 = cdirty[i].x+cdirty[i].w-startX; if (ix1 > bitmapWidth)  ix1 = bitmapWidth;
            int16_t iy1 = cdirty[i].y+cdirty[i].h-startY; if (iy1 > bitmapHeight) iy1 = bitmapHeight;
            if (ix0 < ix1 && iy0 < iy1) {
              if (ix0 < minX)   minX = ix0;
              if (iy0 < minY)   minY = iy0;
              if (ix1-1 > maxX) maxX = ix1-1;
              if (iy1-1 > maxY) maxY = iy1-1;
              cdirty[i] = cdirty[--cdirty_count];
            } else { i++; }
          }
        }
      #endif
      if (maxY < 0) return;

      uint32_t pb = 0;
      for (int16_t row = minY; row <= maxY; row++) {
        #if USE_COLOR_DISPLAY == true
          // stat pushes are full-width slices of the stat canvas, so the
          // bitmap column is the stat column and rows shift by push_stat_dy
          int16_t sy = row + push_stat_dy;
        #endif
        for (int16_t col = minX; col <= maxX; col++) {
          uint16_t fg = foregroundColour;
          #if USE_COLOR_DISPLAY == true
            // Lit pixels inside an active indicator lamp or a depleted
            // battery icon get that element's colour; everything else
            // stays monochrome
            if (foregroundColour == SSD1306_WHITE) {
              if (push_is_stat) {
                int16_t sx = col;
                if      (lamp_rx_lit && sx >= 3 && sx <= 18 && sy >= st_box_y2 && sy <= st_box_y2+15)  { fg = COLOR_LAMP_RX; }
                else if (lamp_tx_lit && sx >= 61 && sx <= 76 && sy >= st_box_y2 && sy <= st_box_y2+15) { fg = COLOR_LAMP_TX; }
                else if (battery_low_lit && sx >= 2 && sx <= 19 && sy >= 88 && sy <= 94)       { fg = COLOR_BAT_LOW; }
                else if (battery_volt_low_lit && sx >= 20 && sx <= 38 && sy >= 87 && sy <= 94) { fg = COLOR_BAT_LOW; }
                else if (bt_enabled_lit && sx >= 3 && sx <= 18 && sy >= st_box_y1 && sy <= st_box_y1+15) {
                  // icon pixels stay light, the rest of the box fills dark blue
                  uint8_t bt_c = sx-3; uint8_t bt_r = sy-st_box_y1;
                  if (!(bm_bt[bt_icon_i*32 + bt_r*2 + bt_c/8] & (0x80 >> (bt_c%8)))) { fg = COLOR_BT_ON; }
                }
                else if (sx >= WF_POS_X && sx < WF_POS_X+WF_PIXEL_WIDTH &&
                         sy >= wf_y && sy < wf_y+WATERFALL_SIZE) {
                  int wf_m = waterfall_meta[(waterfall_head + (sy-wf_y)) % WATERFALL_SIZE];
                  if      (wf_m == WF_M_RX_PKT) { fg = COLOR_LAMP_RX; }
                  else if (wf_m == WF_M_TX)     { fg = COLOR_LAMP_TX; }
                }
              } else if (disp_banner_fg != 0) {
                // status banner fill (checks passed / hw ok / fw corrupt)
                int16_t bx = (startX+col) - p_ad_x;
                int16_t by = (startY+row) - p_ad_y;
                if (bx >= 0 && bx < DISP_AREA_W && by >= 37 && by <= 63) { fg = disp_banner_fg; }
              }
            }
          #endif
          // stored big-endian, ready for the panel
          uint16_t pxc = (bitmap[row * byteWidth + col / 8] & (0x80 >> (col % 8))) ? fg : backgroundColour;
          pushbuf[pb++] = __builtin_bswap16(pxc);
        }
      }

      display.startWrite();
      display.setAddrWindow(startX+minX, startY+minY, maxX-minX+1, maxY-minY+1);
      display.writePixels(pushbuf, pb, true, true);
      display.endWrite();
    }
  #elif DISPLAY_SCALE == 1
    display.drawBitmap(startX, startY, bitmap, bitmapWidth, bitmapHeight, foregroundColour, backgroundColour);
  #else
    for(int16_t row = 0; row < bitmapHeight; row++){
        for(int16_t col = 0; col < bitmapWidth; col++){

            // determine index and bitmask
            int16_t index = row * ((bitmapWidth + 7) / 8) + (col / 8);
            uint8_t bitmask = 1 << (7 - (col % 8));

            // check if the current pixel is set in the bitmap
            if(bitmap[index] & bitmask){
                // draw a scaled rectangle for the foreground pixel
                fillRect(startX + col * DISPLAY_SCALE, startY + row * DISPLAY_SCALE, DISPLAY_SCALE, DISPLAY_SCALE, foregroundColour);
            } else {
                // draw a scaled rectangle for the background pixel
                fillRect(startX + col * DISPLAY_SCALE, startY + row * DISPLAY_SCALE, DISPLAY_SCALE, DISPLAY_SCALE, backgroundColour);
            }

        }
    }
  #endif
}

extern uint8_t wifi_mode;
extern bool wifi_is_connected();
extern bool wifi_host_is_connected();
#if HAS_ETHERNET
extern bool eth_link_up;
#endif
void draw_cable_icon(int px, int py) {
  #if HAS_WIFI
    if (wifi_mode == WR_WIFI_OFF) {
      if      (rns_link_state == RNS_LINK_STATE_DISCONNECTED) { stat_area.drawBitmap(px, py, bm_cable+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
      else if (rns_link_state == RNS_LINK_STATE_CONNECTED)    { stat_area.drawBitmap(px, py, bm_cable+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
    } else {
      if (wifi_mode == WR_WIFI_STA) {
        if (wifi_is_connected()) {
          stat_area.drawBitmap(px, py, bm_wifi+3*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
          if (!wifi_host_is_connected()) { stat_area.fillRect(px+5, py+12, 6, 3, SSD1306_BLACK); }
        } else { stat_area.drawBitmap(px, py, bm_wifi+2*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
      
      } else if (wifi_mode == WR_WIFI_AP) {
        if (wifi_host_is_connected()) { stat_area.drawBitmap(px, py, bm_wifi+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
        else                          { stat_area.drawBitmap(px, py, bm_wifi+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
      
      } else {
        if      (rns_link_state == RNS_LINK_STATE_DISCONNECTED) { stat_area.drawBitmap(px, py, bm_cable+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
        else if (rns_link_state == RNS_LINK_STATE_CONNECTED)    { stat_area.drawBitmap(px, py, bm_cable+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
      }
    }

  #else
  if      (rns_link_state == RNS_LINK_STATE_DISCONNECTED) { stat_area.drawBitmap(px, py, bm_cable+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
  else if (rns_link_state == RNS_LINK_STATE_CONNECTED)    { stat_area.drawBitmap(px, py, bm_cable+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
  #endif
}

void draw_bt_icon(int px, int py) {
  uint8_t bt_i = 0;
  if      (bt_state == BT_STATE_ON)        { bt_i = 1; }
  else if (bt_state == BT_STATE_PAIRING)   { bt_i = 2; }
  else if (bt_state == BT_STATE_CONNECTED) { bt_i = 3; }
  #if BOARD_MODEL == BOARD_HELTEC_T096
    // Lamp-style: the box fills dark blue when bluetooth is enabled, the
    // state icon stays light. The mono canvas holds a fully lit interior;
    // the colourizer separates icon pixels from fill via bm_bt directly.
    bt_enabled_lit = bt_i != 0;
    #if USE_COLOR_DISPLAY == true
      if (bt_i != bt_icon_i) {
        bt_icon_i = bt_i;
        // glyph changes are colour-only on a lit box
        stat_mark_dirty(px, py, 16, 16);
      }
      if (bt_enabled_lit) { stat_area.fillRect(px, py, 16, 16, SSD1306_WHITE); }
      else                { stat_area.drawBitmap(px, py, bm_bt+bt_i*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
    #else
      if (bt_enabled_lit) { stat_area.drawBitmap(px, py, bm_bt+bt_i*32, 16, 16, SSD1306_BLACK, SSD1306_WHITE); }
      else                { stat_area.drawBitmap(px, py, bm_bt+bt_i*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
    #endif
  #else
    stat_area.drawBitmap(px, py, bm_bt+bt_i*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  #endif
}

void draw_lora_icon(int px, int py) {
  if (radio_online) {
    stat_area.drawBitmap(px, py, bm_rf+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  } else {
    stat_area.drawBitmap(px, py, bm_rf+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  }
}

void draw_mw_icon(int px, int py) {
  if (mw_radio_online) {
    stat_area.drawBitmap(px, py, bm_rf+3*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  } else {
    stat_area.drawBitmap(px, py, bm_rf+2*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  }
}

#if HAS_ETHERNET
void draw_eth_icon(int px, int py) {
  // The box interior is actually 17px tall border-to-border (bm_frame's
  // top/bottom border lines sit one row further apart than the 16px icon
  // grid), so a full-height fill needs 17 here or it leaves the bottom
  // interior row showing through as an unfilled dark line
  if (eth_link_up) {
    stat_area.fillRect(px, py, 16, 17, SSD1306_WHITE);
    stat_area.drawBitmap(px+2, py+6, bm_eth_txt, 11, 5, SSD1306_BLACK, SSD1306_WHITE);
  } else {
    stat_area.fillRect(px, py, 16, 17, SSD1306_BLACK);
    stat_area.drawBitmap(px+2, py+6, bm_eth_txt, 11, 5, SSD1306_WHITE, SSD1306_BLACK);
  }
}
#endif

uint8_t charge_tick = 0;
void draw_battery_bars(int px, int py) {
  #if BOARD_MODEL == BOARD_HELTEC_T096
    battery_low_lit = false;
  #endif
  if (pmu_ready) {
    if (battery_ready) {
      if (battery_installed) {
        float battery_value = battery_percent;

        // Disable charging state display for now, since
        // boards without dedicated PMU are completely
        // unreliable for determining actual charging state.
        bool disable_charge_status = false;
        if (battery_indeterminate && battery_state == BATTERY_STATE_CHARGING) {
          disable_charge_status = true;
        }
        
        if (battery_state == BATTERY_STATE_CHARGING && !disable_charge_status) {
          float battery_prog = battery_percent;
          if (battery_prog > 85) { battery_prog = 84; }
          if (charge_tick < battery_prog ) { charge_tick = battery_prog; }
          battery_value = charge_tick;
          charge_tick += 3;
          if (charge_tick > 100) charge_tick = 0;
        }

        if (battery_indeterminate && battery_state == BATTERY_STATE_CHARGING && !disable_charge_status) {
          stat_area.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
          stat_area.drawBitmap(px-2, py-2, bm_plug, 17, 7, SSD1306_WHITE, SSD1306_BLACK);
        } else {
          if (battery_state == BATTERY_STATE_CHARGED) {
            stat_area.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
            stat_area.drawBitmap(px-2, py-2, bm_plug, 17, 7, SSD1306_WHITE, SSD1306_BLACK);
          } else {
            // stat_area.fillRect(px, py, 14, 3, SSD1306_BLACK);
            #if BOARD_MODEL == BOARD_HELTEC_T096
              // 2 sticks or fewer render the icon red
              battery_low_lit = battery_value <= 33;
            #endif
            stat_area.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
            stat_area.drawRect(px-2, py-2, 17, 7, SSD1306_WHITE);
            stat_area.drawLine(px+15, py, px+15, py+3, SSD1306_WHITE);
            if (battery_value > 7) stat_area.drawLine(px, py, px, py+2, SSD1306_WHITE);
            if (battery_value > 20) stat_area.drawLine(px+1*2, py, px+1*2, py+2, SSD1306_WHITE);
            if (battery_value > 33) stat_area.drawLine(px+2*2, py, px+2*2, py+2, SSD1306_WHITE);
            if (battery_value > 46) stat_area.drawLine(px+3*2, py, px+3*2, py+2, SSD1306_WHITE);
            if (battery_value > 59) stat_area.drawLine(px+4*2, py, px+4*2, py+2, SSD1306_WHITE);
            if (battery_value > 72) stat_area.drawLine(px+5*2, py, px+5*2, py+2, SSD1306_WHITE);
            if (battery_value > 85) stat_area.drawLine(px+6*2, py, px+6*2, py+2, SSD1306_WHITE);
          }
        }
      } else {
        stat_area.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
        stat_area.drawBitmap(px-2, py-2, bm_plug, 17, 7, SSD1306_WHITE, SSD1306_BLACK);
      }
    }
  } else {
    stat_area.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
    stat_area.drawBitmap(px-2, py-2, bm_plug, 17, 7, SSD1306_WHITE, SSD1306_BLACK);
  }
}

#if HAS_VSENSE == true
  // No PMU/battery on this board, so the battery-bars slot is otherwise
  // blank (draw_battery_bars() draws nothing when battery_ready is false) -
  // show the raw divider voltage there instead. Same box dims as the
  // battery icon (px-2,py-2,18,7) so it drops straight into that spot.
  #define VSENSE_DISP_REFRESH_INTERVAL 2000
  // Alternates with the CPU temperature every 4s (same dwell time as the
  // rotating info pages in draw_disp_area()) since there's no PMU/battery
  // reading to otherwise fill this slot with
  #define VSENSE_DISP_TOGGLE_INTERVAL 4000
  extern bool pmu_temp_sensor_ready;
  extern float pmu_temperature;
  void draw_vsense_voltage(int px, int py) {
    static uint32_t last_drawn = 0;
    if (last_drawn != 0 && millis()-last_drawn < VSENSE_DISP_REFRESH_INTERVAL) return;
    stat_area.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
    stat_area.setFont(SMALL_FONT); stat_area.setTextWrap(false);
    stat_area.setTextColor(SSD1306_WHITE); stat_area.setTextSize(1);
    stat_area.setCursor(px-2, py+3);
    if (pmu_temp_sensor_ready && (millis()/VSENSE_DISP_TOGGLE_INTERVAL)%2 == 1) {
      stat_area.printf("%.0fC", pmu_temperature);
    } else {
      stat_area.printf("%.1fV", vsense_voltage);
    }
    last_drawn = millis();
  }
#endif

#define Q_SNR_STEP 2.0
#define Q_SNR_MIN_BASE -9.0
#define Q_SNR_MAX 6.0
void draw_quality_bars(int px, int py) {
  stat_area.fillRect(px, py, 13, 7, SSD1306_BLACK);
  if (radio_online) {
    signed char t_snr = (signed int)last_snr_raw;
    int snr_int = (int)t_snr;
    float snr_min = Q_SNR_MIN_BASE-(int)lora_sf*Q_SNR_STEP;
    float snr_span = (Q_SNR_MAX-snr_min);
    float snr = ((int)snr_int) * 0.25;
    float quality = ((snr-snr_min)/(snr_span))*100;
    if (quality > 100.0) quality = 100.0;
    if (quality < 0.0) quality = 0.0;

    // Serial.printf("Last SNR: %.2f\n, quality: %.2f\n", snr, quality);
    if (quality > 0)  stat_area.drawLine(px+0*2, py+7, px+0*2, py+6, SSD1306_WHITE);
    if (quality > 15) stat_area.drawLine(px+1*2, py+7, px+1*2, py+5, SSD1306_WHITE);
    if (quality > 30) stat_area.drawLine(px+2*2, py+7, px+2*2, py+4, SSD1306_WHITE);
    if (quality > 45) stat_area.drawLine(px+3*2, py+7, px+3*2, py+3, SSD1306_WHITE);
    if (quality > 60) stat_area.drawLine(px+4*2, py+7, px+4*2, py+2, SSD1306_WHITE);
    if (quality > 75) stat_area.drawLine(px+5*2, py+7, px+5*2, py+1, SSD1306_WHITE);
    if (quality > 90) stat_area.drawLine(px+6*2, py+7, px+6*2, py+0, SSD1306_WHITE);
  }
}

#if MODEM == SX1280
  #define S_RSSI_MIN -105.0
  #define S_RSSI_MAX -65.0
#else
  #define S_RSSI_MIN -135.0
  #define S_RSSI_MAX -75.0
#endif
#define S_RSSI_SPAN (S_RSSI_MAX-S_RSSI_MIN)
void draw_signal_bars(int px, int py) {
  stat_area.fillRect(px, py, 13, 7, SSD1306_BLACK);

  if (radio_online) {
    int rssi_val = last_rssi;
    if (rssi_val < S_RSSI_MIN) rssi_val = S_RSSI_MIN;
    if (rssi_val > S_RSSI_MAX) rssi_val = S_RSSI_MAX;
    int signal = ((rssi_val - S_RSSI_MIN)*(1.0/S_RSSI_SPAN))*100.0;

    if (signal > 100.0) signal = 100.0;
    if (signal < 0.0) signal = 0.0;

    // Serial.printf("Last SNR: %.2f\n, quality: %.2f\n", snr, quality);
    if (signal > 85) stat_area.drawLine(px+0*2, py+7, px+0*2, py+0, SSD1306_WHITE);
    if (signal > 72) stat_area.drawLine(px+1*2, py+7, px+1*2, py+1, SSD1306_WHITE);
    if (signal > 59) stat_area.drawLine(px+2*2, py+7, px+2*2, py+2, SSD1306_WHITE);
    if (signal > 46) stat_area.drawLine(px+3*2, py+7, px+3*2, py+3, SSD1306_WHITE);
    if (signal > 33) stat_area.drawLine(px+4*2, py+7, px+4*2, py+4, SSD1306_WHITE);
    if (signal > 20) stat_area.drawLine(px+5*2, py+7, px+5*2, py+5, SSD1306_WHITE);
    if (signal > 7)  stat_area.drawLine(px+6*2, py+7, px+6*2, py+6, SSD1306_WHITE);
  }
}

void draw_waterfall(int px, int py) {
  int rssi_val = current_rssi;
  if (rssi_val < WF_RSSI_MIN) rssi_val = WF_RSSI_MIN;
  if (rssi_val > WF_RSSI_MAX) rssi_val = WF_RSSI_MAX;
  int rssi_normalised = ((rssi_val - WF_RSSI_MIN)*(1.0/WF_RSSI_SPAN))*WF_PIXEL_WIDTH;
  if (display_tx) {
    for (uint8_t i = 0; i < WF_TX_SIZE; i++) {
      waterfall_meta[waterfall_head] = WF_M_TX;
      waterfall[waterfall_head++] = -1;
      if (waterfall_head >= WATERFALL_SIZE) waterfall_head = 0;
    }
    display_tx = false;
  } else {
    if      (interference_detected) { waterfall_meta[waterfall_head] = WF_M_NTFR; }
    else if (dcd_led)               { waterfall_meta[waterfall_head] = WF_M_RX_PKT; }
    else                            { waterfall_meta[waterfall_head] = WF_M_RX; }
    waterfall[waterfall_head++] = rssi_normalised;
    if (waterfall_head >= WATERFALL_SIZE) waterfall_head = 0;
  }

  stat_area.fillRect(px,py,WF_PIXEL_WIDTH, WATERFALL_SIZE, SSD1306_BLACK);
  for (int i = 0; i < WATERFALL_SIZE; i++){
    int wi = (waterfall_head+i)%WATERFALL_SIZE;
    int ws = waterfall[wi];
    int wm = waterfall_meta[wi];
    if (ws > 0) {
      if      (wm == WF_M_RX || wm == WF_M_RX_PKT) { stat_area.drawLine(px, py+i, px+ws-1, py+i, SSD1306_WHITE); }
      else if (wm == WF_M_NTFR) {
        uint8_t o = 0;
        for (uint8_t ti = 0; ti < WF_PIXEL_WIDTH/2; ti++) { stat_area.drawPixel(px+ti*2+o, py+i, SSD1306_WHITE); }
      }
    } else if (ws == -1) {
      #if BOARD_MODEL == BOARD_HELTEC_T096
        // Anchor the checker phase to the entry, not the screen row, so
        // the pattern scrolls with the content instead of inverting in
        // place on every frame
        uint8_t o = wi%2;
      #else
        uint8_t o = i%2;
      #endif
      for (uint8_t ti = 0; ti < WF_PIXEL_WIDTH/2; ti++) {
        stat_area.drawPixel(px+ti*2+o, py+i, SSD1306_WHITE);
      }
    }
  }

  #if BOARD_MODEL == BOARD_HELTEC_T096 && USE_COLOR_DISPLAY == true
    // Row colours are looked up from waterfall_meta at push time, but rows
    // whose mono content matches what the panel already shows are skipped
    // by the diff and would keep the colour of the entry displayed there
    // before the scroll. Re-push the whole waterfall rect every scroll.
    stat_mark_dirty(px, py, WF_PIXEL_WIDTH, WATERFALL_SIZE);
  #endif
}

#if BOARD_MODEL == BOARD_HELTEC_T096
// Battery voltage readout in the 19px gap between the battery bars and
// the quality graph; "d.dd" in Org_01 is exactly 19px wide. Refreshed at
// most every 5s so the jittering last decimal doesn't expand the display
// update region on every frame.
#define BAT_V_REFRESH_INTERVAL 5000
void draw_battery_voltage(int px, int py) {
  // 50mV of hysteresis, so measurement noise at the threshold doesn't
  // toggle the tint back and forth
  float v_thr = battery_volt_low_lit ? BAT_V_ALERT+0.05 : BAT_V_ALERT;
  bool volt_low = pmu_ready && battery_ready && battery_installed && battery_voltage <= v_thr;
  if (volt_low != battery_volt_low_lit) {
    battery_volt_low_lit = volt_low;
    // colour-only change; the glyph pixels may be identical
    stat_mark_dirty(px, py-6, 19, 8);
  }
  static uint32_t last_drawn = 0;
  if (last_drawn != 0 && millis()-last_drawn < BAT_V_REFRESH_INTERVAL) return;
  if (pmu_ready && battery_ready && battery_installed) {
    stat_area.fillRect(px, py-6, 19, 8, SSD1306_BLACK);
    stat_area.setFont(SMALL_FONT); stat_area.setTextWrap(false);
    stat_area.setTextColor(SSD1306_WHITE); stat_area.setTextSize(1);
    stat_area.setCursor(px, py);
    stat_area.printf("%.2f", battery_voltage);
    last_drawn = millis();
  }
}
#endif

bool stat_area_intialised = false;
void draw_stat_area() {
  if (device_init_done) {
    #if BOARD_MODEL == BOARD_HELTEC_T096
      if (!stat_area_intialised) {
        if (disp_mode == DISP_MODE_LANDSCAPE) {
          stat_area.drawBitmap(0, 0, bm_frame_t096_land, STAT_AREA_W, STAT_AREA_H, SSD1306_WHITE, SSD1306_BLACK);
        } else {
          stat_area.drawBitmap(0, 0, bm_frame_t096, STAT_AREA_W, STAT_AREA_H, SSD1306_WHITE, SSD1306_BLACK);
        }
        stat_area_intialised = true;
      }

      // Lamp states follow the same signals that drive the RX/TX LEDs:
      // carrier detect here, and display_indicate_tx() called from the
      // transmit paths; both held for LAMP_HOLD_MS so short events stay
      // visible at the display frame rate.
      if (radio_online && dcd_led) { lamp_rx_until = millis()+LAMP_HOLD_MS; }
      if (display_tx) { lamp_tx_until = millis()+LAMP_HOLD_MS; }
      lamp_rx_lit = millis() < lamp_rx_until;
      lamp_tx_lit = millis() < lamp_tx_until;

      // Indicator lamps: labels knocked out of the fill when lit
      if (lamp_rx_lit) { stat_area.drawBitmap(3, st_box_y2, bm_lamp_rx, 16, 16, SSD1306_BLACK, SSD1306_WHITE); }
      else             { stat_area.drawBitmap(3, st_box_y2, bm_lamp_rx, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
      if (lamp_tx_lit) { stat_area.drawBitmap(61, st_box_y2, bm_lamp_tx, 16, 16, SSD1306_BLACK, SSD1306_WHITE); }
      else             { stat_area.drawBitmap(61, st_box_y2, bm_lamp_tx, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }

      // Icon boxes and the status row keep their bm_frame appearance; the
      // row positions differ per orientation (set in update_area_positions)
      draw_cable_icon(3, st_box_y0);
      draw_bt_icon(3, st_box_y1);
      draw_lora_icon(61, st_box_y0);
      draw_mw_icon(61, st_box_y1);
      draw_battery_bars(4, 90);
      // The low-battery tint is colour-only: flipping it doesn't change
      // the mono canvas (the outline pixels stay identical), so force a
      // repaint when it transitions
      static bool battery_low_prev = false;
      if (battery_low_lit != battery_low_prev) {
        battery_low_prev = battery_low_lit;
        stat_mark_dirty(2, 88, 18, 7);
      }
      draw_battery_voltage(20, 93);
      draw_quality_bars(44, 88);
      draw_signal_bars(60, 88);
      if (radio_online) {
        draw_waterfall(WF_POS_X, wf_y);
      }
    #else
      if (!stat_area_intialised) {
        stat_area.drawBitmap(0, 0, bm_frame, 64, 64, SSD1306_WHITE, SSD1306_BLACK);
        stat_area_intialised = true;
      }

      draw_cable_icon(3, 8);
      draw_bt_icon(3, 30);
      draw_lora_icon(45, 8);
      #if BOARD_MODEL == BOARD_MESHPOE_S3
        draw_eth_icon(45, 30);
      #else
        draw_mw_icon(45, 30);
      #endif
      #if HAS_VSENSE == true
        draw_vsense_voltage(4, 58);
      #else
        draw_battery_bars(4, 58);
      #endif
      draw_quality_bars(28, 56);
      draw_signal_bars(44, 56);
      if (radio_online) {
        draw_waterfall(27, 4);
      }
    #endif
  }
}

void update_stat_area() {
  if (eeprom_ok && !firmware_update_mode && !console_active) {

    draw_stat_area();
    if (disp_mode == DISP_MODE_PORTRAIT) {
      #if BOARD_MODEL == BOARD_HELTEC_T096
        push_is_stat = true; push_stat_dy = 0;
      #endif
      drawBitmap(p_as_x, p_as_y, stat_area.getBuffer(), stat_area.width(), stat_area.height(), SSD1306_WHITE, SSD1306_BLACK);
      #if BOARD_MODEL == BOARD_HELTEC_T096
        push_is_stat = false;
      #endif
    } else if (disp_mode == DISP_MODE_LANDSCAPE) {
      #if BOARD_MODEL == BOARD_HELTEC_T096
        // Icon/waterfall cluster (stat rows 0..79) on the right half, the
        // status strip (stat rows 80..95) on the left half under the banner
        push_is_stat = true; push_stat_dy = 0;
        drawBitmap(p_as_x, p_as_y, stat_area.getBuffer(), stat_area.width(), 80, SSD1306_WHITE, SSD1306_BLACK);
        push_stat_dy = 80;
        drawBitmap(p_ss_x, p_ss_y, stat_area.getBuffer() + 80*((STAT_AREA_W+7)/8), stat_area.width(), 16, SSD1306_WHITE, SSD1306_BLACK);
        push_is_stat = false;
      #else
        drawBitmap(p_as_x+2, p_as_y, stat_area.getBuffer(), stat_area.width(), stat_area.height(), SSD1306_WHITE, SSD1306_BLACK);
        if (device_init_done && !disp_ext_fb) drawLine(p_as_x, 0, p_as_x, 64, SSD1306_WHITE);
      #endif
    }

  } else {
    // bm_updating and bm_console are fixed 64x64 images; center them in
    // the status area when it is larger than 64x64
    int bm_x = p_as_x + (stat_area.width()-64)/2;
    int bm_y = p_as_y; if (disp_mode == DISP_MODE_PORTRAIT) bm_y += (stat_area.height()-64)/2;
    if (firmware_update_mode) {
      drawBitmap(bm_x, bm_y, bm_updating, 64, 64, SSD1306_BLACK, SSD1306_WHITE);
    } else if (console_active && device_init_done) {
      drawBitmap(bm_x, bm_y, bm_console, 64, 64, SSD1306_BLACK, SSD1306_WHITE);
      if (disp_mode == DISP_MODE_LANDSCAPE) {
        drawLine(p_as_x, 0, p_as_x, 64, SSD1306_WHITE);
      }
    }
  }
}

// Draws 64px-wide art into the device area, centered. When the device area
// is wider than the art, rows whose edge pixels are lit get stretched into
// the side margins, so full-bleed boxes span the whole area width.
void draw_disp_art(int16_t y, const uint8_t* bitmap, int16_t h) {
  disp_area.drawBitmap(DISP_BM_X, y, bitmap, DISP_BM_W, h, SSD1306_WHITE, SSD1306_BLACK);
  #if DISP_BM_X > 0
    for (int16_t r = 0; r < h; r++) {
      uint16_t lc = (bitmap[r*(DISP_BM_W/8)] & 0x80) ? SSD1306_WHITE : SSD1306_BLACK;
      uint16_t rc = (bitmap[r*(DISP_BM_W/8)+(DISP_BM_W/8)-1] & 0x01) ? SSD1306_WHITE : SSD1306_BLACK;
      disp_area.drawFastHLine(0, y+r, DISP_BM_X, lc);
      disp_area.drawFastHLine(DISP_BM_X+DISP_BM_W, y+r, disp_area.width()-(DISP_BM_X+DISP_BM_W), rc);
    }
  #endif
}

#if BOARD_MODEL == BOARD_HELTEC_T096
// Lights the TX lamp and pushes the status area immediately, so the
// indication appears BEFORE the blocking transmission - the same way
// led_tx_on() lights the physical LED beforehand. Also sets display_tx,
// so the waterfall TX marker lands in the same push.
void display_indicate_tx() {
  lamp_tx_until = millis()+LAMP_HOLD_MS;
  display_tx = true;
  if (disp_ready && !display_blanked && !display_updating) { update_stat_area(); }
}
#endif

#define START_PAGE 0
const uint8_t pages = 4;
uint8_t disp_page = START_PAGE;
#if HAS_WIFI
  extern IPAddress wr_device_ip;
#endif
#if HAS_ETHERNET
  extern bool eth_is_connected;
  extern IPAddress eth_device_ip;
#endif

#if HAS_WIFI || HAS_ETHERNET
void draw_disp_ip_line(const char* label, IPAddress ip) {
  uint8_t ones = 3+one_counts[ip[0]]+one_counts[ip[1]]+one_counts[ip[2]]+one_counts[ip[3]];
  uint8_t chars = 7;
  for (uint8_t i = 0; i<4; i++) { if (ip[i] > 9) { chars++; } if (ip[i] > 99) { chars++; } }
  uint8_t width = chars*6-(ones*4);
  int alignment_offset = disp_area.width()-width;
  int ipxpos = alignment_offset;
  disp_area.setFont(SMALL_FONT); disp_area.setTextWrap(false); disp_area.setTextColor(SSD1306_WHITE); disp_area.setTextSize(1);
  disp_area.fillRect(0, 20, disp_area.width(), 17, SSD1306_BLACK);
  disp_area.setCursor(3, 34-8); disp_area.print(label);
  disp_area.setCursor(ipxpos, 34); disp_area.print(ip);
}
#endif

void draw_disp_area() {
  #if BOARD_MODEL == BOARD_HELTEC_T096
    disp_banner_fg = 0;
  #endif
  if (!device_init_done || firmware_update_mode) {
    uint8_t p_by = 37;
    if (disp_mode == DISP_MODE_LANDSCAPE || firmware_update_mode) {
      p_by = 18;
      disp_area.fillRect(0, 0, disp_area.width(), disp_area.height(), SSD1306_BLACK);
    }
    if (!device_init_done) draw_disp_art(p_by, bm_boot, 27);
    if (firmware_update_mode) draw_disp_art(p_by, bm_fw_update, 27);
  } else {
    if (!disp_ext_fb or bt_ssp_pin != 0) {
      if (radio_online && display_diagnostics) {
        disp_area.fillRect(0,8,disp_area.width(),37, SSD1306_BLACK); disp_area.fillRect(0,37,disp_area.width(),27, SSD1306_WHITE);
        disp_area.setFont(SMALL_FONT); disp_area.setTextWrap(false); disp_area.setTextColor(SSD1306_WHITE); disp_area.setTextSize(1);

        disp_area.setCursor(2, 13);
        disp_area.print("On");
        disp_area.setCursor(14, 13);
        disp_area.print("@");
        disp_area.setCursor(21, 13);
        disp_area.printf("%.1fKbps", (float)lora_bitrate/1000.0);

        //disp_area.setCursor(31, 23-1);
        disp_area.setCursor(2, 23-1);
        disp_area.print("Airtime:");
        
        disp_area.setCursor(11, 33-1);
        if (total_channel_util < 0.099) {
          //disp_area.printf("%.1f%%", total_channel_util*100.0);
          disp_area.printf("%.1f%%", airtime*100.0);
        } else {
          //disp_area.printf("%.0f%%", total_channel_util*100.0);
          disp_area.printf("%.0f%%", airtime*100.0);
        }
        disp_area.drawBitmap(2, 26-1, bm_hg_low, 5, 9, SSD1306_WHITE, SSD1306_BLACK);

        disp_area.setCursor(DIAG_COL2+11, 33-1);
        if (longterm_channel_util < 0.099) {
          //disp_area.printf("%.1f%%", longterm_channel_util*100.0);
          disp_area.printf("%.1f%%", longterm_airtime*100.0);
        } else {
          //disp_area.printf("%.0f%%", longterm_channel_util*100.0);
          disp_area.printf("%.0f%%", longterm_airtime*100.0);
        }
        disp_area.drawBitmap(DIAG_COL2+2, 26-1, bm_hg_high, 5, 9, SSD1306_WHITE, SSD1306_BLACK);


        disp_area.setTextColor(SSD1306_BLACK);
        disp_area.setCursor(2, 46);
        disp_area.print("Channel");
        disp_area.setCursor(DIAG_COL2+6, 46);
        disp_area.print("Load:");
        
        disp_area.setCursor(11, 57);
        if (total_channel_util < 0.099) {
          //disp_area.printf("%.1f%%", airtime*100.0);
          disp_area.printf("%.1f%%", total_channel_util*100.0);
        } else {
          //disp_area.printf("%.0f%%", airtime*100.0);
          disp_area.printf("%.0f%%", total_channel_util*100.0);
        }
        disp_area.drawBitmap(2, 50, bm_hg_low, 5, 9, SSD1306_BLACK, SSD1306_WHITE);

        disp_area.setCursor(DIAG_COL2+11, 57);
        if (longterm_channel_util < 0.099) {
          //disp_area.printf("%.1f%%", longterm_airtime*100.0);
          disp_area.printf("%.1f%%", longterm_channel_util*100.0);
        } else {
          //disp_area.printf("%.0f%%", longterm_airtime*100.0);
          disp_area.printf("%.0f%%", longterm_channel_util*100.0);
        }
        disp_area.drawBitmap(DIAG_COL2+2, 50, bm_hg_high, 5, 9, SSD1306_BLACK, SSD1306_WHITE);

      } else {
        #if BOARD_MODEL == BOARD_HELTEC_T096
          // Full-width header: left-aligned art with the unsigned.io strip
          // and zigzag extended to the whole 80px width
          if (device_signatures_ok()) { disp_area.drawBitmap(0, 0, bm_def_lc_t096, disp_area.width(), 23, SSD1306_WHITE, SSD1306_BLACK); }
          else {                        disp_area.drawBitmap(0, 0, bm_def_t096,    disp_area.width(), 23, SSD1306_WHITE, SSD1306_BLACK); }
        #else
          if (device_signatures_ok()) { draw_disp_art(0, bm_def_lc, 23); }
          else {                        draw_disp_art(0, bm_def, 23); }
        #endif

        bool wifi_ip_ready = false;
        bool eth_ip_ready = false;
        #if HAS_WIFI
          wifi_ip_ready = wifi_is_connected();
        #endif
        #if HAS_ETHERNET
          eth_ip_ready = eth_is_connected;
        #endif

        // Page 1 dwells on the WiFi IP, page 3 on the Ethernet IP, each for
        // a full page_interval, same as the original single WiFi IP page
        bool display_ip = false;
        bool show_wifi_ip = false;
        bool show_eth_ip = false;
        if (wifi_ip_ready && disp_page == 1) {
          display_ip = true;
          show_wifi_ip = true;
        } else if (eth_ip_ready && disp_page == 3) {
          display_ip = true;
          show_eth_ip = true;
        }
        if (display_ip) {
          #if HAS_WIFI
            if (show_wifi_ip) { draw_disp_ip_line("WiFi IP:", wr_device_ip); }
          #endif
          #if HAS_ETHERNET
            if (show_eth_ip) { draw_disp_ip_line("Eth IP:", eth_device_ip); }
          #endif
        } else {
          disp_area.setFont(SMALL_FONT); disp_area.setTextWrap(false); disp_area.setTextColor(SSD1306_WHITE); disp_area.setTextSize(2);
          disp_area.fillRect(0, 20, disp_area.width(), 17, SSD1306_BLACK); uint8_t ofsc = 0;
          if ((bt_dh[14] & 0b00001111) == 0x01) { ofsc += 8; }
          if ((bt_dh[14] >> 4)         == 0x01) { ofsc += 8; }
          if ((bt_dh[15] & 0b00001111) == 0x01) { ofsc += 8; }
          if ((bt_dh[15] >> 4)         == 0x01) { ofsc += 8; }
          #if BOARD_MODEL == BOARD_HELTEC_T096
            // Right-aligned to the screen edge with 3px padding, mirroring
            // the left-aligned RNode logo above
            disp_area.setCursor(31+ofsc, 32); disp_area.printf("%02X%02X", bt_dh[14], bt_dh[15]);
          #else
            disp_area.setCursor(DISP_BM_X+17+ofsc, 32); disp_area.printf("%02X%02X", bt_dh[14], bt_dh[15]);
          #endif
        }
      }

      if (!hw_ready || radio_error || !device_firmware_ok()) {
        if (!device_firmware_ok()) {
          draw_disp_art(37, bm_fw_corrupt, 27);
          #if BOARD_MODEL == BOARD_HELTEC_T096 && USE_COLOR_DISPLAY == true
            disp_banner_fg = COLOR_BANNER_ALERT;
          #endif
        } else {
          if (!modem_installed) {
            draw_disp_art(37, bm_no_radio, 27);
          } else {
            draw_disp_art(37, bm_conf_missing, 27);
          }
        }
      } else if (bt_state == BT_STATE_PAIRING and bt_ssp_pin != 0) {
        char *pin_str = (char*)malloc(DISP_PIN_SIZE+1);
        sprintf(pin_str, "%06d", bt_ssp_pin);

        draw_disp_art(37, bm_pairing, 27);
        for (int i = 0; i < DISP_PIN_SIZE; i++) {
          uint8_t numeric = pin_str[i]-48;
          uint8_t offset = numeric*5;
          disp_area.drawBitmap(DISP_BM_X+7+9*i, 37+16, bm_n_uh+offset, 8, 5, SSD1306_WHITE, SSD1306_BLACK);
        }
        free(pin_str);
      } else {
        if (millis()-last_page_flip >= page_interval) {
          disp_page = (++disp_page%pages);
          last_page_flip = millis();
          if (not community_fw and disp_page == 0) disp_page = 1;
        }

        if (radio_online) {
          if (!display_diagnostics) {
            draw_disp_art(37, bm_online, 27);
          }
        } else {
          if (disp_page == 0) {
            if (true || device_signatures_ok()) {
              draw_disp_art(37, bm_checks, 27);
              #if BOARD_MODEL == BOARD_HELTEC_T096 && USE_COLOR_DISPLAY == true
                disp_banner_fg = COLOR_BANNER_OK;
              #endif
            } else {
              draw_disp_art(37, bm_nfr, 27);
            }
          } else if (disp_page == 1) {
            if (!console_active) {
              draw_disp_art(37, bm_hwok, 27);
              #if BOARD_MODEL == BOARD_HELTEC_T096 && USE_COLOR_DISPLAY == true
                disp_banner_fg = COLOR_BANNER_OK;
              #endif
            } else {
              draw_disp_art(37, bm_console_active, 27);
            }
          } else if (disp_page == 2) {
            draw_disp_art(37, bm_version, 27);
            #if BOARD_MODEL == BOARD_HELTEC_T096 && USE_COLOR_DISPLAY == true
              disp_banner_fg = COLOR_BANNER_OK;
            #endif
            char *v_str = (char*)malloc(3+1);
            sprintf(v_str, "%01d%02d", MAJ_VERS, MIN_VERS);
            for (int i = 0; i < 3; i++) {
              uint8_t numeric = v_str[i]-48; uint8_t bm_offset = numeric*5;
              uint8_t dxp = DISP_BM_X+20;
              if (i == 1) dxp += 9*1+4;
              if (i == 2) dxp += 9*2+4;
              disp_area.drawBitmap(dxp, 37+16, bm_n_uh+bm_offset, 8, 5, SSD1306_WHITE, SSD1306_BLACK);
            }
            free(v_str);
            disp_area.drawLine(DISP_BM_X+27, 37+19, DISP_BM_X+28, 37+19, SSD1306_BLACK);
            disp_area.drawLine(DISP_BM_X+27, 37+20, DISP_BM_X+28, 37+20, SSD1306_BLACK);
          } else if (disp_page == 3) {
            if (!console_active) {
              draw_disp_art(37, bm_hwok, 27);
              #if BOARD_MODEL == BOARD_HELTEC_T096 && USE_COLOR_DISPLAY == true
                disp_banner_fg = COLOR_BANNER_OK;
              #endif
            } else {
              draw_disp_art(37, bm_console_active, 27);
            }
          }
        }
      }
    } else {
      #if DISP_BM_X > 0
        disp_area.fillRect(0, 0, disp_area.width(), disp_area.height(), SSD1306_BLACK);
      #endif
      disp_area.drawBitmap(DISP_BM_X, 0, fb, DISP_BM_W, 64, SSD1306_WHITE, SSD1306_BLACK);
    }
  }
}

void update_disp_area() {
  draw_disp_area();

  #if BOARD_MODEL == BOARD_HELTEC_T096
    static uint16_t banner_fg_prev = 0;
    if (disp_banner_fg != banner_fg_prev) {
      banner_fg_prev = disp_banner_fg;
      colour_mark_dirty(p_ad_x, p_ad_y+37, DISP_AREA_W, 27);
    }
  #endif

  drawBitmap(p_ad_x, p_ad_y, disp_area.getBuffer(), disp_area.width(), disp_area.height(), SSD1306_WHITE, SSD1306_BLACK);
  #if BOARD_MODEL != BOARD_HELTEC_T096
  if (disp_mode == DISP_MODE_LANDSCAPE) {
    if (device_init_done && !firmware_update_mode && !disp_ext_fb) {
      drawLine(0, 0, 0, 63, SSD1306_WHITE);
    }
  }
  #endif
}

void display_recondition() {
  #if PLATFORM == PLATFORM_ESP32
    for (uint8_t iy = 0; iy < disp_area.height(); iy++) {
      unsigned char rand_seg [] = {random(0xFF),random(0xFF),random(0xFF),random(0xFF),random(0xFF),random(0xFF),random(0xFF),random(0xFF)};
      stat_area.drawBitmap(0, iy, rand_seg, 64, 1, SSD1306_WHITE, SSD1306_BLACK);
      disp_area.drawBitmap(0, iy, rand_seg, 64, 1, SSD1306_WHITE, SSD1306_BLACK);
    }

    drawBitmap(p_ad_x, p_ad_y, disp_area.getBuffer(), disp_area.width(), disp_area.height(), SSD1306_WHITE, SSD1306_BLACK);
    if (disp_mode == DISP_MODE_PORTRAIT) {
      drawBitmap(p_as_x, p_as_y, stat_area.getBuffer(), stat_area.width(), stat_area.height(), SSD1306_WHITE, SSD1306_BLACK);
    } else if (disp_mode == DISP_MODE_LANDSCAPE) {
      drawBitmap(p_as_x, p_as_y, stat_area.getBuffer(), stat_area.width(), stat_area.height(), SSD1306_WHITE, SSD1306_BLACK);
    }
  #endif
}

bool epd_blanked = false;
#if BOARD_MODEL == BOARD_TECHO
  void epd_blank(bool full_update = true) {
    display.setFullWindow();
    display.fillScreen(SSD1306_WHITE);
    display.display(full_update);
  }

  void epd_black(bool full_update = true) {
    display.setFullWindow();
    display.fillScreen(SSD1306_BLACK);
    display.display(full_update);
  }
#endif

void update_display(bool blank = false) {
  display_updating = true;
  if (blank == true) {
    last_disp_update = millis()-disp_update_interval-1;
  } else {
    if (display_blanking_enabled && millis()-last_unblank_event >= display_blanking_timeout) {
      blank = true;
      display_blanked = true;
      if (display_intensity != 0) {
        display_unblank_intensity = display_intensity;
      }
      display_intensity = 0;
    } else {
      display_blanked = false;
      if (display_unblank_intensity != 0x00) {
        display_intensity = display_unblank_intensity;
        display_unblank_intensity = 0x00;
      }
    }
  }

  if (blank) {
    if (millis()-last_disp_update >= disp_update_interval) {
      if (display_contrast != display_intensity) {
        display_contrast = display_intensity;
        set_contrast(&display, display_contrast);
      }

      #if BOARD_MODEL == BOARD_TECHO
        if (!epd_blanked) {
          epd_blank();
          epd_blanked = true;
        }
      #endif

      #if BOARD_MODEL == BOARD_HELTEC_T114
        display.clear();
        display.display();
        digitalWrite(PIN_T114_TFT_BLGT, HIGH);
      #elif BOARD_MODEL == BOARD_HELTEC_T096
        // Backlight is already set by set_contrast() above
      #elif BOARD_MODEL != BOARD_TDECK && BOARD_MODEL != BOARD_TECHO
        display.clearDisplay();
        display.display();
      #else
        // TODO: Clear screen
      #endif

      last_disp_update = millis();
    }

  } else {
    if (millis()-last_disp_update >= disp_update_interval) {
      uint32_t current = millis();
      if (display_contrast != display_intensity) {
        display_contrast = display_intensity;
        set_contrast(&display, display_contrast);
      }

      #if BOARD_MODEL == BOARD_HELTEC_T114
        display.clear();
        digitalWrite(PIN_T114_TFT_BLGT, LOW);
      #elif BOARD_MODEL == BOARD_HELTEC_T096
        // Backlight is already set by set_contrast() above
      #elif BOARD_MODEL != BOARD_TDECK && BOARD_MODEL != BOARD_TECHO
        display.clearDisplay();
      #endif

      if (recondition_display) {
        disp_target_fps = 30;
        disp_update_interval = 1000/disp_target_fps;
        display_recondition();
      } else {
        #if BOARD_MODEL == BOARD_TECHO
          display.setFullWindow();
          display.fillScreen(SSD1306_WHITE);
        #endif

        #if HAS_MENU == true
          // The settings menu is always laid out for the panel's native
          // 128x64 landscape shape, regardless of what rotation the main
          // content is using - the panel itself doesn't physically change
          // shape, so forcing rotation 0/2 (both landscape, GFX-wise) here
          // just undoes whatever swap the main content's rotation setting
          // applied. 90 and 270 are the same physical mounting 180 degrees
          // apart, so they need opposite landscape variants (0 vs 2) to
          // still read right-side-up - same for the 0/180 pair.
          static bool menu_was_open = false;
          bool menu_open_now = menu_is_open();
          if (menu_open_now && !menu_was_open) {
            display.setRotation(active_display_rotation & 0x02);
          } else if (!menu_open_now && menu_was_open) {
            display.setRotation(active_display_rotation);
          }
          menu_was_open = menu_open_now;

          if (menu_open_now) {
            draw_settings_menu_disp();
          } else
        #endif
        {
          update_stat_area();
          update_disp_area();
        }
      }
      
      #if BOARD_MODEL == BOARD_TECHO
        if (current-last_epd_refresh >= epd_update_interval) {
          if (current-last_epd_full_refresh >= REFRESH_PERIOD) { display.display(false); last_epd_full_refresh = millis(); }
          else { display.display(true); }
          last_epd_refresh = millis();
          epd_blanked = false;
        }
      #elif BOARD_MODEL != BOARD_TDECK && BOARD_MODEL != BOARD_HELTEC_T096
        display.display();
      #endif

      last_disp_update = millis();
    }
  }
  display_updating = false;
}

void display_unblank() {
  last_unblank_event = millis();
  #if BOARD_MODEL == BOARD_HELTEC_T114
    digitalWrite(PIN_T114_TFT_BLGT, LOW);
  #elif BOARD_MODEL == BOARD_HELTEC_T096
    analogWrite(PIN_T096_TFT_BLGT, 0);
  #endif
}

void ext_fb_enable() {
  disp_ext_fb = true;
}

void ext_fb_disable() {
  disp_ext_fb = false;
}
