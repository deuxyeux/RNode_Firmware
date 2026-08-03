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

#ifndef MENU_H
  #define MENU_H

  // Used only for the WiFi SSID/PSK text-entry screen (draw_menu_text_edit_disp),
  // and only there - every other menu screen stays on SMALL_FONT/Org_01.
  #include "Fonts/Tamsyn6x12.h"
  #define TEXT_ENTRY_FONT &Tamsyn6x12

  // Every draw_*_disp() function below targets MENU_GFX instead of the
  // global `display` object directly. On every board except T096 this is
  // display itself (zero change from before). T096's ST7735 panel has no
  // framebuffer, so its menu renders into an off-screen canvas instead
  // (menu_canvas, Display.h) and gets composited through drawBitmap()
  // rather than drawn straight to the glass - see push_menu_canvas().
  // The handful of layout macros below follow the same pattern: today's
  // literal, hand-tuned 128x64 SSD1306 values everywhere, T096-specific
  // starting points where T096 actually renders a screen today (list and
  // edit - see Boards.h's HAS_WIFI/HAS_ETHERNET/HAS_RTC being absent for
  // why the address/datetime/text-wheel editors don't need this yet).
  // Values are starting points, not final - like every other pixel-level
  // convention in this file, tuning happens live on hardware, not here.
  #if BOARD_MODEL == BOARD_HELTEC_T096
    #define MENU_GFX menu_canvas
    #define MENU_FONT SMALL_FONT
    #define MENU_CONTENT_W (MENU_CANVAS_W - 8) // 4px margin each side
    #define MENU_LIST_ROW_H 11
    // 5 rows * 11px from MENU_LIST_TOP_Y (15) reaches y=70, which is exactly
    // where the footer below now starts - same zero-gap fit the original
    // 4-row layout had at y=59, just one row taller. The previous 4-row
    // layout only used the canvas up to ~y=67 (footer text baseline 63 +
    // descender), leaving a bare ~13px unused at the bottom of the 80px
    // canvas (MENU_CANVAS_H, Display.h) - confirmed on hardware.
    #define MENU_LIST_VISIBLE_ROWS 5
    #define MENU_LIST_TOP_Y 15
    // Org_01's ascent is ~4px, so a 7px baseline offset centers it fine in
    // an 11px row - see MENU_LIST_BASELINE_OFF's use in draw_menu_list_disp().
    #define MENU_LIST_BASELINE_OFF 7
    #define MENU_LIST_FOOTER_HLINE_Y (MENU_CANVAS_H - 10)
    #define MENU_LIST_FOOTER_TEXT_Y (MENU_CANVAS_H - 6)
    #define MENU_EDIT_VALUE_CX (MENU_CANVAS_W / 2)
    #define MENU_EDIT_VALUE_Y 36
    #define MENU_EDIT_ARROW_Y 34
    #define MENU_EDIT_ARROW_R_EDGE (MENU_CANVAS_W - 5)
    // Matches the list screen's footer position above - same footer line
    // height across every T096 menu screen, even though the edit screen's
    // own content doesn't need the reclaimed space.
    #define MENU_EDIT_FOOTER_HLINE_Y (MENU_CANVAS_H - 10)
    #define MENU_EDIT_FOOTER_TEXT_Y (MENU_CANVAS_H - 6)
  #elif BOARD_MODEL == BOARD_HELTEC_T114
    // Same off-screen-canvas reasoning as T096 above (Adafruit_ST7789 has
    // no framebuffer either) - menu_canvas here is portrait, 135x240
    // (Display.h), so there's a lot more vertical room than T096's 80px-
    // tall canvas: more visible rows, roomier row height. Starting points,
    // same as T096's - tune live on hardware.
    //
    // Panel's big enough to afford a bigger font too - Tamsyn6x12 (already
    // linked in for the WiFi text-entry screen, see TEXT_ENTRY_FONT above)
    // instead of the tiny Org_01 every other board's menu uses. Row height/
    // edit-screen Y positions below are bumped up to give its 12px-tall
    // glyphs headroom Org_01 never needed.
    #define MENU_GFX menu_canvas
    #define MENU_FONT TEXT_ENTRY_FONT
    #define MENU_CONTENT_W (MENU_CANVAS_W - 8) // 4px margin each side
    #define MENU_LIST_ROW_H 20
    #define MENU_LIST_VISIBLE_ROWS 8
    #define MENU_LIST_TOP_Y 22
    // Tamsyn6x12's ascent is 9px (glyph top sits 9px above the baseline,
    // per its own GFXglyph yOffset=-9) - a 7px offset (right for Org_01's
    // ~4px ascent) clipped the tops of letters against the selection box.
    // Centering 12px-tall glyphs in a 20px row wants ~4px of margin above
    // and below, so baseline = 4 (margin) + 9 (ascent) = 13.
    #define MENU_LIST_BASELINE_OFF 13
    #define MENU_LIST_FOOTER_HLINE_Y (MENU_CANVAS_H - 21)
    // +3 past the plain "same margin as the row baseline" value - otherwise
    // the footer text's own ascent pokes above MENU_LIST_FOOTER_HLINE_Y.
    #define MENU_LIST_FOOTER_TEXT_Y (MENU_CANVAS_H - 12)
    #define MENU_EDIT_VALUE_CX (MENU_CANVAS_W / 2)
    // Re-centered within the content region between the header hline (y=15)
    // and the footer hline (MENU_EDIT_FOOTER_HLINE_Y, y=219) - the original
    // 102/96 (T096's own starting values, just carried over) sat visibly
    // high in T114's much taller 240px canvas, confirmed offset-up on real
    // hardware. That region's midpoint is 15+(219-15)/2=117; Tamsyn6x12 at
    // size 2 has an 18px ascent and no meaningful descent on digits, so its
    // visual center is baseline-9, giving baseline=117+9=126.
    //
    // Arrows were first kept at a fixed 6px-above-value gap (matching the
    // value visually instead of centering independently), but confirmed
    // still sitting visibly high on real hardware - centering them
    // independently instead: at size 1, Tamsyn6x12's ascent is 9px, so its
    // visual center is baseline-4.5; solving baseline-4.5=117 (the same
    // content-region midpoint the value uses) gives baseline=121.5,
    // rounded to 122.
    #define MENU_EDIT_VALUE_Y 126
    #define MENU_EDIT_ARROW_Y 122
    #define MENU_EDIT_ARROW_R_EDGE (MENU_CANVAS_W - 5)
    #define MENU_EDIT_FOOTER_HLINE_Y (MENU_CANVAS_H - 21)
    // -12 rather than -15 - nudged 3px further down from the footer hline
    // (2px then 1px more), per user request on real hardware.
    #define MENU_EDIT_FOOTER_TEXT_Y (MENU_CANVAS_H - 12)
  #else
    #define MENU_GFX display
    #define MENU_FONT SMALL_FONT
    #define MENU_CONTENT_W 120
    #define MENU_LIST_ROW_H 11
    #define MENU_LIST_VISIBLE_ROWS 4
    #define MENU_LIST_TOP_Y 15
    #define MENU_LIST_BASELINE_OFF 7
    #define MENU_LIST_FOOTER_HLINE_Y 59
    #define MENU_LIST_FOOTER_TEXT_Y 63
    #define MENU_EDIT_VALUE_CX 64
    #define MENU_EDIT_VALUE_Y 36
    #define MENU_EDIT_ARROW_Y 34
    #define MENU_EDIT_ARROW_R_EDGE 123
    #define MENU_EDIT_FOOTER_HLINE_Y 50
    #define MENU_EDIT_FOOTER_TEXT_Y 59
  #endif

  #define MENU_STATE_CLOSED         0
  #define MENU_STATE_LIST           1   // top-level list
  #define MENU_STATE_EDIT           2   // editing a top-level field
  #define MENU_STATE_WIFI_LIST      3   // WiFi submenu list
  #define MENU_STATE_WIFI_EDIT      4   // editing the WiFi Mode field
  #define MENU_STATE_WIFI_TEXT_EDIT 5   // editing WiFi SSID/PSK via character wheel
  #define MENU_STATE_WIFI_TEXT_CONFIRM 6 // "Save?" dialog, long-press from text edit
  #define MENU_STATE_HW_LIST        7   // Hardware submenu list (read-only info)
  #define MENU_STATE_HW_EDIT        8   // editing the Input Voltage/Battery Cal field
  #define MENU_STATE_GPIO_LIST      9   // Hardware > GPIO submenu list
  #define MENU_STATE_GPIO_PIN_EDIT  10  // picking a physical pin for whichever GPIO_ITEM_* is selected
  #define MENU_STATE_ETH_LIST       11  // Ethernet submenu list (MeshPoE-S3 only, HAS_ETHERNET)
  #define MENU_STATE_ETH_EDIT       12  // editing the Ethernet Speed field
  #define MENU_STATE_ETH_ADDR_EDIT  13  // editing one octet of the IP Address or Netmask field
  #define MENU_STATE_WIFI_ADDR_EDIT 14  // editing one octet of WiFi's own IP Address or Netmask field
  #define MENU_STATE_RTC_LIST       15  // RTC submenu list (HAS_RTC boards)
  #define MENU_STATE_RTC_EDIT       16  // Set Time/Date: sequential Year/Month/Day/Hour/Minute/Second editor
  #define MENU_STATE_STATUS_POPUP   17  // Generic transient status box (Sync NTP, Clear Static, ...) - see menu_open_popup()
  #define MENU_STATE_RTC_TZ_EDIT    18  // editing the Timezone display-offset field
  #define MENU_STATE_GNSS_LIST      19  // GNSS submenu list (HAS_GPS boards)
  #define MENU_STATE_GNSS_EDIT      20  // editing the Enabled field
  #define MENU_STATE_SENSORS_LIST   21  // Sensors submenu list (HAS_SENSORS boards) - read-only, no edit state
  #define MENU_STATE_FWUPD_LIST     22  // F/W Update submenu list (HAS_OTA boards) - Current/Latest/Update/Back
  #define MENU_STATE_FWUPD_CONFIRM  23  // UPDATE/CANCEL list before Update actually runs - same pattern as MENU_STATE_WIFI_TEXT_CONFIRM
  #define MENU_STATE_MEM_LIST       24  // Hardware > Memory submenu (MCU_ESP32 boards) - Heap/PSRAM bar graphs, read-only
  #define MENU_STATE_MEM_DETAIL     25  // Memory > Heap or PSRAM detail readout (Total/Used/Free/Min Free), read-only
  #define MENU_STATE_ESPNOW_LIST    26  // ESP-NOW submenu list (HAS_ESPNOW boards) - Enabled/Mode/Back
  #define MENU_STATE_ESPNOW_EDIT    27  // editing whichever of Enabled/Mode/LR was selected
  #define MENU_STATE_ESPNOW_LR_CONFIRM 28 // info row + ENABLE/CANCEL list, shown only when LR is being turned on - same pattern as MENU_STATE_FWUPD_CONFIRM

  // The Hardware page used to only exist when there was board-level info
  // worth showing (battery/voltage sensing via HAS_PMU, or an ESP32-S3's
  // CPU temp) - now that it always has at least Node Uptime (HW_ITEM_UPTIME
  // below, millis()-based, needs no hardware capability at all), that's no
  // longer a reason to hide the page on any board, so this is unconditional.
  #define MENU_HAS_HW_PAGE true

  // CPU temperature (pmu_temperature, Power.h) is only ever populated on
  // IS_ESP32S3 boards (via temperatureRead()) and on nRF52 (every nRF52840
  // has an on-die TEMP peripheral, read via readCPUTemperature() - see
  // init_pmu()/measure_temperature(), Power.h). Plain (non-S3) ESP32 boards
  // have no such sensor - HAS_PMU alone doesn't mean a temp reading exists,
  // several boards set it purely for resistor/analogRead battery sensing.
  #define MENU_HAS_CPU_TEMP (IS_ESP32S3 || MCU_VARIANT == MCU_NRF52)

  // Optional top-level items (WiFi, Hardware) shift indices around, so
  // build them up incrementally rather than hardcoding numbers per case.
  #define MENU_ITEM_DISPLAY_TIMEOUT    0
  #define MENU_ITEM_DISPLAY_BRIGHTNESS 1
  #define MENU_ITEM_ORIENTATION        2

  #if HAS_BUZZER == true
    // No point offering a Sound toggle on boards with no buzzer to make any
    // sound with - sound_enabled/snd_conf_save() (Utilities.h) stay
    // unconditional (the KISS CMD_SOUND command is always accepted
    // regardless of hardware), only this menu entry is gated.
    #define MENU_ITEM_SOUND 3
    #define MENU_NEXT_IDX_S 4
  #else
    #define MENU_NEXT_IDX_S 3
  #endif

  #if HAS_ENCODER == true
    // Whether a physical encoder is actually populated - some boards have
    // it PCB-provisioned but optionally installed (MeshAdventurer-S3), or
    // as a DIY add-on that most builds skip (PROMICRO) - so this is a
    // runtime toggle (encoder_enabled, Utilities.h), not the compile-time
    // HAS_ENCODER capability flag. Only changes the on-screen footer hint
    // (turn/press vs tap/hold) - the encoder itself is always serviced
    // regardless, same as before this existed.
    #define MENU_ITEM_ENCODER MENU_NEXT_IDX_S
    #define MENU_NEXT_IDX_SE  (MENU_NEXT_IDX_S + 1)
  #else
    #define MENU_NEXT_IDX_SE MENU_NEXT_IDX_S
  #endif

  #if HAS_ESPNOW == true
    // Opens the ESP-NOW submenu (Enabled + Mode fields, ESPNOW_ITEM_*,
    // MENU_STATE_ESPNOW_LIST/EDIT) - same shape as GNSS/Sensors below.
    // Both fields reboot on change (espnow_conf_save()/espnow_mode_conf_save(),
    // Utilities.h) - ESP-NOW has no runtime start/stop path, only a
    // boot-time espnow_init() call - so like WiFi's Mode field, they're
    // only staged here and actually written by menu_commit_and_exit().
    #define MENU_ITEM_ESPNOW MENU_NEXT_IDX_SE
    #define MENU_NEXT_IDX_0  (MENU_NEXT_IDX_SE + 1)
  #else
    #define MENU_NEXT_IDX_0 MENU_NEXT_IDX_SE
  #endif

  #if HAS_WIFI == true
    #define MENU_ITEM_WIFI  MENU_NEXT_IDX_0
    #define MENU_NEXT_IDX_A (MENU_NEXT_IDX_0 + 1)
  #else
    #define MENU_NEXT_IDX_A MENU_NEXT_IDX_0
  #endif

  #if HAS_ETHERNET == true
    // MeshPoE-S3 only - see Boards.h. Sits right after WiFi.
    #define MENU_ITEM_ETHERNET MENU_NEXT_IDX_A
    #define MENU_NEXT_IDX_A2   (MENU_NEXT_IDX_A + 1)
  #else
    #define MENU_NEXT_IDX_A2 MENU_NEXT_IDX_A
  #endif

  #if HAS_RTC == true
    #define MENU_ITEM_RTC   MENU_NEXT_IDX_A2
    #define MENU_NEXT_IDX_A3 (MENU_NEXT_IDX_A2 + 1)
  #else
    #define MENU_NEXT_IDX_A3 MENU_NEXT_IDX_A2
  #endif

  #if HAS_GPS == true
    #define MENU_ITEM_GNSS   MENU_NEXT_IDX_A3
    #define MENU_NEXT_IDX_A4 (MENU_NEXT_IDX_A3 + 1)
  #else
    #define MENU_NEXT_IDX_A4 MENU_NEXT_IDX_A3
  #endif

  #if HAS_SENSORS == true
    #define MENU_ITEM_SENSORS MENU_NEXT_IDX_A4
    #define MENU_NEXT_IDX_A5   (MENU_NEXT_IDX_A4 + 1)
  #else
    #define MENU_NEXT_IDX_A5 MENU_NEXT_IDX_A4
  #endif

  #if MENU_HAS_HW_PAGE == true
    #define MENU_ITEM_HARDWARE MENU_NEXT_IDX_A5
    #define MENU_NEXT_IDX_B (MENU_NEXT_IDX_A5 + 1)
  #else
    #define MENU_NEXT_IDX_B MENU_NEXT_IDX_A5
  #endif

  #if HAS_OTA == true
    // Network OTA updates (OTA.h) - see MENU_STATE_FWUPD_LIST/
    // MENU_STATE_FWUPD_CONFIRM below.
    #define MENU_ITEM_FW_UPDATE MENU_NEXT_IDX_B
    #define MENU_NEXT_IDX_C (MENU_NEXT_IDX_B + 1)
  #else
    #define MENU_NEXT_IDX_C MENU_NEXT_IDX_B
  #endif

  #define MENU_ITEM_SAVE_EXIT MENU_NEXT_IDX_C
  #define MENU_ITEM_COUNT     (MENU_ITEM_SAVE_EXIT + 1)

  #if HAS_WIFI == true
    #define WIFI_ITEM_MODE     0
    #define WIFI_ITEM_SSID     1
    #define WIFI_ITEM_PSK      2
    // wr_channel (Config.h/ROM.h's ADDR_CONF_WCHN) - shared by AP mode's
    // softAP() call (Remote.h) and ESP-NOW's broadcast peer (ESPNOW.h),
    // same single setting either way. 1-14, stepped/edited the same way as
    // Mode (MENU_STATE_WIFI_EDIT, step_wifi_channel()) - unlike the
    // existing CMD_WIFI_CHN KISS handler (RNode_Firmware.ino), which only
    // touches EEPROM, committing this here also updates the live wr_channel
    // (see menu_commit_and_exit()), so AP mode picks it up immediately via
    // wifi_remote_init() without needing a reboot.
    #define WIFI_ITEM_CHANNEL  3
    // Static IP/netmask (ADDR_CONF_IP/NM, ROM.h) - same all-zero/all-0xFF-
    // means-unset convention as everywhere else (see addr4_read(),
    // Utilities.h). Unlike Mode/SSID/PSK's WIFI_TEXT_EDIT flow, these don't
    // get their own "Save?" confirm dialog - MENU_STATE_WIFI_ADDR_EDIT
    // finishing just updates staged_wifi_ip/nm in RAM, still deferred to
    // SAVE & EXIT like every other field in this list.
    #define WIFI_ITEM_IP       4
    #define WIFI_ITEM_NETMASK  5
    // Gateway/DNS (ADDR_CONF_GW/DNS, ROM.h) - only meaningful once IP/NM
    // are actually static (DHCP already provides both otherwise), but kept
    // as plain always-present fields rather than conditionally hidden -
    // same reasoning as IP/NM themselves. Needed for anything that must
    // leave the local subnet while on a static IP, e.g. rtc_sync_ntp()
    // (RTC.h).
    #define WIFI_ITEM_GATEWAY  6
    #define WIFI_ITEM_DNS      7
    // Single-confirm action (same as MENU_ITEM_SAVE_EXIT), not a field -
    // stages all four back to 0.0.0.0, still deferred to SAVE & EXIT.
    #define WIFI_ITEM_CLEAR    8
    #define WIFI_ITEM_BACK     9
    #define WIFI_ITEM_COUNT    10
  #endif

  #if HAS_ETHERNET == true
    #define ETH_ITEM_LINK_STATUS 0
    #define ETH_ITEM_SPEED       1
    // No separate DHCP/Manual mode flag - same "all-zero or all-0xFF means
    // unset, fall back to DHCP" convention wifi_remote_start_sta() (Remote.h)
    // already uses for its own static IP. Setting IP Address to 0.0.0.0 (or
    // just never touching it) is how you get DHCP here too.
    #define ETH_ITEM_IP           2
    #define ETH_ITEM_NETMASK      3
    // Gateway/DNS (ADDR_CONF_ETH_GW/DNS, ROM.h) - same reasoning as WiFi's
    // own WIFI_ITEM_GATEWAY/DNS above.
    #define ETH_ITEM_GATEWAY      4
    #define ETH_ITEM_DNS          5
    // A single-confirm action, not a field - same as MENU_ITEM_SAVE_EXIT -
    // zeroes all four ADDR_CONF_ETH_IP/NM/GW/DNS back to "unset" (DHCP).
    #define ETH_ITEM_CLEAR        6
    #define ETH_ITEM_BACK         7
    #define ETH_ITEM_COUNT        8
  #endif

  #if HAS_OTA == true
    #define FWUPD_ITEM_CURRENT 0  // read-only - the running build (BUILD_NUMBER)
    #define FWUPD_ITEM_LATEST  1  // read-only - fetched from the update server once, on opening this list (see menu_confirm_select())
    #define FWUPD_ITEM_UPDATE  2  // opens MENU_STATE_FWUPD_CONFIRM - a plain UPDATE/CANCEL list, same pattern as WiFi's SAVE/DISCARD (MENU_STATE_WIFI_TEXT_CONFIRM)
    #define FWUPD_ITEM_BACK    3
    #define FWUPD_ITEM_COUNT   4
  #endif

  #if HAS_RTC == true
    #define RTC_ITEM_TIME     0   // read-only readout, local (Timezone-shifted)
    #define RTC_ITEM_DATE     1   // read-only readout, local (Timezone-shifted)
    #define RTC_ITEM_TIMEZONE 2   // display-only UTC offset - see rtc_get_tz_offset_qh(), RTC.h
    #define RTC_ITEM_SET      3   // opens the sequential Set Time/Date editor
    // Only present where rtc_sync_ntp() (RTC.h) actually compiles - see
    // its own MCU_VARIANT/HAS_WIFI/HAS_ETHERNET guard.
    #if MCU_VARIANT == MCU_ESP32 && (HAS_WIFI == true || HAS_ETHERNET == true)
      #define RTC_ITEM_SYNC_NTP 4
      #define RTC_NEXT_0 5
    #else
      #define RTC_NEXT_0 4
    #endif

    // No MCU_VARIANT/network guard here, unlike Sync NTP above - reading
    // the GNSS receiver's own NMEA date/time (GNSS.h) has no platform or
    // networking dependency at all, see rtc_sync_gps() (RTC.h).
    #if HAS_GPS == true
      #define RTC_ITEM_SYNC_GPS RTC_NEXT_0
      #define RTC_NEXT_1 (RTC_NEXT_0 + 1)
    #else
      #define RTC_NEXT_1 RTC_NEXT_0
    #endif

    #define RTC_ITEM_BACK  RTC_NEXT_1
    #define RTC_ITEM_COUNT (RTC_ITEM_BACK + 1)
  #endif

  #if HAS_GPS == true
    #define GNSS_ITEM_ENABLED    0   // editable, immediate-commit toggle - power-cycles PIN_GPS_EN live
    // Auto-detected module presence (gnss_module_status_text(), GNSS.h) -
    // shows the chip name once real NMEA bytes have been seen since
    // Enabled last went true, "DETECTING..." during the first
    // GNSS_DETECT_MAX_ATTEMPTS probe attempts, or "NOT DETECTED" once
    // those are exhausted with nothing received. Matters most on boards
    // where the receiver is an optional add-on (MeshAdventurer-S3's
    // ATGM336H) - otherwise turning Enabled on with no module wired would
    // just show permanently-zero Fix/Satellites, indistinguishable from
    // "no sky view yet".
    #define GNSS_ITEM_MODULE     1   // read-only
    #define GNSS_ITEM_FIX        2   // read-only
    #define GNSS_ITEM_SATELLITES 3   // read-only
    #define GNSS_ITEM_LATITUDE   4   // read-only
    #define GNSS_ITEM_LONGITUDE  5   // read-only
    #define GNSS_ITEM_ALTITUDE   6   // read-only
    // GNSS time (UTC) - populates independently of Fix/location (see
    // gnss_time_valid(), GNSS.h) - a receiver typically syncs time before
    // ever achieving a position fix, so this is a genuine diagnostic: Time
    // valid but Fix/Satellites still 0 confirms sentence parsing works
    // end-to-end and it's an antenna/sky-visibility issue, not firmware.
    #define GNSS_ITEM_TIME       7   // read-only
    // Raw link-health counters (gnss_chars_processed()/checksum_passed()/
    // failed(), GNSS.h) - genuine bring-up diagnostics for any HAS_GPS
    // board, not a one-off debug hack: distinguishes "MCU never receives
    // anything" (wrong pins/baud/power) from "receiving garbage" (baud
    // mismatch) from "valid data, chip just isn't getting a fix" (antenna/
    // hardware, not firmware).
    #define GNSS_ITEM_NMEA_CHARS 8   // read-only
    #define GNSS_ITEM_NMEA_CKSUM 9   // read-only - "passed/failed"
    #define GNSS_ITEM_BACK       10
    #define GNSS_ITEM_COUNT      11
  #endif

  #if HAS_ESPNOW == true
    #define ESPNOW_ITEM_ENABLED 0   // editable - staged only, no self-reboot until SAVE & EXIT
    #define ESPNOW_ITEM_MODE    1   // editable - "v1" (classic chunked) / "v2" (unfragmented) - framing only
    #define ESPNOW_ITEM_LR      2   // editable - 802.11 LR mode ON/OFF - PHY rate only, independent of Mode
    // Read-only - same wr_channel WiFi's own Channel field edits (WIFI_ITEM_CHANNEL,
    // Remote.h/ESPNOW.h both use it), not a separate value. Shown here purely
    // for visibility while looking at ESP-NOW's own settings - deliberately not
    // a second editable control for the same byte, see menu_confirm_select()'s
    // own comment on why. Reads the live value directly (not staged/committed
    // through this submenu at all), same as HW_LIST's read-only info rows.
    #define ESPNOW_ITEM_CHANNEL 3
    #define ESPNOW_ITEM_BACK    4
    #define ESPNOW_ITEM_COUNT   5
  #endif

  #if HAS_SENSORS == true
    // Fully read-only - no editable fields, so unlike GNSS's own list above
    // there's no matching MENU_STATE_SENSORS_EDIT, only BACK does anything
    // on confirm (see menu_confirm_select()). Humidity/Pressure read N/A on
    // a BMP280-only board (see sensor_model, Sensors.h) - BMP280 has no
    // humidity element at all, and this firmware doesn't ship a plain
    // BMP180-style pressure-only path.
    #define SENSORS_ITEM_TEMP     0   // read-only
    #define SENSORS_ITEM_HUMIDITY 1   // read-only - N/A on BMP280
    #define SENSORS_ITEM_PRESSURE 2   // read-only
    #define SENSORS_ITEM_MODEL    3   // read-only - sensor_chip_name(), Sensors.h
    #define SENSORS_ITEM_BACK     4
    #define SENSORS_ITEM_COUNT    5
  #endif

  #if MENU_HAS_HW_PAGE == true
    #if MENU_HAS_CPU_TEMP == true
      #define HW_ITEM_TEMP 0
      #define HW_NEXT_0    1
    #else
      #define HW_NEXT_0 0
    #endif

    #if HAS_VSENSE == true
      #define HW_ITEM_VOLTAGE HW_NEXT_0
      #define HW_NEXT_A       (HW_NEXT_0 + 1)
    #else
      #define HW_NEXT_A HW_NEXT_0
    #endif

    #if HAS_BATTERY_DIVIDER == true
      #define HW_ITEM_BATTERY HW_NEXT_A
      #define HW_NEXT_A2      (HW_NEXT_A + 1)
    #else
      #define HW_NEXT_A2 HW_NEXT_A
    #endif

    // No GPS chip-identification item here (removed - redundant with the
    // GNSS page's own Module row, GNSS_ITEM_MODULE, which shows the exact
    // same gnss_module_status_text() value plus the live Enabled/Fix/etc.
    // fields it belongs alongside).
    #define HW_NEXT_A3 HW_NEXT_A2

    #if HAS_WIFI == true
      #define HW_ITEM_WIFI_IP  HW_NEXT_A3
      #define HW_ITEM_WIFI_NM  (HW_NEXT_A3 + 1)
      #define HW_ITEM_WIFI_MAC (HW_NEXT_A3 + 2)
      #define HW_NEXT_B        (HW_NEXT_A3 + 3)
    #else
      #define HW_NEXT_B HW_NEXT_A3
    #endif

    #if HAS_BLUETOOTH == true || HAS_BLE == true
      #define HW_ITEM_BT_MAC HW_NEXT_B
      #define HW_NEXT_C      (HW_NEXT_B + 1)
    #else
      #define HW_NEXT_C HW_NEXT_B
    #endif

    #if HAS_ETHERNET == true
      #define HW_ITEM_ETH_MAC HW_NEXT_C
      #define HW_NEXT_C2      (HW_NEXT_C + 1)
    #else
      #define HW_NEXT_C2 HW_NEXT_C
    #endif

    #if HAS_GPIO_MENU == true
      #define HW_ITEM_GPIO HW_NEXT_C2
      #define HW_NEXT_D    (HW_NEXT_C2 + 1)
    #else
      #define HW_NEXT_D HW_NEXT_C2
    #endif

    // Heap/PSRAM diagnostics - ESP.getFreeHeap()/getPsramSize()/etc are
    // Arduino-ESP32-specific, so this row (and the submenu it opens) is
    // gated on the MCU, not on anything board-specific - unlike most other
    // HW_ITEM_* rows it isn't tied to a particular board's wiring, so it
    // shows on every MCU_ESP32 board that reaches the Hardware page at all
    // (T096/T114/MeshPoE-S3/MeshAdventurer-S3/Heltec32_v4). psramFound() is
    // checked at runtime (not a compile-time PSRAM-enabled guard) so boards
    // without PSRAM wired/enabled just show "N/A" instead of needing their
    // own #if branch here.
    #if MCU_VARIANT == MCU_ESP32
      #define HW_ITEM_MEMORY HW_NEXT_D
      #define HW_NEXT_E      (HW_NEXT_D + 1)
    #else
      #define HW_NEXT_E HW_NEXT_D
    #endif

    // Time since boot, HH:MM:SS - needs no hardware capability at all
    // (millis()-based, same source Display.h's own draw_node_uptime()
    // already uses on T114), so unlike every row above it isn't gated on
    // anything board-specific - always the last real entry before BACK,
    // on every board that reaches this page (which as of MENU_HAS_HW_PAGE
    // above is now every board, period).
    #define HW_ITEM_UPTIME HW_NEXT_E
    #define HW_NEXT_F      (HW_NEXT_E + 1)

    #define HW_ITEM_BACK  HW_NEXT_F
    #define HW_ITEM_COUNT (HW_ITEM_BACK + 1)

    #if HAS_GPIO_MENU == true
      #define GPIO_ITEM_BUZZER 0
      #if HAS_ENCODER == true
        #define GPIO_ITEM_ENC_UP    1
        #define GPIO_ITEM_ENC_DOWN  2
        #define GPIO_ITEM_ENC_PRESS 3
        #define GPIO_NEXT_0 4
      #else
        #define GPIO_NEXT_0 1
      #endif
      #define GPIO_ITEM_BACK  GPIO_NEXT_0
      #define GPIO_ITEM_COUNT (GPIO_ITEM_BACK + 1)
    #endif

    #if MCU_VARIANT == MCU_ESP32
      // Both rows are drawn as bar graphs, not text - selecting either one
      // (not BACK) drops into MENU_STATE_MEM_DETAIL, a plain text readout
      // of that metric, same "list row opens a submenu" pattern as
      // HW_ITEM_GPIO -> MENU_STATE_GPIO_LIST.
      #define MEM_ITEM_HEAP  0
      #define MEM_ITEM_PSRAM 1
      #define MEM_ITEM_BACK  2
      #define MEM_ITEM_COUNT 3

      // Detail screen - fully read-only, only BACK does anything on
      // confirm. Which metric (Heap vs PSRAM) it's showing is tracked by
      // mem_menu_cursor staying at MEM_ITEM_HEAP/MEM_ITEM_PSRAM while this
      // state is active, same reuse-the-parent-cursor pattern HW_EDIT uses
      // for hw_menu_cursor (HW_ITEM_VOLTAGE vs HW_ITEM_BATTERY).
      #define MEM_DETAIL_ITEM_TOTAL   0
      #define MEM_DETAIL_ITEM_USED    1
      #define MEM_DETAIL_ITEM_FREE    2
      #define MEM_DETAIL_ITEM_MINFREE 3
      #define MEM_DETAIL_ITEM_BACK    4
      #define MEM_DETAIL_ITEM_COUNT   5
    #endif
  #endif

  uint8_t menu_state      = MENU_STATE_CLOSED;
  uint8_t menu_cursor     = 0;
  uint8_t menu_edit_field = 0;

  // Main-button double-tap detection for menu_button_press(): a short tap
  // is held pending for MENU_BTN_DOUBLE_TAP_WINDOW ms in case a second one
  // follows (see menu_button_process(), polled from loop()) - if it does,
  // the pair is treated as "go backward" instead of two forward steps.
  #define MENU_BTN_DOUBLE_TAP_WINDOW 200
  bool menu_btn_pending = false;
  unsigned long menu_btn_last_click = 0;

  // Idle-close watchdog (SETTINGS_MENU_TIMEOUT, Config.h) - bumped on every
  // real button/encoder input (see menu_button_press()/menu_encoder_rotate()/
  // menu_encoder_button()), checked from menu_timeout_process() (polled from
  // loop(), same as menu_button_process()).
  unsigned long menu_last_activity_ms = 0;

  // Generic transient status box - a plain centered rectangle, not a
  // navigable submenu (no title/footer chrome, no selectable items).
  // Reused by several unrelated features that just need to show a brief
  // message on top of whatever's currently on screen: the menu-open popup
  // below (Sync NTP/Sync GPS's progress/result, Clear Static's confirmation -
  // WiFi/Ethernet/GPS+RTC-specific, see MENU_STATE_STATUS_POPUP further down) and
  // button_hold_process()'s main-button-hold feedback (menu-closed,
  // applies to every HAS_MENU board regardless of WiFi/Ethernet) - hence
  // living here, ungated, rather than under either feature's own #if.
  #if HAS_INPUT == true || HAS_WIFI == true || HAS_ETHERNET == true
    #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114
      // Same footprint-tracking idea as the generic branch below, but in
      // panel (not canvas-local) coordinates, since the box is pushed
      // through menu_popup_canvas (Display.h) at whatever panel offset
      // centers it - see push_menu_popup_canvas().
      uint16_t menu_popup_prev_panel_x = 0, menu_popup_prev_panel_y = 0;
      uint16_t menu_popup_prev_panel_w = 0, menu_popup_prev_panel_h = 0;
      bool menu_popup_prev_panel_valid = false;

      // Erases the last-pushed box, if any, by forcing a real repaint of
      // whatever's actually supposed to be under its footprint - not by
      // painting it black, which would just swap one wrong thing (a
      // stale box) for another (a hole) instead of restoring the real
      // operational-screen content that belongs there. Needed because,
      // unlike the generic branch below, nothing here relies on a
      // periodic full-screen clear to make a box disappear on its own -
      // T096/T114 only ever repaint the panel regions they explicitly
      // push to. Called by draw_button_hold_overlay() whenever a hold
      // ends without a new box replacing this one - including well after
      // the box was last drawn, e.g. right after exiting a menu session a
      // hold escalated into opening (menu_popup_prev_panel_* is never
      // touched again once menu_is_open() goes true, so it's still
      // sitting on coordinates from before the menu ever opened).
      void menu_status_rect_clear() {
        if (menu_popup_prev_panel_valid) {
          for (uint8_t i = 0; i < REGION_CACHE_SLOTS; i++) region_cache[i].x = -1;
          #if USE_COLOR_DISPLAY == true
            cdirty_count = 0;
          #endif
          update_stat_area();
          update_disp_area();
          menu_popup_prev_panel_valid = false;
        }
      }
    #else
      // Remembers the last-drawn box's footprint so a later, narrower/
      // shorter message can erase just that area instead of the whole
      // screen - see draw_menu_status_rect().
      uint16_t menu_popup_prev_x = 0, menu_popup_prev_y = 0;
      uint16_t menu_popup_prev_w = 0, menu_popup_prev_h = 0;
      bool menu_popup_prev_valid = false;
    #endif

    // Superimposes the box on whatever's already in the display buffer -
    // no clearDisplay() here. Only erases its own previous footprint (if
    // any), not the whole screen, so a shrinking message (e.g.
    // "CONNECTING" -> "SYNCED!") doesn't leave stale pixels poking out
    // around a narrower new box. Box height (11) and baseline offset (+7)
    // reuse draw_menu_list_disp()'s own row_h/baseline convention - the
    // same font at the same tightness, already proven to fit cleanly.
    void draw_menu_status_rect(const char *text) {
      #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114
        // Only ever reached today via draw_button_hold_overlay() (menu
        // closed) - the menu-open popup path (Sync NTP, Clear Static)
        // needs HAS_WIFI/HAS_ETHERNET, which neither board has. Draws
        // into its own small canvas (menu_popup_canvas, Display.h, kept
        // within drawBitmap()'s region-cache cutoff) rather than
        // menu_canvas - that canvas is sized and positioned for the
        // list-screen case only, not this overlay.
        menu_popup_canvas.setFont(SMALL_FONT);
        menu_popup_canvas.setTextWrap(false);
        menu_popup_canvas.setTextSize(1);
        menu_popup_canvas.setTextColor(SSD1306_WHITE);

        int16_t x1, y1; uint16_t tw, th;
        menu_popup_canvas.getTextBounds(text, 0, 0, &x1, &y1, &tw, &th);

        const uint16_t pad_l = 3;
        const uint16_t pad_r = 3;
        uint16_t box_w = tw + pad_l + pad_r;
        if (box_w > MENU_POPUP_CANVAS_W) box_w = MENU_POPUP_CANVAS_W;
        const uint16_t box_h = 11;
        // Centered over the full panel (not this narrower canvas) at
        // whatever orientation is currently active - unlike the menu
        // itself (always landscape, menu_canvas), this overlay only
        // ever runs while the menu is closed (draw_button_hold_overlay()),
        // so the panel is showing the normal operational screen at
        // whatever rotation the user's Orientation setting picked -
        // portrait (80x160) just as often as landscape (160x80).
        // display.width()/height() already reflect the live rotation,
        // so this adapts automatically instead of assuming landscape.
        uint16_t panel_x = (display.width() - box_w) / 2;
        uint16_t panel_y = (display.height() - box_h) / 2;

        if (menu_popup_prev_panel_valid &&
            (menu_popup_prev_panel_x != panel_x || menu_popup_prev_panel_y != panel_y ||
             menu_popup_prev_panel_w != box_w || menu_popup_prev_panel_h != box_h)) {
          // Force a real repaint of whatever's actually supposed to be
          // under the old, wider footprint - not just paint it black
          // (same reasoning as menu_status_rect_clear()). Painting black
          // here would leave a solid black margin around a narrower
          // replacement box instead of restoring the real operational-
          // screen content there, e.g. shrinking from "BT PAIRING" to
          // the narrower "SLEEP" mid-hold.
          for (uint8_t i = 0; i < REGION_CACHE_SLOTS; i++) region_cache[i].x = -1;
          #if USE_COLOR_DISPLAY == true
            cdirty_count = 0;
          #endif
          update_stat_area();
          update_disp_area();
        }

        menu_popup_canvas.fillScreen(SSD1306_BLACK);
        menu_popup_canvas.drawRect(0, 0, box_w, box_h, SSD1306_WHITE);
        menu_popup_canvas.setCursor(pad_l - x1, 7);
        menu_popup_canvas.print(text);
        push_menu_popup_canvas(panel_x, panel_y, box_w, box_h);

        menu_popup_prev_panel_x = panel_x; menu_popup_prev_panel_y = panel_y;
        menu_popup_prev_panel_w = box_w; menu_popup_prev_panel_h = box_h;
        menu_popup_prev_panel_valid = true;
      #else
        if (menu_popup_prev_valid) {
          display.fillRect(menu_popup_prev_x, menu_popup_prev_y, menu_popup_prev_w, menu_popup_prev_h, SSD1306_BLACK);
        }

        display.setFont(SMALL_FONT);
        display.setTextWrap(false);
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);

        int16_t x1, y1; uint16_t tw, th;
        display.getTextBounds(text, 0, 0, &x1, &y1, &tw, &th);

        const uint16_t pad_l = 3;
        const uint16_t pad_r = 3;
        uint16_t box_w = tw + pad_l + pad_r;
        // Clamp against the live panel width, not a hardcoded 128px
        // landscape assumption - display.width()/height() already
        // reflect the current rotation (see the T096 branch above),
        // so this stays correct in portrait orientations too, where
        // the panel can be much narrower than 128px.
        if (box_w > display.width() - 8) box_w = display.width() - 8;
        const uint16_t box_h = 11;
        uint16_t box_x = (display.width() - box_w) / 2;
        const uint16_t box_y = (display.height() - box_h) / 2;

        display.fillRect(box_x, box_y, box_w, box_h, SSD1306_BLACK);
        display.drawRect(box_x, box_y, box_w, box_h, SSD1306_WHITE);
        // Subtracting x1 (the left bearing getTextBounds() reports for this
        // specific string) puts the actual rendered ink pad_l pixels past
        // the box's left edge, not just the raw cursor position - some
        // strings (e.g. "SYNCED!") have a nonzero left bearing that would
        // otherwise throw this off by a pixel or two.
        display.setCursor(box_x + pad_l - x1, box_y + 7);
        display.print(text);

        menu_popup_prev_x = box_x; menu_popup_prev_y = box_y;
        menu_popup_prev_w = box_w; menu_popup_prev_h = box_h;
        menu_popup_prev_valid = true;
      #endif
    }
  #endif

  #if HAS_INPUT == true
    // Live feedback for the main button's hold-duration tiers
    // (button_event(), RNode_Firmware.ino) - which action fires is
    // otherwise only revealed at release, with no indication beforehand
    // of what a given hold length is about to trigger. Shows the box for
    // whichever tier the current hold has reached, updating it live if
    // held further into the next one - lets the user actually see what
    // they're about to do and release (or keep holding) accordingly.
    // Menu-closed only - button_event()'s tiers themselves only apply
    // then (see its own menu_is_open() check), and this reuses
    // draw_menu_status_rect() directly rather than going through the
    // menu-open popup machinery above, which doesn't apply here either.
    #define BUTTON_HOLD_TIER_NONE       0
    #define BUTTON_HOLD_TIER_SLEEP      1
    #define BUTTON_HOLD_TIER_SETTINGS   2
    #define BUTTON_HOLD_TIER_BT_PAIRING 3
    #define BUTTON_HOLD_TIER_CONSOLE    4

    // Mirrors button_event()'s own duration thresholds exactly - keep the
    // two in sync if those ever change.
    uint8_t button_hold_tier(unsigned long held_ms) {
      #if HAS_CONSOLE
        if (held_ms > 10000) return BUTTON_HOLD_TIER_CONSOLE;
      #endif
      #if HAS_SLEEP
        // Past SLEEP_HOLD_CANCEL_MS the box disappears (NONE, not a
        // fall-through to the lower Bt Pairing/Settings tiers below -
        // this duration already committed to the Sleep tier, it doesn't
        // un-commit into a different one) - matches button_event()'s own
        // cutoff for actually triggering sleep on release, see
        // SLEEP_HOLD_CANCEL_MS's own comment (Config.h) for why.
        if (held_ms > 7000) {
          return (held_ms <= 7000 + SLEEP_HOLD_CANCEL_MS) ? BUTTON_HOLD_TIER_SLEEP : BUTTON_HOLD_TIER_NONE;
        }
      #endif
      #if HAS_BLUETOOTH || HAS_BLE
        if (held_ms > 5000) return BUTTON_HOLD_TIER_BT_PAIRING;
      #endif
      // HAS_MENU is implicitly true here - this whole file only compiles
      // when it is.
      if (held_ms > 3000) return BUTTON_HOLD_TIER_SETTINGS;
      return BUTTON_HOLD_TIER_NONE;
    }

    const char *button_hold_tier_text(uint8_t tier) {
      if      (tier == BUTTON_HOLD_TIER_SLEEP)      return "SLEEP";
      else if (tier == BUTTON_HOLD_TIER_SETTINGS)   return "SETTINGS";
      else if (tier == BUTTON_HOLD_TIER_BT_PAIRING) return "BT PAIRING";
      else if (tier == BUTTON_HOLD_TIER_CONSOLE)    return "CONSOLE";
      return "";
    }

    // Called from update_display()'s own normal (menu-closed) redraw path
    // (Display.h), every cycle - NOT a one-shot draw. update_display()
    // unconditionally clears and redraws the whole screen before getting
    // here (same reason draw_settings_menu_disp()'s own
    // MENU_STATE_STATUS_POPUP case has to redraw its underlying content
    // every cycle too - see that comment), so a box only drawn once when
    // the tier first changes gets wiped by the very next ordinary refresh
    // - it has to be redrawn every cycle for as long as the tier's active
    // to actually stay visible. No manual display.display() push needed
    // here (unlike menu_draw_popup()) - this runs as part of the normal
    // per-cycle pipeline, which already pushes once at the end.
    // Tracks the last tier a beep was already played for, across this
    // function's repeated per-cycle calls during a single hold - without
    // this, the tick would replay every redraw for as long as a tier stays
    // active instead of once when it's first reached. Reset to NONE
    // whenever the hold isn't actively progressing through tiers (released,
    // or the menu opened out from under it), so the next hold starts fresh.
    uint8_t button_hold_beeped_tier = BUTTON_HOLD_TIER_NONE;

    void draw_button_hold_overlay() {
      if (menu_is_open() || !button_pressed()) {
        button_hold_beeped_tier = BUTTON_HOLD_TIER_NONE;
        #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114
          // Neither board has a periodic full-screen clear to make a
          // leftover box disappear on its own (see
          // menu_status_rect_clear()'s own comment) - explicitly erase it
          // once the hold that drew it ends, whether that's a release or
          // the menu having opened.
          menu_status_rect_clear();
        #endif
        return;
      }
      uint8_t tier = button_hold_tier(millis() - button_down_last);
      if (tier != BUTTON_HOLD_TIER_NONE) {
        // One tick per newly-reached tier, not per redraw - buzzer_encoder_
        // tick_melody() already no-ops with Sound off (sound_enabled) or on
        // boards with no buzzer at all (HAS_BUZZER, Utilities.h).
        if (tier != button_hold_beeped_tier) {
          buzzer_encoder_tick_melody();
          button_hold_beeped_tier = tier;
        }
        draw_menu_status_rect(button_hold_tier_text(tier));
      } else {
        button_hold_beeped_tier = BUTTON_HOLD_TIER_NONE;
        #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114
          menu_status_rect_clear();
        #endif
      }
    }
  #endif

  // Sync GPS (RTC.h/rtc_sync_gps()) only ever needs this where HAS_RTC is
  // also true - HAS_GPS alone (e.g. T096/T114, no HAS_RTC) would pull in
  // this block for nothing, and its T096/T114-specific footprint-tracking
  // branch just below has never been built against a board with no
  // WiFi/Ethernet of its own, so guarding on bare HAS_GPS risks surfacing
  // bugs in a path nothing would actually use.
  #if HAS_WIFI == true || HAS_ETHERNET == true || (HAS_GPS == true && HAS_RTC == true)
    // Menu-open popup state (MENU_STATE_STATUS_POPUP) - dismissed by any
    // input, returning to menu_popup_return_state. menu_button_press()/
    // menu_encoder_button()/menu_encoder_rotate() each special-case
    // MENU_STATE_STATUS_POPUP before their normal tap/hold/rotate
    // dispatch - see those functions.
    char menu_popup_text[24] = {0};
    uint8_t menu_popup_return_state = MENU_STATE_LIST;

    // Absolute millis() deadline for auto-dismissing the currently-open
    // popup - 0 means "not armed" (the normal case: stays up until the
    // user dismisses it). Checked by menu_popup_process(), polled from
    // loop() same as menu_button_process()/menu_timeout_process().
    unsigned long menu_popup_auto_dismiss_at = 0;

    // Draws AND immediately pushes to hardware, bypassing the normal
    // throttled update_display() path - needed for anything that blocks
    // loop() while showing progress (e.g. rtc_sync_ntp(), RTC.h, which
    // this is also passed to directly as its status_cb), since nothing
    // else would ever redraw the screen during that block otherwise. Safe
    // to use for a one-shot message too (e.g. Clear Static) - just draws
    // once.
    void menu_draw_popup(const char *text) {
      strncpy(menu_popup_text, text, 23); menu_popup_text[23] = 0;
      draw_menu_status_rect(menu_popup_text);
      display.display();
    }

    // Opens the popup (or updates it if already open) showing `text`,
    // returning to `return_state` once the user dismisses it.
    void menu_open_popup(const char *text, uint8_t return_state) {
      menu_popup_return_state = return_state;
      menu_state = MENU_STATE_STATUS_POPUP;
      // Fresh session - don't erase a footprint left over from wherever
      // the box happened to be the last time the popup was used.
      menu_popup_prev_valid = false;
      menu_popup_auto_dismiss_at = 0; // no auto-dismiss unless armed separately, see menu_draw_popup_timed()
      menu_draw_popup(text);
    }

    // Draws `text` on the already-open popup (same as menu_draw_popup())
    // and arms it to auto-dismiss after `ms` with no input needed - used
    // for outcomes that don't need acknowledging (e.g. a successful Sync
    // NTP), as opposed to the default behavior (stays up until dismissed)
    // used for anything worth making sure the user actually saw (errors).
    void menu_draw_popup_timed(const char *text, unsigned long ms) {
      menu_draw_popup(text);
      menu_popup_auto_dismiss_at = millis() + ms;
    }

    // Polled from loop() - auto-dismisses the popup once its timer (see
    // menu_draw_popup_timed()) expires, with no button/encoder input
    // needed. Doesn't touch menu_last_activity_ms - this isn't real user
    // activity, so it shouldn't reset the whole menu's own idle-close
    // watchdog (SETTINGS_MENU_TIMEOUT, menu_timeout_process()).
    void menu_popup_process() {
      if (menu_state == MENU_STATE_STATUS_POPUP && menu_popup_auto_dismiss_at != 0 && millis() >= menu_popup_auto_dismiss_at) {
        menu_popup_auto_dismiss_at = 0;
        menu_state = menu_popup_return_state;
      }
    }
  #endif

  uint8_t staged_display_timeout    = 0;   // seconds, 0-255, 0 = OFF
  uint8_t staged_display_brightness = 0;   // 0-255, raw SSD1306 contrast
  uint8_t staged_display_rotation   = 0;   // 0-3 = 0/90/180/270 degrees
  // Brightness is applied live the moment it's confirmed (see
  // menu_encoder_button()), unlike every other field which only takes
  // effect on the final commit - this is the baseline to diff against at
  // commit time so a live-applied value still gets persisted to EEPROM
  // (display_intensity itself no longer reflects "unchanged" by then).
  uint8_t live_display_brightness   = 0;
  #if HAS_BUZZER == true
    bool staged_sound_enabled = true;
  #endif
  #if HAS_ESPNOW == true
    uint8_t espnow_menu_cursor = 0;
    bool staged_espnow_enabled = true;
    bool staged_espnow_mode_v2 = false;  // false = v1 (default), true = v2 - framing only
    bool staged_espnow_lr_enabled = false; // 802.11 LR mode - PHY rate only, independent of the above
    // 0 = "Disables WiFi" info row (inert, same read-only-row convention as
    // FWUPD_LIST's CURRENT/LATEST or GNSS's Fix/Satellites - selectable but
    // does nothing on confirm, just extra context in the space a 4-row
    // list leaves free), 1 = ENABLE, 2 = CANCEL - same list-with-cursor
    // pattern as fwupd_confirm_cursor, defaulting to CANCEL (2) for the
    // same reason (this is the consequential choice, not the primary/
    // expected one).
    uint8_t espnow_lr_confirm_cursor = 2;
  #endif
  #if HAS_ENCODER == true
    bool staged_encoder_enabled = false;
  #endif
  #if HAS_WIFI == true
    uint8_t wifi_menu_cursor = 0;
    uint8_t staged_wifi_mode = WR_WIFI_OFF;
    uint8_t staged_wifi_channel = WR_CHANNEL_DEFAULT;
    char    staged_wifi_ssid[33] = {0};
    char    staged_wifi_psk[33]  = {0};
    // Snapshot of the actual EEPROM contents taken at the same time as
    // staged_wifi_ssid/psk (see menu_stage_from_live()) - compared against
    // at commit time instead of wr_ssid/wr_psk, which are only populated by
    // wifi_remote_init() and stay stale/empty if WiFi boots in OFF mode.
    char    live_wifi_ssid[33] = {0};
    char    live_wifi_psk[33]  = {0};

    // Same staged/live split as SSID/PSK above (deferred to SAVE & EXIT,
    // see menu_commit_and_exit()) - not immediate-commit like Ethernet's
    // own IP Address/Netmask, since Mode/SSID/PSK in this same submenu
    // already establish the deferred pattern. wifi_addr_octet_idx is
    // shared by both fields, same as eth_addr_octet_idx.
    uint8_t staged_wifi_ip[4] = {0, 0, 0, 0};
    uint8_t staged_wifi_nm[4] = {0, 0, 0, 0};
    uint8_t staged_wifi_gw[4] = {0, 0, 0, 0};
    uint8_t staged_wifi_dns[4] = {0, 0, 0, 0};
    uint8_t live_wifi_ip[4]   = {0, 0, 0, 0};
    uint8_t live_wifi_nm[4]   = {0, 0, 0, 0};
    uint8_t live_wifi_gw[4]   = {0, 0, 0, 0};
    uint8_t live_wifi_dns[4]  = {0, 0, 0, 0};
    uint8_t wifi_addr_octet_idx = 0;

    // Which staged_wifi_* array a WIFI_ITEM_IP/NETMASK/GATEWAY/DNS value
    // maps to - shared by menu_encoder_rotate()/menu_confirm_select()/
    // draw_settings_menu_disp() so the 4-way choice lives in one place.
    uint8_t *wifi_staged_addr_field(uint8_t item) {
      if (item == WIFI_ITEM_IP)      return staged_wifi_ip;
      if (item == WIFI_ITEM_NETMASK) return staged_wifi_nm;
      if (item == WIFI_ITEM_GATEWAY) return staged_wifi_gw;
      return staged_wifi_dns; // WIFI_ITEM_DNS
    }

    // Working state while actively in MENU_STATE_WIFI_TEXT_EDIT, reset fresh
    // every time SSID or PSK is opened from the WiFi list.
    uint8_t text_edit_field = 0;   // WIFI_ITEM_SSID or WIFI_ITEM_PSK
    char    text_edit_buf[33] = {0};
    uint8_t wheel_index = 0;
    uint8_t text_confirm_cursor = 0;   // 0 = SAVE, 1 = DISCARD
  #endif

  #if HAS_ETHERNET == true
    uint8_t eth_menu_cursor = 0;
    // Synced fresh from the live value on entering MENU_STATE_ETH_EDIT (see
    // menu_confirm_select()), not staged at whole-menu-open time like most
    // fields - this one commits to EEPROM (and reboots if changed) the
    // moment you back out of its own edit screen, same immediate-commit
    // pattern as the Hardware page's Voltage Divider Ratio/GPIO pin fields.
    uint8_t staged_eth_speed_mode = ETH_SPEED_AUTO;
    // Same immediate-commit-on-its-own-screen pattern as staged_eth_speed_mode
    // above. Shared by both IP Address and Netmask - which one's being
    // edited is always whichever eth_menu_cursor pointed at when
    // MENU_STATE_ETH_ADDR_EDIT was entered.
    uint8_t staged_eth_ip[4]      = {0, 0, 0, 0};
    uint8_t staged_eth_nm[4]      = {0, 0, 0, 0};
    uint8_t staged_eth_gw[4]      = {0, 0, 0, 0};
    uint8_t staged_eth_dns[4]     = {0, 0, 0, 0};
    uint8_t eth_addr_octet_idx    = 0;

    // Which staged_eth_*/ADDR_CONF_ETH_* pair an ETH_ITEM_IP/NETMASK/
    // GATEWAY/DNS value maps to - same purpose as wifi_staged_addr_field()
    // above, plus the EEPROM address (Ethernet's fields commit immediately,
    // so callers need both).
    uint8_t *eth_staged_addr_field(uint8_t item) {
      if (item == ETH_ITEM_IP)      return staged_eth_ip;
      if (item == ETH_ITEM_NETMASK) return staged_eth_nm;
      if (item == ETH_ITEM_GATEWAY) return staged_eth_gw;
      return staged_eth_dns; // ETH_ITEM_DNS
    }

    int eth_addr_base(uint8_t item) {
      if (item == ETH_ITEM_IP)      return ADDR_CONF_ETH_IP;
      if (item == ETH_ITEM_NETMASK) return ADDR_CONF_ETH_NM;
      if (item == ETH_ITEM_GATEWAY) return ADDR_CONF_ETH_GW;
      return ADDR_CONF_ETH_DNS; // ETH_ITEM_DNS
    }
  #endif

  #if HAS_RTC == true
    uint8_t rtc_menu_cursor = 0;
    // Working copy while inside MENU_STATE_RTC_EDIT (Set Time/Date) - synced
    // fresh from the live RTC reading on entry (see menu_confirm_select()),
    // not staged at whole-menu-open time, since this commits straight to the
    // RTC chip on its own confirm rather than deferring to SAVE & EXIT, same
    // immediate-commit pattern as Ethernet's own IP Address/Speed above.
    int32_t staged_rtc_year   = 2000;
    uint8_t staged_rtc_month  = 1;
    uint8_t staged_rtc_day    = 1;
    uint8_t staged_rtc_hour   = 0;
    uint8_t staged_rtc_minute = 0;
    uint8_t staged_rtc_second = 0;
    // Which of the 6 fields above is currently being adjusted - 0=Year,
    // 1=Month, 2=Day, 3=Hour, 4=Minute, 5=Second (see step_rtc_field()).
    uint8_t rtc_edit_field_idx = 0;

    // Working copy while inside MENU_STATE_RTC_TZ_EDIT - quarter-hours
    // from UTC, synced fresh from the live value on entry (see
    // rtc_get_tz_offset_qh(), RTC.h) and committed immediately on
    // confirm (tz_conf_save(), Utilities.h) - display-only, nothing to
    // reboot or re-init, same reasoning as Ethernet's Speed field.
    int8_t staged_tz_offset_qh = 0;
  #endif

  #if HAS_GPS == true
    uint8_t gnss_menu_cursor = 0;
    // Working copy while inside MENU_STATE_GNSS_EDIT - synced fresh from
    // the live gnss_enabled value on entry (see menu_confirm_select()), not
    // staged at whole-menu-open time, since this commits immediately (and
    // live power-cycles the receiver) on confirm rather than deferring to
    // SAVE & EXIT - same immediate-commit pattern as RTC's own Timezone
    // field above.
    bool staged_gnss_enabled = true;
  #endif

  #if HAS_SENSORS == true
    uint8_t sensors_menu_cursor = 0;
  #endif

  #if MENU_HAS_HW_PAGE == true
    uint8_t hw_menu_cursor = 0;
    #if MCU_VARIANT == MCU_ESP32
      uint8_t mem_menu_cursor = 0;
      uint8_t mem_detail_cursor = 0;
    #endif
    #if HAS_VSENSE == true
      // Raw EEPROM format (ratio*10, see vsr_conf_save()/Utilities.h) -
      // synced fresh from the live value on entering MENU_STATE_HW_EDIT
      // (see menu_confirm_select()), not staged at whole-menu-open time
      // like every other field, since this one commits to EEPROM the
      // moment you back out of its own edit screen rather than waiting
      // for SAVE & EXIT.
      uint8_t staged_vsense_divider_ratio_raw = 0;
    #endif
    #if HAS_BATTERY_DIVIDER == true
      // Same immediate-commit pattern as staged_vsense_divider_ratio_raw
      // above, but a %/of-default correction (see bvs_conf_save(),
      // Utilities.h) rather than a raw ratio*10.
      uint8_t staged_battery_v_scale_pct = 100;
    #endif
    #if HAS_GPIO_MENU == true
      uint8_t gpio_menu_cursor = 0;
      // Index into gpio_free_pin_candidates[] (Boards.h), not the pin
      // number itself - shared by every GPIO_ITEM_* field (Buzzer, and if
      // HAS_ENCODER, Encoder Up/Down/Press), synced fresh from whichever
      // one's live pin variable on entering MENU_STATE_GPIO_PIN_EDIT (see
      // menu_confirm_select()) - gpio_menu_cursor says which. Commits
      // immediately (and reboots if changed - see gpio_conf_save()) on
      // leaving its own edit screen, same as the Input Voltage/Battery Cal
      // fields above.
      uint8_t staged_gpio_pin_idx = 0;
    #endif
  #endif

  #if HAS_OTA == true
    uint8_t fwupd_menu_cursor = 0;
    // Fetched once when MENU_STATE_FWUPD_LIST is opened from the main list
    // (see menu_confirm_select()), not re-fetched on every redraw or when
    // returning here from MENU_STATE_FWUPD_CONFIRM's CANCEL.
    long fwupd_latest_build = 0;
    bool fwupd_latest_ok = false;
    // 0 = UPDATE, 1 = CANCEL - same 2-item list-with-cursor pattern as
    // WiFi's text_confirm_cursor (SAVE/DISCARD), but defaults to CANCEL (1)
    // rather than the primary action, since this one reboots and reflashes
    // the device instead of just saving a text field.
    uint8_t fwupd_confirm_cursor = 1;
  #endif

  bool menu_is_open() {
    return menu_state != MENU_STATE_CLOSED;
  }

  // wrap is only ever true for the main button's cycling (see
  // menu_button_press()/menu_button_process()) - the encoder itself always
  // clamps at the ends of the list, never wraps.
  uint8_t menu_clamp_cursor(uint8_t cursor, int8_t dir, uint8_t count, bool wrap = false) {
    int8_t c = (int8_t)cursor + (dir > 0 ? 1 : -1);
    if (wrap) {
      if (c < 0) c = (int8_t)count - 1;
      if (c > (int8_t)count - 1) c = 0;
    } else {
      if (c < 0) c = 0;
      if (c > (int8_t)count - 1) c = count - 1;
    }
    return (uint8_t)c;
  }

  // Base step for numeric 0-255 fields is 1 (fine control at rest), but
  // rotating quickly ramps the step up so sweeping across the full range
  // doesn't take a hundred detents. Purely a function of wall-clock time
  // between successive detents - resets to base speed after any pause.
  unsigned long last_numeric_rotate_ms = 0;
  uint8_t accelerated_step() {
    unsigned long now = millis();
    unsigned long elapsed = now - last_numeric_rotate_ms;
    last_numeric_rotate_ms = now;
    if (elapsed < 30)  return 20;
    if (elapsed < 60)  return 10;
    if (elapsed < 120) return 4;
    if (elapsed < 220) return 2;
    return 1;
  }

  void menu_step_numeric(uint8_t *field, int8_t dir, bool wrap = false) {
    int16_t v = (int16_t)*field + dir * (int16_t)accelerated_step();
    if (wrap) {
      if (v < 0)   v = 255;
      if (v > 255) v = 0;
    } else {
      if (v < 0)   v = 0;
      if (v > 255) v = 255;
    }
    *field = (uint8_t)v;
  }

  #if DISPLAY_IS_OLED == true
    // OLED contrast has no perceptually useful continuous range (confirmed
    // on hardware), so present it as a 3-state OFF/DIM/BRIGHT pick instead
    // of a raw 0-255 value. Non-OLED (LCD/TFT backlight) boards keep the
    // continuous editor below, since their brightness range is real.
    #define OLED_BRIGHTNESS_OFF    0
    #define OLED_BRIGHTNESS_DIM    1
    #define OLED_BRIGHTNESS_BRIGHT 255

    // Buckets any historical continuous value (e.g. loaded from EEPROM
    // before this board had the 3-state picker) into the nearest state.
    uint8_t oled_brightness_index(uint8_t val) {
      if (val == OLED_BRIGHTNESS_OFF) return 0;
      if (val == OLED_BRIGHTNESS_BRIGHT) return 2;
      return 1;
    }

    void format_brightness(uint8_t val, char *buf) {
      uint8_t idx = oled_brightness_index(val);
      if (idx == 0)      sprintf(buf, "OFF");
      else if (idx == 1) sprintf(buf, "DIM");
      else               sprintf(buf, "BRIGHT");
    }

    void step_brightness(int8_t dir, bool wrap = false) {
      static const uint8_t values[] = { OLED_BRIGHTNESS_OFF, OLED_BRIGHTNESS_DIM, OLED_BRIGHTNESS_BRIGHT };
      int8_t idx = (int8_t)oled_brightness_index(staged_display_brightness) + (dir > 0 ? 1 : -1);
      if (wrap) {
        if (idx < 0) idx = 2;
        if (idx > 2) idx = 0;
      } else {
        if (idx < 0) idx = 0;
        if (idx > 2) idx = 2;
      }
      staged_display_brightness = values[idx];
    }
  #else
    void format_brightness(uint8_t val, char *buf) {
      sprintf(buf, "%u", val);
    }

    void step_brightness(int8_t dir, bool wrap = false) {
      menu_step_numeric(&staged_display_brightness, dir, wrap);
    }
  #endif

  void format_orientation(uint8_t val, char *buf) {
    if      (val == 1) sprintf(buf, "90");
    else if (val == 2) sprintf(buf, "180");
    else if (val == 3) sprintf(buf, "270");
    else                sprintf(buf, "0");
  }

  void step_orientation(int8_t dir, bool wrap = false) {
    int8_t v = (int8_t)staged_display_rotation + (dir > 0 ? 1 : -1);
    if (wrap) {
      if (v < 0) v = 3;
      if (v > 3) v = 0;
    } else {
      if (v < 0) v = 0;
      if (v > 3) v = 3;
    }
    staged_display_rotation = (uint8_t)v;
  }

  #if MENU_HAS_HW_PAGE == true && HAS_VSENSE == true
    // Stored/edited as ratio*10 (e.g. 110 = 11.0) - 1 and 254 keep clear of
    // the 0x00/0xFF "unset, use board default" sentinels vsr_conf_save()
    // and the boot-time EEPROM load both check for.
    void step_vsense_divider(int8_t dir, bool wrap = false) {
      int16_t v = (int16_t)staged_vsense_divider_ratio_raw + dir * (int16_t)accelerated_step();
      if (wrap) {
        if (v < 1)   v = 254;
        if (v > 254) v = 1;
      } else {
        if (v < 1)   v = 1;
        if (v > 254) v = 254;
      }
      staged_vsense_divider_ratio_raw = (uint8_t)v;
    }
  #endif

  #if MENU_HAS_HW_PAGE == true && HAS_BATTERY_DIVIDER == true
    // Stored/edited as a %/of-default correction (e.g. 103 = 103%) - 1 and
    // 254 keep clear of the 0x00/0xFF "unset, use board default" sentinels
    // bvs_conf_save() and the boot-time EEPROM load both check for.
    void step_battery_v_scale_pct(int8_t dir, bool wrap = false) {
      int16_t v = (int16_t)staged_battery_v_scale_pct + dir * (int16_t)accelerated_step();
      if (wrap) {
        if (v < 1)   v = 254;
        if (v > 254) v = 1;
      } else {
        if (v < 1)   v = 1;
        if (v > 254) v = 254;
      }
      staged_battery_v_scale_pct = (uint8_t)v;
    }
  #endif

  #if HAS_GPIO_MENU == true
    void format_gpio_pin(uint8_t idx, char *buf) {
      sprintf(buf, "D%u", gpio_free_pin_candidates[idx]);
    }

    // Shared by menu_confirm_select()'s sync-on-open/commit-on-close for
    // MENU_STATE_GPIO_PIN_EDIT - which live variable and EEPROM address a
    // given GPIO_ITEM_* corresponds to.
    uint8_t gpio_item_live_pin(uint8_t item) {
      if (item == GPIO_ITEM_BUZZER) return buzzer_pin;
      #if HAS_ENCODER == true
        if (item == GPIO_ITEM_ENC_UP)    return pin_encoder_up;
        if (item == GPIO_ITEM_ENC_DOWN)  return pin_encoder_down;
        if (item == GPIO_ITEM_ENC_PRESS) return pin_encoder_press;
      #endif
      return buzzer_pin; // unreachable
    }

    uint8_t gpio_item_addr(uint8_t item) {
      if (item == GPIO_ITEM_BUZZER) return ADDR_CONF_BUZ;
      #if HAS_ENCODER == true
        if (item == GPIO_ITEM_ENC_UP)    return ADDR_CONF_EUP;
        if (item == GPIO_ITEM_ENC_DOWN)  return ADDR_CONF_EDN;
        if (item == GPIO_ITEM_ENC_PRESS) return ADDR_CONF_EPR;
      #endif
      return ADDR_CONF_BUZ; // unreachable
    }

    uint8_t gpio_idx_for_pin(uint8_t pin) {
      for (uint8_t i = 0; i < GPIO_FREE_PIN_CANDIDATE_COUNT; i++) {
        if (gpio_free_pin_candidates[i] == pin) return i;
      }
      return 0; // unreachable - every live pin variable is always one of the candidates
    }

    // True if `pin` is currently the live value of some *other* GPIO_ITEM_*
    // field than the one being edited (gpio_menu_cursor) - the picker below
    // skips these so the same physical pin can't accidentally end up doing
    // two jobs at once.
    bool gpio_pin_taken_elsewhere(uint8_t pin) {
      if (gpio_menu_cursor != GPIO_ITEM_BUZZER && pin == buzzer_pin) return true;
      #if HAS_ENCODER == true
        if (gpio_menu_cursor != GPIO_ITEM_ENC_UP    && pin == pin_encoder_up)    return true;
        if (gpio_menu_cursor != GPIO_ITEM_ENC_DOWN  && pin == pin_encoder_down)  return true;
        if (gpio_menu_cursor != GPIO_ITEM_ENC_PRESS && pin == pin_encoder_press) return true;
      #endif
      return false;
    }

    // A handful of discrete named choices, not a continuous range - reuses
    // menu_clamp_cursor() (the same list-cursor helper hw_menu_cursor/
    // wifi_menu_cursor use) rather than accelerated_step()'s numeric ramp.
    // Skips candidates already claimed by a different GPIO_ITEM_* field
    // (gpio_pin_taken_elsewhere()) - bounded to one full lap so it can
    // never spin forever, and simply stops at a clamped edge (dir not
    // wrapping) even if that candidate is taken, same as every other
    // clamped field.
    void step_gpio_pin_idx(int8_t dir, bool wrap = false) {
      uint8_t idx = staged_gpio_pin_idx;
      for (uint8_t tries = 0; tries < GPIO_FREE_PIN_CANDIDATE_COUNT; tries++) {
        uint8_t prev = idx;
        idx = menu_clamp_cursor(idx, dir, GPIO_FREE_PIN_CANDIDATE_COUNT, wrap);
        if (idx == prev) break;
        if (!gpio_pin_taken_elsewhere(gpio_free_pin_candidates[idx])) break;
      }
      staged_gpio_pin_idx = idx;
    }
  #endif

  #if HAS_WIFI == true
    void format_wifi_mode(uint8_t mode, char *buf) {
      if      (mode == WR_WIFI_STA) sprintf(buf, "STATION");
      else if (mode == WR_WIFI_AP)  sprintf(buf, "AP");
      else                          sprintf(buf, "OFF");
    }

    void step_wifi_mode(int8_t dir, bool wrap = false) {
      int8_t v = (int8_t)staged_wifi_mode + (dir > 0 ? 1 : -1);
      if (wrap) {
        if (v < WR_WIFI_OFF) v = WR_WIFI_AP;
        if (v > WR_WIFI_AP)  v = WR_WIFI_OFF;
      } else {
        if (v < WR_WIFI_OFF) v = WR_WIFI_OFF;
        if (v > WR_WIFI_AP)  v = WR_WIFI_AP;
      }
      staged_wifi_mode = (uint8_t)v;
    }

    void step_wifi_channel(int8_t dir, bool wrap = false) {
      int8_t v = (int8_t)staged_wifi_channel + (dir > 0 ? 1 : -1);
      if (wrap) {
        if (v < 1)  v = 14;
        if (v > 14) v = 1;
      } else {
        if (v < 1)  v = 1;
        if (v > 14) v = 14;
      }
      staged_wifi_channel = (uint8_t)v;
    }

    // Rudimentary on-screen keyboard: a single cyclic wheel of 98 positions
    // (rotary encoder has no separate cursor-move axis, so editing is
    // append + backspace only - see [[project_encoder_settings_menu]] design
    // notes). Common characters first. DEL and SAVE both sit right after
    // space, before 'a' - reachable in 1-2 taps/detents from the default
    // start position without any special gesture: selecting either is the
    // exact same "hold to confirm whatever's currently on the wheel" action
    // used for every ordinary character, on both encoder and button-only
    // boards (see menu_confirm_select()'s MENU_STATE_WIFI_TEXT_EDIT case) -
    // no separate long-press-to-save exists anymore, which used to only be
    // reachable from the encoder and left button-only boards with no way to
    // save at all. A second DEL is kept further round the wheel (after '9',
    // before the symbols) for whoever's mid-cycling through the alphabet
    // and doesn't want to dial all the way back to the start.
    #define WHEEL_QUICKDEL_IDX  1
    #define WHEEL_QUICKSAVE_IDX 2
    #define WHEEL_DEL_IDX       66
    #define WHEEL_LEN           98
    const char WHEEL_CORE[] = " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; // 64 chars - [0] is space (idx 0 direct), [1..63] is 'a'..'9' (idx 3..65, offset by the 2 quick meta-positions)
    const char WHEEL_SYMS[] = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"; // 31 chars, idx 67-97

    bool wheel_is_del(uint8_t idx)  { return idx == WHEEL_QUICKDEL_IDX || idx == WHEEL_DEL_IDX; }
    bool wheel_is_save(uint8_t idx) { return idx == WHEEL_QUICKSAVE_IDX; }

    // Returns 0 for the DEL/SAVE meta positions - callers check those via
    // wheel_is_del()/wheel_is_save() before treating this as a literal
    // character.
    char wheel_char_at(uint8_t idx) {
      if (idx == 0) return WHEEL_CORE[0]; // space
      if (wheel_is_del(idx) || wheel_is_save(idx)) return 0;
      if (idx < WHEEL_DEL_IDX) return WHEEL_CORE[idx-2]; // 'a'..'9'
      return WHEEL_SYMS[idx - (WHEEL_DEL_IDX+1)];
    }

    void wheel_move(int8_t dir) {
      int16_t v = (int16_t)wheel_index + dir * (int16_t)accelerated_step();
      v = v % WHEEL_LEN;
      if (v < 0) v += WHEEL_LEN;
      wheel_index = (uint8_t)v;
    }

    // Dedicated backspace on the main (non-encoder) button while typing -
    // dialing the wheel all the way to DEL every time is tedious when a
    // second physical button is sitting right there doing nothing.
    void menu_main_button_del() {
      if (menu_state == MENU_STATE_WIFI_TEXT_EDIT) {
        buzzer_encoder_click_melody();
        uint8_t len = strlen(text_edit_buf);
        if (len > 0) text_edit_buf[len-1] = 0;
      }
    }
  #else
    void menu_main_button_del() { }
  #endif

  #if HAS_WIFI == true || HAS_ETHERNET == true
    // Shared by WiFi's IP Address/Netmask (WIFI_ITEM_IP/NETMASK, Remote.h's
    // ADDR_CONF_IP/NM) and, on MeshPoE-S3, wired Ethernet's own
    // (ETH_ITEM_IP/NETMASK, Ethernet.h's ADDR_CONF_ETH_IP/NM) - both submenus
    // pass in whichever staged octets/index are theirs.
    void format_addr_octets(uint8_t *octets, char *buf) {
      sprintf(buf, "%u.%u.%u.%u", octets[0], octets[1], octets[2], octets[3]);
    }

    void step_addr_octet(uint8_t *octets, uint8_t octet_idx, int8_t dir, bool wrap = false) {
      int16_t v = (int16_t)octets[octet_idx] + dir;
      if (wrap) {
        if (v < 0)   v = 255;
        if (v > 255) v = 0;
      } else {
        if (v < 0)   v = 0;
        if (v > 255) v = 255;
      }
      octets[octet_idx] = (uint8_t)v;
    }
  #endif

  #if HAS_ETHERNET == true
    void format_eth_speed_mode(uint8_t mode, char *buf) {
      if      (mode == ETH_SPEED_100_FULL) sprintf(buf, "100/FULL");
      else if (mode == ETH_SPEED_100_HALF) sprintf(buf, "100/HALF");
      else if (mode == ETH_SPEED_10_FULL)  sprintf(buf, "10/FULL");
      else if (mode == ETH_SPEED_10_HALF)  sprintf(buf, "10/HALF");
      else if (mode == ETH_SPEED_OFF)      sprintf(buf, "OFF");
      else                                  sprintf(buf, "AUTO");
    }

    void step_eth_speed_mode(int8_t dir, bool wrap = false) {
      int8_t v = (int8_t)staged_eth_speed_mode + (dir > 0 ? 1 : -1);
      if (wrap) {
        if (v < ETH_SPEED_AUTO) v = ETH_SPEED_OFF;
        if (v > ETH_SPEED_OFF)  v = ETH_SPEED_AUTO;
      } else {
        if (v < ETH_SPEED_AUTO) v = ETH_SPEED_AUTO;
        if (v > ETH_SPEED_OFF)  v = ETH_SPEED_OFF;
      }
      staged_eth_speed_mode = (uint8_t)v;
    }
  #endif

  #if HAS_RTC == true
    uint8_t rtc_days_in_month(int32_t year, uint8_t month) {
      static const uint8_t dim[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
      if (month < 1 || month > 12) return 31; // unreachable - month is always clamped below
      if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) return 29;
      return dim[month - 1];
    }

    // Plain +-1-per-detent, no accelerated_step() ramp - same as
    // step_addr_octet() above, which every other sequential-field editor in
    // this menu (WiFi/Ethernet IP Address, Netmask) already uses.
    void step_rtc_field(uint8_t field_idx, int8_t dir, bool wrap = false) {
      if (field_idx == 0) { // Year - DS3231 only stores 2000-2099 (see RTC.h)
        int16_t v = (int16_t)staged_rtc_year + dir;
        if (wrap) { if (v < 2000) v = 2099; if (v > 2099) v = 2000; }
        else      { if (v < 2000) v = 2000; if (v > 2099) v = 2099; }
        staged_rtc_year = v;
      } else if (field_idx == 1) { // Month
        int8_t v = (int8_t)staged_rtc_month + dir;
        if (wrap) { if (v < 1) v = 12; if (v > 12) v = 1; }
        else      { if (v < 1) v = 1;  if (v > 12) v = 12; }
        staged_rtc_month = (uint8_t)v;
      } else if (field_idx == 2) { // Day
        uint8_t max_day = rtc_days_in_month(staged_rtc_year, staged_rtc_month);
        int8_t v = (int8_t)staged_rtc_day + dir;
        if (wrap) { if (v < 1) v = (int8_t)max_day; if (v > (int8_t)max_day) v = 1; }
        else      { if (v < 1) v = 1;               if (v > (int8_t)max_day) v = (int8_t)max_day; }
        staged_rtc_day = (uint8_t)v;
      } else if (field_idx == 3) { // Hour
        int8_t v = (int8_t)staged_rtc_hour + dir;
        if (wrap) { if (v < 0) v = 23; if (v > 23) v = 0; }
        else      { if (v < 0) v = 0;  if (v > 23) v = 23; }
        staged_rtc_hour = (uint8_t)v;
      } else if (field_idx == 4) { // Minute
        int8_t v = (int8_t)staged_rtc_minute + dir;
        if (wrap) { if (v < 0) v = 59; if (v > 59) v = 0; }
        else      { if (v < 0) v = 0;  if (v > 59) v = 59; }
        staged_rtc_minute = (uint8_t)v;
      } else { // Second
        int8_t v = (int8_t)staged_rtc_second + dir;
        if (wrap) { if (v < 0) v = 59; if (v > 59) v = 0; }
        else      { if (v < 0) v = 0;  if (v > 59) v = 59; }
        staged_rtc_second = (uint8_t)v;
      }
      // A Year/Month change can leave Day pointing past the new month's
      // last day (e.g. Mar 31 -> Feb) - clamp it back in range immediately
      // rather than letting an invalid date reach rtc_set_unixtime().
      uint8_t max_day = rtc_days_in_month(staged_rtc_year, staged_rtc_month);
      if (staged_rtc_day > max_day) staged_rtc_day = max_day;
    }

    // "UTC" for a zero offset, else "+HH:MM"/"-HH:MM" - quarter-hour steps
    // (see TZ_OFFSET_QH_MIN/MAX, RTC.h) can land on a non-zero minute part
    // (e.g. UTC+05:30), so this always prints both fields rather than
    // special-casing whole hours.
    void format_tz_offset(int8_t offset_qh, char *buf) {
      if (offset_qh == 0) { sprintf(buf, "UTC"); return; }
      int16_t total_min = (int16_t)offset_qh * 15;
      char sign = (total_min < 0) ? '-' : '+';
      int16_t abs_min = (total_min < 0) ? -total_min : total_min;
      sprintf(buf, "%c%02d:%02d", sign, abs_min / 60, abs_min % 60);
    }

    // Plain +-1-per-detent (one quarter-hour), no accelerated_step() ramp -
    // same reasoning as step_rtc_field() above.
    void step_tz_offset(int8_t dir, bool wrap = false) {
      int16_t v = (int16_t)staged_tz_offset_qh + dir;
      if (wrap) { if (v < TZ_OFFSET_QH_MIN) v = TZ_OFFSET_QH_MAX; if (v > TZ_OFFSET_QH_MAX) v = TZ_OFFSET_QH_MIN; }
      else      { if (v < TZ_OFFSET_QH_MIN) v = TZ_OFFSET_QH_MIN; if (v > TZ_OFFSET_QH_MAX) v = TZ_OFFSET_QH_MAX; }
      staged_tz_offset_qh = (int8_t)v;
    }

    #if MCU_VARIANT == MCU_ESP32 && (HAS_WIFI == true || HAS_ETHERNET == true)
      // Only the success result auto-dismisses (menu_draw_popup_timed()) -
      // nothing to acknowledge there, whereas an error is worth making
      // sure was actually seen, so those still wait for real input.
      #define NTP_SYNC_SUCCESS_POPUP_MS 5000

      const char *ntp_result_text(uint8_t result) {
        if      (result == NTP_SYNC_ERR_NO_RTC)    return "NO RTC FOUND";
        else if (result == NTP_SYNC_ERR_NO_NET)    return "NO NETWORK";
        else if (result == NTP_SYNC_ERR_TIMEOUT)   return "NTP TIMEOUT";
        else if (result == NTP_SYNC_ERR_RTC_WRITE) return "RTC WRITE FAIL";
        return "SYNCED!";
      }
    #endif

    #if HAS_GPS == true
      // Same auto-dismiss-on-success-only reasoning as Sync NTP's own
      // NTP_SYNC_SUCCESS_POPUP_MS above - errors stay up until dismissed,
      // worth making sure they're actually seen.
      #define GPS_SYNC_SUCCESS_POPUP_MS 5000

      const char *gps_result_text(uint8_t result) {
        if      (result == GPS_SYNC_ERR_NO_RTC)    return "NO RTC FOUND";
        else if (result == GPS_SYNC_ERR_DISABLED)  return "GNSS DISABLED";
        else if (result == GPS_SYNC_ERR_NO_FIX)    return "NO GNSS TIME";
        else if (result == GPS_SYNC_ERR_RTC_WRITE) return "RTC WRITE FAIL";
        return "SYNCED!";
      }
    #endif
  #endif

  void menu_stage_from_live() {
    staged_display_timeout    = display_blanking_enabled ? (uint8_t)(display_blanking_timeout / 1000) : 0;
    staged_display_brightness = display_intensity;
    live_display_brightness   = display_intensity;
    #if HAS_BUZZER == true
      staged_sound_enabled = sound_enabled;
    #endif
    #if HAS_ESPNOW == true
      staged_espnow_enabled = espnow_enabled;
      staged_espnow_mode_v2 = (espnow_mode == ESPNOW_MODE_V2);
      staged_espnow_lr_enabled = espnow_lr_enabled;
    #endif
    #if HAS_ENCODER == true
      staged_encoder_enabled = encoder_enabled;
    #endif
    // display_rotation itself is only a local variable inside display_init(),
    // applied once at boot - not a persisted global - so read the actual
    // EEPROM value fresh here, same as WiFi SSID/PSK above.
    #if HAS_EEPROM
      staged_display_rotation = EEPROM.read(eeprom_addr(ADDR_CONF_DROT));
    #elif MCU_VARIANT == MCU_NRF52
      staged_display_rotation = eeprom_read(eeprom_addr(ADDR_CONF_DROT));
    #endif
    if (staged_display_rotation > 3) staged_display_rotation = 0;
    #if HAS_WIFI == true
      staged_wifi_mode = wifi_mode;
      staged_wifi_channel = wr_channel;
      // wr_ssid/wr_psk are only populated by wifi_remote_init(), which only
      // runs at boot if WiFi is already in STA/AP mode - if it boots OFF,
      // those globals stay empty even though EEPROM has real values. Read
      // EEPROM directly instead, mirroring wifi_remote_init()'s own loop.
      live_wifi_ssid[32] = 0; live_wifi_psk[32] = 0;
      for (uint8_t i = 0; i < 32; i++) {
        live_wifi_ssid[i] = EEPROM.read(config_addr(ADDR_CONF_SSID+i));
        if (live_wifi_ssid[i] == (char)0xFF) live_wifi_ssid[i] = 0;
      }
      for (uint8_t i = 0; i < 32; i++) {
        live_wifi_psk[i] = EEPROM.read(config_addr(ADDR_CONF_PSK+i));
        if (live_wifi_psk[i] == (char)0xFF) live_wifi_psk[i] = 0;
      }
      strncpy(staged_wifi_ssid, live_wifi_ssid, 33);
      strncpy(staged_wifi_psk,  live_wifi_psk,  33);

      // Unlike Ethernet's IP Address, staged_wifi_ip must start out exactly
      // equal to live_wifi_ip (no default-value substitution) - this runs
      // at whole-menu-open time, not when IP Address is actually opened for
      // editing, so any mismatch here would get silently written by SAVE &
      // EXIT even if the user never touched IP Address at all this session.
      if (addr4_read(ADDR_CONF_IP, live_wifi_ip)) {
        for (uint8_t i = 0; i < 4; i++) { staged_wifi_ip[i] = live_wifi_ip[i]; }
      } else {
        live_wifi_ip[0] = live_wifi_ip[1] = live_wifi_ip[2] = live_wifi_ip[3] = 0;
        staged_wifi_ip[0] = staged_wifi_ip[1] = staged_wifi_ip[2] = staged_wifi_ip[3] = 0;
      }
      if (addr4_read(ADDR_CONF_NM, live_wifi_nm)) {
        for (uint8_t i = 0; i < 4; i++) { staged_wifi_nm[i] = live_wifi_nm[i]; }
      } else {
        live_wifi_nm[0] = live_wifi_nm[1] = live_wifi_nm[2] = live_wifi_nm[3] = 0;
        staged_wifi_nm[0] = staged_wifi_nm[1] = staged_wifi_nm[2] = staged_wifi_nm[3] = 0;
      }
      if (addr4_read(ADDR_CONF_GW, live_wifi_gw)) {
        for (uint8_t i = 0; i < 4; i++) { staged_wifi_gw[i] = live_wifi_gw[i]; }
      } else {
        live_wifi_gw[0] = live_wifi_gw[1] = live_wifi_gw[2] = live_wifi_gw[3] = 0;
        staged_wifi_gw[0] = staged_wifi_gw[1] = staged_wifi_gw[2] = staged_wifi_gw[3] = 0;
      }
      if (addr4_read(ADDR_CONF_DNS, live_wifi_dns)) {
        for (uint8_t i = 0; i < 4; i++) { staged_wifi_dns[i] = live_wifi_dns[i]; }
      } else {
        live_wifi_dns[0] = live_wifi_dns[1] = live_wifi_dns[2] = live_wifi_dns[3] = 0;
        staged_wifi_dns[0] = staged_wifi_dns[1] = staged_wifi_dns[2] = staged_wifi_dns[3] = 0;
      }
    #endif
  }

  // Single write site: only fields that actually changed get persisted,
  // and only once per menu session - never per detent.
  void menu_commit_and_exit() {
    uint8_t live_timeout = display_blanking_enabled ? (uint8_t)(display_blanking_timeout / 1000) : 0;
    if (staged_display_timeout != live_timeout) {
      db_conf_save(staged_display_timeout);
    }
    if (staged_display_brightness != live_display_brightness) {
      // Compared against live_display_brightness, not display_intensity -
      // brightness is applied live on confirm (see menu_encoder_button()),
      // so display_intensity may already equal staged_display_brightness
      // by the time we get here, which would otherwise look like "nothing
      // changed" and skip persisting it. di_conf_save() only persists to
      // EEPROM - it doesn't update the live display_intensity the display
      // loop actually reads, so set that too (mirrors what the
      // CMD_DISP_INT KISS handler already does; harmless if already set).
      display_intensity = staged_display_brightness;
      di_conf_save(staged_display_brightness);
    }
    #if HAS_BUZZER == true
      if (staged_sound_enabled != sound_enabled) {
        snd_conf_save(staged_sound_enabled);
      }
    #endif
    #if HAS_ESPNOW == true
      // If a long-press skips the explicit ENABLE/CANCEL gate (see
      // MENU_STATE_ESPNOW_LR_CONFIRM, menu_confirm_select()) while LR is
      // still mid-toggle or mid-confirm, treat that as CANCEL rather than
      // silently accepting an unconfirmed "disable WiFi" change - opposite
      // of the flush-in-progress blocks above (those stop a long-press from
      // silently discarding an edit; this one stops it from silently
      // accepting one). Only reverts the OFF->ON case - turning LR off
      // needs no confirmation, so a long-press mid-way there is fine as-is.
      if (!espnow_lr_enabled &&
          ((menu_state == MENU_STATE_ESPNOW_EDIT && espnow_menu_cursor == ESPNOW_ITEM_LR) ||
           menu_state == MENU_STATE_ESPNOW_LR_CONFIRM)) {
        staged_espnow_lr_enabled = false;
      }
      {
        // ESP-NOW's Enabled/Mode/LR fields are committed together rather
        // than each independently self-rebooting (like espnow_conf_save()
        // normally does on its own, e.g. from the CMD_ESPNOW_ENABLE KISS
        // handler, where that's fine since there's nothing else pending).
        // If more than one changed in the same SAVE & EXIT, and each
        // called its own hard_reset() as soon as it saw a change,
        // whichever ran first would reboot before the others' writes ever
        // happened, silently losing them. Write all three raw bytes first,
        // then make one combined reboot decision. espnow_mode_conf_save()/
        // espnow_lr_conf_save() (Utilities.h) deliberately have no
        // self-reboot logic of their own for this reason.
        uint8_t staged_espnow_mode_byte = staged_espnow_mode_v2 ? ESPNOW_MODE_V2 : ESPNOW_MODE_V1;
        bool espnow_mode_changed   = (staged_espnow_mode_byte != espnow_mode);
        bool espnow_lr_changed     = (staged_espnow_lr_enabled != espnow_lr_enabled);
        bool espnow_enable_changed = (staged_espnow_enabled != espnow_enabled);

        if (espnow_mode_changed) {
          espnow_mode_conf_save(staged_espnow_mode_byte);
          espnow_mode = staged_espnow_mode_byte;
        }
        if (espnow_lr_changed) {
          espnow_lr_conf_save(staged_espnow_lr_enabled ? ESPNOW_LR_ENABLE_BYTE : ESPNOW_LR_DISABLE_BYTE);
          espnow_lr_enabled = staged_espnow_lr_enabled;
        }
        if (espnow_enable_changed) {
          eeprom_update(eeprom_addr(ADDR_CONF_ESPNOW), staged_espnow_enabled ? ESPNOW_ENABLE_BYTE : ESPNOW_DISABLE_BYTE);
          espnow_enabled = staged_espnow_enabled;
        }
        if (espnow_mode_changed || espnow_lr_changed || espnow_enable_changed) { hard_reset(); }
      }
    #endif
    #if HAS_ENCODER == true
      if (staged_encoder_enabled != encoder_enabled) {
        enc_conf_save(staged_encoder_enabled);
      }
    #endif
    #if HAS_WIFI == true
      bool wifi_changed = false;
      if (staged_wifi_mode != wifi_mode) {
        // Mirrors the CMD_WIFI_MODE KISS handler exactly: persist, update
        // the live variable, then actually (re)start the WiFi stack.
        wr_conf_save(staged_wifi_mode);
        wifi_mode = staged_wifi_mode;
        wifi_changed = true;
      }
      if (staged_wifi_channel != wr_channel) {
        // Unlike the existing CMD_WIFI_CHN KISS handler (RNode_Firmware.ino),
        // which only writes EEPROM and needs a reboot to take effect, this
        // also updates the live wr_channel before wifi_remote_init() below
        // re-reads it - AP mode's softAP() call (Remote.h) picks the new
        // channel up immediately, no reboot needed. ESP-NOW's own channel
        // (set once in espnow_init(), ESPNOW.h) does NOT get re-applied
        // live though - same pre-existing, documented residual limitation
        // as an external STA AP's channel differing from wr_channel
        // (wifi_remote_reconnect()'s own comment, Remote.h) - a reboot is
        // still needed for ESP-NOW to pick up a changed channel.
        eeprom_update(eeprom_addr(ADDR_CONF_WCHN), staged_wifi_channel);
        wr_channel = staged_wifi_channel;
        wifi_changed = true;
      }
      if (strcmp(staged_wifi_ssid, live_wifi_ssid) != 0) {
        // Same byte-loop the CMD_WIFI_SSID KISS handler uses. wifi_remote_init()
        // re-reads wr_ssid/wr_psk from EEPROM itself, so no need to also
        // assign wr_ssid here.
        uint8_t len = strlen(staged_wifi_ssid);
        for (uint8_t i = 0; i < 33; i++) {
          eeprom_update(config_addr(ADDR_CONF_SSID+i), i < len ? staged_wifi_ssid[i] : 0x00);
        }
        wifi_changed = true;
      }
      if (strcmp(staged_wifi_psk, live_wifi_psk) != 0) {
        uint8_t len = strlen(staged_wifi_psk);
        for (uint8_t i = 0; i < 33; i++) {
          eeprom_update(config_addr(ADDR_CONF_PSK+i), i < len ? staged_wifi_psk[i] : 0x00);
        }
        wifi_changed = true;
      }
      {
        bool ip_changed = false;
        bool nm_changed = false;
        bool gw_changed = false;
        bool dns_changed = false;
        for (uint8_t i = 0; i < 4; i++) { if (staged_wifi_ip[i] != live_wifi_ip[i]) { ip_changed = true; break; } }
        for (uint8_t i = 0; i < 4; i++) { if (staged_wifi_nm[i] != live_wifi_nm[i]) { nm_changed = true; break; } }
        for (uint8_t i = 0; i < 4; i++) { if (staged_wifi_gw[i] != live_wifi_gw[i]) { gw_changed = true; break; } }
        for (uint8_t i = 0; i < 4; i++) { if (staged_wifi_dns[i] != live_wifi_dns[i]) { dns_changed = true; break; } }
        if (ip_changed) {
          for (uint8_t i = 0; i < 4; i++) { eeprom_update(config_addr(ADDR_CONF_IP+i), staged_wifi_ip[i]); }
          wifi_changed = true;
        }
        if (nm_changed) {
          for (uint8_t i = 0; i < 4; i++) { eeprom_update(config_addr(ADDR_CONF_NM+i), staged_wifi_nm[i]); }
          wifi_changed = true;
        }
        if (gw_changed) {
          for (uint8_t i = 0; i < 4; i++) { eeprom_update(config_addr(ADDR_CONF_GW+i), staged_wifi_gw[i]); }
          wifi_changed = true;
        }
        if (dns_changed) {
          for (uint8_t i = 0; i < 4; i++) { eeprom_update(config_addr(ADDR_CONF_DNS+i), staged_wifi_dns[i]); }
          wifi_changed = true;
        }
      }
      // wifi_remote_init() re-reads static IP/netmask/gateway/DNS from
      // EEPROM itself (see wifi_remote_start_sta(), Remote.h), same as it
      // already does for SSID/PSK above - no separate "apply" call needed.
      if (wifi_changed) { wifi_remote_init(); }
    #endif
    #if MENU_HAS_HW_PAGE == true
      // Flush an in-progress immediate-commit screen (Voltage/Battery cal,
      // or the GPIO pin picker below) if a long-press exits the whole menu
      // while one of those screens is still open. They normally commit on
      // their own BACK/confirm (see menu_confirm_select()), but on
      // encoder-only boards (no separate main button - see HAS_INPUT) a
      // long-press from *inside* one of these screens skips that and jumps
      // straight here instead, which used to silently discard the edit.
      #if HAS_VSENSE == true
        if (menu_state == MENU_STATE_HW_EDIT && hw_menu_cursor == HW_ITEM_VOLTAGE) {
          uint8_t live_raw = (uint8_t)(vsense_divider_ratio * 10.0 + 0.5);
          if (staged_vsense_divider_ratio_raw != live_raw) {
            vsr_conf_save(staged_vsense_divider_ratio_raw);
          }
        }
      #endif
      #if HAS_BATTERY_DIVIDER == true
        if (menu_state == MENU_STATE_HW_EDIT && hw_menu_cursor == HW_ITEM_BATTERY) {
          uint8_t live_pct = (uint8_t)((battery_v_scale / BATTERY_V_SCALE_DEFAULT) * 100.0 + 0.5);
          if (staged_battery_v_scale_pct != live_pct) {
            bvs_conf_save(staged_battery_v_scale_pct);
          }
        }
      #endif
    #endif
    #if HAS_RTC == true
      // Same flush-in-progress reasoning as the Voltage/Battery cal block
      // above - a long-press from *inside* the Set Time/Date editor jumps
      // straight here on encoder-only boards, which would otherwise
      // silently discard whatever fields had already been adjusted.
      if (menu_state == MENU_STATE_RTC_EDIT) {
        int32_t days = rtc_days_from_civil(staged_rtc_year, staged_rtc_month, staged_rtc_day);
        uint32_t epoch = (uint32_t)days * 86400UL + (uint32_t)staged_rtc_hour * 3600UL + (uint32_t)staged_rtc_minute * 60UL + staged_rtc_second;
        rtc_set_unixtime(epoch);
      } else if (menu_state == MENU_STATE_RTC_TZ_EDIT) {
        uint8_t live_raw = (uint8_t)(rtc_get_tz_offset_qh() + TZ_OFFSET_RAW_ZERO);
        uint8_t new_raw  = (uint8_t)(staged_tz_offset_qh + TZ_OFFSET_RAW_ZERO);
        if (new_raw != live_raw) { tz_conf_save(new_raw); }
      }
    #endif
    // Must be last: gpio_conf_save()/ethspd_conf_save()/drot_conf_save() may
    // call hard_reset() if the value actually changed, which would otherwise
    // discard any of
    // the above writes that hadn't happened yet.
    #if HAS_GPIO_MENU == true
      if (menu_state == MENU_STATE_GPIO_PIN_EDIT) {
        uint8_t new_pin = gpio_free_pin_candidates[staged_gpio_pin_idx];
        if (new_pin != gpio_item_live_pin(gpio_menu_cursor)) {
          gpio_conf_save(gpio_item_addr(gpio_menu_cursor), new_pin);
        }
      }
    #endif
    #if HAS_ETHERNET == true
      if (menu_state == MENU_STATE_ETH_EDIT) {
        if (staged_eth_speed_mode != eth_speed_mode) {
          ethspd_conf_save(staged_eth_speed_mode);
        }
      } else if (menu_state == MENU_STATE_ETH_ADDR_EDIT) {
        // Safe to commit at any octet index, not just the last one - the
        // not-yet-visited octets still hold their synced-at-entry starting
        // values (see menu_confirm_select()), so the staged field is always
        // a complete, valid 4-byte value, never a partial one.
        int addr_base = eth_addr_base(eth_menu_cursor);
        uint8_t *staged = eth_staged_addr_field(eth_menu_cursor);
        uint8_t live[4];
        addr4_read(addr_base, live);
        bool addr_changed = false;
        for (uint8_t i = 0; i < 4; i++) { if (staged[i] != live[i]) { addr_changed = true; break; } }
        if (addr_changed) {
          ethaddr_conf_save(addr_base, staged);
          // Same auto-netmask as the normal completion path in
          // menu_confirm_select() - see the comment there.
          if (eth_menu_cursor == ETH_ITEM_IP) {
            uint8_t nm_tmp[4];
            if (!addr4_read(ADDR_CONF_ETH_NM, nm_tmp)) {
              uint8_t nm_255[4] = {255, 255, 255, 0};
              ethaddr_conf_save(ADDR_CONF_ETH_NM, nm_255);
            }
          }
          eth_apply_addr_config();
        }
      }
    #endif
    drot_conf_save(staged_display_rotation);
    menu_state  = MENU_STATE_CLOSED;
    menu_cursor = 0;
    display_unblank();
  }

  // Idle-timeout close (see menu_timeout_process()) - discards every
  // deferred, whole-session-staged edit (Display Timeout/Brightness/
  // Orientation/Sound/Encoder/WiFi Mode/SSID/PSK) instead of persisting them
  // like menu_commit_and_exit() does. Fields that commit immediately on
  // their own confirm (Voltage Divider, Battery Cal, GPIO pin, Ethernet
  // Speed) already wrote to EEPROM the moment they were confirmed, so
  // there's nothing to discard for those either way.
  void menu_close_without_saving() {
    // Brightness gets a live preview the instant it's confirmed (see
    // menu_confirm_select()), unlike every other field - undo that here so
    // an unsaved preview doesn't linger after the menu gives up on it.
    display_intensity = live_display_brightness;
    menu_state  = MENU_STATE_CLOSED;
    menu_cursor = 0;
    display_unblank();
  }

  // Polled from loop() (same as menu_button_process()) - closes the menu
  // without saving if it's been sitting open with no button/encoder input
  // for SETTINGS_MENU_TIMEOUT seconds (Config.h). menu_last_activity_ms is
  // bumped by every real input event (menu_button_press()/
  // menu_encoder_rotate()/menu_encoder_button()), so this only fires on
  // genuine inactivity, not e.g. while the user is mid-scroll.
  void menu_timeout_process() {
    if (menu_is_open() && millis() - menu_last_activity_ms > (unsigned long)SETTINGS_MENU_TIMEOUT * 1000UL) {
      menu_close_without_saving();
    }
  }

  void menu_encoder_rotate(int8_t dir, bool wrap) {
    menu_last_activity_ms = millis();
    display_unblank();
    #if HAS_WIFI == true || HAS_ETHERNET == true
      if (menu_state == MENU_STATE_STATUS_POPUP) {
        // Not a real navigable screen - any input at all dismisses it,
        // rotation included, rather than the usual per-state handling
        // below.
        buzzer_encoder_tick_melody();
        menu_state = menu_popup_return_state;
        return;
      }
    #endif
    if (menu_state == MENU_STATE_LIST) {
      buzzer_encoder_tick_melody();
      menu_cursor = menu_clamp_cursor(menu_cursor, dir, MENU_ITEM_COUNT, wrap);
    } else if (menu_state == MENU_STATE_EDIT) {
      buzzer_encoder_tick_melody();
      if (menu_edit_field == MENU_ITEM_DISPLAY_TIMEOUT) {
        menu_step_numeric(&staged_display_timeout, dir, wrap);
      } else if (menu_edit_field == MENU_ITEM_DISPLAY_BRIGHTNESS) {
        step_brightness(dir, wrap);
      } else if (menu_edit_field == MENU_ITEM_ORIENTATION) {
        step_orientation(dir, wrap);
      }
      #if HAS_BUZZER == true
        else if (menu_edit_field == MENU_ITEM_SOUND) {
          staged_sound_enabled = !staged_sound_enabled;
        }
      #endif
      #if HAS_ENCODER == true
        else if (menu_edit_field == MENU_ITEM_ENCODER) {
          staged_encoder_enabled = !staged_encoder_enabled;
        }
      #endif
    }
    #if HAS_WIFI == true
      else if (menu_state == MENU_STATE_WIFI_LIST) {
        buzzer_encoder_tick_melody();
        wifi_menu_cursor = menu_clamp_cursor(wifi_menu_cursor, dir, WIFI_ITEM_COUNT, wrap);
      } else if (menu_state == MENU_STATE_WIFI_EDIT) {
        buzzer_encoder_tick_melody();
        // wifi_menu_cursor still points at whichever field was open when
        // MENU_STATE_WIFI_EDIT was entered - same "list cursor persists
        // across states" trick ESP-NOW's shared edit state relies on.
        if (wifi_menu_cursor == WIFI_ITEM_CHANNEL) { step_wifi_channel(dir, wrap); }
        else                                       { step_wifi_mode(dir, wrap); }
      } else if (menu_state == MENU_STATE_WIFI_TEXT_EDIT) {
        buzzer_encoder_tick_melody();
        wheel_move(dir);
      } else if (menu_state == MENU_STATE_WIFI_TEXT_CONFIRM) {
        buzzer_encoder_tick_melody();
        text_confirm_cursor = menu_clamp_cursor(text_confirm_cursor, dir, 2, wrap);
      } else if (menu_state == MENU_STATE_WIFI_ADDR_EDIT) {
        buzzer_encoder_tick_melody();
        step_addr_octet(wifi_staged_addr_field(wifi_menu_cursor), wifi_addr_octet_idx, dir, wrap);
      }
    #endif
    #if HAS_ETHERNET == true
      else if (menu_state == MENU_STATE_ETH_LIST) {
        buzzer_encoder_tick_melody();
        eth_menu_cursor = menu_clamp_cursor(eth_menu_cursor, dir, ETH_ITEM_COUNT, wrap);
      } else if (menu_state == MENU_STATE_ETH_EDIT) {
        buzzer_encoder_tick_melody();
        step_eth_speed_mode(dir, wrap);
      } else if (menu_state == MENU_STATE_ETH_ADDR_EDIT) {
        buzzer_encoder_tick_melody();
        step_addr_octet(eth_staged_addr_field(eth_menu_cursor), eth_addr_octet_idx, dir, wrap);
      }
    #endif
    #if HAS_RTC == true
      else if (menu_state == MENU_STATE_RTC_LIST) {
        buzzer_encoder_tick_melody();
        rtc_menu_cursor = menu_clamp_cursor(rtc_menu_cursor, dir, RTC_ITEM_COUNT, wrap);
      } else if (menu_state == MENU_STATE_RTC_EDIT) {
        buzzer_encoder_tick_melody();
        step_rtc_field(rtc_edit_field_idx, dir, wrap);
      } else if (menu_state == MENU_STATE_RTC_TZ_EDIT) {
        buzzer_encoder_tick_melody();
        step_tz_offset(dir, wrap);
      }
    #endif
    #if HAS_GPS == true
      else if (menu_state == MENU_STATE_GNSS_LIST) {
        buzzer_encoder_tick_melody();
        gnss_menu_cursor = menu_clamp_cursor(gnss_menu_cursor, dir, GNSS_ITEM_COUNT, wrap);
      } else if (menu_state == MENU_STATE_GNSS_EDIT) {
        buzzer_encoder_tick_melody();
        staged_gnss_enabled = !staged_gnss_enabled;
      }
    #endif
    #if HAS_ESPNOW == true
      else if (menu_state == MENU_STATE_ESPNOW_LIST) {
        buzzer_encoder_tick_melody();
        espnow_menu_cursor = menu_clamp_cursor(espnow_menu_cursor, dir, ESPNOW_ITEM_COUNT, wrap);
      } else if (menu_state == MENU_STATE_ESPNOW_EDIT) {
        buzzer_encoder_tick_melody();
        // espnow_menu_cursor still points at whichever field was open when
        // MENU_STATE_ESPNOW_EDIT was entered (WiFi's Mode field relies on
        // the same "list cursor persists across states" behavior).
        if (espnow_menu_cursor == ESPNOW_ITEM_ENABLED)      staged_espnow_enabled = !staged_espnow_enabled;
        else if (espnow_menu_cursor == ESPNOW_ITEM_MODE)    staged_espnow_mode_v2 = !staged_espnow_mode_v2;
        else                                                staged_espnow_lr_enabled = !staged_espnow_lr_enabled;
      } else if (menu_state == MENU_STATE_ESPNOW_LR_CONFIRM) {
        // 3-item list (info row/ENABLE/CANCEL) - same tap-to-move/hold-to-
        // select navigation as everywhere else, same pattern as F/W
        // Update's UPDATE/CANCEL (MENU_STATE_FWUPD_CONFIRM).
        buzzer_encoder_tick_melody();
        espnow_lr_confirm_cursor = menu_clamp_cursor(espnow_lr_confirm_cursor, dir, 3, wrap);
      }
    #endif
    #if HAS_SENSORS == true
      else if (menu_state == MENU_STATE_SENSORS_LIST) {
        buzzer_encoder_tick_melody();
        sensors_menu_cursor = menu_clamp_cursor(sensors_menu_cursor, dir, SENSORS_ITEM_COUNT, wrap);
      }
    #endif
    #if MENU_HAS_HW_PAGE == true
      else if (menu_state == MENU_STATE_HW_LIST) {
        buzzer_encoder_tick_melody();
        hw_menu_cursor = menu_clamp_cursor(hw_menu_cursor, dir, HW_ITEM_COUNT, wrap);
      }
      #if HAS_VSENSE == true || HAS_BATTERY_DIVIDER == true
        else if (menu_state == MENU_STATE_HW_EDIT) {
          buzzer_encoder_tick_melody();
          #if HAS_VSENSE == true
            if (hw_menu_cursor == HW_ITEM_VOLTAGE) { step_vsense_divider(dir, wrap); }
          #endif
          #if HAS_BATTERY_DIVIDER == true
            if (hw_menu_cursor == HW_ITEM_BATTERY) { step_battery_v_scale_pct(dir, wrap); }
          #endif
        }
      #endif
      #if HAS_GPIO_MENU == true
        else if (menu_state == MENU_STATE_GPIO_LIST) {
          buzzer_encoder_tick_melody();
          gpio_menu_cursor = menu_clamp_cursor(gpio_menu_cursor, dir, GPIO_ITEM_COUNT, wrap);
        } else if (menu_state == MENU_STATE_GPIO_PIN_EDIT) {
          buzzer_encoder_tick_melody();
          step_gpio_pin_idx(dir, wrap);
        }
      #endif
      #if MCU_VARIANT == MCU_ESP32
        else if (menu_state == MENU_STATE_MEM_LIST) {
          buzzer_encoder_tick_melody();
          mem_menu_cursor = menu_clamp_cursor(mem_menu_cursor, dir, MEM_ITEM_COUNT, wrap);
        } else if (menu_state == MENU_STATE_MEM_DETAIL) {
          buzzer_encoder_tick_melody();
          mem_detail_cursor = menu_clamp_cursor(mem_detail_cursor, dir, MEM_DETAIL_ITEM_COUNT, wrap);
        }
      #endif
    #endif
    #if HAS_OTA == true
      else if (menu_state == MENU_STATE_FWUPD_LIST) {
        buzzer_encoder_tick_melody();
        fwupd_menu_cursor = menu_clamp_cursor(fwupd_menu_cursor, dir, FWUPD_ITEM_COUNT, wrap);
      } else if (menu_state == MENU_STATE_FWUPD_CONFIRM) {
        // Plain 2-item list (UPDATE/CANCEL) - same tap-to-move/hold-to-
        // select navigation as everywhere else, same pattern as WiFi's
        // SAVE/DISCARD (MENU_STATE_WIFI_TEXT_CONFIRM/text_confirm_cursor).
        buzzer_encoder_tick_melody();
        fwupd_confirm_cursor = menu_clamp_cursor(fwupd_confirm_cursor, dir, 2, wrap);
      }
    #endif
  }

  void menu_confirm_select();

  // Shared by the encoder's long-press-from-closed and the main button's
  // dedicated open-menu hold duration (see button_event()) - no-ops if the
  // console/firmware-update is active or the device hasn't finished init.
  void menu_open_from_closed() {
    if (!console_active && !firmware_update_mode && device_init_done) {
      buzzer_encoder_click_melody();
      menu_stage_from_live();
      menu_state  = MENU_STATE_LIST;
      menu_cursor = 0;
      // Reached both via menu_encoder_button() (already sets this) and
      // directly from button_event() on a plain main-button hold (which
      // doesn't) - set unconditionally so the idle-close timer (see
      // menu_timeout_process()) always starts fresh from the actual open,
      // not from menu_last_activity_ms's stale/zero value.
      menu_last_activity_ms = millis();
    }
  }

  void menu_encoder_button(unsigned long duration) {
    menu_last_activity_ms = millis();
    display_unblank();

    #if HAS_WIFI == true || HAS_ETHERNET == true
      if (menu_state == MENU_STATE_STATUS_POPUP) {
        // Not a real navigable screen - any input (short click or long
        // press alike) dismisses it, rather than the usual short=confirm/
        // long=commit-and-exit split below (which would otherwise close
        // the *entire* menu on a long press here, not just this popup).
        buzzer_encoder_click_melody();
        menu_state = menu_popup_return_state;
        return;
      }
    #endif

    if (duration > 700) {
      // Long-press: identical from anywhere inside the menu - commit & exit.
      // Text entry no longer needs an exception here - SAVE is a wheel
      // position now (see WHEEL_QUICKSAVE_IDX), reached with the exact same
      // confirm gesture as any character, so there's no longer a "dialing
      // all the way around" tedium to work around, and this can behave
      // like every other state.
      if (menu_state != MENU_STATE_CLOSED) {
        buzzer_encoder_click_melody();
        menu_commit_and_exit();
      } else {
        menu_open_from_closed();
      }
      return;
    }

    if (menu_state != MENU_STATE_CLOSED) buzzer_encoder_click_melody();
    menu_confirm_select();
  }

  // The "confirm/select at the current level" action - shared by the
  // encoder's short-click and the main button's long-press (see
  // menu_button_press()), so both controls behave identically here.
  void menu_confirm_select() {
    if (menu_state == MENU_STATE_LIST) {
      if (menu_cursor == MENU_ITEM_SAVE_EXIT) {
        menu_commit_and_exit();
      }
      #if HAS_WIFI == true
        else if (menu_cursor == MENU_ITEM_WIFI) {
          menu_state = MENU_STATE_WIFI_LIST;
          wifi_menu_cursor = 0;
        }
      #endif
      #if HAS_ETHERNET == true
        else if (menu_cursor == MENU_ITEM_ETHERNET) {
          menu_state = MENU_STATE_ETH_LIST;
          eth_menu_cursor = 0;
        }
      #endif
      #if HAS_RTC == true
        else if (menu_cursor == MENU_ITEM_RTC) {
          menu_state = MENU_STATE_RTC_LIST;
          rtc_menu_cursor = 0;
        }
      #endif
      #if HAS_GPS == true
        else if (menu_cursor == MENU_ITEM_GNSS) {
          menu_state = MENU_STATE_GNSS_LIST;
          gnss_menu_cursor = 0;
        }
      #endif
      #if HAS_ESPNOW == true
        else if (menu_cursor == MENU_ITEM_ESPNOW) {
          menu_state = MENU_STATE_ESPNOW_LIST;
          espnow_menu_cursor = 0;
        }
      #endif
      #if HAS_SENSORS == true
        else if (menu_cursor == MENU_ITEM_SENSORS) {
          menu_state = MENU_STATE_SENSORS_LIST;
          sensors_menu_cursor = 0;
        }
      #endif
      #if MENU_HAS_HW_PAGE == true
        else if (menu_cursor == MENU_ITEM_HARDWARE) {
          menu_state = MENU_STATE_HW_LIST;
          hw_menu_cursor = 0;
        }
      #endif
      #if HAS_OTA == true
        else if (menu_cursor == MENU_ITEM_FW_UPDATE) {
          menu_state = MENU_STATE_FWUPD_LIST;
          fwupd_menu_cursor = 0;
          // Fetched fresh every time this list is opened from the main
          // menu (not on every redraw, and not re-fetched if you back out
          // of MENU_STATE_FWUPD_CONFIRM's CANCEL back to here) - same
          // "blocks briefly, live popup" reasoning as Sync NTP.
          if (ota_network_up()) {
            menu_draw_popup("CHECKING...");
            long current; bool newer;
            fwupd_latest_ok = ota_do_check(&current, &fwupd_latest_build, &newer);
          } else {
            fwupd_latest_ok = false;
          }
        }
      #endif
      else {
        menu_edit_field = menu_cursor;
        menu_state = MENU_STATE_EDIT;
        last_numeric_rotate_ms = millis(); // first tick on a field always starts at base speed
      }
    } else if (menu_state == MENU_STATE_EDIT) {
      if (menu_edit_field == MENU_ITEM_DISPLAY_BRIGHTNESS) {
        // Brightness gets a live preview the moment it's confirmed, rather
        // than waiting for the whole menu to be committed - EEPROM write
        // is still deferred to menu_commit_and_exit() (see
        // live_display_brightness).
        display_intensity = staged_display_brightness;
      }
      menu_state = MENU_STATE_LIST; // confirms staged value, no EEPROM write yet
    }
    #if HAS_WIFI == true
      else if (menu_state == MENU_STATE_WIFI_LIST) {
        if (wifi_menu_cursor == WIFI_ITEM_BACK) {
          menu_state = MENU_STATE_LIST;
        } else if (wifi_menu_cursor == WIFI_ITEM_MODE || wifi_menu_cursor == WIFI_ITEM_CHANNEL) {
          menu_state = MENU_STATE_WIFI_EDIT;
        } else if (wifi_menu_cursor == WIFI_ITEM_IP || wifi_menu_cursor == WIFI_ITEM_NETMASK ||
                   wifi_menu_cursor == WIFI_ITEM_GATEWAY || wifi_menu_cursor == WIFI_ITEM_DNS) {
          // A starting point to adjust from is friendlier than dialing
          // every octet up from zero - but only applied here, the moment
          // the field is actually opened for editing, not at whole-menu-
          // open time (see menu_stage_from_live()) - staying unset until
          // someone actually opens the field is the whole point. Netmask
          // has no similarly obvious default, so it alone stays at 0.0.0.0.
          if (wifi_menu_cursor == WIFI_ITEM_IP &&
              staged_wifi_ip[0] == 0 && staged_wifi_ip[1] == 0 && staged_wifi_ip[2] == 0 && staged_wifi_ip[3] == 0) {
            staged_wifi_ip[0] = 192; staged_wifi_ip[1] = 168; staged_wifi_ip[2] = 0; staged_wifi_ip[3] = 32;
          }
          if (wifi_menu_cursor == WIFI_ITEM_GATEWAY &&
              staged_wifi_gw[0] == 0 && staged_wifi_gw[1] == 0 && staged_wifi_gw[2] == 0 && staged_wifi_gw[3] == 0) {
            staged_wifi_gw[0] = 192; staged_wifi_gw[1] = 168; staged_wifi_gw[2] = 0; staged_wifi_gw[3] = 1;
          }
          if (wifi_menu_cursor == WIFI_ITEM_DNS &&
              staged_wifi_dns[0] == 0 && staged_wifi_dns[1] == 0 && staged_wifi_dns[2] == 0 && staged_wifi_dns[3] == 0) {
            staged_wifi_dns[0] = 1; staged_wifi_dns[1] = 1; staged_wifi_dns[2] = 1; staged_wifi_dns[3] = 1;
          }
          wifi_addr_octet_idx = 0;
          menu_state = MENU_STATE_WIFI_ADDR_EDIT;
        } else if (wifi_menu_cursor == WIFI_ITEM_CLEAR) {
          // Single-confirm action (same as SAVE & EXIT above) - staged
          // only, deferred to the whole menu's SAVE & EXIT like everything
          // else here (the popup below returns to WIFI_LIST, not straight
          // out of the menu, so that's still true after dismissing it).
          staged_wifi_ip[0] = staged_wifi_ip[1] = staged_wifi_ip[2] = staged_wifi_ip[3] = 0;
          staged_wifi_nm[0] = staged_wifi_nm[1] = staged_wifi_nm[2] = staged_wifi_nm[3] = 0;
          staged_wifi_gw[0] = staged_wifi_gw[1] = staged_wifi_gw[2] = staged_wifi_gw[3] = 0;
          staged_wifi_dns[0] = staged_wifi_dns[1] = staged_wifi_dns[2] = staged_wifi_dns[3] = 0;
          menu_open_popup("CLEARED", MENU_STATE_WIFI_LIST);
        } else if (wifi_menu_cursor == WIFI_ITEM_SSID || wifi_menu_cursor == WIFI_ITEM_PSK) {
          // Fresh text-edit session, preloaded from the current staged
          // value, wheel starts at 'a'.
          text_edit_field = wifi_menu_cursor;
          const char *src = (text_edit_field == WIFI_ITEM_SSID) ? staged_wifi_ssid : staged_wifi_psk;
          strncpy(text_edit_buf, src, 32); text_edit_buf[32] = 0;
          wheel_index = 3; // 'a' (0=space, 1=DEL, 2=SAVE)
          menu_state = MENU_STATE_WIFI_TEXT_EDIT;
        }
      } else if (menu_state == MENU_STATE_WIFI_EDIT) {
        menu_state = MENU_STATE_WIFI_LIST; // confirms staged value, no write yet
      } else if (menu_state == MENU_STATE_WIFI_ADDR_EDIT) {
        if (wifi_addr_octet_idx < 3) {
          // Not the last octet yet - just advance, same screen.
          wifi_addr_octet_idx++;
        } else {
          // A manually-set IP with no netmask configured yet is a common
          // footgun - default to the overwhelmingly common /24 rather than
          // requiring a separate trip through Netmask too. Only fills in an
          // unset netmask (checked against the staged working copy, not
          // live EEPROM - both stay in sync for the whole menu session
          // here, unlike Ethernet's per-field-immediate-commit design)
          // - leaves an already-set one alone.
          if (wifi_menu_cursor == WIFI_ITEM_IP) {
            bool nm_unset = (staged_wifi_nm[0] == 0 && staged_wifi_nm[1] == 0 && staged_wifi_nm[2] == 0 && staged_wifi_nm[3] == 0);
            if (nm_unset) {
              staged_wifi_nm[0] = 255; staged_wifi_nm[1] = 255; staged_wifi_nm[2] = 255; staged_wifi_nm[3] = 0;
            }
          }
          // No write here - deferred to SAVE & EXIT (menu_commit_and_exit())
          // like every other field in this submenu, unlike Ethernet's own
          // IP Address/Netmask which commit immediately.
          menu_state = MENU_STATE_WIFI_LIST;
        }
      } else if (menu_state == MENU_STATE_WIFI_TEXT_EDIT) {
        uint8_t len = strlen(text_edit_buf);
        if (wheel_is_del(wheel_index)) {
          if (len > 0) text_edit_buf[len-1] = 0;
        } else if (wheel_is_save(wheel_index)) {
          // Same dialog a long-press used to reach on encoder boards only -
          // now reachable identically (dial to it, then the same confirm
          // gesture as any character) on both encoder and button-only
          // boards, so that special-cased long-press no longer exists (see
          // menu_encoder_button()).
          text_confirm_cursor = 0; // default to SAVE
          menu_state = MENU_STATE_WIFI_TEXT_CONFIRM;
        } else if (len < 32) {
          text_edit_buf[len] = wheel_char_at(wheel_index);
          text_edit_buf[len+1] = 0;
          wheel_index = 3; // reset to 'a' for the next character
        }
      } else if (menu_state == MENU_STATE_WIFI_TEXT_CONFIRM) {
        if (text_confirm_cursor == 0) { // SAVE
          char *dst = (text_edit_field == WIFI_ITEM_SSID) ? staged_wifi_ssid : staged_wifi_psk;
          strncpy(dst, text_edit_buf, 32); dst[32] = 0;
        }
        // DISCARD: leave staged_wifi_ssid/psk untouched.
        menu_state = MENU_STATE_WIFI_LIST;
      }
    #endif
    #if HAS_ETHERNET == true
      else if (menu_state == MENU_STATE_ETH_LIST) {
        // Link Status is read-only - only BACK, Speed, IP Address, and
        // Netmask do anything.
        if (eth_menu_cursor == ETH_ITEM_BACK) {
          menu_state = MENU_STATE_LIST;
        } else if (eth_menu_cursor == ETH_ITEM_SPEED) {
          staged_eth_speed_mode = eth_speed_mode;
          menu_state = MENU_STATE_ETH_EDIT;
        } else if (eth_menu_cursor == ETH_ITEM_IP || eth_menu_cursor == ETH_ITEM_NETMASK ||
                   eth_menu_cursor == ETH_ITEM_GATEWAY || eth_menu_cursor == ETH_ITEM_DNS) {
          int addr_base = eth_addr_base(eth_menu_cursor);
          uint8_t *staged = eth_staged_addr_field(eth_menu_cursor);
          if (!addr4_read(addr_base, staged)) {
            // Never configured - a starting point to adjust from is friendlier
            // than making someone dial every octet up from zero. Netmask has
            // no similarly obvious default, so it still starts at 0.0.0.0.
            if (eth_menu_cursor == ETH_ITEM_IP) {
              staged[0] = 192; staged[1] = 168; staged[2] = 0; staged[3] = 32;
            } else if (eth_menu_cursor == ETH_ITEM_GATEWAY) {
              staged[0] = 192; staged[1] = 168; staged[2] = 0; staged[3] = 1;
            } else if (eth_menu_cursor == ETH_ITEM_DNS) {
              staged[0] = 1; staged[1] = 1; staged[2] = 1; staged[3] = 1;
            } else {
              staged[0] = staged[1] = staged[2] = staged[3] = 0;
            }
          }
          eth_addr_octet_idx = 0;
          menu_state = MENU_STATE_ETH_ADDR_EDIT;
        } else if (eth_menu_cursor == ETH_ITEM_CLEAR) {
          // Single-confirm action (same as SAVE & EXIT above), not a field -
          // stays on ETH_LIST, which redraws showing "DHCP"/"NONE" for all
          // four rows.
          uint8_t tmp[4];
          bool any_set = addr4_read(ADDR_CONF_ETH_IP, tmp) || addr4_read(ADDR_CONF_ETH_NM, tmp) ||
                         addr4_read(ADDR_CONF_ETH_GW, tmp) || addr4_read(ADDR_CONF_ETH_DNS, tmp);
          if (any_set) {
            uint8_t zero[4] = {0, 0, 0, 0};
            ethaddr_conf_save(ADDR_CONF_ETH_IP, zero);
            ethaddr_conf_save(ADDR_CONF_ETH_NM, zero);
            ethaddr_conf_save(ADDR_CONF_ETH_GW, zero);
            ethaddr_conf_save(ADDR_CONF_ETH_DNS, zero);
            // true: actually un-applying a previously-set static config
            // here, unlike init_ethernet()'s boot-time call - see
            // eth_apply_addr_config()'s own comment (Ethernet.h).
            eth_apply_addr_config(true);
            menu_open_popup("CLEARED", MENU_STATE_ETH_LIST);
          }
        }
      } else if (menu_state == MENU_STATE_ETH_EDIT) {
        // Commits straight to EEPROM here rather than staging until SAVE &
        // EXIT - this only takes effect at boot (see ethspd_conf_save(),
        // Utilities.h), so there's no reason to make leaving it uncommitted
        // discard the change like every other field does.
        if (staged_eth_speed_mode != eth_speed_mode) {
          ethspd_conf_save(staged_eth_speed_mode);
        }
        menu_state = MENU_STATE_ETH_LIST;
      } else if (menu_state == MENU_STATE_ETH_ADDR_EDIT) {
        if (eth_addr_octet_idx < 3) {
          // Not the last octet yet - just advance, same screen.
          eth_addr_octet_idx++;
        } else {
          // Last octet confirmed - commit and apply live (no reboot needed,
          // see ethaddr_conf_save()/eth_apply_addr_config()).
          int addr_base = eth_addr_base(eth_menu_cursor);
          uint8_t *staged = eth_staged_addr_field(eth_menu_cursor);
          uint8_t live[4];
          addr4_read(addr_base, live);
          bool addr_changed = false;
          for (uint8_t i = 0; i < 4; i++) { if (staged[i] != live[i]) { addr_changed = true; break; } }
          if (addr_changed) {
            ethaddr_conf_save(addr_base, staged);
            // A manually-set IP with no netmask configured yet is a common
            // footgun - default to the overwhelmingly common /24 rather
            // than requiring a separate trip through Netmask too. Leaves an
            // already-set netmask alone - only fills in an unset one.
            if (eth_menu_cursor == ETH_ITEM_IP) {
              uint8_t nm_tmp[4];
              if (!addr4_read(ADDR_CONF_ETH_NM, nm_tmp)) {
                uint8_t nm_255[4] = {255, 255, 255, 0};
                ethaddr_conf_save(ADDR_CONF_ETH_NM, nm_255);
              }
            }
            eth_apply_addr_config();
          }
          menu_state = MENU_STATE_ETH_LIST;
        }
      }
    #endif
    #if HAS_RTC == true
      else if (menu_state == MENU_STATE_RTC_LIST) {
        // Time/Date are read-only readouts - only BACK and Set Time/Date do
        // anything, same as Ethernet's Link Status above.
        if (rtc_menu_cursor == RTC_ITEM_BACK) {
          menu_state = MENU_STATE_LIST;
        } else if (rtc_menu_cursor == RTC_ITEM_SET) {
          // Sync fresh from the live RTC reading, not a whole-menu-session
          // staged copy - this field commits straight to the RTC chip on
          // its own confirm, not deferred to SAVE & EXIT, so a live reading
          // is the only value that can ever be stale here (same reasoning
          // as Ethernet's own IP Address/Speed, Hardware's Voltage Divider).
          uint32_t epoch = rtc_get_unixtime();
          int32_t days = (int32_t)(epoch / 86400UL);
          uint32_t rem  = epoch % 86400UL;
          staged_rtc_hour   = (uint8_t)(rem / 3600); rem %= 3600;
          staged_rtc_minute = (uint8_t)(rem / 60);
          staged_rtc_second = (uint8_t)(rem % 60);
          uint32_t m, d;
          rtc_civil_from_days(days, staged_rtc_year, m, d);
          staged_rtc_month = (uint8_t)m;
          staged_rtc_day   = (uint8_t)d;
          rtc_edit_field_idx = 0;
          menu_state = MENU_STATE_RTC_EDIT;
        } else if (rtc_menu_cursor == RTC_ITEM_TIMEZONE) {
          // Sync fresh from the live value, same immediate-commit
          // reasoning as Set Time/Date above.
          staged_tz_offset_qh = rtc_get_tz_offset_qh();
          menu_state = MENU_STATE_RTC_TZ_EDIT;
        }
        #if MCU_VARIANT == MCU_ESP32 && (HAS_WIFI == true || HAS_ETHERNET == true)
          else if (rtc_menu_cursor == RTC_ITEM_SYNC_NTP) {
            // Blocks briefly (up to NTP_SYNC_TIMEOUT_MS, RTC.h, on failure)
            // - acceptable for an explicit, rarely-used action. Opens the
            // popup showing "CONNECTING" first, then rtc_sync_ntp() itself
            // updates it live ("Resolving"/"Syncing") as it moves through
            // each stage (menu_draw_popup matches its status_cb signature
            // exactly) - this blocks loop() the whole time, so nothing
            // else would ever redraw the screen otherwise. The final
            // result then replaces it the same way. On success there's
            // nothing to acknowledge, so it auto-dismisses back to
            // RTC_LIST after NTP_SYNC_SUCCESS_POPUP_MS with no input
            // needed (menu_popup_process(), polled from loop()) - whose
            // Time/Date rows already recompute from the live RTC on every
            // redraw, so the synced time is visible there right away. Any
            // failure instead stays up until dismissed, same as before -
            // worth making sure that was actually seen.
            menu_open_popup("CONNECTING", MENU_STATE_RTC_LIST);
            uint8_t result = rtc_sync_ntp(menu_draw_popup);
            if (result == NTP_SYNC_OK) {
              menu_draw_popup_timed(ntp_result_text(result), NTP_SYNC_SUCCESS_POPUP_MS);
            } else {
              menu_draw_popup(ntp_result_text(result));
            }
          }
        #endif
        #if HAS_GPS == true
          else if (rtc_menu_cursor == RTC_ITEM_SYNC_GPS) {
            // Unlike Sync NTP above, this never blocks - GNSS.h continuously
            // parses NMEA in the background (gnss_update(), polled from
            // loop()), so there's no "CONNECTING" stage to show first, just
            // the final result (rtc_sync_gps(), RTC.h). Same auto-dismiss-
            // on-success-only behavior as Sync NTP otherwise.
            uint8_t result = rtc_sync_gps();
            menu_open_popup(gps_result_text(result), MENU_STATE_RTC_LIST);
            if (result == GPS_SYNC_OK) {
              menu_popup_auto_dismiss_at = millis() + GPS_SYNC_SUCCESS_POPUP_MS;
            }
          }
        #endif
      } else if (menu_state == MENU_STATE_RTC_EDIT) {
        if (rtc_edit_field_idx < 5) {
          // Not the last field yet - just advance, same screen.
          rtc_edit_field_idx++;
        } else {
          // Last field confirmed - commit straight to the RTC chip (no
          // reboot needed, unlike GPIO/Ethernet Speed's pin/link changes).
          int32_t days = rtc_days_from_civil(staged_rtc_year, staged_rtc_month, staged_rtc_day);
          uint32_t epoch = (uint32_t)days * 86400UL + (uint32_t)staged_rtc_hour * 3600UL + (uint32_t)staged_rtc_minute * 60UL + staged_rtc_second;
          rtc_set_unixtime(epoch);
          menu_state = MENU_STATE_RTC_LIST;
        }
      } else if (menu_state == MENU_STATE_RTC_TZ_EDIT) {
        // Commits straight to EEPROM here rather than staging until SAVE &
        // EXIT - display-only, nothing to reboot or re-init, same
        // immediate-commit pattern as Set Time/Date above.
        uint8_t live_raw = (uint8_t)(rtc_get_tz_offset_qh() + TZ_OFFSET_RAW_ZERO);
        uint8_t new_raw  = (uint8_t)(staged_tz_offset_qh + TZ_OFFSET_RAW_ZERO);
        if (new_raw != live_raw) { tz_conf_save(new_raw); }
        menu_state = MENU_STATE_RTC_LIST;
      }
      // No MENU_STATE_STATUS_POPUP branch here - it's not reached via this
      // path at all. menu_button_press()/menu_encoder_button()/
      // menu_encoder_rotate() each dismiss it directly, before ever
      // calling into menu_confirm_select() (see those functions).
    #endif
    #if HAS_GPS == true
      else if (menu_state == MENU_STATE_GNSS_LIST) {
        // Chip/Fix/Satellites/Latitude/Longitude/Altitude are read-only -
        // only BACK and Enabled do anything, same as RTC's Time/Date rows.
        if (gnss_menu_cursor == GNSS_ITEM_BACK) {
          menu_state = MENU_STATE_LIST;
        } else if (gnss_menu_cursor == GNSS_ITEM_ENABLED) {
          // Sync fresh from the live value, same immediate-commit
          // reasoning as RTC's own Timezone field above.
          staged_gnss_enabled = gnss_enabled;
          menu_state = MENU_STATE_GNSS_EDIT;
        }
      } else if (menu_state == MENU_STATE_GNSS_EDIT) {
        // Commits + live power-cycles the receiver here rather than
        // staging until SAVE & EXIT - a power-saving toggle should apply
        // the instant it's confirmed, same immediate-commit pattern as
        // RTC's Timezone field.
        if (staged_gnss_enabled != gnss_enabled) {
          gnss_conf_save(staged_gnss_enabled);
          gnss_set_enabled(staged_gnss_enabled);
        }
        menu_state = MENU_STATE_GNSS_LIST;
      }
    #endif
    #if HAS_ESPNOW == true
      else if (menu_state == MENU_STATE_ESPNOW_LIST) {
        // Channel is read-only (see its own declaration) - only
        // ENABLED/MODE/LR actually open the shared edit screen.
        if (espnow_menu_cursor == ESPNOW_ITEM_BACK) {
          menu_state = MENU_STATE_LIST;
        } else if (espnow_menu_cursor == ESPNOW_ITEM_ENABLED ||
                   espnow_menu_cursor == ESPNOW_ITEM_MODE ||
                   espnow_menu_cursor == ESPNOW_ITEM_LR) {
          menu_state = MENU_STATE_ESPNOW_EDIT;
        }
      } else if (menu_state == MENU_STATE_ESPNOW_EDIT) {
        // Same deferred-commit reasoning as WiFi's own Mode field - all
        // three fields reboot on change (see menu_commit_and_exit()'s
        // combined-write comment), so nothing is written here, only staged.
        // Exception: turning LR ON specifically also disables WiFi remote
        // (see the wifi_remote_init() veto, RNode_Firmware.ino) - warn and
        // require an explicit confirm before accepting that, same as F/W
        // Update's UPDATE/CANCEL gate. Only trips on the OFF->ON edge (not
        // if LR was already on and is just being left alone), so re-opening
        // an already-enabled LR field doesn't nag needlessly.
        if (espnow_menu_cursor == ESPNOW_ITEM_LR && staged_espnow_lr_enabled && !espnow_lr_enabled) {
          espnow_lr_confirm_cursor = 2; // default CANCEL - see its own declaration
          menu_state = MENU_STATE_ESPNOW_LR_CONFIRM;
        } else {
          menu_state = MENU_STATE_ESPNOW_LIST;
        }
      } else if (menu_state == MENU_STATE_ESPNOW_LR_CONFIRM) {
        if (espnow_lr_confirm_cursor == 0) {
          // Info row ("Disables WiFi") - inert, same read-only-row
          // convention as FWUPD_LIST/GNSS_LIST's non-actionable rows.
          // Stay on this screen rather than falling through to either
          // choice below.
        } else {
          if (espnow_lr_confirm_cursor == 2) { // CANCEL - revert to off
            staged_espnow_lr_enabled = false;
          }
          // ENABLE (1) just keeps staged_espnow_lr_enabled as-is (true) -
          // either way, nothing is written yet, same deferred-commit
          // reasoning above.
          menu_state = MENU_STATE_ESPNOW_LIST;
        }
      }
    #endif
    #if HAS_SENSORS == true
      else if (menu_state == MENU_STATE_SENSORS_LIST) {
        // Every row is read-only - only BACK does anything, same as the
        // Hardware page's own info-only rows.
        if (sensors_menu_cursor == SENSORS_ITEM_BACK) {
          menu_state = MENU_STATE_LIST;
        }
      }
    #endif
    #if MENU_HAS_HW_PAGE == true
      else if (menu_state == MENU_STATE_HW_LIST) {
        // Mostly read-only info page - only BACK, and Input Voltage/Battery
        // Voltage (if present), do anything.
        if (hw_menu_cursor == HW_ITEM_BACK) {
          menu_state = MENU_STATE_LIST;
        }
        #if HAS_VSENSE == true
          else if (hw_menu_cursor == HW_ITEM_VOLTAGE) {
            // Sync from the live value fresh every time this screen is
            // opened, not from a whole-menu-session staged copy - this
            // field commits to EEPROM immediately on its own BACK/confirm,
            // not deferred to SAVE & EXIT, so live vsense_divider_ratio is
            // the only value that can ever be stale here.
            staged_vsense_divider_ratio_raw = (uint8_t)(vsense_divider_ratio * 10.0 + 0.5);
            menu_state = MENU_STATE_HW_EDIT;
            last_numeric_rotate_ms = millis();
          }
        #endif
        #if HAS_BATTERY_DIVIDER == true
          else if (hw_menu_cursor == HW_ITEM_BATTERY) {
            // Same immediate-commit-on-its-own-screen pattern as Input
            // Voltage above - see bvs_conf_save().
            staged_battery_v_scale_pct = (uint8_t)((battery_v_scale / BATTERY_V_SCALE_DEFAULT) * 100.0 + 0.5);
            menu_state = MENU_STATE_HW_EDIT;
            last_numeric_rotate_ms = millis();
          }
        #endif
        #if HAS_GPIO_MENU == true
          else if (hw_menu_cursor == HW_ITEM_GPIO) {
            menu_state = MENU_STATE_GPIO_LIST;
            gpio_menu_cursor = 0;
          }
        #endif
        #if MCU_VARIANT == MCU_ESP32
          else if (hw_menu_cursor == HW_ITEM_MEMORY) {
            menu_state = MENU_STATE_MEM_LIST;
            mem_menu_cursor = MEM_ITEM_BACK; // read-only info screen - default to BACK, not the first graph row
          }
        #endif
      }
      #if HAS_VSENSE == true || HAS_BATTERY_DIVIDER == true
        else if (menu_state == MENU_STATE_HW_EDIT) {
          // Commits straight to EEPROM here rather than staging until
          // SAVE & EXIT - this is board calibration data, not a live
          // setting, so there's no reason to make leaving it uncommitted
          // discard the change like every other field does.
          #if HAS_VSENSE == true
            if (hw_menu_cursor == HW_ITEM_VOLTAGE) {
              uint8_t live_raw = (uint8_t)(vsense_divider_ratio * 10.0 + 0.5);
              if (staged_vsense_divider_ratio_raw != live_raw) {
                vsr_conf_save(staged_vsense_divider_ratio_raw);
              }
            }
          #endif
          #if HAS_BATTERY_DIVIDER == true
            if (hw_menu_cursor == HW_ITEM_BATTERY) {
              uint8_t live_pct = (uint8_t)((battery_v_scale / BATTERY_V_SCALE_DEFAULT) * 100.0 + 0.5);
              if (staged_battery_v_scale_pct != live_pct) {
                bvs_conf_save(staged_battery_v_scale_pct);
              }
            }
          #endif
          menu_state = MENU_STATE_HW_LIST;
        }
      #endif
      #if HAS_GPIO_MENU == true
        else if (menu_state == MENU_STATE_GPIO_LIST) {
          if (gpio_menu_cursor == GPIO_ITEM_BACK) {
            menu_state = MENU_STATE_HW_LIST;
          } else {
            // Sync from the live pin fresh every time this screen is
            // opened, same reasoning as Input Voltage/Battery Cal above.
            staged_gpio_pin_idx = gpio_idx_for_pin(gpio_item_live_pin(gpio_menu_cursor));
            menu_state = MENU_STATE_GPIO_PIN_EDIT;
          }
        } else if (menu_state == MENU_STATE_GPIO_PIN_EDIT) {
          // Commits straight to EEPROM (and reboots if changed, since a
          // pin reassignment only takes effect at boot - see
          // gpio_conf_save()), same immediate-commit pattern as Input
          // Voltage/Battery Cal above.
          uint8_t new_pin = gpio_free_pin_candidates[staged_gpio_pin_idx];
          if (new_pin != gpio_item_live_pin(gpio_menu_cursor)) {
            gpio_conf_save(gpio_item_addr(gpio_menu_cursor), new_pin);
          }
          menu_state = MENU_STATE_GPIO_LIST;
        }
      #endif
      #if MCU_VARIANT == MCU_ESP32
        else if (menu_state == MENU_STATE_MEM_LIST) {
          // Selecting Heap or PSRAM drops into its detail readout - same
          // "list row opens a submenu" pattern as HW_ITEM_GPIO. mem_menu_cursor
          // itself is left as-is (still MEM_ITEM_HEAP/MEM_ITEM_PSRAM), so
          // MENU_STATE_MEM_DETAIL's draw/confirm code can tell which metric
          // it's showing - same reuse-the-parent-cursor pattern HW_EDIT
          // uses for hw_menu_cursor.
          if (mem_menu_cursor == MEM_ITEM_BACK) {
            menu_state = MENU_STATE_HW_LIST;
          } else {
            mem_detail_cursor = 0;
            menu_state = MENU_STATE_MEM_DETAIL;
          }
        } else if (menu_state == MENU_STATE_MEM_DETAIL) {
          // Fully read-only - only BACK does anything.
          if (mem_detail_cursor == MEM_DETAIL_ITEM_BACK) {
            menu_state = MENU_STATE_MEM_LIST;
          }
        }
      #endif
    #endif
    #if HAS_OTA == true
      else if (menu_state == MENU_STATE_FWUPD_LIST) {
        // CURRENT/LATEST are read-only info rows (fetched once on opening
        // this list from the main menu - see MENU_ITEM_FW_UPDATE above) -
        // only BACK and UPDATE do anything, same as HW_LIST's read-only
        // rows above.
        if (fwupd_menu_cursor == FWUPD_ITEM_BACK) {
          menu_state = MENU_STATE_LIST;
        } else if (fwupd_menu_cursor == FWUPD_ITEM_UPDATE) {
          fwupd_confirm_cursor = 1; // default CANCEL - see its own declaration
          menu_state = MENU_STATE_FWUPD_CONFIRM;
        }
      } else if (menu_state == MENU_STATE_FWUPD_CONFIRM) {
        if (fwupd_confirm_cursor == 0) { // UPDATE
          // Blocks for the whole download - menu_draw_popup as the
          // progress callback keeps the screen live throughout (same
          // pattern as Sync NTP/rtc_sync_ntp()). Success reboots from
          // inside ota_reboot() and never returns.
          const esp_partition_t *target = ota_do_pull_download(menu_draw_popup);
          if (target && ota_verify_and_set_boot(target)) {
            menu_draw_popup("INSTALLING...");
            ota_reboot(menu_draw_popup);
          } else {
            menu_open_popup("UPDATE FAILED", MENU_STATE_FWUPD_LIST);
          }
        } else { // CANCEL
          menu_state = MENU_STATE_FWUPD_LIST;
        }
      }
    #endif
    // menu_state == MENU_STATE_CLOSED + short click: no-op (reserved).
  }

  // The main button drives the menu everywhere except WIFI_TEXT_EDIT (which
  // keeps it as a dedicated backspace key - see menu_main_button_del(),
  // called separately by button_event() for that one state). On boards
  // without an encoder (HAS_MENU without HAS_ENCODER - see Boards.h) this is
  // the only control; on encoder boards it's an alternate one. Short press
  // cycles forward
  // through the current level, same as one encoder detent; a quick second
  // short press (double-tap) cycles backward instead - see
  // menu_btn_pending/menu_button_process(). Long press confirms/selects,
  // same as an encoder short-click. 150-499ms is a dead zone (no-op) so an
  // imprecise press doesn't do either by accident.
  void menu_button_press(unsigned long duration) {
    menu_last_activity_ms = millis();
    display_unblank();
    #if HAS_WIFI == true || HAS_ETHERNET == true
      if (menu_state == MENU_STATE_STATUS_POPUP) {
        // Not a real navigable screen - a single short tap dismisses it
        // immediately, no need for the usual double-tap-pending wait
        // (there's nothing to go "back" from here) or to wait for a long
        // press. duration < 150 is still the dead zone below this, same
        // as everywhere else.
        if (duration >= 150) {
          menu_btn_pending = false;
          buzzer_encoder_click_melody();
          menu_state = menu_popup_return_state;
        }
        return;
      }
    #endif
    if (duration < 150) {
      unsigned long now = millis();
      if (menu_btn_pending && (now - menu_btn_last_click) <= MENU_BTN_DOUBLE_TAP_WINDOW) {
        // Second tap of a double-tap: cancel the deferred single-tap
        // forward step and go backward instead.
        menu_btn_pending = false;
        menu_encoder_rotate(-1, true);
      } else {
        // First tap: hold off in case a second tap follows within the
        // window - menu_button_process() (polled from loop()) fires the
        // forward step once the window elapses without a second tap.
        menu_btn_pending = true;
        menu_btn_last_click = now;
      }
    } else if (duration >= 500) {
      menu_btn_pending = false; // long-press supersedes any pending tap
      if (menu_state != MENU_STATE_CLOSED) buzzer_encoder_click_melody();
      menu_confirm_select();
    }
  }

  // Fires the deferred single-tap forward step once the double-tap window
  // has elapsed without a second tap. Polled from loop() regardless of
  // menu state - harmless no-op when nothing is pending.
  void menu_button_process() {
    if (menu_btn_pending && millis()-menu_btn_last_click > MENU_BTN_DOUBLE_TAP_WINDOW) {
      menu_btn_pending = false;
      menu_encoder_rotate(1, true);
    }
  }

  // Org_01 glyphs sit 4px above and 1px below the setCursor() baseline, so
  // the highlight rect must start a few px above the row's text baseline,
  // not right at it, or the glyph tops get clipped by the rect's own top
  // edge. Shows up to 4 rows at a time, scrolling to keep the cursor
  // visible - lists have grown past 4 items and will likely keep growing.
  void draw_menu_list_disp(const char *title, const char **labels, char valbufs[][24], uint8_t count, uint8_t cursor) {
    MENU_GFX.setFont(MENU_FONT);
    MENU_GFX.setTextSize(1);
    MENU_GFX.setTextColor(SSD1306_WHITE);
    MENU_GFX.setCursor(6, 8);
    MENU_GFX.print(title);
    MENU_GFX.drawFastHLine(4, 12, MENU_CONTENT_W, SSD1306_WHITE);

    const uint8_t row_h = MENU_LIST_ROW_H;
    const uint8_t visible_rows = MENU_LIST_VISIBLE_ROWS;
    uint8_t first = 0;
    if (count > visible_rows) {
      if (cursor >= visible_rows) first = cursor - visible_rows + 1;
      if (first > count - visible_rows) first = count - visible_rows;
    }

    for (uint8_t vi = 0; vi < visible_rows && (first + vi) < count; vi++) {
      uint8_t i = first + vi;
      uint8_t row_top = MENU_LIST_TOP_Y + vi * row_h;
      uint8_t y = row_top + MENU_LIST_BASELINE_OFF; // text baseline
      if (i == cursor) {
        MENU_GFX.fillRect(4, row_top, MENU_CONTENT_W, row_h - 1, SSD1306_WHITE);
        MENU_GFX.setTextColor(SSD1306_BLACK);
      } else {
        MENU_GFX.setTextColor(SSD1306_WHITE);
      }
      MENU_GFX.setCursor(8, y);
      MENU_GFX.print(labels[i]);

      if (valbufs[i][0] != 0) {
        int16_t x1, y1; uint16_t w, h;
        MENU_GFX.getTextBounds(valbufs[i], 0, 0, &x1, &y1, &w, &h);
        MENU_GFX.setCursor(4 + MENU_CONTENT_W - 2 - w, y);
        MENU_GFX.print(valbufs[i]);
      }
    }

    MENU_GFX.setTextColor(SSD1306_WHITE);
    MENU_GFX.drawFastHLine(4, MENU_LIST_FOOTER_HLINE_Y, MENU_CONTENT_W, SSD1306_WHITE);
    MENU_GFX.setCursor(6, MENU_LIST_FOOTER_TEXT_Y);
    // Whether an encoder is actually populated is a runtime choice
    // (encoder_enabled) on boards where it's optional, not the compile-time
    // HAS_ENCODER capability flag - see MENU_ITEM_ENCODER.
    #if HAS_ENCODER == true
      if (encoder_enabled) MENU_GFX.print("turn:move press:open");
      else                 MENU_GFX.print("tap:next hold:open");
    #else
      MENU_GFX.print("tap:next hold:open");
    #endif
  }

  #if MCU_VARIANT == MCU_ESP32
    // Compact horizontal bar meter, sized to fit inside one MENU_LIST_ROW_H
    // row (outlined rect + a filled portion proportional to `frac`, no
    // embedded number) - the user asked for graphs "the same size as
    // regular lines", not a second text readout of a value HW_LIST's own
    // rows already print elsewhere.
    void draw_menu_bar_meter(int16_t x, int16_t y, int16_t w, int16_t h, float frac, uint16_t color) {
      if (frac < 0) frac = 0;
      if (frac > 1) frac = 1;
      MENU_GFX.drawRect(x, y, w, h, color);
      int16_t fill_w = (int16_t)((w - 2) * frac + 0.5);
      if (fill_w > 0) MENU_GFX.fillRect(x + 1, y + 1, fill_w, h - 2, color);
    }

    void draw_menu_memory_disp() {
      MENU_GFX.setFont(MENU_FONT);
      MENU_GFX.setTextSize(1);
      MENU_GFX.setTextColor(SSD1306_WHITE);
      MENU_GFX.setCursor(6, 8);
      MENU_GFX.print("MEMORY");
      MENU_GFX.drawFastHLine(4, 12, MENU_CONTENT_W, SSD1306_WHITE);

      const uint8_t row_h = MENU_LIST_ROW_H;
      const int16_t bar_x = 46;
      const int16_t bar_h = (row_h > 6) ? (row_h - 5) : (row_h - 2);

      // Percentage field is reserved at "100%"'s width (not each row's
      // actual string width) so the bar's right edge doesn't shift around
      // as the digit count changes - only the text within the field is
      // right-aligned per row.
      int16_t x1, y1; uint16_t pct_field_w, th;
      MENU_GFX.getTextBounds("100%", 0, 0, &x1, &y1, &pct_field_w, &th);
      const int16_t row_right  = 4 + MENU_CONTENT_W - 2;
      const int16_t pct_left   = row_right - pct_field_w;
      const int16_t bar_w      = (pct_left - 4) - bar_x;

      for (uint8_t i = 0; i < MEM_ITEM_COUNT; i++) {
        uint8_t row_top = MENU_LIST_TOP_Y + i * row_h;
        uint8_t y = row_top + MENU_LIST_BASELINE_OFF;
        uint16_t fg = SSD1306_WHITE;
        if (i == mem_menu_cursor) {
          MENU_GFX.fillRect(4, row_top, MENU_CONTENT_W, row_h - 1, SSD1306_WHITE);
          fg = SSD1306_BLACK;
        }
        MENU_GFX.setTextColor(fg);
        MENU_GFX.setCursor(8, y);

        if (i == MEM_ITEM_BACK) {
          MENU_GFX.print("BACK");
          continue;
        }

        int16_t bar_y = row_top + (row_h - bar_h) / 2;
        bool has_psram = psramFound();
        uint32_t total = 0, free_b = 0;
        bool have_reading = true;
        if (i == MEM_ITEM_HEAP) {
          MENU_GFX.print("Heap");
          total = ESP.getHeapSize();
          free_b = ESP.getFreeHeap();
        } else { // MEM_ITEM_PSRAM
          MENU_GFX.print("PSRAM");
          if (has_psram) {
            total = ESP.getPsramSize();
            free_b = ESP.getFreePsram();
          } else {
            have_reading = false;
          }
        }

        if (have_reading) {
          float used_frac = total ? (float)(total - free_b) / total : 0;
          draw_menu_bar_meter(bar_x, bar_y, bar_w, bar_h, used_frac, fg);

          char pctbuf[6];
          sprintf(pctbuf, "%d%%", (int)(used_frac * 100.0f + 0.5f));
          uint16_t pw, ph;
          MENU_GFX.getTextBounds(pctbuf, 0, 0, &x1, &y1, &pw, &ph);
          MENU_GFX.setCursor(row_right - pw, y);
          MENU_GFX.print(pctbuf);
        } else {
          uint16_t w, h;
          MENU_GFX.getTextBounds("N/A", 0, 0, &x1, &y1, &w, &h);
          MENU_GFX.setCursor(row_right - w, y);
          MENU_GFX.print("N/A");
        }
      }

      MENU_GFX.setTextColor(SSD1306_WHITE);
      MENU_GFX.drawFastHLine(4, MENU_LIST_FOOTER_HLINE_Y, MENU_CONTENT_W, SSD1306_WHITE);
      MENU_GFX.setCursor(6, MENU_LIST_FOOTER_TEXT_Y);
      #if HAS_ENCODER == true
        if (encoder_enabled) MENU_GFX.print("turn:move press:open");
        else                 MENU_GFX.print("tap:next hold:open");
      #else
        MENU_GFX.print("tap:next hold:open");
      #endif
    }
  #endif

  void draw_menu_edit_disp(const char *title, const char *valbuf) {
    MENU_GFX.setFont(MENU_FONT);
    MENU_GFX.setTextSize(1);
    MENU_GFX.setTextColor(SSD1306_WHITE);
    MENU_GFX.setCursor(6, 9);
    MENU_GFX.print(title);
    MENU_GFX.drawFastHLine(4, 15, MENU_CONTENT_W, SSD1306_WHITE);

    MENU_GFX.setTextSize(2);
    int16_t x1, y1; uint16_t w, h;
    MENU_GFX.getTextBounds(valbuf, 0, 0, &x1, &y1, &w, &h);
    // MENU_EDIT_VALUE_Y, not the arrows' own MENU_EDIT_ARROW_Y - Org_01
    // glyphs render above the setCursor() baseline, not straddling it
    // (see [[feedback_org01_font_baseline]]), and that headroom roughly
    // doubles at size 2 vs the arrows' size 1, so matching baselines
    // would leave the value looking a few px too high. This offset
    // centers the two visually instead of literally.
    MENU_GFX.setCursor(MENU_EDIT_VALUE_CX - w/2, MENU_EDIT_VALUE_Y);
    MENU_GFX.print(valbuf);
    MENU_GFX.setTextSize(1);
    // Pinned near the box edges rather than a fixed inset - frees up the
    // center for wider size-2 values like "100/HALF" (Speed,
    // ETH_ITEM_SPEED) without colliding with the arrows. ">" is measured
    // rather than hardcoded so it can't run past the right edge.
    MENU_GFX.setCursor(5, MENU_EDIT_ARROW_Y);
    MENU_GFX.print("<");
    int16_t ax1, ay1; uint16_t aw, ah;
    MENU_GFX.getTextBounds(">", 0, 0, &ax1, &ay1, &aw, &ah);
    MENU_GFX.setCursor(MENU_EDIT_ARROW_R_EDGE - aw, MENU_EDIT_ARROW_Y);
    MENU_GFX.print(">");

    MENU_GFX.drawFastHLine(4, MENU_EDIT_FOOTER_HLINE_Y, MENU_CONTENT_W, SSD1306_WHITE);
    MENU_GFX.setCursor(6, MENU_EDIT_FOOTER_TEXT_Y);
    #if HAS_ENCODER == true
      if (encoder_enabled) MENU_GFX.print("turn:adjust press:ok");
      else                 MENU_GFX.print("tap:adjust hold:ok");
    #else
      MENU_GFX.print("tap:adjust hold:ok");
    #endif
  }

  #if HAS_WIFI == true || HAS_ETHERNET == true
    // Shared by WiFi's IP Address/Netmask and, on MeshPoE-S3, wired
    // Ethernet's own - shows the whole address at once (max
    // "255.255.255.255", 15 chars, comfortably fits at size 1 - unlike the
    // 32-char SSID/PSK wheel below, no windowing needed) with the octet
    // currently being adjusted highlighted - same fillRect+invert-color
    // trick draw_menu_text_edit_disp() uses for its wheel candidate, just
    // per octet instead of per character. Already-confirmed octets sit to
    // the left of the highlight, not-yet-visited ones (still holding
    // whatever they started this edit session with) to its right, so the
    // highlight visibly moves rightward - and everything left of it
    // accumulates legible - as each octet is confirmed.
    void draw_menu_addr_edit_disp(const char *title, uint8_t *octets, uint8_t active_idx) {
      MENU_GFX.setFont(MENU_FONT);
      MENU_GFX.setTextSize(1);
      MENU_GFX.setTextColor(SSD1306_WHITE);
      MENU_GFX.setCursor(6, 9);
      MENU_GFX.print(title);
      MENU_GFX.drawFastHLine(4, 15, 120, SSD1306_WHITE);

      // Same font as the SSID/PSK screen's typed content (TEXT_ENTRY_FONT,
      // Tamsyn6x12 - see draw_menu_text_edit_disp()) - title/divider/footer
      // stay on SMALL_FONT/Org_01, same split that screen uses. Narrower
      // per-glyph than Org_01 at the size Org_01 would otherwise need to be
      // legible here, so the full "255.255.255.255" fits comfortably.
      MENU_GFX.setFont(TEXT_ENTRY_FONT);
      MENU_GFX.setTextSize(1);

      char full[16];
      format_addr_octets(octets, full);
      int16_t fx1, fy1; uint16_t fw, fh;
      MENU_GFX.getTextBounds(full, 0, 0, &fx1, &fy1, &fw, &fh);
      uint16_t x = 64 - fw/2;
      const uint16_t y = 32;

      char seg[4];
      int16_t sx1, sy1; uint16_t sw, sh;
      for (uint8_t i = 0; i < 4; i++) {
        sprintf(seg, "%u", octets[i]);
        MENU_GFX.getTextBounds(seg, 0, 0, &sx1, &sy1, &sw, &sh);
        if (i == active_idx) {
          // Same box geometry as draw_menu_text_edit_disp()'s wheel
          // candidate - Tamsyn6x12 glyphs span roughly baseline-9 to
          // baseline+4, a taller box than Org_01 would need.
          MENU_GFX.fillRect(x - 1, 21, sw + 2, 18, SSD1306_WHITE);
          MENU_GFX.setTextColor(SSD1306_BLACK);
        } else {
          MENU_GFX.setTextColor(SSD1306_WHITE);
        }
        MENU_GFX.setCursor(x, y);
        MENU_GFX.print(seg);
        x += sw;
        MENU_GFX.setTextColor(SSD1306_WHITE);
        if (i < 3) {
          MENU_GFX.setCursor(x, y);
          MENU_GFX.print(".");
          int16_t dx1, dy1; uint16_t dw, dh;
          MENU_GFX.getTextBounds(".", 0, 0, &dx1, &dy1, &dw, &dh);
          x += dw;
        }
      }

      MENU_GFX.setFont(MENU_FONT);
      MENU_GFX.setTextSize(1);
      MENU_GFX.setTextColor(SSD1306_WHITE);
      MENU_GFX.drawFastHLine(4, 50, 120, SSD1306_WHITE);
      MENU_GFX.setCursor(6, 59);
      #if HAS_ENCODER == true
        if (encoder_enabled) MENU_GFX.print("turn:adjust press:ok");
        else                 MENU_GFX.print("tap:adjust hold:ok");
      #else
        MENU_GFX.print("tap:adjust hold:ok");
      #endif
    }
  #endif

  #if HAS_RTC == true
    // Date row (Year/Month/Day, active_idx 0-2) and time row (Hour/Minute/
    // Second, active_idx 3-5), each using the same per-segment highlight
    // trick as draw_menu_addr_edit_disp() - split across two rows (rather
    // than one screen per row) so the whole date+time and whichever field
    // is being adjusted stay visible together. Segment boxes are shorter
    // (12px, not draw_menu_addr_edit_disp's 18px) since there's only room
    // for two rows in the same vertical space that screen uses for one,
    // and digits/separators here have no descenders to leave room for.
    void draw_menu_datetime_edit_disp(uint8_t active_idx) {
      MENU_GFX.setFont(MENU_FONT);
      MENU_GFX.setTextSize(1);
      MENU_GFX.setTextColor(SSD1306_WHITE);
      MENU_GFX.setCursor(6, 9);
      // "UTC" suffix - the RTC list's own Time/Date rows show local
      // (Timezone-shifted) time, but this editor always reads/writes the
      // RTC in UTC, same as CMD_TIME/rtc_sync_ntp() - worth being explicit
      // about here since it'd otherwise be the one screen on this page
      // that doesn't match what's shown everywhere else.
      MENU_GFX.print("SET TIME/DATE UTC");
      MENU_GFX.drawFastHLine(4, 15, 120, SSD1306_WHITE);

      MENU_GFX.setFont(TEXT_ENTRY_FONT);
      MENU_GFX.setTextSize(1);

      char segs[6][5];
      sprintf(segs[0], "%04d", (int)staged_rtc_year);
      sprintf(segs[1], "%02u", staged_rtc_month);
      sprintf(segs[2], "%02u", staged_rtc_day);
      sprintf(segs[3], "%02u", staged_rtc_hour);
      sprintf(segs[4], "%02u", staged_rtc_minute);
      sprintf(segs[5], "%02u", staged_rtc_second);
      // Separator printed right after each segment - empty for the last
      // segment in its row.
      const char *seps[6] = { "-", "-", "", ":", ":", "" };

      for (uint8_t row = 0; row < 2; row++) {
        uint8_t first = (row == 0) ? 0 : 3;
        const uint16_t y = (row == 0) ? 28 : 44;

        uint16_t total_w = 0;
        int16_t bx1, by1; uint16_t bw, bh;
        for (uint8_t i = first; i < first + 3; i++) {
          MENU_GFX.getTextBounds(segs[i], 0, 0, &bx1, &by1, &bw, &bh);
          total_w += bw;
          if (seps[i][0]) { MENU_GFX.getTextBounds(seps[i], 0, 0, &bx1, &by1, &bw, &bh); total_w += bw; }
        }

        uint16_t x = 64 - total_w / 2;
        for (uint8_t i = first; i < first + 3; i++) {
          MENU_GFX.getTextBounds(segs[i], 0, 0, &bx1, &by1, &bw, &bh);
          if (i == active_idx) {
            MENU_GFX.fillRect(x - 1, y - 9, bw + 2, 12, SSD1306_WHITE);
            MENU_GFX.setTextColor(SSD1306_BLACK);
          } else {
            MENU_GFX.setTextColor(SSD1306_WHITE);
          }
          MENU_GFX.setCursor(x, y);
          MENU_GFX.print(segs[i]);
          x += bw;
          MENU_GFX.setTextColor(SSD1306_WHITE);
          if (seps[i][0]) {
            MENU_GFX.setCursor(x, y);
            MENU_GFX.print(seps[i]);
            MENU_GFX.getTextBounds(seps[i], 0, 0, &bx1, &by1, &bw, &bh);
            x += bw;
          }
        }
      }

      MENU_GFX.setFont(MENU_FONT);
      MENU_GFX.setTextSize(1);
      MENU_GFX.setTextColor(SSD1306_WHITE);
      MENU_GFX.drawFastHLine(4, 50, 120, SSD1306_WHITE);
      MENU_GFX.setCursor(6, 59);
      #if HAS_ENCODER == true
        if (encoder_enabled) MENU_GFX.print("turn:adjust press:ok");
        else                 MENU_GFX.print("tap:adjust hold:ok");
      #else
        MENU_GFX.print("tap:adjust hold:ok");
      #endif
    }
  #endif

  #if HAS_WIFI == true
    // Character-count windowed (not pixel-precise) so a 32-char SSID/PSK
    // doesn't need to fit on screen at once - shows only the trailing
    // portion of the string plus the pending wheel selection, which is
    // always what's being actively edited (append-only, see plan notes).
    void draw_menu_text_edit_disp(const char *title, const char *text_buf, uint8_t wheel_idx) {
      MENU_GFX.setFont(MENU_FONT);
      MENU_GFX.setTextSize(1);
      MENU_GFX.setTextColor(SSD1306_WHITE);
      MENU_GFX.setCursor(6, 9);
      MENU_GFX.print(title);
      MENU_GFX.drawFastHLine(4, 15, 120, SSD1306_WHITE);

      char candidate[6];
      if (wheel_is_del(wheel_idx))       sprintf(candidate, "DEL");
      else if (wheel_is_save(wheel_idx)) sprintf(candidate, "SAVE");
      else { candidate[0] = wheel_char_at(wheel_idx); candidate[1] = 0; }

      // FreeMono9pt7b only for the actual typed content, at its natural
      // size (it's already a proper 9pt font, unlike Org_01 which needs
      // setTextSize(2) to be legible) - title/divider/footer stay on
      // SMALL_FONT/Org_01, same as every other menu screen.
      MENU_GFX.setFont(TEXT_ENTRY_FONT);
      MENU_GFX.setTextSize(1);

      // FreeMono9pt7b is monospace, but the window-growing logic still
      // measures real pixel widths rather than assuming a fixed advance,
      // so it stays correct if the font is ever swapped again.
      const uint16_t max_width = 120;
      int16_t cx1, cy1; uint16_t cw, ch;
      MENU_GFX.getTextBounds(candidate, 0, 0, &cx1, &cy1, &cw, &ch);
      uint16_t remaining_width = (max_width > cw + 3) ? (max_width - cw - 3) : 0;

      uint8_t text_len = strlen(text_buf);
      uint8_t prefix_len = 0;
      for (uint8_t try_len = 1; try_len <= text_len; try_len++) {
        int16_t px1, py1; uint16_t pw, ph;
        MENU_GFX.getTextBounds(text_buf + (text_len - try_len), 0, 0, &px1, &py1, &pw, &ph);
        if (pw > remaining_width) break;
        prefix_len = try_len;
      }
      const char *prefix_start = text_buf + (text_len - prefix_len);

      int16_t x1, y1; uint16_t pw, ph;
      MENU_GFX.getTextBounds(prefix_start, 0, 0, &x1, &y1, &pw, &ph);
      uint16_t cand_x = 4 + pw;

      MENU_GFX.setTextColor(SSD1306_WHITE);
      MENU_GFX.setCursor(4, 32);
      MENU_GFX.print(prefix_start);

      // FreeMono9pt7b glyphs span roughly baseline-9 (ascenders) to
      // baseline+4 (descenders like g/p/y) at this size - a taller box
      // than Org_01 needed.
      MENU_GFX.fillRect(cand_x, 21, cw + 3, 18, SSD1306_WHITE);
      MENU_GFX.setTextColor(SSD1306_BLACK);
      MENU_GFX.setCursor(cand_x + 1, 32);
      MENU_GFX.print(candidate);

      MENU_GFX.setFont(MENU_FONT);
      MENU_GFX.setTextSize(1);
      MENU_GFX.setTextColor(SSD1306_WHITE);
      MENU_GFX.drawFastHLine(4, 50, 120, SSD1306_WHITE);
      MENU_GFX.setCursor(6, 59);
      // Same encoder_enabled branch as every other footer hint in this
      // file (e.g. the main list's "turn:move press:open" vs "tap:next
      // hold:open") - this one just never had it, leaving button-only
      // boards shown a caption for input hardware they don't have. "hold:
      // ok" (not "hold:save") since a hold just confirms whatever the
      // wheel is currently on - a character, DEL, or SAVE (WHEEL_QUICKSAVE_
      // IDX) - not something SAVE-specific.
      #if HAS_ENCODER == true
        if (encoder_enabled) MENU_GFX.print("turn:char hold:ok");
        else                 MENU_GFX.print("tap:char hold:ok");
      #else
        MENU_GFX.print("tap:char hold:ok");
      #endif
    }
  #endif

  void draw_settings_menu_disp() {
    #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_T114
      // Unlike the other boards (whose display.clearDisplay() call in
      // update_display() wipes the whole panel buffer before getting
      // here every cycle), nothing clears menu_canvas on its own - it's
      // just an off-screen buffer that draw calls accumulate into.
      // Without this, switching screens (e.g. list -> edit) or the
      // cursor moving leaves stale pixels from the previous draw behind,
      // since fillRect/print only ever touch the specific pixels the
      // new content needs, never the ones it doesn't.
      menu_canvas.fillScreen(SSD1306_BLACK);
    #endif
    MENU_GFX.setTextWrap(false);

    if (menu_state == MENU_STATE_LIST) {
      const char *labels[MENU_ITEM_COUNT];
      char valbufs[MENU_ITEM_COUNT][24];

      labels[MENU_ITEM_DISPLAY_TIMEOUT] = "Display Timeout";
      if (staged_display_timeout == 0) sprintf(valbufs[MENU_ITEM_DISPLAY_TIMEOUT], "OFF");
      else                              sprintf(valbufs[MENU_ITEM_DISPLAY_TIMEOUT], "%us", staged_display_timeout);

      labels[MENU_ITEM_DISPLAY_BRIGHTNESS] = "Brightness";
      format_brightness(staged_display_brightness, valbufs[MENU_ITEM_DISPLAY_BRIGHTNESS]);

      labels[MENU_ITEM_ORIENTATION] = "Orientation";
      format_orientation(staged_display_rotation, valbufs[MENU_ITEM_ORIENTATION]);

      #if HAS_BUZZER == true
        labels[MENU_ITEM_SOUND] = "Sound";
        sprintf(valbufs[MENU_ITEM_SOUND], staged_sound_enabled ? "ON" : "OFF");
      #endif

      #if HAS_ESPNOW == true
        labels[MENU_ITEM_ESPNOW] = "ESP-NOW";
        sprintf(valbufs[MENU_ITEM_ESPNOW], ">"); // opens a submenu, not an inline value
      #endif

      #if HAS_ENCODER == true
        labels[MENU_ITEM_ENCODER] = "Encoder";
        sprintf(valbufs[MENU_ITEM_ENCODER], staged_encoder_enabled ? "ON" : "OFF");
      #endif

      #if HAS_WIFI == true
        labels[MENU_ITEM_WIFI] = "WiFi";
        sprintf(valbufs[MENU_ITEM_WIFI], ">"); // opens a submenu, not an inline value
      #endif

      #if HAS_ETHERNET == true
        labels[MENU_ITEM_ETHERNET] = "Ethernet";
        sprintf(valbufs[MENU_ITEM_ETHERNET], ">"); // opens a submenu, not an inline value
      #endif

      #if HAS_RTC == true
        labels[MENU_ITEM_RTC] = "RTC";
        sprintf(valbufs[MENU_ITEM_RTC], ">"); // opens a submenu, not an inline value
      #endif

      #if HAS_GPS == true
        labels[MENU_ITEM_GNSS] = "GNSS";
        sprintf(valbufs[MENU_ITEM_GNSS], ">"); // opens a submenu, not an inline value
      #endif

      #if HAS_SENSORS == true
        labels[MENU_ITEM_SENSORS] = "Sensors";
        sprintf(valbufs[MENU_ITEM_SENSORS], ">"); // opens a submenu, not an inline value
      #endif

      #if MENU_HAS_HW_PAGE == true
        labels[MENU_ITEM_HARDWARE] = "Hardware";
        sprintf(valbufs[MENU_ITEM_HARDWARE], ">"); // opens a submenu, not an inline value
      #endif

      #if HAS_OTA == true
        labels[MENU_ITEM_FW_UPDATE] = "F/W Update";
        sprintf(valbufs[MENU_ITEM_FW_UPDATE], ">"); // opens a submenu, not an inline value
      #endif

      labels[MENU_ITEM_SAVE_EXIT] = "SAVE & EXIT";
      valbufs[MENU_ITEM_SAVE_EXIT][0] = 0;

      draw_menu_list_disp("RNODE SETTINGS", labels, valbufs, MENU_ITEM_COUNT, menu_cursor);

    } else if (menu_state == MENU_STATE_EDIT) {
      char valbuf[8];
      const char *title = "";
      if (menu_edit_field == MENU_ITEM_DISPLAY_TIMEOUT) {
        title = "DISPLAY TIMEOUT";
        if (staged_display_timeout == 0) sprintf(valbuf, "OFF");
        else                              sprintf(valbuf, "%u", staged_display_timeout);
      } else if (menu_edit_field == MENU_ITEM_DISPLAY_BRIGHTNESS) {
        title = "BRIGHTNESS";
        format_brightness(staged_display_brightness, valbuf);
      } else if (menu_edit_field == MENU_ITEM_ORIENTATION) {
        title = "ORIENTATION";
        format_orientation(staged_display_rotation, valbuf);
      }
      #if HAS_BUZZER == true
        else if (menu_edit_field == MENU_ITEM_SOUND) {
          title = "SOUND";
          sprintf(valbuf, staged_sound_enabled ? "ON" : "OFF");
        }
      #endif
      #if HAS_ENCODER == true
        else if (menu_edit_field == MENU_ITEM_ENCODER) {
          title = "ENCODER";
          sprintf(valbuf, staged_encoder_enabled ? "ON" : "OFF");
        }
      #endif
      draw_menu_edit_disp(title, valbuf);
    }
    #if HAS_WIFI == true
      else if (menu_state == MENU_STATE_WIFI_LIST) {
        const char *labels[WIFI_ITEM_COUNT];
        char valbufs[WIFI_ITEM_COUNT][24];
        labels[WIFI_ITEM_MODE] = "Mode";
        format_wifi_mode(staged_wifi_mode, valbufs[WIFI_ITEM_MODE]);

        labels[WIFI_ITEM_SSID] = "SSID";
        // Short label ("SSID") leaves plenty of the 120px row for the value -
        // show up to 18 trailing chars rather than an overly aggressive cutoff.
        uint8_t ssid_len = strlen(staged_wifi_ssid);
        if (ssid_len > 18) sprintf(valbufs[WIFI_ITEM_SSID], "%s", staged_wifi_ssid + (ssid_len - 18));
        else                sprintf(valbufs[WIFI_ITEM_SSID], "%s", staged_wifi_ssid);

        labels[WIFI_ITEM_PSK] = "PSK";
        sprintf(valbufs[WIFI_ITEM_PSK], strlen(staged_wifi_psk) > 0 ? "SET" : "");

        labels[WIFI_ITEM_CHANNEL] = "Channel";
        sprintf(valbufs[WIFI_ITEM_CHANNEL], "%u", staged_wifi_channel);

        // "DHCP" (rather than "0.0.0.0") when unset - matching the same
        // all-zero-means-unset convention Ethernet's page uses. Shows
        // staged_wifi_ip/nm (the working copy), same as SSID/PSK above -
        // not live EEPROM, since this whole submenu defers to SAVE & EXIT.
        labels[WIFI_ITEM_IP] = "IP Address";
        if (staged_wifi_ip[0]==0 && staged_wifi_ip[1]==0 && staged_wifi_ip[2]==0 && staged_wifi_ip[3]==0) sprintf(valbufs[WIFI_ITEM_IP], "DHCP");
        else format_addr_octets(staged_wifi_ip, valbufs[WIFI_ITEM_IP]);

        labels[WIFI_ITEM_NETMASK] = "Netmask";
        if (staged_wifi_nm[0]==0 && staged_wifi_nm[1]==0 && staged_wifi_nm[2]==0 && staged_wifi_nm[3]==0) sprintf(valbufs[WIFI_ITEM_NETMASK], "DHCP");
        else format_addr_octets(staged_wifi_nm, valbufs[WIFI_ITEM_NETMASK]);

        // "NONE" rather than "DHCP" when unset - unlike IP/Netmask, there's
        // no DHCP client running once IP/NM are static, so an unset
        // Gateway/DNS just means "none configured," not "provided by DHCP."
        labels[WIFI_ITEM_GATEWAY] = "Gateway";
        if (staged_wifi_gw[0]==0 && staged_wifi_gw[1]==0 && staged_wifi_gw[2]==0 && staged_wifi_gw[3]==0) sprintf(valbufs[WIFI_ITEM_GATEWAY], "NONE");
        else format_addr_octets(staged_wifi_gw, valbufs[WIFI_ITEM_GATEWAY]);

        labels[WIFI_ITEM_DNS] = "DNS";
        if (staged_wifi_dns[0]==0 && staged_wifi_dns[1]==0 && staged_wifi_dns[2]==0 && staged_wifi_dns[3]==0) sprintf(valbufs[WIFI_ITEM_DNS], "NONE");
        else format_addr_octets(staged_wifi_dns, valbufs[WIFI_ITEM_DNS]);

        labels[WIFI_ITEM_CLEAR] = "Clear Static";
        valbufs[WIFI_ITEM_CLEAR][0] = 0;

        labels[WIFI_ITEM_BACK] = "BACK";
        valbufs[WIFI_ITEM_BACK][0] = 0;
        draw_menu_list_disp("WIFI", labels, valbufs, WIFI_ITEM_COUNT, wifi_menu_cursor);
      } else if (menu_state == MENU_STATE_WIFI_EDIT) {
        char valbuf[8];
        if (wifi_menu_cursor == WIFI_ITEM_CHANNEL) {
          sprintf(valbuf, "%u", staged_wifi_channel);
          draw_menu_edit_disp("CHANNEL", valbuf);
        } else {
          format_wifi_mode(staged_wifi_mode, valbuf);
          draw_menu_edit_disp("MODE", valbuf);
        }
      } else if (menu_state == MENU_STATE_WIFI_ADDR_EDIT) {
        const char *title = "DNS";
        if      (wifi_menu_cursor == WIFI_ITEM_IP)      title = "IP ADDRESS";
        else if (wifi_menu_cursor == WIFI_ITEM_NETMASK) title = "NETMASK";
        else if (wifi_menu_cursor == WIFI_ITEM_GATEWAY) title = "GATEWAY";
        draw_menu_addr_edit_disp(title, wifi_staged_addr_field(wifi_menu_cursor), wifi_addr_octet_idx);
      } else if (menu_state == MENU_STATE_WIFI_TEXT_EDIT) {
        const char *title = (text_edit_field == WIFI_ITEM_SSID) ? "SSID" : "PSK";
        draw_menu_text_edit_disp(title, text_edit_buf, wheel_index);
      } else if (menu_state == MENU_STATE_WIFI_TEXT_CONFIRM) {
        const char *labels[2] = { "SAVE", "DISCARD" };
        char valbufs[2][24];
        valbufs[0][0] = 0;
        valbufs[1][0] = 0;
        const char *title = (text_edit_field == WIFI_ITEM_SSID) ? "SAVE SSID?" : "SAVE PSK?";
        draw_menu_list_disp(title, labels, valbufs, 2, text_confirm_cursor);
      }
    #endif
    #if HAS_ETHERNET == true
      else if (menu_state == MENU_STATE_ETH_LIST) {
        const char *labels[ETH_ITEM_COUNT];
        char valbufs[ETH_ITEM_COUNT][24];

        // eth_link_up (Ethernet.h) tracks ARDUINO_EVENT_ETH_CONNECTED/
        // _DISCONNECTED - ETH.linkSpeed()/fullDuplex() (10/100, the W5500
        // has no gigabit mode) are otherwise stale/meaningless while link
        // is down. "N/FULL" or "N/HALF" - same naming as the Speed field's
        // own ETH_SPEED_* values (format_eth_speed_mode()) - is the actual
        // negotiated result, not an echo of that setting, so it'll read
        // differently if the far end doesn't support what Speed forces.
        labels[ETH_ITEM_LINK_STATUS] = "Link Status";
        if (eth_link_up) sprintf(valbufs[ETH_ITEM_LINK_STATUS], "%u/%s", ETH.linkSpeed(), ETH.fullDuplex() ? "FULL" : "HALF");
        else             sprintf(valbufs[ETH_ITEM_LINK_STATUS], "DOWN");

        labels[ETH_ITEM_SPEED] = "Speed";
        format_eth_speed_mode(eth_speed_mode, valbufs[ETH_ITEM_SPEED]);

        // "DHCP" (rather than "0.0.0.0") when unset, matching the same
        // all-zero/all-0xFF-means-unset convention addr4_read() (Utilities.h)
        // already checks - there's no separate DHCP/Manual mode flag.
        labels[ETH_ITEM_IP] = "IP Address";
        {
          uint8_t ip_octets[4];
          if (addr4_read(ADDR_CONF_ETH_IP, ip_octets)) format_addr_octets(ip_octets, valbufs[ETH_ITEM_IP]);
          else                                             sprintf(valbufs[ETH_ITEM_IP], "DHCP");
        }

        labels[ETH_ITEM_NETMASK] = "Netmask";
        {
          uint8_t nm_octets[4];
          if (addr4_read(ADDR_CONF_ETH_NM, nm_octets)) format_addr_octets(nm_octets, valbufs[ETH_ITEM_NETMASK]);
          else                                             sprintf(valbufs[ETH_ITEM_NETMASK], "DHCP");
        }

        // "NONE" rather than "DHCP" when unset - unlike IP/Netmask, there's
        // no DHCP client running once IP/NM are static, so an unset
        // Gateway/DNS just means "none configured," not "provided by DHCP."
        labels[ETH_ITEM_GATEWAY] = "Gateway";
        {
          uint8_t gw_octets[4];
          if (addr4_read(ADDR_CONF_ETH_GW, gw_octets)) format_addr_octets(gw_octets, valbufs[ETH_ITEM_GATEWAY]);
          else                                             sprintf(valbufs[ETH_ITEM_GATEWAY], "NONE");
        }

        labels[ETH_ITEM_DNS] = "DNS";
        {
          uint8_t dns_octets[4];
          if (addr4_read(ADDR_CONF_ETH_DNS, dns_octets)) format_addr_octets(dns_octets, valbufs[ETH_ITEM_DNS]);
          else                                              sprintf(valbufs[ETH_ITEM_DNS], "NONE");
        }

        labels[ETH_ITEM_CLEAR] = "Clear Static";
        valbufs[ETH_ITEM_CLEAR][0] = 0;

        labels[ETH_ITEM_BACK] = "BACK";
        valbufs[ETH_ITEM_BACK][0] = 0;
        draw_menu_list_disp("ETHERNET", labels, valbufs, ETH_ITEM_COUNT, eth_menu_cursor);
      } else if (menu_state == MENU_STATE_ETH_EDIT) {
        char valbuf[9];
        format_eth_speed_mode(staged_eth_speed_mode, valbuf);
        draw_menu_edit_disp("SPEED", valbuf);
      } else if (menu_state == MENU_STATE_ETH_ADDR_EDIT) {
        const char *title = "DNS";
        if      (eth_menu_cursor == ETH_ITEM_IP)      title = "IP ADDRESS";
        else if (eth_menu_cursor == ETH_ITEM_NETMASK) title = "NETMASK";
        else if (eth_menu_cursor == ETH_ITEM_GATEWAY) title = "GATEWAY";
        draw_menu_addr_edit_disp(title, eth_staged_addr_field(eth_menu_cursor), eth_addr_octet_idx);
      }
    #endif
    #if HAS_RTC == true
      else if (menu_state == MENU_STATE_RTC_LIST) {
        const char *labels[RTC_ITEM_COUNT];
        char valbufs[RTC_ITEM_COUNT][24];

        // Time/Date are shown in local (Timezone-shifted) time - everything
        // else in this firmware (CMD_TIME, rtc_sync_ntp(), the RTC chip
        // itself) stays strictly UTC; rtc_apply_tz_offset() (RTC.h) only
        // shifts this display copy of the epoch.
        uint32_t epoch = rtc_apply_tz_offset(rtc_get_unixtime());
        int32_t days = (int32_t)(epoch / 86400UL);
        uint32_t rem  = epoch % 86400UL;
        uint8_t hh = (uint8_t)(rem / 3600); rem %= 3600;
        uint8_t mi = (uint8_t)(rem / 60);
        uint8_t ss = (uint8_t)(rem % 60);
        int32_t yy; uint32_t mo, dd;
        rtc_civil_from_days(days, yy, mo, dd);

        labels[RTC_ITEM_TIME] = "Time";
        if (rtc_present) sprintf(valbufs[RTC_ITEM_TIME], "%02u:%02u:%02u", hh, mi, ss);
        else              sprintf(valbufs[RTC_ITEM_TIME], "N/A");

        labels[RTC_ITEM_DATE] = "Date";
        if (rtc_present) sprintf(valbufs[RTC_ITEM_DATE], "%04d-%02u-%02u", (int)yy, mo, dd);
        else              sprintf(valbufs[RTC_ITEM_DATE], "N/A");

        labels[RTC_ITEM_TIMEZONE] = "Timezone";
        format_tz_offset(rtc_get_tz_offset_qh(), valbufs[RTC_ITEM_TIMEZONE]);

        #if MCU_VARIANT == MCU_ESP32 && (HAS_WIFI == true || HAS_ETHERNET == true)
          labels[RTC_ITEM_SYNC_NTP] = "Sync via NTP";
          valbufs[RTC_ITEM_SYNC_NTP][0] = 0;
        #endif

        #if HAS_GPS == true
          labels[RTC_ITEM_SYNC_GPS] = "Sync via GNSS";
          valbufs[RTC_ITEM_SYNC_GPS][0] = 0;
        #endif

        labels[RTC_ITEM_SET] = "Set Time/Date";
        valbufs[RTC_ITEM_SET][0] = 0;

        labels[RTC_ITEM_BACK] = "BACK";
        valbufs[RTC_ITEM_BACK][0] = 0;

        draw_menu_list_disp("RTC", labels, valbufs, RTC_ITEM_COUNT, rtc_menu_cursor);
      } else if (menu_state == MENU_STATE_RTC_EDIT) {
        draw_menu_datetime_edit_disp(rtc_edit_field_idx);
      } else if (menu_state == MENU_STATE_RTC_TZ_EDIT) {
        char valbuf[8];
        format_tz_offset(staged_tz_offset_qh, valbuf);
        draw_menu_edit_disp("TIMEZONE", valbuf);
      }
    #endif
    #if HAS_GPS == true
      else if (menu_state == MENU_STATE_GNSS_LIST) {
        const char *labels[GNSS_ITEM_COUNT];
        char valbufs[GNSS_ITEM_COUNT][24];

        labels[GNSS_ITEM_ENABLED] = "Enabled";
        sprintf(valbufs[GNSS_ITEM_ENABLED], gnss_enabled ? "ON" : "OFF");

        labels[GNSS_ITEM_MODULE] = "Module";
        sprintf(valbufs[GNSS_ITEM_MODULE], "%s", gnss_module_status_text());

        labels[GNSS_ITEM_FIX] = "Fix";
        sprintf(valbufs[GNSS_ITEM_FIX], gnss_has_fix() ? "YES" : "NO");

        labels[GNSS_ITEM_SATELLITES] = "Satellites";
        sprintf(valbufs[GNSS_ITEM_SATELLITES], "%u", (unsigned)gnss_satellite_count());

        labels[GNSS_ITEM_LATITUDE] = "Latitude";
        if (gnss_has_fix()) sprintf(valbufs[GNSS_ITEM_LATITUDE], "%.5f", gnss_latitude());
        else                 sprintf(valbufs[GNSS_ITEM_LATITUDE], "N/A");

        labels[GNSS_ITEM_LONGITUDE] = "Longitude";
        if (gnss_has_fix()) sprintf(valbufs[GNSS_ITEM_LONGITUDE], "%.5f", gnss_longitude());
        else                 sprintf(valbufs[GNSS_ITEM_LONGITUDE], "N/A");

        labels[GNSS_ITEM_ALTITUDE] = "Altitude";
        if (gnss_has_fix()) sprintf(valbufs[GNSS_ITEM_ALTITUDE], "%.0fm", gnss_altitude_meters());
        else                 sprintf(valbufs[GNSS_ITEM_ALTITUDE], "N/A");

        labels[GNSS_ITEM_TIME] = "GNSS Time";
        if (gnss_time_valid()) sprintf(valbufs[GNSS_ITEM_TIME], "%02u:%02u:%02u", gnss_time_hour(), gnss_time_minute(), gnss_time_second());
        else                    sprintf(valbufs[GNSS_ITEM_TIME], "N/A");

        labels[GNSS_ITEM_NMEA_CHARS] = "NMEA Chars";
        sprintf(valbufs[GNSS_ITEM_NMEA_CHARS], "%lu", (unsigned long)gnss_chars_processed());

        labels[GNSS_ITEM_NMEA_CKSUM] = "NMEA OK/Err";
        sprintf(valbufs[GNSS_ITEM_NMEA_CKSUM], "%lu/%lu", (unsigned long)gnss_checksum_passed(), (unsigned long)gnss_checksum_failed());

        labels[GNSS_ITEM_BACK] = "BACK";
        valbufs[GNSS_ITEM_BACK][0] = 0;

        draw_menu_list_disp("GNSS", labels, valbufs, GNSS_ITEM_COUNT, gnss_menu_cursor);
      } else if (menu_state == MENU_STATE_GNSS_EDIT) {
        draw_menu_edit_disp("GNSS ENABLED", staged_gnss_enabled ? "ON" : "OFF");
      }
    #endif
    #if HAS_ESPNOW == true
      else if (menu_state == MENU_STATE_ESPNOW_LIST) {
        const char *labels[ESPNOW_ITEM_COUNT];
        char valbufs[ESPNOW_ITEM_COUNT][24];

        labels[ESPNOW_ITEM_ENABLED] = "Enabled";
        sprintf(valbufs[ESPNOW_ITEM_ENABLED], staged_espnow_enabled ? "ON" : "OFF");

        labels[ESPNOW_ITEM_MODE] = "Version";
        sprintf(valbufs[ESPNOW_ITEM_MODE], staged_espnow_mode_v2 ? "v2.0" : "v1.0");

        labels[ESPNOW_ITEM_LR] = "LR Mode";
        sprintf(valbufs[ESPNOW_ITEM_LR], staged_espnow_lr_enabled ? "ON" : "OFF");

        labels[ESPNOW_ITEM_CHANNEL] = "Channel";
        sprintf(valbufs[ESPNOW_ITEM_CHANNEL], "%u", wr_channel);

        labels[ESPNOW_ITEM_BACK] = "BACK";
        valbufs[ESPNOW_ITEM_BACK][0] = 0;

        draw_menu_list_disp("ESP-NOW", labels, valbufs, ESPNOW_ITEM_COUNT, espnow_menu_cursor);
      } else if (menu_state == MENU_STATE_ESPNOW_EDIT) {
        if (espnow_menu_cursor == ESPNOW_ITEM_ENABLED) {
          draw_menu_edit_disp("ESP-NOW ENABLED", staged_espnow_enabled ? "ON" : "OFF");
        } else if (espnow_menu_cursor == ESPNOW_ITEM_MODE) {
          draw_menu_edit_disp("ESP-NOW VERSION", staged_espnow_mode_v2 ? "v2.0" : "v1.0");
        } else {
          draw_menu_edit_disp("ESP-NOW LR MODE", staged_espnow_lr_enabled ? "ON" : "OFF");
        }
      } else if (menu_state == MENU_STATE_ESPNOW_LR_CONFIRM) {
        // 3-item list, same draw_menu_list_disp() as everywhere else - same
        // pattern as F/W Update's UPDATE/CANCEL (MENU_STATE_FWUPD_CONFIRM),
        // with an inert info row first spelling out the actual consequence
        // (the title alone can't fit "enabling LR disables WiFi" on a
        // 128px-wide single line) - MENU_LIST_VISIBLE_ROWS is 4 here, so a
        // 3-item list still fits with room to spare.
        const char *labels[3] = { "Disables WiFi", "ENABLE", "CANCEL" };
        char valbufs[3][24];
        valbufs[0][0] = 0;
        valbufs[1][0] = 0;
        valbufs[2][0] = 0;
        draw_menu_list_disp("ENABLE LR MODE?", labels, valbufs, 3, espnow_lr_confirm_cursor);
      }
    #endif
    #if HAS_SENSORS == true
      else if (menu_state == MENU_STATE_SENSORS_LIST) {
        const char *labels[SENSORS_ITEM_COUNT];
        char valbufs[SENSORS_ITEM_COUNT][24];

        labels[SENSORS_ITEM_TEMP] = "Temp";
        if (sensor_present) sprintf(valbufs[SENSORS_ITEM_TEMP], "%.1fC", sensor_temperature_c());
        else                 sprintf(valbufs[SENSORS_ITEM_TEMP], "N/A");

        labels[SENSORS_ITEM_HUMIDITY] = "Humidity";
        if (sensor_present && sensor_model == SENSOR_MODEL_BME280) sprintf(valbufs[SENSORS_ITEM_HUMIDITY], "%.1f%%", sensor_humidity_percent());
        else                                                        sprintf(valbufs[SENSORS_ITEM_HUMIDITY], "N/A");

        labels[SENSORS_ITEM_PRESSURE] = "Pressure";
        if (sensor_present) sprintf(valbufs[SENSORS_ITEM_PRESSURE], "%.0fhPa", sensor_pressure_hpa());
        else                 sprintf(valbufs[SENSORS_ITEM_PRESSURE], "N/A");

        labels[SENSORS_ITEM_MODEL] = "Sensor";
        sprintf(valbufs[SENSORS_ITEM_MODEL], "%s", sensor_chip_name());

        labels[SENSORS_ITEM_BACK] = "BACK";
        valbufs[SENSORS_ITEM_BACK][0] = 0;

        draw_menu_list_disp("SENSORS", labels, valbufs, SENSORS_ITEM_COUNT, sensors_menu_cursor);
      }
    #endif
    #if HAS_WIFI == true || HAS_ETHERNET == true
      else if (menu_state == MENU_STATE_STATUS_POPUP) {
        // update_display() (Display.h) unconditionally clears the whole
        // screen before calling draw_settings_menu_disp() on every normal
        // redraw cycle - so "superimposed on the screen underneath" only
        // holds up if that screen gets redrawn fresh every time too, not
        // just once when the popup first opened. Temporarily swapping in
        // menu_popup_return_state and recursing draws exactly whatever
        // menu_confirm_select() would show at that state (RTC/WiFi/
        // Ethernet list, ...); the box goes on top of that, not a blank
        // screen. Safe to recurse - the substituted state can never itself
        // be MENU_STATE_STATUS_POPUP, so this is always exactly one level
        // deep.
        uint8_t real_state = menu_state;
        menu_state = menu_popup_return_state;
        draw_settings_menu_disp();
        menu_state = real_state;

        draw_menu_status_rect(menu_popup_text);
      }
    #endif
    #if MENU_HAS_HW_PAGE == true
      else if (menu_state == MENU_STATE_HW_LIST) {
        const char *labels[HW_ITEM_COUNT];
        char valbufs[HW_ITEM_COUNT][24];

        #if MENU_HAS_CPU_TEMP == true
          labels[HW_ITEM_TEMP] = "CPU Temp";
          sprintf(valbufs[HW_ITEM_TEMP], "%.1fC", pmu_temperature);
        #endif

        #if HAS_VSENSE == true
          labels[HW_ITEM_VOLTAGE] = "Input Voltage";
          sprintf(valbufs[HW_ITEM_VOLTAGE], "%.2fV", vsense_voltage);
        #endif

        #if HAS_BATTERY_DIVIDER == true
          labels[HW_ITEM_BATTERY] = "Battery Voltage";
          if (battery_ready) sprintf(valbufs[HW_ITEM_BATTERY], "%.2fV", battery_voltage);
          else                sprintf(valbufs[HW_ITEM_BATTERY], "N/A");
        #endif

        // No GNSS Chip row here anymore - redundant with the GNSS page's
        // own Module row (GNSS_ITEM_MODULE), see HW_NEXT_A3's own comment.

        #if HAS_WIFI == true
          // wr_device_ip/subnet only mean anything once actually connected
          // (STA) or an AP is up - wifi_is_connected() covers both, since
          // AP mode also sets wr_wifi_status = WL_CONNECTED (Remote.h).
          labels[HW_ITEM_WIFI_IP] = "WiFi IP";
          if (wifi_is_connected()) sprintf(valbufs[HW_ITEM_WIFI_IP], "%s", wr_device_ip.toString().c_str());
          else                     sprintf(valbufs[HW_ITEM_WIFI_IP], "N/A");

          labels[HW_ITEM_WIFI_NM] = "Netmask";
          if (wifi_is_connected()) {
            // AP netmask is a fixed constant (ap_nm, Remote.h) - STA's is
            // whatever DHCP/static config negotiated, not cached anywhere,
            // so read it fresh.
            if (wifi_mode == WR_WIFI_AP) sprintf(valbufs[HW_ITEM_WIFI_NM], "%s", ap_nm.toString().c_str());
            else                          sprintf(valbufs[HW_ITEM_WIFI_NM], "%s", WiFi.subnetMask().toString().c_str());
          } else {
            sprintf(valbufs[HW_ITEM_WIFI_NM], "N/A");
          }

          // Hardware-burned base MAC, always readable regardless of
          // whether the WiFi radio is currently on - not cached anywhere
          // in this codebase, so read fresh (same esp_read_mac() call
          // Bluetooth.h already uses for the BT MAC).
          labels[HW_ITEM_WIFI_MAC] = "WiFi";
          {
            uint8_t mac[6];
            esp_read_mac(mac, ESP_MAC_WIFI_STA);
            sprintf(valbufs[HW_ITEM_WIFI_MAC], "%02X:%02X:%02X:%02X:%02X:%02X",
              mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
          }
        #endif

        #if HAS_BLUETOOTH == true || HAS_BLE == true
          labels[HW_ITEM_BT_MAC] = "BT";
          #if MCU_VARIANT == MCU_ESP32
            {
              uint8_t mac[6];
              esp_read_mac(mac, ESP_MAC_BT);
              sprintf(valbufs[HW_ITEM_BT_MAC], "%02X:%02X:%02X:%02X:%02X:%02X",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            }
          #elif MCU_VARIANT == MCU_NRF52
            // Bluefruit.getAddr() only returns a real address once the BLE
            // stack has actually started (bt_start()/bt_ready, Bluetooth.h) -
            // unlike ESP32's esp_read_mac(), there's no hardware-burned MAC
            // readable before that.
            if (bt_ready) {
              ble_gap_addr_t gap_addr = Bluefruit.getAddr();
              sprintf(valbufs[HW_ITEM_BT_MAC], "%02X:%02X:%02X:%02X:%02X:%02X",
                gap_addr.addr[5], gap_addr.addr[4], gap_addr.addr[3],
                gap_addr.addr[2], gap_addr.addr[1], gap_addr.addr[0]);
            } else {
              sprintf(valbufs[HW_ITEM_BT_MAC], "N/A");
            }
          #endif
        #endif

        #if HAS_ETHERNET == true
          // Unlike WiFi/BT's hardware-burned MAC (readable via esp_read_mac()
          // at any time), the W5500's MAC is a locally-administered address
          // ETH.begin() derives from the base MAC and hands to the netif
          // (see ETH.cpp) - only valid once init_ethernet() has run, which
          // happens unconditionally at boot on this board, so this is
          // effectively always populated by the time the menu is reachable.
          labels[HW_ITEM_ETH_MAC] = "Eth";
          {
            uint8_t mac[6];
            if (ETH.macAddress(mac) != NULL) {
              sprintf(valbufs[HW_ITEM_ETH_MAC], "%02X:%02X:%02X:%02X:%02X:%02X",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            } else {
              sprintf(valbufs[HW_ITEM_ETH_MAC], "N/A");
            }
          }
        #endif

        #if HAS_GPIO_MENU == true
          labels[HW_ITEM_GPIO] = "GPIO";
          sprintf(valbufs[HW_ITEM_GPIO], ">"); // opens a submenu, not an inline value
        #endif

        #if MCU_VARIANT == MCU_ESP32
          labels[HW_ITEM_MEMORY] = "Memory";
          sprintf(valbufs[HW_ITEM_MEMORY], ">"); // opens a submenu, not an inline value
        #endif

        // Same millis()/1000 source as Display.h's draw_node_uptime() -
        // wraps back to 0 after ~49 days like every other millis()-based
        // timer in this codebase already does.
        labels[HW_ITEM_UPTIME] = "Node Uptime";
        {
          uint32_t up_s = millis()/1000;
          sprintf(valbufs[HW_ITEM_UPTIME], "%02lu:%02lu:%02lu",
            (unsigned long)(up_s/3600), (unsigned long)((up_s/60)%60), (unsigned long)(up_s%60));
        }

        labels[HW_ITEM_BACK] = "BACK";
        valbufs[HW_ITEM_BACK][0] = 0;

        draw_menu_list_disp("HARDWARE", labels, valbufs, HW_ITEM_COUNT, hw_menu_cursor);
      }
      #if HAS_VSENSE == true || HAS_BATTERY_DIVIDER == true
        else if (menu_state == MENU_STATE_HW_EDIT) {
          char valbuf[8];
          #if HAS_VSENSE == true
            if (hw_menu_cursor == HW_ITEM_VOLTAGE) {
              // Shown as the divider ratio (e.g. "11.0"), not the raw 0-254
              // EEPROM byte or the live voltage reading - that's what's
              // actually being calibrated here.
              sprintf(valbuf, "%.1f", staged_vsense_divider_ratio_raw / 10.0);
              draw_menu_edit_disp("VOLTAGE DIVIDER", valbuf);
            }
          #endif
          #if HAS_BATTERY_DIVIDER == true
            if (hw_menu_cursor == HW_ITEM_BATTERY) {
              // Shown as a percentage of the compiled-in default scale, not
              // the raw 0-254 EEPROM byte - see bvs_conf_save().
              sprintf(valbuf, "%u%%", staged_battery_v_scale_pct);
              draw_menu_edit_disp("BATTERY CAL", valbuf);
            }
          #endif
        }
      #endif
      #if HAS_GPIO_MENU == true
        else if (menu_state == MENU_STATE_GPIO_LIST) {
          const char *labels[GPIO_ITEM_COUNT];
          char valbufs[GPIO_ITEM_COUNT][24];

          labels[GPIO_ITEM_BUZZER] = "Buzzer";
          format_gpio_pin(gpio_idx_for_pin(buzzer_pin), valbufs[GPIO_ITEM_BUZZER]);

          #if HAS_ENCODER == true
            labels[GPIO_ITEM_ENC_UP] = "Encoder Up";
            format_gpio_pin(gpio_idx_for_pin(pin_encoder_up), valbufs[GPIO_ITEM_ENC_UP]);

            labels[GPIO_ITEM_ENC_DOWN] = "Encoder Down";
            format_gpio_pin(gpio_idx_for_pin(pin_encoder_down), valbufs[GPIO_ITEM_ENC_DOWN]);

            labels[GPIO_ITEM_ENC_PRESS] = "Encoder Press";
            format_gpio_pin(gpio_idx_for_pin(pin_encoder_press), valbufs[GPIO_ITEM_ENC_PRESS]);
          #endif

          labels[GPIO_ITEM_BACK] = "BACK";
          valbufs[GPIO_ITEM_BACK][0] = 0;

          draw_menu_list_disp("GPIO", labels, valbufs, GPIO_ITEM_COUNT, gpio_menu_cursor);
        } else if (menu_state == MENU_STATE_GPIO_PIN_EDIT) {
          char valbuf[8];
          format_gpio_pin(staged_gpio_pin_idx, valbuf);
          const char *title = "BUZZER PIN";
          #if HAS_ENCODER == true
            if      (gpio_menu_cursor == GPIO_ITEM_ENC_UP)    title = "ENCODER UP PIN";
            else if (gpio_menu_cursor == GPIO_ITEM_ENC_DOWN)  title = "ENCODER DOWN PIN";
            else if (gpio_menu_cursor == GPIO_ITEM_ENC_PRESS) title = "ENCODER PRESS PIN";
          #endif
          draw_menu_edit_disp(title, valbuf);
        }
      #endif
      #if MCU_VARIANT == MCU_ESP32
        else if (menu_state == MENU_STATE_MEM_LIST) {
          draw_menu_memory_disp();
        } else if (menu_state == MENU_STATE_MEM_DETAIL) {
          const char *labels[MEM_DETAIL_ITEM_COUNT];
          char valbufs[MEM_DETAIL_ITEM_COUNT][24];

          uint32_t total = 0, free_b = 0, min_free = 0;
          bool have_reading = true;
          if (mem_menu_cursor == MEM_ITEM_HEAP) {
            total    = ESP.getHeapSize();
            free_b   = ESP.getFreeHeap();
            min_free = ESP.getMinFreeHeap();
          } else { // MEM_ITEM_PSRAM
            if (psramFound()) {
              total    = ESP.getPsramSize();
              free_b   = ESP.getFreePsram();
              min_free = ESP.getMinFreePsram();
            } else {
              have_reading = false;
            }
          }

          labels[MEM_DETAIL_ITEM_TOTAL]   = "Total";
          labels[MEM_DETAIL_ITEM_USED]    = "Used";
          labels[MEM_DETAIL_ITEM_FREE]    = "Free";
          labels[MEM_DETAIL_ITEM_MINFREE] = "Min Free";
          if (have_reading) {
            sprintf(valbufs[MEM_DETAIL_ITEM_TOTAL],   "%.1fKB", total / 1024.0);
            sprintf(valbufs[MEM_DETAIL_ITEM_USED],    "%.1fKB", (total - free_b) / 1024.0);
            sprintf(valbufs[MEM_DETAIL_ITEM_FREE],    "%.1fKB", free_b / 1024.0);
            sprintf(valbufs[MEM_DETAIL_ITEM_MINFREE], "%.1fKB", min_free / 1024.0);
          } else {
            sprintf(valbufs[MEM_DETAIL_ITEM_TOTAL],   "N/A");
            sprintf(valbufs[MEM_DETAIL_ITEM_USED],    "N/A");
            sprintf(valbufs[MEM_DETAIL_ITEM_FREE],    "N/A");
            sprintf(valbufs[MEM_DETAIL_ITEM_MINFREE], "N/A");
          }

          labels[MEM_DETAIL_ITEM_BACK] = "BACK";
          valbufs[MEM_DETAIL_ITEM_BACK][0] = 0;

          draw_menu_list_disp(mem_menu_cursor == MEM_ITEM_HEAP ? "HEAP" : "PSRAM",
            labels, valbufs, MEM_DETAIL_ITEM_COUNT, mem_detail_cursor);
        }
      #endif
    #endif
    #if HAS_OTA == true
      else if (menu_state == MENU_STATE_FWUPD_LIST) {
        const char *labels[FWUPD_ITEM_COUNT];
        char valbufs[FWUPD_ITEM_COUNT][24];

        labels[FWUPD_ITEM_CURRENT] = "Current";
        snprintf(valbufs[FWUPD_ITEM_CURRENT], 24, "%s.%d", ota_current_version().c_str(), BUILD_NUMBER);

        labels[FWUPD_ITEM_LATEST] = "Latest";
        if (fwupd_latest_ok) snprintf(valbufs[FWUPD_ITEM_LATEST], 24, "%s.%ld", ota_current_version().c_str(), fwupd_latest_build);
        else                  sprintf(valbufs[FWUPD_ITEM_LATEST], "N/A");

        labels[FWUPD_ITEM_UPDATE] = "Update";
        sprintf(valbufs[FWUPD_ITEM_UPDATE], ">"); // opens a submenu, not an inline value

        labels[FWUPD_ITEM_BACK] = "BACK";
        valbufs[FWUPD_ITEM_BACK][0] = 0;

        draw_menu_list_disp("F/W UPDATE", labels, valbufs, FWUPD_ITEM_COUNT, fwupd_menu_cursor);
      } else if (menu_state == MENU_STATE_FWUPD_CONFIRM) {
        // Plain 2-item list, same draw_menu_list_disp() as everywhere else -
        // same pattern as WiFi's SAVE/DISCARD (MENU_STATE_WIFI_TEXT_CONFIRM).
        const char *labels[2] = { "UPDATE", "CANCEL" };
        char valbufs[2][24];
        valbufs[0][0] = 0;
        valbufs[1][0] = 0;
        draw_menu_list_disp("UPDATE?", labels, valbufs, 2, fwupd_confirm_cursor);
      }
    #endif
  }

#endif
