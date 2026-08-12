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
    #include <Adafruit_ST7789.h>
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
#include "Fonts/PicoPixel.h"
#define DISP_W 128
#define DISP_H 64

#if BOARD_MODEL == BOARD_HELTEC_T114
  // Full-panel (240x135 landscape) RGB565 boot splash - too large and too
  // full-color to fit the 1bpp disp_area/colourizer pipeline every other
  // piece of T114 art goes through, so it's pushed directly with its own
  // raw writePixels() call instead (see draw_disp_area()'s T114 branch).
  #include "SplashT114.h"
#endif

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
  Adafruit_ST7789 display = Adafruit_ST7789(&SPI1, DISPLAY_CS, DISPLAY_DC, DISPLAY_RST);
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
  #if HAS_INPUT == true
    // Main-button hold-tier feedback (Menu.h) - drawn as part of the
    // normal (menu-closed) redraw path below, every cycle, not just once
    // when the tier changes - see draw_button_hold_overlay()'s own
    // comment for why.
    void draw_button_hold_overlay();
  #endif
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
#elif BOARD_MODEL == BOARD_HELTEC_T114
  // 135x240 panel: content canvases are the panel's full 135px width, no
  // side padding - portrait stacks them (110+130=240 exactly). This no
  // longer divides evenly into landscape's 240px width the way a 120-wide
  // canvas would (135+135=270), so landscape placement (update_area_
  // positions()) is a known rough spot until that orientation gets its
  // own pass - portrait is what's actually been tuned on hardware so far.
  // Bitmap art stays 64px wide, centered with a DISP_BM_X offset.
  #define WATERFALL_SIZE 115
  #define STAT_AREA_W 135
  #define STAT_AREA_H 130
  #define DISP_AREA_W 135
  // Was briefly grown to 120 rows (from 110) to fit a landscape-only 5th
  // line in the radio-params/GNSS box (Display.h, draw_disp_area()'s
  // BOARD_HELTEC_T114 branch) - that line (GNSS Time) got merged onto the
  // Fix line instead, so the extra rows are unused again; reverted back to
  // matching DISP_AREA_PORTRAIT_H exactly, same as before that 5th line
  // ever existed.
  #define DISP_AREA_H 110
  #define DISP_AREA_PORTRAIT_H 110
  #define DISP_BM_X 36
  #define DIAG_COL2 64
  // Waterfall fills the whole free column to the right of the reused
  // 64x64 generic icon frame (see draw_stat_area()'s T114 branch, which
  // sits flush left at x0-63), all the way to the canvas's right edge.
  // WF_BORDER_* describes the border box and legend row drawn once around
  // it (draw_stat_area()); WF_POS_X/Y and WF_PIXEL_WIDTH/WATERFALL_SIZE
  // describe the content area 2px inside that border, with a small legend
  // row above showing WF_RSSI_MIN/MAX.
  #define WF_BORDER_X 61
  #define WF_BORDER_Y 11
  #define WF_BORDER_W (STAT_AREA_W-WF_BORDER_X)
  #define WF_BORDER_H 119
  #define WF_LEGEND_Y 3 // black margin above the legend box
  #define WF_LEGEND_H (WF_BORDER_Y-WF_LEGEND_Y) // legend box, flush against the border below (no gap)
  #define WF_POS_X (WF_BORDER_X+2)
  #define WF_POS_Y (WF_BORDER_Y+2)

  // Landscape (240x135 physical) needs its own canvases - disp_area/
  // stat_area above are each the full 135px portrait width, and 135+135
  // overflows the 240px landscape width by 30px (see update_area_
  // positions()'s old comment). 130+110 fits exactly. Icon boxes/lamps/
  // RSSI-SNR gauge/battery/CPU temp/uptime all reuse their portrait local
  // coordinates as-is in stat_area_land (they only ever used the left
  // ~60px of the 135-wide portrait canvas anyway, well within 110px too);
  // only the waterfall - which fills whatever's left of the canvas width -
  // needs its own narrower geometry below.
  // disp_area itself (135 wide) is reused as-is for landscape, unlike
  // stat_area - no need for a second disp_area_land object - but only
  // DISP_AREA_LAND_W of its own columns actually get pushed to the panel
  // in landscape (see update_disp_area()), leaving the rest of the 240px
  // width for a wider stat_area_land/waterfall instead of wasting it on
  // disp_area's unused right margin.
  #define DISP_AREA_LAND_W 120
  // The radio-params box's own drawn/pushed width - 2px short of
  // DISP_AREA_LAND_W (where stat_area_land actually starts) so there's a
  // visible gap between the box's right border and the icon boxes, rather
  // than the two sitting flush against each other.
  #define DISP_AREA_LAND_BOX_W (DISP_AREA_LAND_W-2)
  #define STAT_AREA_LAND_W (240-DISP_AREA_LAND_W)
  #define STAT_AREA_LAND_H 135
  // WF_BORDER_X/Y/H, WF_LEGEND_Y/H and WF_POS_X/Y all come out numerically
  // identical whether the border sits in a 135-wide or 105-wide canvas (the
  // border's left edge, not its width, is what those describe) - only the
  // border's *width* actually differs by canvas width, so that's the only
  // landscape-specific macro needed here.
  #define LWF_BORDER_W (STAT_AREA_LAND_W-WF_BORDER_X)
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
#elif BOARD_MODEL == BOARD_HELTEC_T114
  // Runtime, not compile-time: landscape's stat_area_land is narrower than
  // portrait's stat_area (STAT_AREA_LAND_W < STAT_AREA_W), so the waterfall
  // needs a different rendered width per orientation. Every existing use of
  // WF_PIXEL_WIDTH (draw_waterfall(), update_waterfall(), the colourizer)
  // stays untouched - this shadows the macro onto the variable instead of
  // rewriting each call site. Set explicitly for both orientations in
  // update_area_positions(); this initializer is just the portrait default.
  int16_t t114_wf_pixel_width = (WF_BORDER_W-4);
  #define WF_PIXEL_WIDTH t114_wf_pixel_width
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
#elif BOARD_MODEL == BOARD_HELTEC_T114
  // Unlike T096, the same stat_area content is drawn regardless of
  // orientation - both STAT_AREA_H (130) and DISP_AREA_H (110) fit within
  // either orientation's available space (240 stacked in portrait, 135
  // individually in landscape), so there's no landscape strip-split and
  // these positions don't need to vary per orientation - only the push
  // position does, see update_area_positions().
  // Row positions within the reused 64x64 generic icon frame (bm_frame,
  // placed flush at the canvas's top-left - see draw_stat_area()'s
  // T114 branch). y0/y1 match the frame art's own baked box positions
  // (same values the generic non-T096/T114 boards use, since it's the
  // same bitmap) so the drawn icon glyphs land inside their outlines
  // instead of merely near them; y2 (lamps) and the quality/signal/RSSI-
  // SNR rows below extend into space this canvas has that a plain 64x64
  // one doesn't, forming one continuous left-hand column down to the
  // battery indicator at the bottom.
  const int16_t st_box_y0 = 4;    // first icon row (cable / lora)
  const int16_t st_box_y1 = 26;   // second icon row (bt / 2.4G)
  const int16_t st_box_y2 = 49;   // lamp row (rx / tx) - same 22px row-to-row
                                   // spacing as the frame's own box0->box1 gap
  const int16_t st_info_y0 = 92;  // RSSI text row
  const int16_t st_info_y1 = 104; // SNR text row
  const int16_t st_qs_y   = 108;  // quality/signal bars, below the RSSI/SNR
                                   // gauges and right above the battery
  const int16_t wf_y = WF_POS_Y; // waterfall content top
  bool push_is_stat = false;
  int16_t push_stat_dy = 0;
#endif

GFXcanvas1 stat_area(STAT_AREA_W, STAT_AREA_H);
GFXcanvas1 disp_area(DISP_AREA_W, DISP_AREA_H);
#if BOARD_MODEL == BOARD_HELTEC_T114
  GFXcanvas1 stat_area_land(STAT_AREA_LAND_W, STAT_AREA_LAND_H);
#endif

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
    // Content canvases are the panel's full 135px width (see STAT_AREA_W/
    // DISP_AREA_W above), so portrait needs no centering offset - it stacks
    // disp_area above stat_area (110+130=240 exactly). Landscape places
    // disp_area on the left, but only pushes its own left DISP_AREA_LAND_W
    // columns (see update_disp_area()) - narrower than its full 135, so
    // stat_area_land gets the reclaimed width instead of it going to
    // disp_area's own unused right margin. DISP_AREA_LAND_W+STAT_AREA_LAND_W
    // = 240 exactly, so no centering math is needed here either.
    if (disp_mode == DISP_MODE_PORTRAIT) {
      p_ad_x = (135-DISP_AREA_W)/2;
      p_ad_y = 0;
      p_as_x = (135-STAT_AREA_W)/2;
      // DISP_AREA_PORTRAIT_H, same value as DISP_AREA_H now - kept as its
      // own constant since portrait's stacking should stay pinned to it
      // regardless of whether DISP_AREA_H ever grows again for some other
      // reason (see its own comment).
      p_as_y = DISP_AREA_PORTRAIT_H;
      t114_wf_pixel_width = WF_BORDER_W-4;
    } else if (disp_mode == DISP_MODE_LANDSCAPE) {
      p_ad_x = 0;
      p_ad_y = 3; // lines disp_area's top edge up with the icon boxes'
      p_as_x = DISP_AREA_LAND_W;
      p_as_y = 0;
      t114_wf_pixel_width = LWF_BORDER_W-4;
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
  // Perceived brightness follows duty^(1/gamma), not duty linearly - see the
  // identical table on BOARD_HELTEC_T096 for the full rationale.
  static const uint8_t t114_backlight_gamma[256] = {
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

  void set_contrast(Adafruit_ST7789 *display, uint8_t value) {
    // Backlight is active-low, so duty cycle is inverted.
    uint8_t pwm = t114_backlight_gamma[value];
    analogWrite(PIN_T114_TFT_BLGT, 255-pwm);
  }
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
    // Assumed 135x240 panel (1.14" ST7789) - verify against the real
    // hardware and adjust if the image is offset/clipped on first bring-up.
    display.init(135, 240);
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
        #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114
          // Native orientation (rotation 0) is portrait on both panels, so
          // rotations 1 and 3 yield landscape and 0/2 yield portrait.
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
          display.setRotation(0);
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

      display.cp437(true);

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
        display.fillScreen(SSD1306_BLACK);
        pinMode(PIN_T114_TFT_BLGT, OUTPUT);
        set_contrast(&display, display_intensity);
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
  display.drawLine(x, y, width, height, colour);
}

// Draws a filled rectangle on the screen
void fillRect(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t colour) {
  display.fillRect(x, y, width, height, colour);
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
    #define COLOR_INTERFERENCE COLOR565(0xE8, 0x50, 0xE8)  // magenta waterfall rows (WF_M_NTFR)
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

  // Settings menu (Menu.h) renders into this off-screen canvas, at the
  // panel's full landscape size (matches the operational screen's own
  // orientation - the menu never rotates the panel, see the menu-open/
  // close handling in update_display() below), rather than drawing raw
  // primitives straight to the unbuffered ST7735 - see MENU_GFX in
  // Menu.h. Composited through the same drawBitmap() pipeline as
  // stat_area/disp_area above, pushed as two 80-wide column halves (see
  // push_menu_canvas()) since drawBitmap()'s region cache and shared
  // pushbuf are both sized for bitmaps <=80px wide.
  #define MENU_CANVAS_W 160
  #define MENU_CANVAS_H 80
  GFXcanvas1 menu_canvas(MENU_CANVAS_W, MENU_CANVAS_H);
  // Last-pushed copy of menu_canvas's buffer. At 160x80 the canvas is
  // bigger than REGION_CACHE_BYTES (and wider than the cache's 80px
  // cutoff) so drawBitmap()'s own cache never dedupes it - without this
  // shadow, an unchanged menu screen (the common case between button
  // presses) would re-push the full frame on every update_display()
  // cycle (~7fps) for as long as the menu sits open.
  uint8_t menu_canvas_shadow[((MENU_CANVAS_W+7)/8) * MENU_CANVAS_H];
  bool menu_canvas_shadow_valid = false;

  // Set around menu_canvas/menu_popup_canvas pushes so the colourizer in
  // drawBitmap can tell menu content apart from a disp_area push - both
  // land on overlapping panel coordinates (the menu occupies the same
  // full-screen footprint the operational screen does), but menu content
  // must never pick up disp_area's banner tint (see drawBitmap's
  // colourizer) the way real disp_area pixels legitimately do.
  bool push_is_menu = false;

  // Small popup box canvas for draw_menu_status_rect()/
  // draw_button_hold_overlay(), kept within drawBitmap()'s
  // bitmapWidth<=80 cache cutoff so it's deduped on its own, no separate
  // shadow buffer needed. Landscape (menu-closed) geometry only - the
  // menu-open status popup (Sync NTP/Clear Static) needs HAS_WIFI/
  // HAS_ETHERNET, which this board doesn't have, so it's unreachable
  // today.
  #define MENU_POPUP_CANVAS_W 80
  #define MENU_POPUP_CANVAS_H 16
  GFXcanvas1 menu_popup_canvas(MENU_POPUP_CANVAS_W, MENU_POPUP_CANVAS_H);
#elif BOARD_MODEL == BOARD_HELTEC_T114
  // Same no-framebuffer/flicker-avoidance rationale as BOARD_HELTEC_T096
  // above (Adafruit_ST7789 has no internal framebuffer either), but sized
  // for T114's own, larger canvases and cache budget.
  #define REGION_CACHE_SLOTS 4
  #define REGION_CACHE_BYTES 2300 // enough for a 135x130 mono bitmap
  // Widest single bitmap the region cache/pushbuf below will diff and
  // batch into one push - covers stat_area/disp_area (135 wide) and the
  // menu canvas's own 135-wide horizontal bands (see push_menu_canvas()).
  #define T114_CACHE_MAX_W 135
  #if USE_COLOR_DISPLAY == true
    #define COLOR_LAMP_RX COLOR565(0x3E, 0xD8, 0x60)
    #define COLOR_LAMP_TX COLOR565(0x48, 0x96, 0xFF)
    #define COLOR_BAT_LOW COLOR565(0xEB, 0x4C, 0x42)
    #define COLOR_BANNER_OK COLOR565(0x28, 0x90, 0x40)    // darker green for status banners
    #define COLOR_BANNER_ALERT COLOR565(0xFF, 0xA0, 0x20)  // amber for warning banners
    #define COLOR_BT_ON COLOR565(0x28, 0x60, 0xC0)         // darker blue bluetooth box fill
    #define COLOR_INTERFERENCE COLOR565(0xE8, 0x50, 0xE8)  // magenta waterfall rows (WF_M_NTFR)
  #endif
  // RSSI/SNR gauges - a red-to-green filled bar with a 1px border and the
  // value centered inside in white, matching the style used by LoRaMon/
  // rns-wardrive-tools' map view (map_server.py's .rf-bar/.rf-mask/
  // .rf-text) - a fixed gradient revealed proportionally by the fill
  // boundary, rather than the whole bar tinted one solid colour.
  #define RF_BAR_X 25
  #define RF_BAR_W 33
  #define RF_BAR_H 11
  #define RF_RSSI_BAR_Y 68
  #define RF_SNR_BAR_Y 81
  // RSSI -110..-30dBm and SNR -20..+10dB -> 0-100%, matching
  // map_server.py's rssiToIntensity()/mkBar() scaling exactly.
  #define RF_RSSI_MIN -110
  #define RF_RSSI_MAX -30
  #define RF_SNR_MIN -20
  #define RF_SNR_MAX 10
  // A mono canvas can't tell "this lit pixel is glyph ink" apart from
  // "this lit pixel is fill" - both are just bit=1 - so a coordinate-box
  // guess (an earlier version of this) can't reliably tell the colourizer
  // which pixels to leave white: getTextBounds() doesn't agree with
  // print()'s real advance widths closely enough to trust. Instead, the
  // value text is rendered into these small offscreen masks too (in
  // addition to stat_area itself), and the colourizer bit-tests the mask
  // directly - the same "inspect the real bitmap" approach draw_bt_icon()
  // already uses to separate icon-shape pixels from box fill (see
  // bt_enabled_lit's own colourizer branch).
  // Sized to the full bar interior (not just a guessed glyph width) so no
  // string that fits on-screen can ever run past the mask's right edge -
  // an earlier, narrower mask (24px) clipped "-35"'s rightmost column,
  // leaving that sliver of the glyph untracked and gradient-tinted instead
  // of forced white.
  #define RF_MASK_W (RF_BAR_W-2)
  #define RF_MASK_H 9
  GFXcanvas1 rf_rssi_mask(RF_MASK_W, RF_MASK_H);
  GFXcanvas1 rf_snr_mask(RF_MASK_W, RF_MASK_H);
  // Where each mask's (0,0) lands within stat_area - x1 < x0 (see the
  // initializers) means "no text this frame" (bar empty, no reading yet),
  // never coincidentally matching a real position.
  int16_t rf_rssi_mask_x=0, rf_rssi_mask_y=-1;
  int16_t rf_snr_mask_x=0, rf_snr_mask_y=-1;

  // Same red->yellow->green interpolation as map_server.py's pctToColor(),
  // keyed off a bar-relative x position (0..RF_BAR_W-1) rather than the
  // reading's overall percent, so a static gradient is revealed by the
  // fill boundary instead of the whole bar being one solid colour.
  uint16_t rf_gradient_color(int16_t bar_rel_x) {
    float t = (float)bar_rel_x / (float)(RF_BAR_W-1);
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    uint8_t r, g, b = 51;
    if (t < 0.5) {
      float u = t*2.0;
      r = 204;
      g = (uint8_t)(51.0 + (204.0-51.0)*u);
    } else {
      float u = (t-0.5)*2.0;
      r = (uint8_t)(204.0 - 204.0*u);
      g = (uint8_t)(204.0 - 51.0*u);
    }
    return COLOR565(r, g, b);
  }
  // Background tint of the currently displayed status banner, 0 = none
  uint16_t disp_banner_fg = 0;
  // RX/TX indicator lamps and low-battery warning; states set in
  // draw_stat_area, read by the push-time colourizer in drawBitmap
  #define LAMP_HOLD_MS 300
  // Voltage readout turns red when approaching the critical voltage
  // (BAT_V_MIN; defined later in Power.h)
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
  // T114's stat_area content is orientation-independent (see st_box_y0's
  // own comment above), so unlike T096 there's no landscape strip-split -
  // every stat-canvas rectangle maps straight onto the panel at p_as.
  void stat_mark_dirty(int16_t sx, int16_t sy, int16_t w, int16_t h) {
    colour_mark_dirty(p_as_x+sx, p_as_y+sy, w, h);
  }

  // Settings menu (Menu.h) renders into this off-screen canvas at the
  // panel's full portrait size - the menu always forces portrait
  // regardless of the operational screen's orientation (see the menu-open/
  // close handling in update_display() below) - rather than drawing raw
  // primitives straight to the unbuffered ST7789. See MENU_GFX in Menu.h.
  // Composited through the same drawBitmap() pipeline as stat_area/
  // disp_area above; unlike T096's menu_canvas, this one needs no separate
  // shadow-buffer/dedup mechanism - push_menu_canvas() pushes it in three
  // full-width horizontal bands (135x80 each, tightly packed rows so no
  // srcRowBytes trick is needed), each individually within
  // T114_CACHE_MAX_W/REGION_CACHE_BYTES, so the ordinary per-region cache
  // above already diffs and dedupes them like any other push.
  #define MENU_CANVAS_W 135
  #define MENU_CANVAS_H 240
  GFXcanvas1 menu_canvas(MENU_CANVAS_W, MENU_CANVAS_H);

  // Set around menu_canvas/menu_popup_canvas pushes so the colourizer in
  // drawBitmap can tell menu content apart from a disp_area push - both
  // can land on overlapping panel coordinates, but menu content must never
  // pick up disp_area's banner tint (see drawBitmap's colourizer) the way
  // real disp_area pixels legitimately do.
  bool push_is_menu = false;

  // Small popup box canvas for draw_menu_status_rect()/
  // draw_button_hold_overlay(), kept within the region cache's cutoff so
  // it's deduped there like any other push - no separate shadow buffer.
  #define MENU_POPUP_CANVAS_W 80
  #define MENU_POPUP_CANVAS_H 16
  GFXcanvas1 menu_popup_canvas(MENU_POPUP_CANVAS_W, MENU_POPUP_CANVAS_H);
#endif

// Draws a bitmap to the display and auto scales it based on the boards configured DISPLAY_SCALE
void drawBitmap(int16_t startX, int16_t startY, const uint8_t* bitmap, int16_t bitmapWidth, int16_t bitmapHeight, uint16_t foregroundColour, uint16_t backgroundColour, int16_t srcRowBytes = 0) {
  #if BOARD_MODEL == BOARD_HELTEC_T096
    {
      // The whole changed rect is assembled here and sent as a single DMA
      // transfer. Splitting the push into per-row writePixels calls (one
      // DMA setup each) stretches the write burst several-fold, and the
      // panel visibly dips in brightness for the duration of a burst.
      static uint16_t pushbuf[STAT_AREA_W*STAT_AREA_H];
      int16_t byteWidth = (bitmapWidth + 7) / 8;
      // srcRowBytes lets a caller push a narrower slice out of a wider
      // source buffer (e.g. one 80px-wide column out of menu_canvas's
      // 160px-wide row) without a real copy - only menu_canvas's push
      // uses this (see push_menu_canvas()). Every other call site omits
      // it, so rowStride == byteWidth and this is a no-op there.
      int16_t rowStride = (srcRowBytes > 0) ? srcRowBytes : byteWidth;
      int32_t bitmapBytes = (int32_t)byteWidth * bitmapHeight;
      // A non-default stride means the source isn't tightly packed, so
      // the region cache's flat memcpy (which assumes byteWidth-spaced
      // rows) can't snapshot it correctly - always do a full, uncached
      // push in that case rather than teach the cache about stride too.
      bool cacheable = srcRowBytes == 0 && bitmapBytes <= REGION_CACHE_BYTES && bitmapWidth <= 80;

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
            int32_t idx = (int32_t)row * rowStride + bc;
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
                  else if (wf_m == WF_M_NTFR)   { fg = COLOR_INTERFERENCE; }
                }
              } else if (!push_is_menu && disp_banner_fg != 0) {
                // status banner fill (checks passed / hw ok / fw corrupt).
                // push_is_menu excludes this - the menu occupies the same
                // panel footprint disp_area does, but menu content is
                // never banner-tinted.
                int16_t bx = (startX+col) - p_ad_x;
                int16_t by = (startY+row) - p_ad_y;
                if (bx >= 0 && bx < DISP_AREA_W && by >= 37 && by <= 63) { fg = disp_banner_fg; }
              }
            }
          #endif
          // stored big-endian, ready for the panel. rowStride (not
          // byteWidth) here since this is the one path that also runs
          // for a strided source - see rowStride's own comment.
          uint16_t pxc = (bitmap[row * rowStride + col / 8] & (0x80 >> (col % 8))) ? fg : backgroundColour;
          pushbuf[pb++] = __builtin_bswap16(pxc);
        }
      }

      display.startWrite();
      display.setAddrWindow(startX+minX, startY+minY, maxX-minX+1, maxY-minY+1);
      display.writePixels(pushbuf, pb, true, true);
      display.endWrite();
    }
  #elif BOARD_MODEL == BOARD_HELTEC_T114
    {
      // Same single-DMA-burst-per-push rationale as BOARD_HELTEC_T096
      // above, sized for T114's own canvases/cache budget.
      static uint16_t pushbuf[STAT_AREA_W*STAT_AREA_H];
      int16_t byteWidth = (bitmapWidth + 7) / 8;
      int16_t rowStride = (srcRowBytes > 0) ? srcRowBytes : byteWidth;
      int32_t bitmapBytes = (int32_t)byteWidth * bitmapHeight;
      bool cacheable = srcRowBytes == 0 && bitmapBytes <= REGION_CACHE_BYTES && bitmapWidth <= T114_CACHE_MAX_W;

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
            int32_t idx = (int32_t)row * rowStride + bc;
            if (bitmap[idx] != reg->back[idx]) {
              uint8_t diff = bitmap[idx] ^ reg->back[idx];
              if (row < minY) minY = row;
              if (row > maxY) maxY = row;
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
          int16_t sy = row + push_stat_dy;
        #endif
        for (int16_t col = minX; col <= maxX; col++) {
          uint16_t fg = foregroundColour;
          #if USE_COLOR_DISPLAY == true
            // Placeholder icon-box coordinates - a first pass matched to
            // draw_stat_area()'s T114 layout below; tune both together on
            // hardware.
            if (foregroundColour == SSD1306_WHITE) {
              if (push_is_stat) {
                int16_t sx = col;
                if      (lamp_rx_lit && sx >= 1 && sx <= 16 && sy >= st_box_y2 && sy <= st_box_y2+15)   { fg = COLOR_LAMP_RX; }
                else if (lamp_tx_lit && sx >= 21 && sx <= 36 && sy >= st_box_y2 && sy <= st_box_y2+15)  { fg = COLOR_LAMP_TX; }
                else if (battery_low_lit && sx >= 2 && sx <= 19 && sy >= 123 && sy <= 129)       { fg = COLOR_BAT_LOW; }
                else if (battery_volt_low_lit && sx >= 22 && sx <= 40 && sy >= 122 && sy <= 129)  { fg = COLOR_BAT_LOW; }
                else if (bt_enabled_lit && sx >= 1 && sx <= 16 && sy >= st_box_y1 && sy <= st_box_y1+15) {
                  uint8_t bt_c = sx-1; uint8_t bt_r = sy-st_box_y1;
                  if (!(bm_bt[bt_icon_i*32 + bt_r*2 + bt_c/8] & (0x80 >> (bt_c%8)))) { fg = COLOR_BT_ON; }
                }
                else if (sx >= RF_BAR_X+1 && sx <= RF_BAR_X+RF_BAR_W-2 &&
                         sy >= RF_RSSI_BAR_Y+1 && sy <= RF_RSSI_BAR_Y+RF_BAR_H-2) {
                  fg = rf_gradient_color(sx - RF_BAR_X);
                  int16_t mx = sx - rf_rssi_mask_x, my = sy - rf_rssi_mask_y;
                  if (mx >= 0 && mx < RF_MASK_W && my >= 0 && my < RF_MASK_H) {
                    uint8_t mask_row_bytes = (RF_MASK_W+7)/8;
                    if (rf_rssi_mask.getBuffer()[my*mask_row_bytes + mx/8] & (0x80 >> (mx%8))) { fg = SSD1306_WHITE; }
                  }
                }
                else if (sx >= RF_BAR_X+1 && sx <= RF_BAR_X+RF_BAR_W-2 &&
                         sy >= RF_SNR_BAR_Y+1 && sy <= RF_SNR_BAR_Y+RF_BAR_H-2) {
                  fg = rf_gradient_color(sx - RF_BAR_X);
                  int16_t mx = sx - rf_snr_mask_x, my = sy - rf_snr_mask_y;
                  if (mx >= 0 && mx < RF_MASK_W && my >= 0 && my < RF_MASK_H) {
                    uint8_t mask_row_bytes = (RF_MASK_W+7)/8;
                    if (rf_snr_mask.getBuffer()[my*mask_row_bytes + mx/8] & (0x80 >> (mx%8))) { fg = SSD1306_WHITE; }
                  }
                }
                else if (sx >= WF_POS_X && sx < WF_POS_X+WF_PIXEL_WIDTH &&
                         sy >= wf_y && sy < wf_y+WATERFALL_SIZE) {
                  int wf_m = waterfall_meta[(waterfall_head + (sy-wf_y)) % WATERFALL_SIZE];
                  if      (wf_m == WF_M_RX_PKT) { fg = COLOR_LAMP_RX; }
                  else if (wf_m == WF_M_TX)     { fg = COLOR_LAMP_TX; }
                  else if (wf_m == WF_M_NTFR)   { fg = COLOR_INTERFERENCE; }
                }
              } else if (!push_is_menu && disp_banner_fg != 0) {
                int16_t bx = (startX+col) - p_ad_x;
                int16_t by = (startY+row) - p_ad_y;
                if (bx >= 0 && bx < DISP_AREA_W && by >= 37 && by <= 63) { fg = disp_banner_fg; }
              }
            }
          #endif
          uint16_t pxc = (bitmap[row * rowStride + col / 8] & (0x80 >> (col % 8))) ? fg : backgroundColour;
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

#if BOARD_MODEL == BOARD_HELTEC_T096
// Pushes menu_canvas (landscape, 160x80) to the panel as two 80-wide
// column halves (left 0-79, right 80-159) rather than one 160-wide
// push - drawBitmap()'s shared pushbuf and region cache are both sized/
// capped for <=80px-wide bitmaps, and splitting this way avoids growing
// that shared buffer. Each half is a genuine 80px-wide slice taken
// straight out of the wider 160px-wide canvas via drawBitmap()'s
// srcRowBytes param (the canvas's own row stride, 20 bytes) rather than
// a real copy. Skips the push entirely when the canvas content hasn't
// changed since the last call - see menu_canvas_shadow's own comment
// for why that's required, not just an optimization.
void push_menu_canvas() {
  uint8_t *buf = menu_canvas.getBuffer();
  size_t buf_bytes = ((MENU_CANVAS_W+7)/8) * MENU_CANVAS_H;
  if (menu_canvas_shadow_valid && memcmp(buf, menu_canvas_shadow, buf_bytes) == 0) {
    return;
  }
  memcpy(menu_canvas_shadow, buf, buf_bytes);
  menu_canvas_shadow_valid = true;

  int16_t canvas_row_bytes = (MENU_CANVAS_W+7)/8; // 20 - the real row stride
  push_is_menu = true;
  drawBitmap(0,  0, buf,     80, MENU_CANVAS_H, SSD1306_WHITE, SSD1306_BLACK, canvas_row_bytes);
  drawBitmap(80, 0, buf + 10, 80, MENU_CANVAS_H, SSD1306_WHITE, SSD1306_BLACK, canvas_row_bytes);
  push_is_menu = false;
}

// Forces the next push_menu_canvas() call to push unconditionally -
// needed once per menu-open/menu-close transition, since content drawn
// before the transition may look byte-identical to content drawn after
// it despite the panel itself having changed underneath (e.g. the
// operational screen having been redrawn while the menu was open).
void invalidate_menu_canvas_shadow() {
  menu_canvas_shadow_valid = false;
}

// Pushes menu_popup_canvas at the given on-panel offset. Narrow enough
// (width <= 80) to stay within drawBitmap()'s own region-cache cutoff,
// so unchanged content is already deduped there - no separate shadow
// buffer needed here, unlike push_menu_canvas() above.
void push_menu_popup_canvas(int16_t panel_x, int16_t panel_y, int16_t w, int16_t h) {
  push_is_menu = true;
  // w (the box's actual, text-fitted width) is usually narrower than
  // the canvas's own declared width (MENU_POPUP_CANVAS_W, 80) - the
  // canvas's real row stride stays fixed at that full width regardless,
  // so it has to be passed explicitly here too (same reason
  // push_menu_canvas() needs it) or drawBitmap() reads each row at the
  // wrong offset, since it would otherwise derive a stride from w
  // instead of the buffer's actual layout.
  int16_t canvas_row_bytes = (MENU_POPUP_CANVAS_W+7)/8;
  drawBitmap(panel_x, panel_y, menu_popup_canvas.getBuffer(), w, h, SSD1306_WHITE, SSD1306_BLACK, canvas_row_bytes);
  push_is_menu = false;
}
#elif BOARD_MODEL == BOARD_HELTEC_T114
// Pushes menu_canvas (portrait, 135x240) to the panel in three full-width
// horizontal bands (0-79, 80-159, 160-239) rather than one push - each
// band is 135x80, comfortably under STAT_AREA_W*STAT_AREA_H's shared
// pushbuf/cache budget above. Unlike T096's menu_canvas, each band is a
// tightly-packed, full-width slice of the canvas (no column-splitting),
// so no srcRowBytes stride override is needed - the ordinary per-region
// cache in drawBitmap() diffs and dedupes each band like any other push,
// no separate shadow buffer required.
void push_menu_canvas() {
  uint8_t *buf = menu_canvas.getBuffer();
  int16_t canvas_row_bytes = (MENU_CANVAS_W+7)/8;
  push_is_menu = true;
  for (int16_t band = 0; band < 3; band++) {
    drawBitmap(0, band*80, buf + band*80*canvas_row_bytes, MENU_CANVAS_W, 80, SSD1306_WHITE, SSD1306_BLACK);
  }
  push_is_menu = false;
}

// Pushes menu_popup_canvas at the given on-panel offset - see T096's
// identical function above for the rationale (canvas_row_bytes must be
// passed explicitly since w is usually narrower than the canvas's own
// declared width).
void push_menu_popup_canvas(int16_t panel_x, int16_t panel_y, int16_t w, int16_t h) {
  push_is_menu = true;
  int16_t canvas_row_bytes = (MENU_POPUP_CANVAS_W+7)/8;
  drawBitmap(panel_x, panel_y, menu_popup_canvas.getBuffer(), w, h, SSD1306_WHITE, SSD1306_BLACK, canvas_row_bytes);
  push_is_menu = false;
}
#endif

extern uint8_t wifi_mode;
extern bool wifi_is_connected();
extern bool wifi_host_is_connected();
#if HAS_ETHERNET
extern bool eth_link_up;
extern uint16_t eth_link_speed;
extern bool eth_full_duplex;
extern bool eth_disabled;
#endif
#if HAS_ESPNOW == true
// espnow_wifi_disabled() (ESPNOW.h) isn't declared yet at this point -
// Display.h is #include'd (Utilities.h) before ESPNOW.h - same reasoning
// as the wifi_mode extern above. wifi_mode itself still reads STA/AP while
// this is true (the EEPROM byte is untouched, only wifi_remote_init() was
// skipped at boot - see that function's own comment), so draw_cable_icon()
// below needs this separately, not just wifi_mode alone, or the icon would
// keep showing WiFi even though no WiFi connection actually ever came up.
extern bool espnow_wifi_disabled();
#define ESPNOW_WIFI_DISABLED() espnow_wifi_disabled()
#else
#define ESPNOW_WIFI_DISABLED() false
#endif
void draw_cable_icon(int px, int py, Adafruit_GFX &gfx = stat_area) {
  #if HAS_WIFI
    if (wifi_mode == WR_WIFI_OFF || ESPNOW_WIFI_DISABLED()) {
      if      (rns_link_state == RNS_LINK_STATE_DISCONNECTED) { gfx.drawBitmap(px, py, bm_cable+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
      else if (rns_link_state == RNS_LINK_STATE_CONNECTED)    { gfx.drawBitmap(px, py, bm_cable+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
    } else {
      if (wifi_mode == WR_WIFI_STA) {
        if (wifi_is_connected()) {
          gfx.drawBitmap(px, py, bm_wifi+3*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
          if (!wifi_host_is_connected()) { gfx.fillRect(px+5, py+12, 6, 3, SSD1306_BLACK); }
        } else { gfx.drawBitmap(px, py, bm_wifi+2*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }

      } else if (wifi_mode == WR_WIFI_AP) {
        if (wifi_host_is_connected()) { gfx.drawBitmap(px, py, bm_wifi+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
        else                          { gfx.drawBitmap(px, py, bm_wifi+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }

      } else {
        if      (rns_link_state == RNS_LINK_STATE_DISCONNECTED) { gfx.drawBitmap(px, py, bm_cable+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
        else if (rns_link_state == RNS_LINK_STATE_CONNECTED)    { gfx.drawBitmap(px, py, bm_cable+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
      }
    }

  #else
  if      (rns_link_state == RNS_LINK_STATE_DISCONNECTED) { gfx.drawBitmap(px, py, bm_cable+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
  else if (rns_link_state == RNS_LINK_STATE_CONNECTED)    { gfx.drawBitmap(px, py, bm_cable+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
  #endif
}

void draw_bt_icon(int px, int py, Adafruit_GFX &gfx = stat_area) {
  uint8_t bt_i = 0;
  if      (bt_state == BT_STATE_ON)        { bt_i = 1; }
  else if (bt_state == BT_STATE_PAIRING)   { bt_i = 2; }
  else if (bt_state == BT_STATE_CONNECTED) { bt_i = 3; }
  #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114
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
      if (bt_enabled_lit) { gfx.fillRect(px, py, 16, 16, SSD1306_WHITE); }
      else                { gfx.drawBitmap(px, py, bm_bt+bt_i*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
    #else
      if (bt_enabled_lit) { gfx.drawBitmap(px, py, bm_bt+bt_i*32, 16, 16, SSD1306_BLACK, SSD1306_WHITE); }
      else                { gfx.drawBitmap(px, py, bm_bt+bt_i*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
    #endif
  #else
    gfx.drawBitmap(px, py, bm_bt+bt_i*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  #endif
}

#if HAS_URNS == true
// messenger_has_unread() (Messenger.h) isn't declared yet at this point -
// Display.h is #include'd (Utilities.h) before Messenger.h - same
// reasoning as the wifi_mode/eth_link_up/espnow_ui_active externs above.
extern bool messenger_has_unread();
#define MSNGR_ENVELOPE_BLINK_MS 500
#endif

void draw_lora_icon(int px, int py, Adafruit_GFX &gfx = stat_area) {
  #if HAS_URNS == true
    if (messenger_has_unread()) {
      static bool envelope_frame = false;
      static unsigned long envelope_last_ms = 0;
      unsigned long now = millis();
      if (now - envelope_last_ms >= MSNGR_ENVELOPE_BLINK_MS) {
        envelope_last_ms = now;
        envelope_frame = !envelope_frame;
      }
      gfx.drawBitmap(px, py, bm_envelope+(envelope_frame ? 1 : 0)*34, 16, 17, SSD1306_WHITE, SSD1306_BLACK);
      return;
    }
  #endif
  if (radio_online) {
    gfx.drawBitmap(px, py, bm_rf+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  } else {
    gfx.drawBitmap(px, py, bm_rf+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  }
}

void draw_mw_icon(int px, int py, Adafruit_GFX &gfx = stat_area) {
  if (mw_radio_online) {
    gfx.drawBitmap(px, py, bm_rf+3*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  } else {
    gfx.drawBitmap(px, py, bm_rf+2*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  }
}

#if HAS_GPS == true
// Plain on/off indicator for gnss_enabled (GNSS.h) - not a fix/data
// readout (T114's own alternating GNSS info page, draw_disp_area()'s
// BOARD_HELTEC_T114 branch, already covers that in detail) - just whether
// the receiver is currently switched on, same semantic level as
// draw_lora_icon()/draw_mw_icon() above (radio_online/mw_radio_online -
// "is this subsystem active", not link/fix detail). Text label instead of
// new bitmap art, same shortcut draw_espnow_icon() below already takes.
void draw_gps_icon(int px, int py, Adafruit_GFX &gfx = stat_area) {
  bool active = gnss_enabled;
  // 17px-tall fill (not 16), same as draw_espnow_icon()/draw_eth_icon()
  // below - bm_frame's border lines sit one row further apart than the
  // 16px icon grid, so a 16px fill leaves the bottom interior row
  // showing through as an unfilled dark line.
  gfx.fillRect(px, py, 16, 17, active ? SSD1306_WHITE : SSD1306_BLACK);
  gfx.setFont(&Picopixel);
  gfx.setTextSize(1);
  gfx.setTextWrap(false);
  gfx.setTextColor(active ? SSD1306_BLACK : SSD1306_WHITE);

  const char *buf = "GPS";
  int16_t tx1, ty1; uint16_t tw, th;
  gfx.getTextBounds(buf, 0, 0, &tx1, &ty1, &tw, &th);
  gfx.setCursor(px + (16 - (int16_t)tw) / 2, py + 11);
  gfx.print(buf);
}
#endif

#if HAS_ESPNOW == true
// espnow_ui_active()/espnow_display_rx/espnow_display_tx (ESPNOW.h) aren't
// declared yet at this point - Display.h is #include'd (Utilities.h) before
// ESPNOW.h - same reasoning as the wifi_mode/eth_link_up externs above.
// Needed regardless of HAS_ETHERNET (draw_waterfall() below uses these on
// every HAS_ESPNOW board, including MeshPoE-S3) - only draw_espnow_icon()
// itself is Ethernet-slot-conditional.
extern bool espnow_ui_active();
extern bool espnow_display_rx;
extern bool espnow_display_tx;
#define ESPNOW_UI_ACTIVE() espnow_ui_active()
#else
// Boards without ESP-NOW at all (e.g. Heltec T096) never compile the extern
// above, so waterfall/LED call sites that OR against it need a stand-in
// that's always false rather than #if-ing out the whole condition twice.
#define ESPNOW_UI_ACTIVE() false
#endif

#if HAS_ESPNOW == true && HAS_ETHERNET == false
// Replaces draw_mw_icon()'s call site (this box position) on boards that
// have ESP-NOW but no wired Ethernet - mw_radio_online is declared but
// never actually set anywhere in the codebase, so that icon always
// rendered its permanently-off frame. Boards with HAS_ETHERNET keep using
// draw_eth_icon() in this same slot instead (untouched by this).
void draw_espnow_icon(int px, int py) {
  bool active = espnow_ui_active();
  // The box interior is actually 17px tall border-to-border (bm_frame's
  // top/bottom border lines sit one row further apart than the 16px icon
  // grid - see draw_eth_icon() above), so a 16px-tall fill leaves the
  // bottom interior row showing through as an unfilled dark line.
  stat_area.fillRect(px, py, 16, 17, active ? SSD1306_WHITE : SSD1306_BLACK);
  stat_area.setFont(&Picopixel);
  stat_area.setTextSize(1);
  stat_area.setTextWrap(false);
  stat_area.setTextColor(active ? SSD1306_BLACK : SSD1306_WHITE);

  const char *top_buf = "ESP";
  int16_t tx1, ty1; uint16_t tw, th;
  stat_area.getTextBounds(top_buf, 0, 0, &tx1, &ty1, &tw, &th);
  stat_area.setCursor(px + (16 - (int16_t)tw) / 2, py + 7);
  stat_area.print(top_buf);

  const char *bot_buf = "NOW";
  int16_t bx1, by1; uint16_t bw, bh;
  stat_area.getTextBounds(bot_buf, 0, 0, &bx1, &by1, &bw, &bh);
  stat_area.setCursor(px + (16 - (int16_t)bw) / 2, py + 14);
  stat_area.print(bot_buf);
}
#endif

#if HAS_ETHERNET
void draw_eth_icon(int px, int py) {
  // The box interior is actually 17px tall border-to-border (bm_frame's
  // top/bottom border lines sit one row further apart than the 16px icon
  // grid), so a full-height fill needs 17 here or it leaves the bottom
  // interior row showing through as an unfilled dark line
  if (eth_disabled) {
    // Same shifted-up layout as the linked-with-speed case below (room for
    // a second line), but the black/inactive coloring the plain-down case
    // uses - "off" (never even tried to link, see ETH_SPEED_OFF/
    // init_ethernet(), Ethernet.h) reads differently from "down" (tried,
    // no link yet), which stays centered with no caption at all, below.
    stat_area.fillRect(px, py, 16, 17, SSD1306_BLACK);
    stat_area.drawBitmap(px+2, py+2, bm_eth_txt, 11, 5, SSD1306_WHITE, SSD1306_BLACK);

    const char *off_buf = "OFF";
    stat_area.setFont(&Picopixel);
    stat_area.setTextSize(1);
    stat_area.setTextWrap(false);
    stat_area.setTextColor(SSD1306_WHITE);
    int16_t ox1, oy1; uint16_t ow, oh;
    stat_area.getTextBounds(off_buf, 0, 0, &ox1, &oy1, &ow, &oh);
    stat_area.setCursor(px + (16 - (int16_t)ow) / 2, py + 12);
    stat_area.print(off_buf);
  } else if (eth_link_up) {
    stat_area.fillRect(px, py, 16, 17, SSD1306_WHITE);
    // Shifted up from the old vertically-centered py+6 to leave room below
    // for the negotiated speed/duplex (PicoPixel, 6px-tall font - see
    // Fonts/PicoPixel.h) - eth_link_speed/eth_full_duplex (Ethernet.h) are
    // a snapshot taken on link-up, not queried live here (can't call
    // ETH.linkSpeed()/fullDuplex() directly from this file - Display.h is
    // #include'd before Ethernet.h, same reason eth_link_up above is an
    // extern rather than a live ETH.linkUp() call).
    stat_area.drawBitmap(px+2, py+2, bm_eth_txt, 11, 5, SSD1306_BLACK, SSD1306_WHITE);

    // "100F"/"100H"/"10F"/"10H" - at PicoPixel's per-glyph advance widths
    // (see Fonts/PicoPixel.h) "100F"/"100H" measure 14px, "10F"/"10H" 10px,
    // both fitting inside the 16px-wide box with room to spare - measured
    // and centered rather than hardcoded so it stays correct if the font
    // ever changes.
    char speed_buf[5];
    sprintf(speed_buf, "%u%s", eth_link_speed, eth_full_duplex ? "F" : "H");
    stat_area.setFont(&Picopixel);
    stat_area.setTextSize(1);
    stat_area.setTextWrap(false);
    stat_area.setTextColor(SSD1306_BLACK);
    int16_t sx1, sy1; uint16_t sw, sh;
    stat_area.getTextBounds(speed_buf, 0, 0, &sx1, &sy1, &sw, &sh);
    stat_area.setCursor(px + (16 - (int16_t)sw) / 2, py + 12);
    stat_area.print(speed_buf);
  } else {
    // Back to its original vertically-centered spot - only shifts up (see
    // above) when there's a speed/duplex line to make room for.
    stat_area.fillRect(px, py, 16, 17, SSD1306_BLACK);
    stat_area.drawBitmap(px+2, py+6, bm_eth_txt, 11, 5, SSD1306_WHITE, SSD1306_BLACK);
  }
}
#endif

uint8_t charge_tick = 0;
void draw_battery_bars(int px, int py, Adafruit_GFX &gfx = stat_area) {
  #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114
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
          gfx.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
          gfx.drawBitmap(px-2, py-2, bm_plug, 17, 7, SSD1306_WHITE, SSD1306_BLACK);
        } else {
          if (battery_state == BATTERY_STATE_CHARGED) {
            gfx.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
            gfx.drawBitmap(px-2, py-2, bm_plug, 17, 7, SSD1306_WHITE, SSD1306_BLACK);
          } else {
            // gfx.fillRect(px, py, 14, 3, SSD1306_BLACK);
            #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114
              // 2 sticks or fewer render the icon red
              battery_low_lit = battery_value <= 33;
            #endif
            gfx.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
            gfx.drawRect(px-2, py-2, 17, 7, SSD1306_WHITE);
            gfx.drawLine(px+15, py, px+15, py+3, SSD1306_WHITE);
            if (battery_value > 7) gfx.drawLine(px, py, px, py+2, SSD1306_WHITE);
            if (battery_value > 20) gfx.drawLine(px+1*2, py, px+1*2, py+2, SSD1306_WHITE);
            if (battery_value > 33) gfx.drawLine(px+2*2, py, px+2*2, py+2, SSD1306_WHITE);
            if (battery_value > 46) gfx.drawLine(px+3*2, py, px+3*2, py+2, SSD1306_WHITE);
            if (battery_value > 59) gfx.drawLine(px+4*2, py, px+4*2, py+2, SSD1306_WHITE);
            if (battery_value > 72) gfx.drawLine(px+5*2, py, px+5*2, py+2, SSD1306_WHITE);
            if (battery_value > 85) gfx.drawLine(px+6*2, py, px+6*2, py+2, SSD1306_WHITE);
          }
        }
      } else {
        gfx.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
        gfx.drawBitmap(px-2, py-2, bm_plug, 17, 7, SSD1306_WHITE, SSD1306_BLACK);
      }
    }
  } else {
    gfx.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
    gfx.drawBitmap(px-2, py-2, bm_plug, 17, 7, SSD1306_WHITE, SSD1306_BLACK);
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

void draw_waterfall(int px, int py, Adafruit_GFX &gfx = stat_area) {
  bool pushed = false;
  #if HAS_ESPNOW == true
    // ESP-NOW has no equivalent of LoRa's continuous ambient current_rssi
    // sampling, so rather than a per-refresh RSSI trace, push a 2-row mark
    // per RX/TX event (espnow_display_rx/tx, ESPNOW.h) and a blank row
    // otherwise. Only takes over the push once ESP-NOW is actually the
    // active "radio" (espnow_ui_active(), not just enabled/ready) and LoRa
    // isn't - both call sites already gate the call itself on
    // (radio_online || espnow_ui_active()), so this only has to tell the
    // two cases apart.
    if (!radio_online && espnow_ui_active()) {
      pushed = true;
      if (espnow_display_tx) {
        for (uint8_t i = 0; i < 2; i++) {
          waterfall_meta[waterfall_head] = WF_M_TX;
          waterfall[waterfall_head++] = -1;
          if (waterfall_head >= WATERFALL_SIZE) waterfall_head = 0;
        }
        espnow_display_tx = false;
      } else if (espnow_display_rx) {
        for (uint8_t i = 0; i < 2; i++) {
          waterfall_meta[waterfall_head] = WF_M_RX_PKT;
          waterfall[waterfall_head++] = WF_PIXEL_WIDTH;
          if (waterfall_head >= WATERFALL_SIZE) waterfall_head = 0;
        }
        espnow_display_rx = false;
      } else {
        waterfall_meta[waterfall_head] = WF_M_RX;
        waterfall[waterfall_head++] = 0;
        if (waterfall_head >= WATERFALL_SIZE) waterfall_head = 0;
      }
    }
  #endif
  if (!pushed) {
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
  }

  gfx.fillRect(px,py,WF_PIXEL_WIDTH, WATERFALL_SIZE, SSD1306_BLACK);
  for (int i = 0; i < WATERFALL_SIZE; i++){
    int wi = (waterfall_head+i)%WATERFALL_SIZE;
    int ws = waterfall[wi];
    int wm = waterfall_meta[wi];
    if (ws > 0) {
      if      (wm == WF_M_RX || wm == WF_M_RX_PKT) { gfx.drawLine(px, py+i, px+ws-1, py+i, SSD1306_WHITE); }
      else if (wm == WF_M_NTFR) {
        uint8_t o = 0;
        for (uint8_t ti = 0; ti < WF_PIXEL_WIDTH/2; ti++) { gfx.drawPixel(px+ti*2+o, py+i, SSD1306_WHITE); }
      }
    } else if (ws == -1) {
      #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114
        // Anchor the checker phase to the entry, not the screen row, so
        // the pattern scrolls with the content instead of inverting in
        // place on every frame
        uint8_t o = wi%2;
      #else
        uint8_t o = i%2;
      #endif
      for (uint8_t ti = 0; ti < WF_PIXEL_WIDTH/2; ti++) {
        gfx.drawPixel(px+ti*2+o, py+i, SSD1306_WHITE);
      }
    }
  }

  #if (BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114) && USE_COLOR_DISPLAY == true
    // Row colours are looked up from waterfall_meta at push time, but rows
    // whose mono content matches what the panel already shows are skipped
    // by the diff and would keep the colour of the entry displayed there
    // before the scroll. Re-push the whole waterfall rect every scroll.
    stat_mark_dirty(px, py, WF_PIXEL_WIDTH, WATERFALL_SIZE);
  #endif
}

#if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114
// Battery voltage readout in the 19px gap between the battery bars and
// the quality graph; "d.dV" in Org_01 is exactly 19px wide. Refreshed at
// most every 5s so the jittering last decimal doesn't expand the display
// update region on every frame.
#define BAT_V_REFRESH_INTERVAL 5000
void draw_battery_voltage(int px, int py, Adafruit_GFX &gfx = stat_area) {
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
    gfx.fillRect(px, py-6, 19, 8, SSD1306_BLACK);
    gfx.setFont(SMALL_FONT); gfx.setTextWrap(false);
    gfx.setTextColor(SSD1306_WHITE); gfx.setTextSize(1);
    gfx.setCursor(px, py);
    gfx.printf("%.1fV", battery_voltage);
    last_drawn = millis();
  }
}
#endif

#if BOARD_MODEL == BOARD_HELTEC_T114
// CPU temperature readout to the right of the battery voltage box. The
// nRF52840's TEMP peripheral (pmu_temp_sensor_ready, Power.h) is fixed
// silicon, not gated behind pmu_ready/battery_ready like the voltage
// reading next to it, so this shows even without a battery installed.
#define CPU_TEMP_REFRESH_INTERVAL 5000
extern bool pmu_temp_sensor_ready;
extern float pmu_temperature;
void draw_cpu_temperature(int px, int py, Adafruit_GFX &gfx = stat_area) {
  static uint32_t last_drawn = 0;
  if (last_drawn != 0 && millis()-last_drawn < CPU_TEMP_REFRESH_INTERVAL) return;
  // measure_temperature() (Power.h) leaves pmu_temperature at -31 (its
  // PMU_TEMP_MIN-1 sentinel - Power.h isn't included yet at this point in
  // the file, hence the literal rather than the macro) until the first
  // periodic reading actually lands - don't show a bogus "-31C" in that
  // brief startup window.
  if (pmu_temp_sensor_ready && pmu_temperature > -31) {
    gfx.fillRect(px, py-6, 19, 8, SSD1306_BLACK);
    gfx.setFont(SMALL_FONT); gfx.setTextWrap(false);
    gfx.setTextColor(SSD1306_WHITE); gfx.setTextSize(1);
    gfx.setCursor(px, py);
    gfx.printf("%.0fC", pmu_temperature);
    last_drawn = millis();
  }
}

// Node uptime - millis() itself is elapsed time since boot, so no separate
// boot-time variable is needed; wraps back to 0 after ~49 days like every
// other millis()-based timer in this codebase already does.
#define NODE_UPTIME_REFRESH_INTERVAL 1000
void draw_node_uptime(int px, int py, Adafruit_GFX &gfx = stat_area) {
  static uint32_t last_drawn = 0;
  if (last_drawn != 0 && millis()-last_drawn < NODE_UPTIME_REFRESH_INTERVAL) return;
  uint32_t s = millis()/1000;
  gfx.fillRect(px, py-7, 60, 15, SSD1306_BLACK);
  gfx.setFont(SMALL_FONT); gfx.setTextWrap(false);
  gfx.setTextColor(SSD1306_WHITE); gfx.setTextSize(1);
  gfx.setCursor(px, py);
  gfx.print("Node uptime:");
  gfx.setCursor(px, py+7);
  gfx.printf("%02lu:%02lu:%02lu", (unsigned long)(s/3600), (unsigned long)((s/60)%60), (unsigned long)(s%60));
  last_drawn = millis();
}
#endif

#if BOARD_MODEL == BOARD_HELTEC_T114
  // Full-panel RGB565 boot splash (SplashT114.h) - too large/full-color for
  // the 1bpp disp_area/colourizer pipeline every other T114 screen uses, so
  // it bypasses that entirely with its own raw writePixels() push, forcing
  // landscape rotation to match the image's own 240x135 orientation
  // regardless of the user's actual Orientation setting (same idea as the
  // menu forcing its own fixed rotation while open). Stays up for a fixed
  // SPLASH_T114_DURATION_MS regardless of how quickly device init actually
  // finishes.
  #define SPLASH_T114_DURATION_MS 4000
  bool draw_t114_splash() {
    static bool pushed = false;
    static uint32_t start = 0;
    static bool rotation_forced = false;
    if (!pushed) { start = millis(); pushed = true; }

    if (millis() - start >= SPLASH_T114_DURATION_MS) {
      if (rotation_forced) {
        display.setRotation(active_display_rotation);
        // Blank the whole panel before the operational screen's own first
        // push - disp_area/stat_area(_land) don't necessarily cover every
        // physical pixel (landscape has an uncovered gap around disp_area's
        // own column, same issue the menu-close transition has - see its
        // own comment), so without this, splash pixels outside whatever
        // gets redrawn this cycle just stay on screen. display.width()/
        // height() reflect whichever rotation was just restored, so this
        // covers either orientation with one call.
        fillRect(0, 0, display.width(), display.height(), SSD1306_BLACK);
        // Same reasoning as the menu open/close transition (Display.h,
        // update_display()) - a different push path (raw writePixels here,
        // not stat_area/disp_area's own tracked pushes) just wrote over the
        // same panel space, so a stale cache entry could wrongly compare
        // equal to content pushed after it and get skipped.
        for (uint8_t i = 0; i < REGION_CACHE_SLOTS; i++) region_cache[i].x = -1;
        #if USE_COLOR_DISPLAY == true
          cdirty_count = 0;
        #endif
        rotation_forced = false;
      }
      return false;
    }

    if (!rotation_forced) {
      // Rotation 1 assumed right-side-up for landscape - flip to 3 if the
      // image comes up flipped/mirrored on real hardware.
      display.setRotation(1);
      rotation_forced = true;
      display.startWrite();
      display.setAddrWindow(0, 0, SPLASH_T114_W, SPLASH_T114_H);
      // nRF52's SPIM peripheral drives writePixels() via EasyDMA, which can
      // only read from RAM, not flash - splash_t114 (PROGMEM) has to be
      // copied into a RAM chunk first or the transfer silently reads
      // garbage/zeroes (a black screen, exactly what showed up on
      // hardware). 16 rows/chunk keeps the scratch buffer small (~7.5KB)
      // while still batching most of the image into a handful of bursts.
      #define SPLASH_T114_CHUNK_ROWS 16
      static uint16_t splash_chunk[SPLASH_T114_W*SPLASH_T114_CHUNK_ROWS];
      for (uint16_t row = 0; row < SPLASH_T114_H; row += SPLASH_T114_CHUNK_ROWS) {
        uint16_t rows_this_chunk = SPLASH_T114_CHUNK_ROWS;
        if (row+rows_this_chunk > SPLASH_T114_H) rows_this_chunk = SPLASH_T114_H-row;
        uint32_t px_count = (uint32_t)SPLASH_T114_W*rows_this_chunk;
        memcpy(splash_chunk, splash_t114+(uint32_t)row*SPLASH_T114_W, px_count*sizeof(uint16_t));
        display.writePixels(splash_chunk, px_count, true, true);
      }
      display.endWrite();
    }
    return true;
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
      if (radio_online || ESPNOW_UI_ACTIVE()) {
        draw_waterfall(WF_POS_X, wf_y);
      }
    #elif BOARD_MODEL == BOARD_HELTEC_T114
      // Six explicitly-drawn icon/lamp boxes (cable/bt on the left, lora/
      // 2.4G/RX/TX on the right) plus lamps, quality/signal, RSSI/SNR
      // readout and battery laid out as one continuous left-hand column
      // (st_box_y0/y1/st_qs_y/st_box_y2/st_info_y0/y1, see their own
      // comment) so it reads as a single panel instead of disconnected
      // floating pieces, with the waterfall as a distinct column to the
      // right (x64+, see WF_POS_X) spanning nearly the canvas's full
      // height.
      if (disp_mode == DISP_MODE_LANDSCAPE) {
        // Landscape's own canvas (stat_area_land, sized to whatever's left
        // of the 240px landscape width after disp_area's own 135 - see
        // STAT_AREA_LAND_W's own comment) - reuses portrait's exact local Y
        // positions/icon-box layout as-is (none of it ever needed the full
        // 135px width anyway), just a narrower waterfall (LWF_BORDER_W) to
        // fit the narrower canvas. disp_area itself needs no landscape
        // variant at all - it's reused unchanged, see update_area_
        // positions()'s landscape branch.
        static bool stat_area_land_intialised = false;
        if (!stat_area_land_intialised) {
          int16_t rx_box_y = st_box_y2-2;
          stat_area_land.drawRect(0,  st_box_y0-1, 18, 19, SSD1306_WHITE);  // cable
          stat_area_land.drawRect(0,  st_box_y1-1, 18, 19, SSD1306_WHITE);  // bt
          stat_area_land.drawRect(20, st_box_y0-1, 18, 19, SSD1306_WHITE);  // lora
          stat_area_land.drawRect(20, st_box_y1-1, 18, 19, SSD1306_WHITE);  // 2.4G
          stat_area_land.drawRect(0,  rx_box_y,    18, 19, SSD1306_WHITE);  // RX
          stat_area_land.drawRect(20, rx_box_y,    18, 19, SSD1306_WHITE);  // TX

          stat_area_land.drawRect(40, st_box_y0-1, 18, 19, SSD1306_WHITE);
          stat_area_land.drawRect(40, st_box_y1-1, 18, 19, SSD1306_WHITE);
          stat_area_land.drawRect(40, rx_box_y,    18, 19, SSD1306_WHITE);

          stat_area_land.fillRect(WF_BORDER_X, WF_LEGEND_Y, LWF_BORDER_W, WF_LEGEND_H, SSD1306_WHITE);
          stat_area_land.drawRect(WF_BORDER_X, WF_BORDER_Y, LWF_BORDER_W, WF_BORDER_H, SSD1306_WHITE);
          stat_area_land.setFont(SMALL_FONT); stat_area_land.setTextWrap(false);
          stat_area_land.setTextColor(SSD1306_BLACK); stat_area_land.setTextSize(1);
          stat_area_land.setCursor(WF_BORDER_X+2, 9);
          stat_area_land.printf("%d", WF_RSSI_MIN);
          char wf_max_buf[8];
          sprintf(wf_max_buf, "%d", WF_RSSI_MAX);
          int16_t mx1, my1; uint16_t mw, mh;
          stat_area_land.getTextBounds(wf_max_buf, 0, 0, &mx1, &my1, &mw, &mh);
          stat_area_land.setCursor(WF_BORDER_X+LWF_BORDER_W-2-(int16_t)mw, 9);
          stat_area_land.print(wf_max_buf);

          stat_area_land_intialised = true;
        }

        if (radio_online && dcd_led) { lamp_rx_until = millis()+LAMP_HOLD_MS; }
        if (display_tx) { lamp_tx_until = millis()+LAMP_HOLD_MS; }
        lamp_rx_lit = millis() < lamp_rx_until;
        lamp_tx_lit = millis() < lamp_tx_until;

        if (lamp_rx_lit) { stat_area_land.drawBitmap(1, st_box_y2, bm_lamp_rx, 16, 16, SSD1306_BLACK, SSD1306_WHITE); }
        else             { stat_area_land.drawBitmap(1, st_box_y2, bm_lamp_rx, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
        if (lamp_tx_lit) { stat_area_land.drawBitmap(21, st_box_y2, bm_lamp_tx, 16, 16, SSD1306_BLACK, SSD1306_WHITE); }
        else             { stat_area_land.drawBitmap(21, st_box_y2, bm_lamp_tx, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }

        draw_cable_icon(1, st_box_y0, stat_area_land);
        draw_bt_icon(1, st_box_y1, stat_area_land);
        draw_lora_icon(21, st_box_y0, stat_area_land);
        #if HAS_GPS == true
          draw_gps_icon(41, st_box_y0, stat_area_land);
        #endif
        #if BOARD_MODEL == BOARD_MESHPOE_S3
          draw_eth_icon(21, st_box_y1, stat_area_land);
        #elif HAS_ESPNOW == true
          draw_espnow_icon(21, st_box_y1, stat_area_land);
        #else
          draw_mw_icon(21, st_box_y1, stat_area_land);
        #endif
        draw_node_uptime(1, 113, stat_area_land);

        if (radio_online) {
          stat_area_land.setFont(SMALL_FONT); stat_area_land.setTextWrap(false); stat_area_land.setTextSize(1);
          stat_area_land.setTextColor(SSD1306_WHITE);
          stat_area_land.setCursor(1, RF_RSSI_BAR_Y+7);
          stat_area_land.print("RSSI");
          stat_area_land.setCursor(3, RF_SNR_BAR_Y+7);
          stat_area_land.print("SNR");
          stat_area_land.drawRect(RF_BAR_X, RF_RSSI_BAR_Y, RF_BAR_W, RF_BAR_H, SSD1306_WHITE);
          stat_area_land.drawRect(RF_BAR_X, RF_SNR_BAR_Y, RF_BAR_W, RF_BAR_H, SSD1306_WHITE);
          stat_area_land.fillRect(RF_BAR_X+1, RF_RSSI_BAR_Y+1, RF_BAR_W-2, RF_BAR_H-2, SSD1306_BLACK);
          stat_area_land.fillRect(RF_BAR_X+1, RF_SNR_BAR_Y+1, RF_BAR_W-2, RF_BAR_H-2, SSD1306_BLACK);
          rf_rssi_mask_x = RF_BAR_X+1; rf_rssi_mask_y = RF_RSSI_BAR_Y+1;
          rf_snr_mask_x = RF_BAR_X+1;  rf_snr_mask_y = RF_SNR_BAR_Y+1;
          rf_rssi_mask.fillScreen(0);
          rf_snr_mask.fillScreen(0);

          if (last_rssi != -292) {
            float pct = ((float)last_rssi - RF_RSSI_MIN) / (float)(RF_RSSI_MAX-RF_RSSI_MIN) * 100.0;
            if (pct < 0) pct = 0; if (pct > 100) pct = 100;
            int16_t fill_w = (int16_t)((RF_BAR_W-2) * pct / 100.0);
            if (fill_w > 0) stat_area_land.fillRect(RF_BAR_X+1, RF_RSSI_BAR_Y+1, fill_w, RF_BAR_H-2, SSD1306_WHITE);
            char buf[10]; sprintf(buf, "%d", (int)last_rssi);
            int16_t bx1, by1; uint16_t bw, bh;
            stat_area_land.getTextBounds(buf, 0, 0, &bx1, &by1, &bw, &bh);
            int16_t tx = RF_BAR_X + (RF_BAR_W-(int16_t)bw)/2;
            int16_t ty = RF_RSSI_BAR_Y + RF_BAR_H - 4;
            stat_area_land.setTextColor(SSD1306_BLACK);
            stat_area_land.setCursor(tx-bx1-1, ty);   stat_area_land.print(buf);
            stat_area_land.setCursor(tx-bx1+1, ty);   stat_area_land.print(buf);
            stat_area_land.setCursor(tx-bx1,   ty-1); stat_area_land.print(buf);
            stat_area_land.setCursor(tx-bx1,   ty+1); stat_area_land.print(buf);
            stat_area_land.setTextColor(SSD1306_WHITE);
            stat_area_land.setCursor(tx-bx1, ty);
            stat_area_land.print(buf);
            rf_rssi_mask.setFont(SMALL_FONT); rf_rssi_mask.setTextWrap(false); rf_rssi_mask.setTextSize(1);
            rf_rssi_mask.setTextColor(1);
            rf_rssi_mask.setCursor(tx-bx1-rf_rssi_mask_x, ty-rf_rssi_mask_y);
            rf_rssi_mask.print(buf);
          }

          if (last_rssi != -292) { // SNR uses the same "has a real packet" gate as RSSI
            float snr = (float)((signed char)last_snr_raw)*0.25;
            float pct = (snr - RF_SNR_MIN) / (float)(RF_SNR_MAX-RF_SNR_MIN) * 100.0;
            if (pct < 0) pct = 0; if (pct > 100) pct = 100;
            int16_t fill_w = (int16_t)((RF_BAR_W-2) * pct / 100.0);
            if (fill_w > 0) stat_area_land.fillRect(RF_BAR_X+1, RF_SNR_BAR_Y+1, fill_w, RF_BAR_H-2, SSD1306_WHITE);
            char buf[10]; sprintf(buf, "%.1f", snr);
            int16_t bx1, by1; uint16_t bw, bh;
            stat_area_land.getTextBounds(buf, 0, 0, &bx1, &by1, &bw, &bh);
            int16_t tx = RF_BAR_X + (RF_BAR_W-(int16_t)bw)/2;
            int16_t ty = RF_SNR_BAR_Y + RF_BAR_H - 4;
            stat_area_land.setTextColor(SSD1306_BLACK);
            stat_area_land.setCursor(tx-bx1-1, ty);   stat_area_land.print(buf);
            stat_area_land.setCursor(tx-bx1+1, ty);   stat_area_land.print(buf);
            stat_area_land.setCursor(tx-bx1,   ty-1); stat_area_land.print(buf);
            stat_area_land.setCursor(tx-bx1,   ty+1); stat_area_land.print(buf);
            stat_area_land.setTextColor(SSD1306_WHITE);
            stat_area_land.setCursor(tx-bx1, ty);
            stat_area_land.print(buf);
            rf_snr_mask.setFont(SMALL_FONT); rf_snr_mask.setTextWrap(false); rf_snr_mask.setTextSize(1);
            rf_snr_mask.setTextColor(1);
            rf_snr_mask.setCursor(tx-bx1-rf_snr_mask_x, ty-rf_snr_mask_y);
            rf_snr_mask.print(buf);
          }

          stat_area_land.fillRect(0, RF_SNR_BAR_Y+RF_BAR_H+1, 60, 15, SSD1306_BLACK);
          stat_area_land.setFont(SMALL_FONT); stat_area_land.setTextWrap(false); stat_area_land.setTextSize(1);
          stat_area_land.setTextColor(SSD1306_WHITE);
          stat_area_land.setCursor(1, RF_SNR_BAR_Y+RF_BAR_H+7);
          stat_area_land.printf("RXPKT:%04lu", (unsigned long)packet_rx_count);
          stat_area_land.setCursor(1, RF_SNR_BAR_Y+RF_BAR_H+14);
          stat_area_land.printf("TXPKT:%04lu", (unsigned long)packet_tx_count);
        }

        draw_battery_bars(4, 125, stat_area_land);
        static bool battery_low_prev_land = false;
        if (battery_low_lit != battery_low_prev_land) {
          battery_low_prev_land = battery_low_lit;
          stat_mark_dirty(2, 123, 18, 7);
        }
        draw_battery_voltage(22, 128, stat_area_land);
        draw_cpu_temperature(40, 128, stat_area_land);
        if (radio_online || ESPNOW_UI_ACTIVE()) {
          draw_waterfall(WF_POS_X, wf_y, stat_area_land);
        }
      } else {
      if (!stat_area_intialised) {
        // Every box border is drawn explicitly (not bm_frame's baked art)
        // so repositioning any of the six - raising/lowering a row,
        // shifting the right-hand column left - is just a coordinate
        // change here instead of fighting bitmap content that can't move.
        int16_t rx_box_y = st_box_y2-2;
        stat_area.drawRect(0,  st_box_y0-1, 18, 19, SSD1306_WHITE);  // cable
        stat_area.drawRect(0,  st_box_y1-1, 18, 19, SSD1306_WHITE);  // bt
        stat_area.drawRect(20, st_box_y0-1, 18, 19, SSD1306_WHITE);  // lora
        stat_area.drawRect(20, st_box_y1-1, 18, 19, SSD1306_WHITE);  // 2.4G
        stat_area.drawRect(0,  rx_box_y,    18, 19, SSD1306_WHITE);  // RX
        stat_area.drawRect(20, rx_box_y,    18, 19, SSD1306_WHITE);  // TX

        // Three extra boxes, same size/spacing, one to the right of each of
        // lora/2.4G/TX - the first (lora's row) now holds draw_gps_icon()
        // (HAS_GPS boards only); the other two stay reserved, no icon drawn
        // inside yet.
        stat_area.drawRect(40, st_box_y0-1, 18, 19, SSD1306_WHITE);
        stat_area.drawRect(40, st_box_y1-1, 18, 19, SSD1306_WHITE);
        stat_area.drawRect(40, rx_box_y,    18, 19, SSD1306_WHITE);

        // Waterfall border + a small min/max RSSI legend box above it -
        // static, so drawn once here rather than every draw_waterfall()
        // call (which only clears/redraws the content area 2px inside the
        // main border, see WF_POS_X/Y's own comment). The legend gets its
        // own separate boxed-off row (white fill, black text) rather than
        // sharing the main border, with a 1px black gap between the two.
        stat_area.fillRect(WF_BORDER_X, WF_LEGEND_Y, WF_BORDER_W, WF_LEGEND_H, SSD1306_WHITE);
        stat_area.drawRect(WF_BORDER_X, WF_BORDER_Y, WF_BORDER_W, WF_BORDER_H, SSD1306_WHITE);
        stat_area.setFont(SMALL_FONT); stat_area.setTextWrap(false);
        stat_area.setTextColor(SSD1306_BLACK); stat_area.setTextSize(1);
        stat_area.setCursor(WF_BORDER_X+2, 9);
        stat_area.printf("%d", WF_RSSI_MIN);
        char wf_max_buf[8];
        sprintf(wf_max_buf, "%d", WF_RSSI_MAX);
        int16_t mx1, my1; uint16_t mw, mh;
        stat_area.getTextBounds(wf_max_buf, 0, 0, &mx1, &my1, &mw, &mh);
        stat_area.setCursor(WF_BORDER_X+WF_BORDER_W-2-(int16_t)mw, 9);
        stat_area.print(wf_max_buf);

        stat_area_intialised = true;
      }

      if (radio_online && dcd_led) { lamp_rx_until = millis()+LAMP_HOLD_MS; }
      if (display_tx) { lamp_tx_until = millis()+LAMP_HOLD_MS; }
      lamp_rx_lit = millis() < lamp_rx_until;
      lamp_tx_lit = millis() < lamp_tx_until;

      if (lamp_rx_lit) { stat_area.drawBitmap(1, st_box_y2, bm_lamp_rx, 16, 16, SSD1306_BLACK, SSD1306_WHITE); }
      else             { stat_area.drawBitmap(1, st_box_y2, bm_lamp_rx, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
      if (lamp_tx_lit) { stat_area.drawBitmap(21, st_box_y2, bm_lamp_tx, 16, 16, SSD1306_BLACK, SSD1306_WHITE); }
      else             { stat_area.drawBitmap(21, st_box_y2, bm_lamp_tx, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }

      draw_cable_icon(1, st_box_y0);
      draw_bt_icon(1, st_box_y1);
      draw_lora_icon(21, st_box_y0);
      #if HAS_GPS == true
        draw_gps_icon(41, st_box_y0);
      #endif
      #if BOARD_MODEL == BOARD_MESHPOE_S3
        draw_eth_icon(21, st_box_y1);
      #elif HAS_ESPNOW == true
        draw_espnow_icon(21, st_box_y1);
      #else
        draw_mw_icon(21, st_box_y1);
      #endif
      draw_node_uptime(1, 113);

      // RSSI/SNR gauges - red-to-green filled bar with a 1px border and
      // the value centered inside in white (see rf_gradient_color()'s own
      // comment for the LoRaMon/rns-wardrive-tools source this matches).
      // Bars stay empty (border only, no fill/text) until a real packet
      // updates last_rssi/last_snr_raw - see their never-received-a-
      // packet sentinel defaults in Config.h.
      if (radio_online) {
        stat_area.setFont(SMALL_FONT); stat_area.setTextWrap(false); stat_area.setTextSize(1);
        stat_area.setTextColor(SSD1306_WHITE);
        stat_area.setCursor(1, RF_RSSI_BAR_Y+7);
        stat_area.print("RSSI");
        stat_area.setCursor(3, RF_SNR_BAR_Y+7);
        stat_area.print("SNR");
        stat_area.drawRect(RF_BAR_X, RF_RSSI_BAR_Y, RF_BAR_W, RF_BAR_H, SSD1306_WHITE);
        stat_area.drawRect(RF_BAR_X, RF_SNR_BAR_Y, RF_BAR_W, RF_BAR_H, SSD1306_WHITE);
        stat_area.fillRect(RF_BAR_X+1, RF_RSSI_BAR_Y+1, RF_BAR_W-2, RF_BAR_H-2, SSD1306_BLACK);
        stat_area.fillRect(RF_BAR_X+1, RF_SNR_BAR_Y+1, RF_BAR_W-2, RF_BAR_H-2, SSD1306_BLACK);
        // Mask's local (0,0) is pinned to each bar's own interior top-left
        // corner, so mask-space coordinates are just stat_area coordinates
        // offset by a fixed, known amount - no per-frame bookkeeping needed
        // beyond the fillScreen()/print() below.
        rf_rssi_mask_x = RF_BAR_X+1; rf_rssi_mask_y = RF_RSSI_BAR_Y+1;
        rf_snr_mask_x = RF_BAR_X+1;  rf_snr_mask_y = RF_SNR_BAR_Y+1;
        rf_rssi_mask.fillScreen(0);
        rf_snr_mask.fillScreen(0);

        if (last_rssi != -292) {
          float pct = ((float)last_rssi - RF_RSSI_MIN) / (float)(RF_RSSI_MAX-RF_RSSI_MIN) * 100.0;
          if (pct < 0) pct = 0; if (pct > 100) pct = 100;
          int16_t fill_w = (int16_t)((RF_BAR_W-2) * pct / 100.0);
          if (fill_w > 0) stat_area.fillRect(RF_BAR_X+1, RF_RSSI_BAR_Y+1, fill_w, RF_BAR_H-2, SSD1306_WHITE);
          // No unit suffix (dBm) here - the bar is only 36px of interior
          // width and text that wide leaves no room for any gradient to
          // actually show alongside it; the "RSSI" label already says
          // what's being measured.
          char buf[10]; sprintf(buf, "%d", (int)last_rssi);
          int16_t bx1, by1; uint16_t bw, bh;
          stat_area.getTextBounds(buf, 0, 0, &bx1, &by1, &bw, &bh);
          int16_t tx = RF_BAR_X + (RF_BAR_W-(int16_t)bw)/2;
          int16_t ty = RF_RSSI_BAR_Y + RF_BAR_H - 4;
          // A 1px-per-side black outline, same idea as the reference
          // design's CSS text-shadow, drawn straight into stat_area for
          // real black/gradient contrast. Separately, the glyph's own
          // white ink is also stamped into rf_rssi_mask at the matching
          // local offset - the colourizer bit-tests that mask directly to
          // know exactly which lit pixels are "text" vs "fill", instead of
          // guessing from a coordinate box (which forced the whole
          // bounding rectangle white, blanking the gradient in the gaps
          // between glyphs too).
          stat_area.setTextColor(SSD1306_BLACK);
          stat_area.setCursor(tx-bx1-1, ty);   stat_area.print(buf);
          stat_area.setCursor(tx-bx1+1, ty);   stat_area.print(buf);
          stat_area.setCursor(tx-bx1,   ty-1); stat_area.print(buf);
          stat_area.setCursor(tx-bx1,   ty+1); stat_area.print(buf);
          stat_area.setTextColor(SSD1306_WHITE);
          stat_area.setCursor(tx-bx1, ty);
          stat_area.print(buf);
          rf_rssi_mask.setFont(SMALL_FONT); rf_rssi_mask.setTextWrap(false); rf_rssi_mask.setTextSize(1);
          rf_rssi_mask.setTextColor(1);
          rf_rssi_mask.setCursor(tx-bx1-rf_rssi_mask_x, ty-rf_rssi_mask_y);
          rf_rssi_mask.print(buf);
        }

        if (last_rssi != -292) { // SNR uses the same "has a real packet" gate as RSSI
          float snr = (float)((signed char)last_snr_raw)*0.25;
          float pct = (snr - RF_SNR_MIN) / (float)(RF_SNR_MAX-RF_SNR_MIN) * 100.0;
          if (pct < 0) pct = 0; if (pct > 100) pct = 100;
          int16_t fill_w = (int16_t)((RF_BAR_W-2) * pct / 100.0);
          if (fill_w > 0) stat_area.fillRect(RF_BAR_X+1, RF_SNR_BAR_Y+1, fill_w, RF_BAR_H-2, SSD1306_WHITE);
          char buf[10]; sprintf(buf, "%.1f", snr);
          int16_t bx1, by1; uint16_t bw, bh;
          stat_area.getTextBounds(buf, 0, 0, &bx1, &by1, &bw, &bh);
          int16_t tx = RF_BAR_X + (RF_BAR_W-(int16_t)bw)/2;
          int16_t ty = RF_SNR_BAR_Y + RF_BAR_H - 4;
          stat_area.setTextColor(SSD1306_BLACK);
          stat_area.setCursor(tx-bx1-1, ty);   stat_area.print(buf);
          stat_area.setCursor(tx-bx1+1, ty);   stat_area.print(buf);
          stat_area.setCursor(tx-bx1,   ty-1); stat_area.print(buf);
          stat_area.setCursor(tx-bx1,   ty+1); stat_area.print(buf);
          stat_area.setTextColor(SSD1306_WHITE);
          stat_area.setCursor(tx-bx1, ty);
          stat_area.print(buf);
          rf_snr_mask.setFont(SMALL_FONT); rf_snr_mask.setTextWrap(false); rf_snr_mask.setTextSize(1);
          rf_snr_mask.setTextColor(1);
          rf_snr_mask.setCursor(tx-bx1-rf_snr_mask_x, ty-rf_snr_mask_y);
          rf_snr_mask.print(buf);
        }

        // Packet counters - packet_rx_count/packet_tx_count (Config.h) are
        // incremented in kiss_write_packet()/transmit() on every board, not
        // just this one; this is just the first board to show them.
        stat_area.fillRect(0, RF_SNR_BAR_Y+RF_BAR_H+1, 60, 15, SSD1306_BLACK);
        stat_area.setFont(SMALL_FONT); stat_area.setTextWrap(false); stat_area.setTextSize(1);
        stat_area.setTextColor(SSD1306_WHITE);
        stat_area.setCursor(1, RF_SNR_BAR_Y+RF_BAR_H+7);
        stat_area.printf("RXPKT:%04lu", (unsigned long)packet_rx_count);
        stat_area.setCursor(1, RF_SNR_BAR_Y+RF_BAR_H+14);
        stat_area.printf("TXPKT:%04lu", (unsigned long)packet_tx_count);
      }

      // Battery indicator anchored to the canvas's bottom-left corner, as
      // low as it can go without its box (7 rows, py-2..py+4) clipping the
      // canvas's bottom edge (max row index STAT_AREA_H-1 = 129).
      draw_battery_bars(4, 125);
      static bool battery_low_prev = false;
      if (battery_low_lit != battery_low_prev) {
        battery_low_prev = battery_low_lit;
        stat_mark_dirty(2, 123, 18, 7);
      }
      draw_battery_voltage(22, 128);
      draw_cpu_temperature(40, 128);
      if (radio_online || ESPNOW_UI_ACTIVE()) {
        draw_waterfall(WF_POS_X, wf_y);
      }
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
      #elif HAS_ESPNOW == true
        draw_espnow_icon(45, 30);
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
      if (radio_online || ESPNOW_UI_ACTIVE()) {
        draw_waterfall(27, 4);
      }
    #endif
  }
}

void update_stat_area() {
  if (eeprom_ok && !firmware_update_mode && !console_active) {

    draw_stat_area();
    #if BOARD_MODEL == BOARD_HELTEC_T114
      // Landscape pushes stat_area_land (its own, narrower canvas - see
      // STAT_AREA_LAND_W's own comment) instead of stat_area; both share
      // the same local widget coordinates (draw_stat_area()'s landscape
      // branch), so push_is_stat's coordinate rules in the colourizer don't
      // need a per-orientation split - only which buffer gets pushed does.
      push_is_stat = true;
      if (disp_mode == DISP_MODE_LANDSCAPE) {
        drawBitmap(p_as_x, p_as_y, stat_area_land.getBuffer(), stat_area_land.width(), stat_area_land.height(), SSD1306_WHITE, SSD1306_BLACK);
      } else {
        drawBitmap(p_as_x, p_as_y, stat_area.getBuffer(), stat_area.width(), stat_area.height(), SSD1306_WHITE, SSD1306_BLACK);
      }
      push_is_stat = false;
    #else
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
    #endif

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
// One extra rotating info page for Date/Time when HAS_RTC - same "always
// in the rotation, falls back to the BT MAC page when not applicable"
// treatment as the WiFi/Ethernet IP pages below, just conditional on
// HAS_RTC since (unlike WiFi/Ethernet) it's still a rare board capability,
// not worth extending the rotation on every other board for.
#if HAS_RTC == true
  const uint8_t pages = 5;
#else
  const uint8_t pages = 4;
#endif
uint8_t disp_page = START_PAGE;
#if HAS_WIFI
  extern IPAddress wr_device_ip;
#endif
#if HAS_ETHERNET
  extern bool eth_is_connected;
  extern IPAddress eth_device_ip;
#endif
#if HAS_RTC == true
  // RTC.h is #include'd later (Utilities.h) than this file, same reason
  // the WiFi/Ethernet globals above are forward-declared rather than
  // #include'd directly.
  extern bool rtc_present;
  uint32_t rtc_get_unixtime();
  void rtc_civil_from_days(int32_t z, int32_t &y, uint32_t &m, uint32_t &d);
  bool rtc_time_valid();
  uint32_t rtc_apply_tz_offset(uint32_t utc_epoch);
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

#if HAS_RTC == true
// Date on the label row, time on the value row - unlike draw_disp_ip_line()
// above, both are fixed-width (always the same digit count), so neither
// needs that function's per-glyph width math to right-align - a plain
// left-aligned print is already stable. Shows local (Timezone-shifted)
// time - see rtc_apply_tz_offset(), RTC.h - same as the RTC Settings
// page's own Time/Date rows.
void draw_disp_datetime_line() {
  uint32_t epoch = rtc_apply_tz_offset(rtc_get_unixtime());
  int32_t days = (int32_t)(epoch / 86400UL);
  uint32_t rem  = epoch % 86400UL;
  uint8_t hh = (uint8_t)(rem / 3600); rem %= 3600;
  uint8_t mi = (uint8_t)(rem / 60);
  uint8_t ss = (uint8_t)(rem % 60);
  int32_t yy; uint32_t mo, dd;
  rtc_civil_from_days(days, yy, mo, dd);

  char date_str[11]; sprintf(date_str, "%04d-%02u-%02u", (int)yy, mo, dd);
  char time_str[9];  sprintf(time_str, "%02u:%02u:%02u", hh, mi, ss);

  disp_area.setFont(SMALL_FONT); disp_area.setTextWrap(false); disp_area.setTextColor(SSD1306_WHITE); disp_area.setTextSize(1);
  disp_area.fillRect(0, 20, disp_area.width(), 17, SSD1306_BLACK);
  disp_area.setCursor(3, 34-8); disp_area.print(date_str);
  disp_area.setCursor(3, 34);   disp_area.print(time_str);
}
#endif

void draw_disp_area() {
  #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114
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
    // bt_ssp_pin != 0 used to gate this (a proxy for "showing the pairing PIN
    // screen"), but it's a residual value set once by bt_confirm_pairing() and
    // only cleared by bt_disable_pairing()/bt_pairing_complete() - any path
    // that leaves pairing without going through those (e.g. bt_stop() called
    // mid-confirmation) left it stuck non-zero forever, permanently hiding the
    // external framebuffer with no way to tell it apart from "pairing is
    // actually in progress". bt_state == BT_STATE_PAIRING is the real,
    // continuously-maintained "are we pairing right now" signal (the PIN
    // screen itself below is already gated on this exact condition), so the
    // framebuffer feature no longer depends on Bluetooth's internal pairing
    // bookkeeping being flawless.
    if (!disp_ext_fb or bt_state == BT_STATE_PAIRING) {
      #if BOARD_MODEL != BOARD_HELTEC_T096 && BOARD_MODEL != BOARD_HELTEC_T114
        // The "unsigned.io" branding strip (bm_def/bm_def_lc's top 8 rows -
        // matches the airtime/channel-load panel's own fillRect(0,8,...)
        // boundary a few lines down, which was already deliberately
        // leaving this exact band untouched) used to only get drawn by the
        // header-art branch further below (the "else" of the very next
        // if). That was fine as long as that branch ran at least once
        // before radio_online first went true, since disp_area is a
        // persistent canvas - nothing clears it between frames (no
        // fillScreen() call anywhere), so whatever's drawn here just sits
        // until something overwrites it.
        //
        // URNS (project_microreticulum_onboard_node memory) broke that
        // assumption: device_init_done only flips true inside
        // device_init() (Device.h, called from validate_status()'s
        // success path), which doesn't itself call update_display() - the
        // first real redraw with device_init_done true happens on loop()'s
        // first iteration, by which point urns_radio_bringup() (also in
        // setup(), right after validate_status()) has already set
        // radio_online=true. So the header branch - and this strip - never
        // ran even once, leaving these rows in disp_area's untouched
        // (blank) initial state permanently. The same root cause would
        // affect any host-driven config that auto-starts the radio at
        // boot too (op_mode restored from EEPROM, validate_status()'s own
        // startRadio() call) - just far rarer to hit before URNS started
        // keeping the radio on by default every boot with no host at all.
        //
        // Drawing it here, unconditionally, every frame, removes the
        // ordering dependency entirely instead of chasing it further -
        // matches the fillRect(0,8,...) boundary below exactly, so
        // there's no double-draw seam where the two meet.
        draw_disp_art(0, device_signatures_ok() ? bm_def_lc : bm_def, 8);
      #endif
      if (radio_online && display_diagnostics) {
        // Alternates this whole airtime/channel-load panel with a GNSS
        // info page every RADIO_PARAMS_PAGE_MS while the receiver is
        // enabled - same static-local toggle/timer pattern as T114's own
        // radio-parameters/GNSS alternation (Display.h, BOARD_HELTEC_T114
        // branch), reset (and held on this page) the instant gnss_enabled
        // goes false. Excludes only T114 specifically (not every color
        // display) - T114 already has its own dedicated alternation in a
        // board-specific branch further down and would otherwise get a
        // second, redundant/out-of-sync one here too, since this "airtime
        // stats" panel isn't board-gated at all. T096 has no such
        // dedicated alternation of its own despite also being a color
        // display, and reaches this exact generic path (own disp_area is
        // 80x64 vs the monochrome boards' 64x64 - same height, so the
        // same fixed Y positions below still fit), so it's included here.
        bool show_gnss_page = false;
        #if HAS_GPS == true && BOARD_MODEL != BOARD_HELTEC_T114
          #ifndef RADIO_PARAMS_PAGE_MS
            #define RADIO_PARAMS_PAGE_MS 10000
          #endif
          {
            static unsigned long airtime_gnss_last_switch_ms = millis();
            static bool airtime_gnss_toggle = false;
            if (!gnss_enabled) {
              airtime_gnss_toggle = false;
            } else if (millis() - airtime_gnss_last_switch_ms >= RADIO_PARAMS_PAGE_MS) {
              airtime_gnss_toggle = !airtime_gnss_toggle;
              airtime_gnss_last_switch_ms = millis();
            }
            show_gnss_page = airtime_gnss_toggle;
          }
        #endif

        #if HAS_GPS == true && BOARD_MODEL != BOARD_HELTEC_T114
        if (show_gnss_page) {
          // One field per line with a plain, unabbreviated label each -
          // the previous crammed-two-per-line version with single-letter
          // codes (S/A, LA/LO, chip:Y/N) proved genuinely unreadable at a
          // glance (confirmed by the user misreading it), not just
          // theoretically tight. Chip name dropped (not requested here -
          // it's static and already visible on the Settings menu/Hardware
          // page). Coordinates still at 2 decimal places (~1km precision)
          // to keep each line short enough for this 64px-wide canvas -
          // first-pass fit, not pixel-verified against real hardware.
          disp_area.fillRect(0, 8, disp_area.width(), disp_area.height()-8, SSD1306_BLACK);
          disp_area.drawFastHLine(0, disp_area.height()-1, disp_area.width(), SSD1306_WHITE);
          disp_area.setFont(SMALL_FONT); disp_area.setTextWrap(false);
          disp_area.setTextColor(SSD1306_WHITE); disp_area.setTextSize(1);

          // 9px spacing (tighter than this file's usual 10-11px convention)
          // to fit 6 lines in 56px of content height - safe here since none
          // of Fix/Sats/Lat/Long/Alt/Time's labels or values have any
          // descenders to clip against the canvas's bottom edge.
          disp_area.setCursor(2, 13);
          disp_area.printf("Fix: %s", gnss_has_fix() ? "YES" : "NO");

          disp_area.setCursor(2, 22);
          disp_area.printf("Sats: %u", (unsigned)gnss_satellite_count());

          disp_area.setCursor(2, 31);
          if (gnss_has_fix()) disp_area.printf("Lat: %.2f", gnss_latitude());
          else                 disp_area.printf("Lat: N/A");

          disp_area.setCursor(2, 40);
          if (gnss_has_fix()) disp_area.printf("Long: %.2f", gnss_longitude());
          else                 disp_area.printf("Long: N/A");

          disp_area.setCursor(2, 49);
          if (gnss_has_fix()) disp_area.printf("Alt: %.0fm", gnss_altitude_meters());
          else                 disp_area.printf("Alt: N/A");

          disp_area.setCursor(2, 58);
          if (gnss_time_valid()) disp_area.printf("Time:%02u:%02u:%02u", gnss_time_hour(), gnss_time_minute(), gnss_time_second());
          else                    disp_area.printf("Time:N/A");
        } else
        #endif
        {
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
        }

      } else {
        #if BOARD_MODEL == BOARD_HELTEC_T096
          // Full-width header: left-aligned art with the unsigned.io strip
          // and zigzag extended to the whole 80px width
          if (device_signatures_ok()) { disp_area.drawBitmap(0, 0, bm_def_lc_t096, disp_area.width(), 23, SSD1306_WHITE, SSD1306_BLACK); }
          else {                        disp_area.drawBitmap(0, 0, bm_def_t096,    disp_area.width(), 23, SSD1306_WHITE, SSD1306_BLACK); }
        #elif BOARD_MODEL == BOARD_HELTEC_T114
          {
            // Same 64px-wide generic art as the non-T096/T114 boards, but
            // split into two independently-aligned pieces instead of
            // drawn as one block: the "unsigned.io" strip (bitmap rows
            // 0-5, a solid white background) stays flush at the canvas's
            // left edge, extending each row's right-edge colour out to
            // the canvas's right edge (same idea as draw_disp_art()'s
            // DISP_BM_X side-fill, one-sided since there's no left margin
            // here) so the white strip reads edge-to-edge. Everything
            // below that (RNODE + the model number, rows 6-22) is
            // centered instead, at DISP_BM_X, since only the unsigned.io
            // line itself was asked to be left-aligned.
            const uint8_t *hdr_bm = device_signatures_ok() ? bm_def_lc : bm_def;
            disp_area.fillRect(0, 0, disp_area.width(), 23, SSD1306_BLACK);
            disp_area.drawBitmap(0, 0, hdr_bm, 64, 6, SSD1306_WHITE, SSD1306_BLACK);
            for (int16_t r = 0; r < 6; r++) {
              uint16_t rc = (hdr_bm[r*(64/8)+(64/8)-1] & 0x01) ? SSD1306_WHITE : SSD1306_BLACK;
              disp_area.drawFastHLine(64, r, disp_area.width()-64, rc);
            }
            disp_area.drawBitmap(DISP_BM_X, 6, hdr_bm+6*(64/8), 64, 23-6, SSD1306_WHITE, SSD1306_BLACK);
          }
        #else
          if (device_signatures_ok()) { draw_disp_art(0, bm_def_lc, 23); }
          else {                        draw_disp_art(0, bm_def, 23); }
        #endif

        bool wifi_ip_ready = false;
        bool eth_ip_ready = false;
        bool rtc_time_ready = false;
        #if HAS_WIFI
          wifi_ip_ready = wifi_is_connected();
        #endif
        #if HAS_ETHERNET
          eth_ip_ready = eth_is_connected;
        #endif
        #if HAS_RTC == true
          rtc_time_ready = rtc_time_valid();
        #endif

        // Page 1 dwells on the WiFi IP, page 3 on the Ethernet IP, page 4
        // (HAS_RTC only) on Date/Time - each for a full page_interval, same
        // as the original single WiFi IP page.
        bool display_alt = false;
        bool show_wifi_ip = false;
        bool show_eth_ip = false;
        bool show_datetime = false;
        if (wifi_ip_ready && disp_page == 1) {
          display_alt = true;
          show_wifi_ip = true;
        } else if (eth_ip_ready && disp_page == 3) {
          display_alt = true;
          show_eth_ip = true;
        } else if (rtc_time_ready && disp_page == 4) {
          display_alt = true;
          show_datetime = true;
        }
        if (display_alt) {
          #if HAS_WIFI
            if (show_wifi_ip) { draw_disp_ip_line("WiFi IP:", wr_device_ip); }
          #endif
          #if HAS_ETHERNET
            if (show_eth_ip) { draw_disp_ip_line("Eth IP:", eth_device_ip); }
          #endif
          #if HAS_RTC == true
            if (show_datetime) { draw_disp_datetime_line(); }
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
          #if (BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114) && USE_COLOR_DISPLAY == true
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
        } else if (ESPNOW_UI_ACTIVE()) {
          // No pre-rendered art for this state (unlike bm_online above) -
          // a matching bitmap asset isn't practical to generate here, so
          // this overlays live text instead, same idiom as draw_eth_icon()'s
          // "OFF" label. Two lines rather than one "ESP-NOW ACTIVE" string:
          // this board family's disp_area art column is only 64px wide
          // (DISP_BM_W), too narrow for that at SMALL_FONT size 1.
          //
          // Deliberately no "if (!display_diagnostics)" guard here, unlike
          // the radio_online branch above - that guard is a no-op there
          // (the outer "if (radio_online && display_diagnostics)",
          // Display.h:1831, already diverts to the airtime/channel-load
          // panel and skips this whole block whenever both are true, so by
          // the time this is reached with radio_online true,
          // display_diagnostics is always already false). ESPNOW_UI_ACTIVE()
          // has no such outer diversion, and display_diagnostics defaults
          // true and is never toggled anywhere - copying that guard here
          // silently skipped this draw every time, leaving whatever idle-
          // carousel bitmap was last on screen frozen in place.
          disp_area.fillRect(0, 37, disp_area.width(), 27, SSD1306_BLACK);
          // Top/bottom 1px rule lines framing the banner as a box, since
          // there's no bitmap border art to fall back on here (unlike
          // bm_online above).
          disp_area.drawFastHLine(0, 37, disp_area.width(), SSD1306_WHITE);
          disp_area.drawFastHLine(0, 63, disp_area.width(), SSD1306_WHITE);
          disp_area.setFont(SMALL_FONT); disp_area.setTextWrap(false);
          disp_area.setTextColor(SSD1306_WHITE); disp_area.setTextSize(1);

          const char *top_buf = "ESP-NOW";
          int16_t tx1, ty1; uint16_t tw, th;
          disp_area.getTextBounds(top_buf, 0, 0, &tx1, &ty1, &tw, &th);
          disp_area.setCursor(DISP_BM_X + (DISP_BM_W - (int16_t)tw) / 2, 47);
          disp_area.print(top_buf);

          // The channel ESP-NOW is actually locked to (wr_channel, Config.h -
          // set by espnow_init()'s esp_wifi_set_channel()/peer.channel,
          // ESPNOW.h) is more useful here than a static "ACTIVE" label,
          // especially given the known STA-mode channel-drift caveat
          // (wifi_remote_reconnect(), Remote.h) - this makes the currently-
          // locked channel visible at a glance instead of hidden state.
          char bot_buf[11];
          sprintf(bot_buf, "CHANNEL %u", wr_channel);
          int16_t bx1, by1; uint16_t bw, bh;
          disp_area.getTextBounds(bot_buf, 0, 0, &bx1, &by1, &bw, &bh);
          disp_area.setCursor(DISP_BM_X + (DISP_BM_W - (int16_t)bw) / 2, 57);
          disp_area.print(bot_buf);
        } else {
          if (disp_page == 0) {
            if (true || device_signatures_ok()) {
              draw_disp_art(37, bm_checks, 27);
              #if (BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114) && USE_COLOR_DISPLAY == true
                disp_banner_fg = COLOR_BANNER_OK;
              #endif
            } else {
              draw_disp_art(37, bm_nfr, 27);
            }
          } else if (disp_page == 1) {
            if (!console_active) {
              draw_disp_art(37, bm_hwok, 27);
              #if (BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114) && USE_COLOR_DISPLAY == true
                disp_banner_fg = COLOR_BANNER_OK;
              #endif
            } else {
              draw_disp_art(37, bm_console_active, 27);
            }
          } else if (disp_page == 2) {
            draw_disp_art(37, bm_version, 27);
            #if (BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114) && USE_COLOR_DISPLAY == true
              disp_banner_fg = COLOR_BANNER_OK;
            #endif
            // MAJ.MIN plus a "-BBB" BUILD_NUMBER suffix needs more room than
            // the old MAJ.MIN-only layout left on this 64px canvas, so every
            // board shifts the whole line left and packs digits tighter
            // (8px pitch = glyph width, i.e. no gap, vs. the old 9px/1px-gap
            // spacing) - originally confirmed only against MeshPoE-S3's real
            // hardware (the first HAS_OTA board), now applied everywhere
            // since every board displays the suffix. Re-check on real
            // hardware per board as they're tested - canvas width (DISP_BM_X/
            // DISP_BM_W) varies enough between boards that this may still
            // need small per-board nudges.
            int16_t vbase = DISP_BM_X+4;
            uint8_t vpitch = 8; uint8_t vgap = 3;
            char *v_str = (char*)malloc(3+1);
            sprintf(v_str, "%01d%02d", MAJ_VERS, MIN_VERS);
            for (int i = 0; i < 3; i++) {
              uint8_t numeric = v_str[i]-48; uint8_t bm_offset = numeric*5;
              int16_t dxp = vbase;
              if (i == 1) dxp += vpitch*1+vgap;
              if (i == 2) dxp += vpitch*2+vgap;
              disp_area.drawBitmap(dxp, 37+16, bm_n_uh+bm_offset, 8, 5, SSD1306_WHITE, SSD1306_BLACK);
            }
            free(v_str);
            disp_area.drawLine(vbase+7, 37+19, vbase+8, 37+19, SSD1306_BLACK);
            disp_area.drawLine(vbase+7, 37+20, vbase+8, 37+20, SSD1306_BLACK);

            // "-BBB" suffix: last 3 digits of BUILD_NUMBER (0 on any board
            // not built through the Makefile's git-commit-count injection,
            // see BUILD_NUMBER's own fallback, Boards.h), same bm_n_uh digit
            // font as MAJ.MIN above, right after MIN_VERS. Separator mark
            // uses the same 2x2 dot technique as the MAJ/MIN decimal point
            // above (a plain drawFastHLine wasn't visible here) rather than
            // an actual "-" glyph.
            int16_t min_end = vbase + vpitch*2+vgap + 8;
            disp_area.drawLine(min_end-1, 37+19, min_end+0, 37+19, SSD1306_BLACK);
            disp_area.drawLine(min_end-1, 37+20, min_end+0, 37+20, SSD1306_BLACK);
            // Not tied to the dot's position above - there's a 3px gap to
            // the dot now, comfortably clear of its solid-background
            // bitmap overwriting the dot's right pixel (see prior note -
            // that's what shaved it to 1px wide before a gap existed).
            int16_t bbase = min_end+3;
            long build_num = ((long)BUILD_NUMBER) % 1000;
            char *b_str = (char*)malloc(3+1);
            sprintf(b_str, "%03ld", build_num);
            for (int i = 0; i < 3; i++) {
              uint8_t numeric = b_str[i]-48; uint8_t bm_offset = numeric*5;
              int16_t dxp = bbase+i*8;
              disp_area.drawBitmap(dxp, 37+16, bm_n_uh+bm_offset, 8, 5, SSD1306_WHITE, SSD1306_BLACK);
            }
            free(b_str);
          } else if (disp_page == 3) {
            if (!console_active) {
              draw_disp_art(37, bm_hwok, 27);
              #if (BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114) && USE_COLOR_DISPLAY == true
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
    #if BOARD_MODEL == BOARD_HELTEC_T114
      // Radio parameters, in the space this taller canvas has below the
      // logo/banner (or the diagnostics airtime/channel-load panel) that a
      // plain 64x64 disp_area doesn't have - shown regardless of which of
      // those two is active above it, AND regardless of whether a host has
      // taken over rows 0-63 with its own pushed external framebuffer
      // (disp_ext_fb, the "else" above): that only ever draws into the top
      // 64 rows (DISP_BM_W x 64, see its own drawBitmap call), so rows 64+
      // are free real estate in every case, not just the two normal-drawing
      // branches this used to be scoped to. Redrawn/cleared every cycle
      // here rather than inside any of those branches, since none of them
      // touch rows 64+ on their own - the ext_fb branch's own fillRect
      // clears this region too (it clears the whole canvas), so without
      // this running unconditionally afterward it would just go blank
      // instead of showing stale content.
      if (radio_online) {
        // disp_area itself is always 135 wide (reused as-is for
        // landscape, see STAT_AREA_LAND_W's own comment), but landscape
        // only pushes/draws its own narrower left slice of it
        // (DISP_AREA_LAND_BOX_W, 2px short of where stat_area_land
        // actually starts so there's a visible gap between them) - draw
        // this box to match whatever's actually going to be visible, or
        // its right edge would be silently cropped off mid-push.
        int16_t disp_w = (disp_mode == DISP_MODE_LANDSCAPE) ? DISP_AREA_LAND_BOX_W : disp_area.width();
        // disp_area.height() and DISP_AREA_PORTRAIT_H are the same value
        // now (see DISP_AREA_H's own comment) - box_h works out identical
        // either way, kept as a mode-conditional expression rather than
        // collapsed to one so this doesn't silently change again if
        // DISP_AREA_H ever grows for some other reason.
        int16_t box_h = ((disp_mode == DISP_MODE_LANDSCAPE) ? disp_area.height() : DISP_AREA_PORTRAIT_H) - 64;
        disp_area.fillRect(0, 64, disp_w, box_h, SSD1306_BLACK);
        // Border box, matching the waterfall's - drawn every cycle
        // (unlike the waterfall's, which is drawn once) since the
        // fillRect above already clears this whole region every time.
        // Spans right down to the canvas's last row - see the
        // waterfall's own box for the same "use the full available
        // height" treatment.
        disp_area.drawRect(0, 64, disp_w, box_h, SSD1306_WHITE);
        disp_area.setFont(SMALL_FONT); disp_area.setTextWrap(false);
        disp_area.setTextColor(SSD1306_WHITE); disp_area.setTextSize(1);
        // Landscape-only nudge - portrait's own rows (76/86/96/106) are
        // already tuned against the box's portrait height, unaffected.
        // -1 rather than -3 - moved 2px further down for both the LoRa-
        // params and GNSS-params pages, per user request on real hardware
        // (T114 only - this whole block is already scoped to
        // BOARD_HELTEC_T114, every other board's layout is untouched).
        int16_t rp_y_off = (disp_mode == DISP_MODE_LANDSCAPE) ? -1 : 0;

        // Alternates this box with a GNSS info page (same fields as the
        // Settings menu's own GNSS page, Menu.h) every RADIO_PARAMS_PAGE_MS
        // while the receiver is enabled - static locals so the toggle/timer
        // persist across calls without a global. Reset (and held on the
        // radio page) the instant gnss_enabled goes false, so there's no
        // stale mid-cycle GNSS page left on screen and no switching at all
        // while it's off, per the user's request.
        bool show_gnss_page = false;
        #if HAS_GPS == true
          #define RADIO_PARAMS_PAGE_MS 10000
          {
            static unsigned long radio_params_last_switch_ms = millis();
            static bool radio_params_toggle = false;
            if (!gnss_enabled) {
              radio_params_toggle = false;
            } else if (millis() - radio_params_last_switch_ms >= RADIO_PARAMS_PAGE_MS) {
              radio_params_toggle = !radio_params_toggle;
              radio_params_last_switch_ms = millis();
            }
            show_gnss_page = radio_params_toggle;
          }
        #endif

        #if HAS_GPS == true
          if (show_gnss_page) {
            // Same caption wording as the generic OLED alternation panel
            // and the Settings-menu GNSS page (Fix/Sats/Lat/Long/Alt) -
            // this box used to say FIX/SATS/LAT/LON, which the user found
            // unreadable at a glance on the OLED version; kept in sync
            // here too rather than leaving T114 on the old wording. Chip
            // name dropped per explicit request - matches every other
            // variant's page exactly now, not just the wording.
            disp_area.setCursor(4, 73+rp_y_off);
            // Fix and Time share this line in both orientations - the fit
            // (measured against DISP_AREA_LAND_BOX_W's 118px-wide box,
            // landscape's narrower of the two, Org_01/SMALL_FONT, ~100px
            // used) doesn't depend on disp_mode at all, so this doesn't
            // need its own landscape/portrait branch the way some of this
            // box's other layout tweaks do. No space after either colon
            // (unlike the other lines here) to stay inside that margin.
            if (gnss_time_valid()) disp_area.printf("Fix: %s Time: %02u:%02u:%02u", gnss_has_fix() ? "YES" : "NO", gnss_time_hour(), gnss_time_minute(), gnss_time_second());
            else                    disp_area.printf("Fix: %s Time: N/A", gnss_has_fix() ? "YES" : "NO");

            disp_area.setCursor(4, 83+rp_y_off);
            if (gnss_has_fix()) disp_area.printf("Sats: %u  Alt: %.0fm", (unsigned)gnss_satellite_count(), gnss_altitude_meters());
            else                 disp_area.printf("Sats: %u  Alt: N/A", (unsigned)gnss_satellite_count());

            disp_area.setCursor(4, 93+rp_y_off);
            if (gnss_has_fix()) disp_area.printf("Lat: %.5f", gnss_latitude());
            else                 disp_area.printf("Lat: N/A");

            disp_area.setCursor(4, 103+rp_y_off);
            if (gnss_has_fix()) disp_area.printf("Long: %.5f", gnss_longitude());
            else                 disp_area.printf("Long: N/A");
          } else
        #endif
        {
          disp_area.setCursor(4, 73+rp_y_off);
          disp_area.printf("%.3fMHz", (float)lora_freq/1000000.0);
          disp_area.setCursor(4, 83+rp_y_off);
          disp_area.printf("BW %.0fK SF%d CR4:%d", (float)lora_bw/1000.0, lora_sf, lora_cr);
          disp_area.setCursor(4, 93+rp_y_off);
          disp_area.printf("TX POWER %ddBm", lora_txp);
          // noise_floor (Config.h) defaults to -292 (same never-sampled
          // sentinel as last_rssi) until update_noise_floor() has
          // collected a full NOISE_FLOOR_SAMPLES window (RNode_Firmware.
          // ino) - stays blank until then rather than showing that.
          if (noise_floor != -292) {
            disp_area.setCursor(4, 103+rp_y_off);
            disp_area.printf("NOISE FLOOR %ddBm", noise_floor);
          }
        }
      }
    #endif
  }
}

void update_disp_area() {
  draw_disp_area();

  #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114
    static uint16_t banner_fg_prev = 0;
    if (disp_banner_fg != banner_fg_prev) {
      banner_fg_prev = disp_banner_fg;
      #if BOARD_MODEL == BOARD_HELTEC_T114
        // Landscape only pushes DISP_AREA_LAND_BOX_W columns of disp_area
        // (see the push below) - marking the full DISP_AREA_W here would
        // reach past p_as_x into stat_area_land's own screen territory.
        int16_t banner_dirty_w = (disp_mode == DISP_MODE_LANDSCAPE) ? DISP_AREA_LAND_BOX_W : DISP_AREA_W;
        colour_mark_dirty(p_ad_x, p_ad_y+37, banner_dirty_w, 27);
      #else
        colour_mark_dirty(p_ad_x, p_ad_y+37, DISP_AREA_W, 27);
      #endif
    }
  #endif

  #if BOARD_MODEL == BOARD_HELTEC_T114
    if (disp_mode == DISP_MODE_LANDSCAPE) {
      // Only push disp_area's own left DISP_AREA_LAND_BOX_W columns -
      // narrower than its full stored width (135), and 2px short of where
      // stat_area_land starts (DISP_AREA_LAND_W) so there's a visible gap
      // between them rather than the two sitting flush. srcRowBytes must
      // be disp_area's own true row stride (not DISP_AREA_LAND_BOX_W's),
      // since the buffer itself is still laid out at its full 135px width -
      // passing a non-zero srcRowBytes opts this particular push out of
      // the region-cache fast path (see its own cacheable check), so this
      // costs a full repush every cycle rather than a diffed one.
      int16_t disp_row_bytes = (disp_area.width()+7)/8;
      drawBitmap(p_ad_x, p_ad_y, disp_area.getBuffer(), DISP_AREA_LAND_BOX_W, disp_area.height(), SSD1306_WHITE, SSD1306_BLACK, disp_row_bytes);
    } else {
      // DISP_AREA_PORTRAIT_H, same value as disp_area.height() now (see
      // DISP_AREA_H's own comment) - kept explicit rather than switched to
      // disp_area.height() so portrait stays pinned to it (not
      // stat_area's own territory, p_as_y is pinned to
      // DISP_AREA_PORTRAIT_H too, see update_area_positions()) if
      // DISP_AREA_H ever grows again for some other reason.
      drawBitmap(p_ad_x, p_ad_y, disp_area.getBuffer(), disp_area.width(), DISP_AREA_PORTRAIT_H, SSD1306_WHITE, SSD1306_BLACK);
    }
  #else
    drawBitmap(p_ad_x, p_ad_y, disp_area.getBuffer(), disp_area.width(), disp_area.height(), SSD1306_WHITE, SSD1306_BLACK);
  #endif
  #if BOARD_MODEL != BOARD_HELTEC_T096 && BOARD_MODEL != BOARD_HELTEC_T114
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

      #if BOARD_MODEL == BOARD_HELTEC_T114 || BOARD_MODEL == BOARD_HELTEC_T096
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

      #if BOARD_MODEL == BOARD_HELTEC_T114 || BOARD_MODEL == BOARD_HELTEC_T096
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
          static bool menu_was_open = false;
          bool menu_open_now = menu_is_open();
          #if BOARD_MODEL == BOARD_HELTEC_T096
            // The menu always shows in landscape, regardless of the
            // Orientation the user has picked for the main screen (the
            // menu_canvas above is landscape-shaped, 160x80, and would
            // never fit sensibly in portrait) - force rotation 1 (this
            // panel's standard landscape, see display_init()) on open,
            // and restore whatever the main screen actually uses on
            // close.
            if (menu_open_now && !menu_was_open) {
              display.setRotation(1);
            } else if (!menu_open_now && menu_was_open) {
              display.setRotation(active_display_rotation);
            }
            if (menu_open_now != menu_was_open) {
              // The menu occupies the same full-panel footprint the
              // operational screen's disp_area/stat_area do (landscape,
              // same as always) - cached regions there store the panel
              // content as of their last push, but the menu just wrote
              // over that same panel space through a different push
              // path (push_menu_canvas(), not disp_area/stat_area's own
              // pushes), so a stale cache entry from before this
              // transition can wrongly compare equal to content pushed
              // after it and get skipped - leaving real leftover pixels
              // on screen. Force every region to repaint in full across
              // the transition, both ways. (This also covers the rarer
              // case of an actual rotation change, if the main screen's
              // Orientation isn't already landscape-1 - a rotation
              // change never retroactively repaints anything already
              // latched into the panel's GRAM either.)
              for (uint8_t i = 0; i < REGION_CACHE_SLOTS; i++) region_cache[i].x = -1;
              #if USE_COLOR_DISPLAY == true
                cdirty_count = 0;
              #endif
              invalidate_menu_canvas_shadow();
            }
          #elif BOARD_MODEL == BOARD_HELTEC_T114
            // The menu always shows in portrait (menu_canvas above is
            // portrait-shaped, 135x240, T114's native orientation) -
            // force rotation 0 on open, and restore whatever the main
            // screen actually uses on close.
            if (menu_open_now && !menu_was_open) {
              display.setRotation(0);
            } else if (!menu_open_now && menu_was_open) {
              display.setRotation(active_display_rotation);
            }
            if (menu_open_now != menu_was_open) {
              // Same reasoning as T096 above: the menu and the
              // operational screen share the same panel footprint through
              // two independent push paths, so a stale cache entry from
              // before this transition can wrongly compare equal to
              // content pushed after it.
              for (uint8_t i = 0; i < REGION_CACHE_SLOTS; i++) region_cache[i].x = -1;
              #if USE_COLOR_DISPLAY == true
                cdirty_count = 0;
              #endif
              if (!menu_open_now && disp_mode == DISP_MODE_LANDSCAPE) {
                // Closing the menu (portrait, 135x240 - every physical
                // pixel) back into landscape: disp_area only ever pushes
                // rows p_ad_y..p_ad_y+disp_area.height()-1 in its own
                // column (0..DISP_AREA_LAND_W-1) - with radio_online false
                // there's nothing to redraw that column's rows above/below
                // that span, so a cache invalidation alone has nothing to
                // repaint there and the menu's last content just stays put.
                // Blanking the whole column up front guarantees no menu
                // leftovers survive regardless of what disp_area/stat_area
                // actually redraw this cycle.
                fillRect(0, 0, DISP_AREA_LAND_W, 135, SSD1306_BLACK);
              }
            }
          #else
            // The settings menu is always laid out for the panel's native
            // 128x64 landscape shape, regardless of what rotation the main
            // content is using - the panel itself doesn't physically change
            // shape, so forcing rotation 0/2 (both landscape, GFX-wise) here
            // just undoes whatever swap the main content's rotation setting
            // applied. 90 and 270 are the same physical mounting 180 degrees
            // apart, so they need opposite landscape variants (0 vs 2) to
            // still read right-side-up - same for the 0/180 pair.
            if (menu_open_now && !menu_was_open) {
              display.setRotation(active_display_rotation & 0x02);
            } else if (!menu_open_now && menu_was_open) {
              display.setRotation(active_display_rotation);
            }
          #endif
          menu_was_open = menu_open_now;

          if (menu_open_now) {
            draw_settings_menu_disp();
            #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114
              push_menu_canvas();
            #endif
          } else
        #endif
        {
          #if BOARD_MODEL == BOARD_HELTEC_T114
            if (!draw_t114_splash())
          #endif
          {
            update_stat_area();
            update_disp_area();
            #if HAS_MENU == true && HAS_INPUT == true
              draw_button_hold_overlay();
            #endif
          }
        }
      }
      
      #if BOARD_MODEL == BOARD_TECHO
        if (current-last_epd_refresh >= epd_update_interval) {
          if (current-last_epd_full_refresh >= REFRESH_PERIOD) { display.display(false); last_epd_full_refresh = millis(); }
          else { display.display(true); }
          last_epd_refresh = millis();
          epd_blanked = false;
        }
      #elif BOARD_MODEL != BOARD_TDECK && BOARD_MODEL != BOARD_HELTEC_T096 && BOARD_MODEL != BOARD_HELTEC_T114
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
    // Only force the backlight to full when actually waking from a
    // blanked/dimmed state - see BOARD_HELTEC_T096's identical guard
    // below for the full rationale (this is called on every button event,
    // not just real wakes, and would otherwise override the user's
    // configured brightness on every menu tap).
    if (display_blanked) {
      analogWrite(PIN_T114_TFT_BLGT, 0);
    }
  #elif BOARD_MODEL == BOARD_HELTEC_T096
    // Only force the backlight to full when actually waking from a
    // blanked/dimmed state. This is called unconditionally on every
    // button/encoder event to keep the away-timer alive (see
    // menu_encoder_rotate(), Menu.h, called on every menu tap) - forcing
    // max brightness every time, rather than just on a real wake, would
    // override whatever level the user has configured (Menu.h's
    // Brightness field, display_intensity) back to maximum on every
    // single menu tap.
    if (display_blanked) {
      analogWrite(PIN_T096_TFT_BLGT, 0);
    }
  #endif
}

void ext_fb_enable() {
  disp_ext_fb = true;
}

void ext_fb_disable() {
  disp_ext_fb = false;
}
