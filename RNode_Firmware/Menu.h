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
  #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_WTRACKER_V2 || BOARD_MODEL == BOARD_HELTEC_T1
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
    // Pre-remodel value (was the shared function's own hardcoded
    // setCursor(6, 8)/drawFastHLine(4, 12, ...) before every screen's
    // title/divider got pulled onto these two macros) - this board isn't
    // part of the current UI remodel (128x64 only), so kept as-is.
    #define MENU_HEADER_TEXT_Y 8
    #define MENU_HEADER_HLINE_Y 12
    // Nudged 1px down from the original MENU_CANVAS_H-10 per user request on
    // real hardware, alongside the footer text's own 1px nudge below.
    #define MENU_LIST_FOOTER_HLINE_Y (MENU_CANVAS_H - 9)
    // Org_01's 'p' (the deepest descender in "tap:next hold:open"/"turn:move
    // press:open") has yOffset=-3, height=5, so its bottom row sits at
    // baseline+1 - MENU_CANVAS_H-2 landed that row exactly on the canvas's
    // last physical pixel row, flush with the screen's bottom edge. Nudged
    // one more pixel down per user request on real hardware - the 'p'
    // descender's very last row now falls past the canvas edge and is
    // clipped, but every other row (including the rest of that descender)
    // still renders fine.
    #define MENU_LIST_FOOTER_TEXT_Y (MENU_CANVAS_H - 1)
    #define MENU_EDIT_VALUE_CX (MENU_CANVAS_W / 2)
    // Matches the list screen's header hline (y=12, draw_menu_list_disp())
    // per user request on real hardware - was its own hardcoded 15.
    #define MENU_EDIT_HEADER_HLINE_Y 12
    // Centered within the content region between the header hline
    // (MENU_EDIT_HEADER_HLINE_Y, y=12) and the footer hline
    // (MENU_EDIT_FOOTER_HLINE_Y, y=71) - same approach T114 uses above.
    // That region's midpoint is 12+(71-12)/2=41.5. Org_01's digits/caps are
    // yOffset=-4, height=5, so at size 2 a glyph spans
    // [baseline-8, baseline+1] - visual center baseline-3.5 - giving
    // baseline=41.5+3.5=45.
    //
    // Arrows are size 1, so the same glyphs span [baseline-4, baseline] -
    // visual center baseline-2 - solving baseline-2=41.5 gives 43.5,
    // rounded to 43; kept 2px above the value baseline like the original
    // 34-vs-36 spacing, since that gap was already tuned to visually match
    // the two different-sized glyph sets rather than their raw baselines.
    #define MENU_EDIT_VALUE_Y 45
    #define MENU_EDIT_ARROW_Y 43
    #define MENU_EDIT_ARROW_R_EDGE (MENU_CANVAS_W - 5)
    // Matches the list screen's footer position above - same footer line/
    // text height across every T096 menu screen, even though the edit
    // screen's own content doesn't need the reclaimed space.
    #define MENU_EDIT_FOOTER_HLINE_Y MENU_LIST_FOOTER_HLINE_Y
    #define MENU_EDIT_FOOTER_TEXT_Y MENU_LIST_FOOTER_TEXT_Y
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
    // 8 rows (22 to 22+8*20=182) left 37px of clear space before the footer
    // hline at MENU_CANVAS_H-21=219 - room for one more 20px row (182-202)
    // with 17px still spare; a 10th would collide with the footer hline.
    #define MENU_LIST_VISIBLE_ROWS 9
    #define MENU_LIST_TOP_Y 22
    // Tamsyn6x12's ascent is 9px (glyph top sits 9px above the baseline,
    // per its own GFXglyph yOffset=-9) - a 7px offset (right for Org_01's
    // ~4px ascent) clipped the tops of letters against the selection box.
    // Centering 12px-tall glyphs in a 20px row wants ~4px of margin above
    // and below, so baseline = 4 (margin) + 9 (ascent) = 13.
    #define MENU_LIST_BASELINE_OFF 13
    // Pre-remodel value (was the shared function's own hardcoded
    // setCursor(6, 8)/drawFastHLine(4, 12, ...) before every screen's
    // title/divider got pulled onto these two macros) - this board isn't
    // part of the current UI remodel (128x64 only), so kept as-is.
    #define MENU_HEADER_TEXT_Y 8
    #define MENU_HEADER_HLINE_Y 12
    #define MENU_LIST_FOOTER_HLINE_Y (MENU_CANVAS_H - 21)
    // +3 past the plain "same margin as the row baseline" value - otherwise
    // the footer text's own ascent pokes above MENU_LIST_FOOTER_HLINE_Y.
    #define MENU_LIST_FOOTER_TEXT_Y (MENU_CANVAS_H - 12)
    #define MENU_EDIT_VALUE_CX (MENU_CANVAS_W / 2)
    #define MENU_EDIT_HEADER_HLINE_Y 15
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
    // 5px up from the original 15 - tracks MENU_HEADER_HLINE_Y below 1:1
    // (both nudged 1px lower than their tightest-fit values), per user
    // request on real hardware.
    #define MENU_LIST_TOP_Y 10
    #define MENU_LIST_BASELINE_OFF 7
    // Same Org_01 ~4px-ascent title/caption baseline as the T096 group above.
    #define MENU_HEADER_TEXT_Y 4
    // Header divider, shared by every screen (list, edit, and the bespoke
    // address/datetime/text-wheel editors below). Org_01 glyphs at
    // MENU_HEADER_TEXT_Y=4 bottom out at baseline+1 (its ~1px descent), so 6
    // would be the tightest the line could sit without touching a glyph's
    // own pixels - nudged 1px lower than that per user request on real
    // hardware.
    #define MENU_HEADER_HLINE_Y 7
    // Zero-clip max: same 'p'-descender geometry as the T096 group's own
    // MENU_LIST_FOOTER_TEXT_Y comment - MENU_CANVAS_H isn't defined for this
    // (unbuffered-display) branch, so this is the literal 64px SSD1306
    // height's own -2.
    #define MENU_LIST_FOOTER_TEXT_Y 62
    // 1px above the footer caption's own top row (MENU_LIST_FOOTER_TEXT_Y
    // minus its ~4px ascent) would be the tightest fit, same "hug the
    // caption" rule the header line above uses - nudged 2px higher than
    // that per user request on real hardware.
    #define MENU_LIST_FOOTER_HLINE_Y (MENU_LIST_FOOTER_TEXT_Y - 4 - 1 - 2)
    #define MENU_EDIT_VALUE_CX 64
    // Same header line as every other screen on this board (including the
    // bespoke address/datetime/text-wheel editors below, which used to
    // duplicate this as their own hardcoded 15) - no more a separately-
    // tracked value that has to be kept in sync by hand.
    #define MENU_EDIT_HEADER_HLINE_Y MENU_HEADER_HLINE_Y
    // Centered on the physical 128x64 canvas (64/2=32), not on the header/
    // footer hline region like the T096/T114 groups above - unlike those
    // boards' much larger off-screen canvases, this one was tuned directly
    // against the fixed display height, so moving the hlines above doesn't
    // change these: 32+3.5=35.5 rounds to 36 (size-2 digits/caps, same
    // formula as T096/T114's own MENU_EDIT_VALUE_Y derivation), 32+2=34
    // (size-1 arrows, kept 2px above the value baseline).
    #define MENU_EDIT_VALUE_Y 36
    #define MENU_EDIT_ARROW_Y 34
    #define MENU_EDIT_ARROW_R_EDGE 123
    // Same footer line as the list screen (and the bespoke editors below,
    // which used to duplicate this as their own hardcoded 50/59) - also
    // pushes this board's edit-screen footer caption all the way to the
    // bottom, which it wasn't before.
    #define MENU_EDIT_FOOTER_HLINE_Y MENU_LIST_FOOTER_HLINE_Y
    #define MENU_EDIT_FOOTER_TEXT_Y MENU_LIST_FOOTER_TEXT_Y
  #endif

  // Optional leading icon column for draw_menu_list_disp() rows - board-
  // independent (the icons themselves are small fixed-size Piskel glyphs,
  // see bm_menu_icon_* in Graphics.h), unlike the row metrics above which
  // vary per board/canvas. Only lists that build their own icons[] table
  // (draw_settings_menu_disp()'s RNODE SETTINGS list, MESSENGER's top
  // screen) reserve this full shared column - every other list's labels
  // still start at the plain x=8 those use when the icons param is left at
  // its nullptr default. See MENU_BACK_TEXT_X below for the separate,
  // narrower treatment every plain "BACK" row gets automatically instead.
  #define MENU_ROW_ICON_X 6
  #define MENU_ROW_ICON_COL_W MENU_ICON_W_INBOX // widest icon (19px)
  #define MENU_ROW_ICON_GAP 3
  #if BOARD_MODEL == BOARD_HELTEC_T114
    // Same shared-column math as every other board, but against T114's
    // much taller 20px row (MENU_LIST_ROW_H above) and bigger Tamsyn6x12
    // font the label still read as sitting too far right of its icon -
    // moved closer, confirmed on real hardware (first pass -10, nudged
    // back +2 after a follow-up look).
    #define MENU_ROW_TEXT_X_ICONS (MENU_ROW_ICON_X + MENU_ROW_ICON_COL_W + MENU_ROW_ICON_GAP - 8)
    // Row-centered icon (draw_menu_list_disp() below) still read a touch
    // off relative to the label - confirmed on real hardware (first pass
    // -1, nudged back +1 after a follow-up look, netting 0/no adjustment
    // from plain row-centering).
    #define MENU_ICON_Y_NUDGE 0
  #else
    // -8: requested cut to the icon/label gap on non-T114 boards (-5, then
    // a further -3 after a look on real hardware), same "nudge the shared
    // column, don't touch per-icon widths" idiom T114 uses above.
    #define MENU_ROW_TEXT_X_ICONS (MENU_ROW_ICON_X + MENU_ROW_ICON_COL_W + MENU_ROW_ICON_GAP - 8)
    #define MENU_ICON_Y_NUDGE 0
  #endif
  // Deliberately not MENU_ROW_TEXT_X_ICONS - that column is sized for the
  // widest icon of any list that opts in (bm_menu_icon_inbox, 19px), which
  // would collide with plain lists' own longer labels/inline values (e.g.
  // URNS's "Remote Management" + ON/OFF) if every BACK row reserved it too.
  // BACK only ever needs room for its own narrow (9px) glyph.
  #define MENU_BACK_TEXT_X (MENU_ROW_ICON_X + MENU_ICON_W_BACK + MENU_ROW_ICON_GAP)

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
  #define MENU_STATE_MEM_LIST       24  // Hardware > Memory submenu - Heap (MCU_ESP32 also gets PSRAM) bar graphs, read-only
  #define MENU_STATE_MEM_DETAIL     25  // Memory > Heap or PSRAM detail readout (Total/Used/Free, MCU_ESP32 also gets Min Free), read-only
  #define MENU_STATE_ESPNOW_LIST    26  // ESP-NOW submenu list (HAS_ESPNOW boards) - Enabled/Mode/Back
  #define MENU_STATE_ESPNOW_EDIT    27  // editing whichever of Enabled/Mode/LR was selected
  #define MENU_STATE_ESPNOW_LR_CONFIRM 28 // info row + ENABLE/CANCEL list, shown only when LR is being turned on - same pattern as MENU_STATE_FWUPD_CONFIRM
  #define MENU_STATE_URNS_LIST      29  // URNS submenu list (HAS_URNS boards) - Enabled/Transport Mode/Path Table/Free/Back
  #define MENU_STATE_URNS_EDIT      30  // editing whichever of Enabled/Transport Mode was selected
  #define MENU_STATE_URNS_PATHS     31  // Path Table submenu (HAS_URNS boards) - read-only, no edit state
  #define MENU_STATE_URNS_PATH_DETAIL 32 // one path entry's hash (enterable, opens MENU_STATE_URNS_PATH_HASH_VIEW) + expiry (HAS_URNS boards) - read-only
  #define MENU_STATE_MSNGR_LIST     33 // Messenger app top screen (HAS_URNS boards) - Inbox/Bookmarks/Announces/Announce Node/Back
  #define MENU_STATE_MSNGR_INBOX    34 // list of conversations (peers who've messaged us), recent-first
  #define MENU_STATE_MSNGR_BOOKMARKS 35 // list of saved bookmark addresses
  #define MENU_STATE_MSNGR_ANNOUNCES 36 // list of 1-hop LXMF announces heard on-air since boot
  #define MENU_STATE_MSNGR_PEER     37 // unified per-peer screen - recent messages + Send Hi/Bye/SOS + Bookmark toggle
  #define MENU_STATE_MSNGR_MSG_DETAIL 38 // one message's full text, opened from MENU_STATE_MSNGR_PEER
  #define MENU_STATE_MSNGR_DELETE_CONFIRM 39 // DELETE/CANCEL list before a single message is actually deleted - same pattern as MENU_STATE_FWUPD_CONFIRM
  #define MENU_STATE_MSNGR_CLEAR_CONFIRM  40 // CLEAR/CANCEL list before a whole conversation is actually cleared - same pattern as MENU_STATE_FWUPD_CONFIRM
  #define MENU_STATE_MSNGR_TEXT_ENTRY     41 // on-screen keyboard for composing a free-text message, opened from MSNGR_PEER_FIXED_ACTION_SEND_CUSTOM
  #define MENU_STATE_MSNGR_DISCARD_CONFIRM 42 // DISCARD/CANCEL list before leaving MENU_STATE_MSNGR_TEXT_ENTRY with unsent text - same pattern as MENU_STATE_FWUPD_CONFIRM
  #define MENU_STATE_MSNGR_PING_RESULT 43 // live status + BACK, opened from MSNGR_PEER_FIXED_ACTION_PING (Messenger.h's messenger_ping_start())
  #define MENU_STATE_URNS_PATH_HASH_VIEW 44 // full path hash, two plain lines, no captions - opened from MENU_STATE_URNS_PATH_DETAIL's Hash row, dismissed by any input
  #define MENU_STATE_URNS_FREE_DETAIL 45 // urns partition usage broken down by data type (HAS_URNS boards), opened from MENU_STATE_URNS_LIST's Free row - read-only, computed once on entry (never in a draw path - see project_urns_partition_growth memory)
  #define MENU_STATE_MSNGR_SEND_RESULT 46 // live status (Sending.../Delivered/No Confirmation) + BACK, opened from MENU_STATE_MSNGR_PEER's Send Hi/Bye/SOS and MENU_STATE_MSNGR_TEXT_ENTRY's Send key - same "live status + BACK" shape as MENU_STATE_MSNGR_PING_RESULT, auto-dismisses on Delivered/No Confirmation (msngr_send_result_process(), polled from loop()) unlike Ping's manual-only dismiss
  #define MENU_STATE_URNS_RADIO_LIST 47 // Radio submenu list - Frequency/Bandwidth/SF/CR/TX Power/Back - top-level item, sits right under URNS on boards that have it, not nested inside URNS_LIST, and not gated on HAS_URNS (configures the same TNC-mode EEPROM fields a connected host already uses independently of the onboard URNS node)
  #define MENU_STATE_URNS_RADIO_EDIT 48 // editing whichever of Frequency/Bandwidth/SF/CR/TX Power was selected
  #if HAS_GPS == true && HAS_GNSS_DEBUG_MENU == true
    #define MENU_STATE_GNSS_DIAG      49 // verbose GNSS diagnostics summary - opened from MENU_STATE_GNSS_LIST's Diagnostics row
    #define MENU_STATE_GNSS_DIAG_SATS 50 // satellites-in-view list (PRN/El/Az/SNR, from GPGSV) - opened from MENU_STATE_GNSS_DIAG's Sats In View row
  #endif
  #define MENU_STATE_BT_LIST 51 // Bluetooth submenu list (HAS_BLUETOOTH/HAS_BLE boards) - Settings (HAS_BLE only, opens MENU_STATE_BT_SETTINGS)/MAC/Bonds (MCU_ESP32 only)/Forget Bonds (MCU_ESP32 only)/Back
  // 52 (formerly MENU_STATE_BT_EDIT) intentionally left unused, not
  // reassigned - Legacy Pairing/Just Works moved to MENU_STATE_BT_SETTINGS/
  // _EDIT below; renumbering every constant after it wasn't worth it for a
  // single freed slot.
  #define MENU_STATE_BT_UNPAIR_CONFIRM 53 // FORGET/CANCEL list before bt_debond_all() actually runs (HAS_BLE boards) - same pattern as MENU_STATE_FWUPD_CONFIRM
  #define MENU_STATE_MSNGR_SETTINGS 54 // Messenger's own Settings submenu (HAS_URNS boards) - Retries/Back, opened from MENU_STATE_MSNGR_LIST
  #define MENU_STATE_MSNGR_SETTINGS_EDIT 55 // editing the Retries field (the only editable row in MENU_STATE_MSNGR_SETTINGS today)
  #define MENU_STATE_BT_SETTINGS 56 // Bluetooth's own Settings submenu (HAS_BLE only) - Legacy Pairing/Just Works (moved from MENU_STATE_BT_LIST, MCU_ESP32 only)/Auto Start (every HAS_BLE board)/Battery Service (every HAS_BLE board)/Back, opened from MENU_STATE_BT_LIST
  #define MENU_STATE_BT_SETTINGS_EDIT 57 // editing whichever of Legacy Pairing/Just Works/Auto Start/Battery Service was selected
  #define MENU_STATE_MSNGR_PRESETS 58 // configurable quick-send buttons list (0-5) - Add Preset row (hidden once full) + one row per configured preset + BACK, opened from MENU_STATE_MSNGR_SETTINGS' own Preset Messages row
  #define MENU_STATE_MSNGR_PRESET_DETAIL 59 // Edit/Delete/BACK for one existing preset, opened by selecting its row in MENU_STATE_MSNGR_PRESETS - same 3-row tail shape as MENU_STATE_MSNGR_MSG_DETAIL's Reply/Delete/BACK

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

  #if HAS_NP == true
    // NeoPixel intensity scalar (led_set_intensity()/np_intensity,
    // Utilities.h) - scales the existing RX/TX colors down, doesn't change
    // them. Grouped with the other optional-hardware rows (Sound, Encoder)
    // right after Sound.
    #define MENU_ITEM_NEOPIXEL_BRIGHTNESS MENU_NEXT_IDX_S
    #define MENU_NEXT_IDX_SN (MENU_NEXT_IDX_S + 1)
  #else
    #define MENU_NEXT_IDX_SN MENU_NEXT_IDX_S
  #endif

  #if HAS_ENCODER == true
    // Whether a physical encoder is actually populated - some boards have
    // it PCB-provisioned but optionally installed (MeshAdventurer-S3), or
    // as a DIY add-on that most builds skip (PROMICRO) - so this is a
    // runtime toggle (encoder_enabled, Utilities.h), not the compile-time
    // HAS_ENCODER capability flag. Only changes the on-screen footer hint
    // (turn/press vs tap/hold) - the encoder itself is always serviced
    // regardless, same as before this existed.
    #define MENU_ITEM_ENCODER MENU_NEXT_IDX_SN
    #define MENU_NEXT_IDX_SE  (MENU_NEXT_IDX_SN + 1)
  #else
    #define MENU_NEXT_IDX_SE MENU_NEXT_IDX_SN
  #endif

  #if HAS_LXMF == true
    // Opens the Messenger app's top screen (MENU_STATE_MSNGR_LIST) - the
    // same screen a long main-button hold jumps to directly
    // (BUTTON_HOLD_TIER_MESSENGER below). Sits right above the URNS
    // diagnostics submenu, since both depend on the same onboard node.
    // Moved above ESP-NOW per user request - both items depend on the
    // same onboard microReticulum node, kept near the top of the list.
    #define MENU_ITEM_MESSENGER MENU_NEXT_IDX_SE
    #define MENU_NEXT_IDX_SE2 (MENU_NEXT_IDX_SE + 1)
  #else
    #define MENU_NEXT_IDX_SE2 MENU_NEXT_IDX_SE
  #endif

  #if HAS_URNS == true
    // Opens the URNS submenu (Enabled + Path Table, URNS_ITEM_*,
    // MENU_STATE_URNS_LIST/EDIT) - same shape as ESP-NOW below. Enabled
    // reboots on change (urns_init()/urns_radio_bringup() are boot-only,
    // no live start/stop path) - staged here, actually written by
    // menu_commit_and_exit(), same as ESP-NOW's own Enabled field.
    #define MENU_ITEM_URNS MENU_NEXT_IDX_SE2
    #define MENU_NEXT_IDX_SE2R (MENU_NEXT_IDX_SE2 + 1)
  #else
    #define MENU_NEXT_IDX_SE2R MENU_NEXT_IDX_SE2
  #endif

  // Opens the Radio submenu (Frequency/Bandwidth/SF/CR/TX Power,
  // URNS_RADIO_ITEM_*, MENU_STATE_URNS_RADIO_LIST/EDIT) - sits right under
  // URNS on boards that have it, per user request. NOT gated on HAS_URNS:
  // it configures the same lora_freq/bw/sf/cr/txp globals and EEPROM
  // fields (eeprom_conf_save() etc, Utilities.h) the classic host-attached
  // TNC mode already uses via CMD_FREQUENCY/CMD_CONF_SAVE, which works
  // independently of the onboard URNS node - so this menu item is useful
  // on any board, whether or not HAS_URNS is compiled in.
  #define MENU_ITEM_URNS_RADIO MENU_NEXT_IDX_SE2R
  #define MENU_NEXT_IDX_SE3 (MENU_NEXT_IDX_SE2R + 1)

  #if HAS_ESPNOW == true
    // Opens the ESP-NOW submenu (Enabled + Mode fields, ESPNOW_ITEM_*,
    // MENU_STATE_ESPNOW_LIST/EDIT) - same shape as GNSS/Sensors below.
    // Both fields reboot on change (espnow_conf_save()/espnow_mode_conf_save(),
    // Utilities.h) - ESP-NOW has no runtime start/stop path, only a
    // boot-time espnow_init() call - so like WiFi's Mode field, they're
    // only staged here and actually written by menu_commit_and_exit().
    #define MENU_ITEM_ESPNOW MENU_NEXT_IDX_SE3
    #define MENU_NEXT_IDX_0  (MENU_NEXT_IDX_SE3 + 1)
  #else
    #define MENU_NEXT_IDX_0 MENU_NEXT_IDX_SE3
  #endif

  #if HAS_WIFI == true
    #define MENU_ITEM_WIFI  MENU_NEXT_IDX_0
    #define MENU_NEXT_IDX_A (MENU_NEXT_IDX_0 + 1)
  #else
    #define MENU_NEXT_IDX_A MENU_NEXT_IDX_0
  #endif

  #if HAS_BLUETOOTH == true || HAS_BLE == true
    // Opens the Bluetooth submenu (Legacy Pairing [MCU_ESP32 && HAS_BLE
    // only] + MAC, BT_ITEM_*, MENU_STATE_BT_LIST/EDIT) - sits right under
    // WiFi per user request.
    #define MENU_ITEM_BLUETOOTH MENU_NEXT_IDX_A
    #define MENU_NEXT_IDX_A1    (MENU_NEXT_IDX_A + 1)
  #else
    #define MENU_NEXT_IDX_A1 MENU_NEXT_IDX_A
  #endif

  #if HAS_ETHERNET == true
    // MeshPoE-S3 only - see Boards.h. Sits right after WiFi/Bluetooth.
    #define MENU_ITEM_ETHERNET MENU_NEXT_IDX_A1
    #define MENU_NEXT_IDX_A2   (MENU_NEXT_IDX_A1 + 1)
  #else
    #define MENU_NEXT_IDX_A2 MENU_NEXT_IDX_A1
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
    #define RTC_ITEM_TIMEZONE 2   // display-only UTC offset - see get_tz_offset_qh(), Utilities.h
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
    #define GNSS_ITEM_ENABLED 0   // editable, immediate-commit toggle - power-cycles PIN_GPS_EN live
    #define GNSS_NEXT_0 1

    // Duty-cycled acquisition interval (GNSS.h) - only present on boards
    // that can actually gate GNSS power (PIN_GPS_EN and/or PIN_GPS_STANDBY
    // - see the per-board audit, Boards.h). Boards without a gating pin
    // get no real benefit from duty-cycling the receiver, so this row is
    // compiled out there entirely rather than exposing a setting that
    // can't deliver what it implies.
    #if GNSS_DUTY_CYCLE_CAPABLE == true
      #define GNSS_ITEM_UPDATE_INTERVAL GNSS_NEXT_0 // editable, immediate-commit - same pattern as Enabled above
      #define GNSS_NEXT_1 (GNSS_NEXT_0 + 1)
    #else
      #define GNSS_NEXT_1 GNSS_NEXT_0
    #endif

    // Duty State (gnss_pstate_text(), GNSS.h) - universal across every
    // HAS_GPS board now, not just ones with both Diagnostics and duty-cycle
    // capability. Chip identity (former "Module" row, gnss_chip_name()) is
    // no longer shown here at all - per user request, boards with
    // Diagnostics (HAS_GNSS_DEBUG_MENU) still expose it there
    // (GNSS_DIAG_ITEM_MODULE below), boards without it just don't show
    // chip identity in the menu anymore. On a board that can't actually
    // duty-cycle (GNSS_DUTY_CYCLE_CAPABLE false), gnss_pstate_text() simply
    // always reads ACTIVE (or OFF when disabled) - still accurate, just
    // static, since gnss_duty_cycle_update() (GNSS.h) never leaves ACTIVE
    // there.
    #define GNSS_ITEM_DUTY_STATE GNSS_NEXT_1 // read-only
    #define GNSS_NEXT_2 (GNSS_NEXT_1 + 1)

    #define GNSS_ITEM_FIX        GNSS_NEXT_2       // read-only
    #define GNSS_ITEM_SATELLITES (GNSS_NEXT_2 + 1) // read-only
    #define GNSS_ITEM_LATITUDE   (GNSS_NEXT_2 + 2) // read-only
    #define GNSS_ITEM_LONGITUDE  (GNSS_NEXT_2 + 3) // read-only
    #define GNSS_ITEM_ALTITUDE   (GNSS_NEXT_2 + 4) // read-only
    // GNSS time (UTC) - populates independently of Fix/location (see
    // gnss_time_valid(), GNSS.h) - a receiver typically syncs time before
    // ever achieving a position fix, so this is a genuine diagnostic: Time
    // valid but Fix/Satellites still 0 confirms sentence parsing works
    // end-to-end and it's an antenna/sky-visibility issue, not firmware.
    #define GNSS_ITEM_TIME       (GNSS_NEXT_2 + 5) // read-only
    // Display-only UTC offset, same field/EEPROM byte/editor as RTC_ITEM_
    // TIMEZONE below (get_tz_offset_qh()/apply_tz_offset(), Utilities.h) -
    // shown here too since GNSS is a real time source in its own right on
    // boards with no RTC chip at all. Shows on both pages when a board has
    // both (same value either way, editable from either) - simplest,
    // avoids extra suppression logic for a harmless redundancy.
    #define GNSS_ITEM_TIMEZONE   (GNSS_NEXT_2 + 6)
    #define GNSS_NEXT_3          (GNSS_NEXT_2 + 7)

    #if HAS_GNSS_DEBUG_MENU == true
      #define GNSS_ITEM_DIAGNOSTICS GNSS_NEXT_3 // opens MENU_STATE_GNSS_DIAG
      #define GNSS_NEXT_3A (GNSS_NEXT_3 + 1)
    #else
      #define GNSS_NEXT_3A GNSS_NEXT_3
    #endif

    // One-shot action, not a toggle - pins draw_disp_area()'s generic OLED
    // GNSS panel (Display.h) on screen full-time, including with the radio
    // off, until the next reboot (gnss_banner_forced, GNSS.h - RAM-only,
    // never persisted, no way back from the menu). Last real row before
    // BACK, on purpose - a rarely-needed manual override shouldn't sit
    // ahead of the actual live readouts above it. Not offered on T114: its
    // GNSS content already lives in a separate, always-visible box below
    // the main 64x64 canvas (a different board-specific branch further down
    // in draw_disp_area()), never sharing screen space with the RNode/ID
    // banner or the checks/hardware-OK/version carousel the way the generic
    // path's panel does, so there's nothing here for T114 to fix.
    #if BOARD_MODEL != BOARD_HELTEC_T114
      #define GNSS_ITEM_SHOW_BANNER GNSS_NEXT_3A
      #define GNSS_NEXT_4 (GNSS_NEXT_3A + 1)
    #else
      #define GNSS_NEXT_4 GNSS_NEXT_3A
    #endif

    #define GNSS_ITEM_BACK  GNSS_NEXT_4
    #define GNSS_ITEM_COUNT (GNSS_ITEM_BACK + 1)

    #if HAS_GNSS_DEBUG_MENU == true
      // GNSS_DIAG_ITEM_* - verbose diagnostics summary page
      // (MENU_STATE_GNSS_DIAG), opened via GNSS_ITEM_DIAGNOSTICS above.
      // Uses the Hardware page's _NEXT_-chaining convention (more
      // conditional rows here than the flat two-branch GNSS_ITEM_* style
      // above handles cleanly).
      #define GNSS_DIAG_ITEM_MODULE      0   // gnss_chip_name() - chip identity, not shown on the main GNSS page at all anymore
      #define GNSS_DIAG_ITEM_FIX_QUALITY 1
      #define GNSS_DIAG_ITEM_FIX_MODE    2
      #define GNSS_DIAG_ITEM_HDOP        3
      #define GNSS_DIAG_ITEM_PDOP        4
      #define GNSS_DIAG_ITEM_VDOP        5
      #define GNSS_DIAG_ITEM_SPEED       6
      #define GNSS_DIAG_ITEM_COURSE      7
      #define GNSS_DIAG_ITEM_DATE        8
      #define GNSS_DIAG_ITEM_SATS_USED   9   // gnss_satellite_count(), GGA - kept for direct comparison against Sats In View below
      #define GNSS_DIAG_ITEM_SATS_VIEW   10  // opens MENU_STATE_GNSS_DIAG_SATS
      #define GNSS_DIAG_ITEM_CHK_PASSED  11
      #define GNSS_DIAG_ITEM_CHK_FAILED  12
      #define GNSS_DIAG_ITEM_CHK_RATE    13
      #define GNSS_DIAG_ITEM_CHARS       14
      #define GNSS_DIAG_NEXT_0           15

      #if GNSS_DUTY_CYCLE_CAPABLE == true
        #define GNSS_DIAG_ITEM_DUTY_STATE     GNSS_DIAG_NEXT_0
        #define GNSS_DIAG_ITEM_LOCK_COUNT     (GNSS_DIAG_NEXT_0 + 1)
        #define GNSS_DIAG_ITEM_FAIL_COUNT     (GNSS_DIAG_NEXT_0 + 2)
        #define GNSS_DIAG_ITEM_PREDICTED      (GNSS_DIAG_NEXT_0 + 3)
        // Always present when duty-capable (not conditionally shown/hidden
        // on live gnss_pstate) - item-enum indices are compile-time
        // constants everywhere else in this file; only this row's value
        // text changes ("--" outside SLEEP, "Ns" inside it).
        #define GNSS_DIAG_ITEM_WAKE_COUNTDOWN (GNSS_DIAG_NEXT_0 + 4)
        #define GNSS_DIAG_NEXT_1 (GNSS_DIAG_NEXT_0 + 5)
      #else
        #define GNSS_DIAG_NEXT_1 GNSS_DIAG_NEXT_0
      #endif

      #define GNSS_DIAG_ITEM_BACK  GNSS_DIAG_NEXT_1
      #define GNSS_DIAG_ITEM_COUNT (GNSS_DIAG_ITEM_BACK + 1)
    #endif
  #endif

  #if HAS_ESPNOW == true
    #define ESPNOW_ITEM_ENABLED 0   // editable - staged only, no self-reboot until SAVE & EXIT
    #define ESPNOW_ITEM_MODE    1   // editable - "v1" (classic chunked) / "v2" (unfragmented) - framing only
    #define ESPNOW_ITEM_LR      2   // editable - 802.11 LR mode ON/OFF - PHY rate only, independent of Mode
    // Editable - same wr_channel WiFi's own Channel field edits (WIFI_ITEM_CHANNEL,
    // Remote.h/ESPNOW.h both use it), not a separate value - shares the exact
    // same staged_wifi_channel/step_wifi_channel() (HAS_WIFI section above,
    // guaranteed present since HAS_ESPNOW implies HAS_WIFI on every board
    // that has it) rather than a second staged variable for the same byte,
    // so editing it from either submenu can never drift out of sync. Exposed
    // here too so an ESP-NOW-only workflow (e.g. URNS Interface set to
    // ESP-NOW, see URNS.h) never needs to dig into the unrelated WiFi
    // submenu just to change the channel that actually matters to it.
    #define ESPNOW_ITEM_CHANNEL 3
    #define ESPNOW_ITEM_BACK    4
    #define ESPNOW_ITEM_COUNT   5
  #endif

  #if HAS_BLUETOOTH == true || HAS_BLE == true
    // Read-only - reads the live value directly, not staged/committed
    // through this submenu at all - same convention as ESP-NOW's own
    // Channel row. Used to be duplicated on the Hardware page too
    // (HW_ITEM_BT_MAC) - removed from there per user request, this is now
    // the only place the BT MAC shows.
    #define BT_ITEM_MAC 0
    #if HAS_BLE == true
      // Opens MENU_STATE_BT_SETTINGS (Battery Service and Auto Start on
      // every HAS_BLE board; Legacy Pairing/Just Works added on MCU_ESP32
      // only) - below MAC, not above it. Widened from MCU_ESP32-only to
      // every HAS_BLE board (this used to be ESP32/NimBLE-only, since
      // Bluefruit already forces Legacy Pairing unconditionally and had no
      // auto-start toggle) so MCU_NRF52 gets a place to host the new
      // Battery Service toggle - see BT_SETTINGS_ITEM_* below.
      #define BT_ITEM_SETTINGS (BT_ITEM_MAC + 1)
      // Bonds is read-only (bt_bond_count(), Bluetooth.h). Forget Bonds opens
      // MENU_STATE_BT_UNPAIR_CONFIRM and calls bt_debond_all() - both are now
      // defined on every HAS_BLE board (MCU_ESP32/NimBLE and MCU_NRF52/
      // Bluefruit). Classic HAS_BLUETOOTH (Bluedroid SPP, e.g. MeshAdventurer/
      // DIY-V1) has no bond-list API and never reaches this branch - it takes
      // the #else at BT_ITEM_MAC's own HAS_BLE check above instead.
      #define BT_ITEM_BONDS (BT_ITEM_SETTINGS + 1)
      #define BT_ITEM_UNPAIR (BT_ITEM_BONDS + 1)
      #define BT_ITEM_BACK (BT_ITEM_UNPAIR + 1)
    #else
      #define BT_ITEM_BACK (BT_ITEM_MAC + 1)
    #endif
    #define BT_ITEM_COUNT (BT_ITEM_BACK + 1)

    // MENU_STATE_BT_SETTINGS/_EDIT (HAS_BLE only) - Legacy Pairing/Just
    // Works/Auto Start (MCU_ESP32 only, moved here from the top-level
    // BT_LIST above per user request) plus Battery Service (every HAS_BLE
    // board, both MCU_ESP32/NimBLE and MCU_NRF52/Bluefruit - see
    // bt_battery_service_conf_save(), Utilities.h). Own item-index space,
    // same "list + single shared EDIT state, cursor-dispatched" shape as
    // MSNGR_SETTINGS/_EDIT.
    #if HAS_BLE == true
      #if MCU_VARIANT == MCU_ESP32
        #define BT_SETTINGS_ITEM_LEGACY_PAIRING 0
        #define BT_SETTINGS_ITEM_JUST_WORKS     1
        // Whether BLE auto-starts at boot - see ADDR_CONF_BT_AUTO_START
        // (ROM.h) and the one-shot boot check (RNode_Firmware.ino) for the
        // full reasoning, including the historical bt_init() comment
        // (Bluetooth.h) confirming this exact behavior existed once before
        // and was deliberately removed over a BLE+ESP-NOW heap-exhaustion
        // crash - default OFF here for the same reason. MCU_ESP32-only:
        // nRF52's own bt_init() has no such deferral to override - it
        // already calls bt_start() unconditionally at boot whenever the
        // persisted bt_enabled flag is set, so an Auto Start toggle would
        // be a no-op there and is deliberately not offered.
        #define BT_SETTINGS_ITEM_AUTO_START     2
        #define BT_SETTINGS_ITEM_BATTERY_SERVICE 3
        #define BT_SETTINGS_ITEM_BACK           4
      #else // MCU_NRF52 - only Battery Service exists here today
        #define BT_SETTINGS_ITEM_BATTERY_SERVICE 0
        #define BT_SETTINGS_ITEM_BACK            1
      #endif
      #define BT_SETTINGS_ITEM_COUNT (BT_SETTINGS_ITEM_BACK + 1)
    #endif
  #endif

  #if HAS_URNS == true
    #define URNS_ITEM_ENABLED   0   // editable - staged only, no self-reboot until SAVE & EXIT
    // Editable - RNS::Reticulum::transport_enabled() (urns_init(), URNS.h;
    // ADDR_CONF_URNS_TRANSPORT, ROM.h). Same "staged, no self-reboot until
    // SAVE & EXIT" shape as Enabled just above - both are boot-only
    // settings with no live start/stop path. Defaults OFF: this board is
    // deliberately a leaf/client node, not a relay - see
    // project_microreticulum_onboard_node memory.
    #define URNS_ITEM_TRANSPORT 1
    #if HAS_ESPNOW == true
      // Editable, 3-way cycle (not a bool toggle) - LoRa Only / ESP-NOW
      // Only / Both, selecting which interface(s) urns_init() (URNS.h)
      // registers with RNS::Transport (ADDR_CONF_URNS_INTERFACE, ROM.h).
      // Same "staged, no self-reboot until SAVE & EXIT" shape as Enabled/
      // Transport Mode above. Only exists on boards that also have
      // HAS_ESPNOW - a LoRa-less onboard node makes no sense otherwise.
      #define URNS_ITEM_INTERFACE (URNS_ITEM_TRANSPORT + 1)
      #define URNS_ITEM_LINK_MTU_DISCOVERY_BASE URNS_ITEM_INTERFACE
    #else
      #define URNS_ITEM_LINK_MTU_DISCOVERY_BASE URNS_ITEM_TRANSPORT
    #endif
    // Editable - RNS::Reticulum::link_mtu_discovery()/remote_management_
    // enabled()/probe_destination_enabled() (urns_init(), URNS.h;
    // ADDR_CONF_URNS_LINK_MTU_DISCOVERY/_REMOTE_MGMT/_PROBE_DEST, ROM.h).
    // Same "staged, no self-reboot until SAVE & EXIT" shape as Enabled/
    // Transport Mode above - all are boot-only settings with no live
    // start/stop path, same reasoning as the built-in "uReticulum General
    // Config" Provisioning namespace's own bool fields for these
    // (BuiltinNamespaces.cpp) exposing the exact same accessors.
    #define URNS_ITEM_LINK_MTU_DISCOVERY (URNS_ITEM_LINK_MTU_DISCOVERY_BASE + 1)
    // Defaults ON - matches this file's own prior hardcoded
    // remote_management_enabled(true) (see urns_init()'s own comment on
    // why that was safe unconditionally: the real security gate is the
    // separate, empty-by-default remote_management_allowed() ALLOW_LIST).
    #define URNS_ITEM_REMOTE_MGMT (URNS_ITEM_LINK_MTU_DISCOVERY + 1)
    // Defaults OFF - matches RNS::Reticulum::probe_destination_enabled()'s
    // own library default (Reticulum.cpp).
    #define URNS_ITEM_PROBE_DEST (URNS_ITEM_REMOTE_MGMT + 1)
    // Opens MENU_STATE_URNS_PATHS - a read-only, scrollable dump of
    // RNS::Transport's live path table (destination hash + hop count),
    // same "own submenu, only BACK does anything" shape as SENSORS_LIST.
    #define URNS_ITEM_PATHS (URNS_ITEM_PROBE_DEST + 1)
    // Read-only info row - remaining free space on the "urns" LittleFS
    // partition (identity/path-table persistence + the LXMF MessageStore,
    // see MessageStore.h). LittleFS.usedBytes()/totalBytes() report for
    // whatever partition LittleFS is currently mounted to, which is
    // always "urns" on this board (urns_init(), URNS.h) since HAS_CONSOLE
    // is false here and Console.h's separate SPIFFS instance is never
    // begin()'d. Clicking it opens MENU_STATE_URNS_FREE_DETAIL - a
    // breakdown of that usage by data type, same "own submenu, only BACK
    // does anything" shape as URNS_PATH_DETAIL/SENSORS_LIST. Computed
    // once on entry (urns_free_detail_refresh(), Menu.h) rather than in
    // the draw path - see the row's own draw-code comment for why.
    #define URNS_ITEM_FREE (URNS_ITEM_PATHS + 1)
    #define URNS_ITEM_BACK (URNS_ITEM_FREE + 1)
    #define URNS_ITEM_COUNT (URNS_ITEM_BACK + 1)

    // MENU_STATE_URNS_FREE_DETAIL rows - each is one data-type bucket on
    // the urns partition (see urns_free_detail_refresh()):
    //   Identity - /urns/identity, the node's own key file
    //   Announce - RNS::Identity's known-destinations cache
    //     (URNS_KNOWN_STORE_PATH, URNS.h) - what lets Identity::recall()
    //     find a peer's public key without a fresh announce every time
    //   Paths    - RNS::Transport's persisted path table
    //     (URNS_PATH_STORE_PATH)
    //   Messages - the LXMF MessageStore (URNS_MESSAGES_PATH) - payloads,
    //     conversation index, per-peer metadata
    //   Other    - remainder (total_used minus the four buckets above) -
    //     bookmarks.json, display_name, and LittleFS's own metadata
    //     overhead, not worth walking individually
    #define URNS_FREE_DETAIL_ITEM_IDENTITY 0
    #define URNS_FREE_DETAIL_ITEM_ANNOUNCE 1
    #define URNS_FREE_DETAIL_ITEM_PATHS    2
    #define URNS_FREE_DETAIL_ITEM_MESSAGES 3
    #define URNS_FREE_DETAIL_ITEM_OTHER    4
    #define URNS_FREE_DETAIL_ITEM_BACK     5
    #define URNS_FREE_DETAIL_ITEM_COUNT    6

    // Path table rows are runtime-sized (RNS_PATH_TABLE_MAX is 100,
    // Transport.cpp) - this caps how many get built into on-screen rows
    // per draw call. draw_menu_list_disp() already scrolls to keep the
    // cursor visible past MENU_LIST_VISIBLE_ROWS, so this only bounds
    // stack usage / iteration cost, not what's reachable by scrolling.
    #define MENU_URNS_PATH_MAX_ROWS 24

    // MENU_STATE_URNS_PATH_DETAIL - opened by clicking a real path row
    // (not BACK, not the "No Paths" placeholder) in MENU_STATE_URNS_PATHS.
    // The list row only shows the destination hash's first 8 hex chars;
    // Hash here shows a longer preview and is itself enterable, opening
    // MENU_STATE_URNS_PATH_HASH_VIEW for the full 32 hex chars with
    // nothing else on screen. Expiry shows time remaining until
    // RNS::Persistence::DestinationEntry's own _expires timestamp.
    #define URNS_PATH_DETAIL_ITEM_HASH   0
    #define URNS_PATH_DETAIL_ITEM_EXPIRY 1
    #define URNS_PATH_DETAIL_ITEM_BACK   2
    #define URNS_PATH_DETAIL_ITEM_COUNT  3

  #if HAS_LXMF == true
    // Messenger app (Messenger.h) - MENU_STATE_MSNGR_LIST's own item rows.
    #define MSNGR_TOP_ITEM_INBOX         0
    #define MSNGR_TOP_ITEM_BOOKMARKS     1
    #define MSNGR_TOP_ITEM_ANNOUNCES     2
    #define MSNGR_TOP_ITEM_ANNOUNCE_NODE 3 // sends our own LXMF delivery destination announce
    #define MSNGR_TOP_ITEM_SETTINGS      4 // opens MENU_STATE_MSNGR_SETTINGS
    #define MSNGR_TOP_ITEM_BACK          5
    #define MSNGR_TOP_ITEM_COUNT         6

    // MENU_STATE_MSNGR_SETTINGS - just Retries + Back today, but its own
    // item-index space (mirrors MSNGR_TOP_ITEM_* above) so more Messenger
    // settings can be added later without renumbering the top screen.
    #define MSNGR_SETTINGS_ITEM_RETRIES           0
    #define MSNGR_SETTINGS_ITEM_RETRY_DELAY       1
    #define MSNGR_SETTINGS_ITEM_ANNOUNCE_START    2
    #define MSNGR_SETTINGS_ITEM_ANNOUNCE_INTERVAL 3
    #define MSNGR_SETTINGS_ITEM_DISPLAY_NAME      4 // opens MENU_STATE_MSNGR_TEXT_ENTRY (reused from the message composer), not MSNGR_SETTINGS_EDIT
    #define MSNGR_SETTINGS_ITEM_PRESETS           5 // opens MENU_STATE_MSNGR_PRESETS
    #define MSNGR_SETTINGS_ITEM_BACK              6
    #define MSNGR_SETTINGS_ITEM_COUNT             7

    // "ANNOUNCED" has nothing to acknowledge (unlike "NOT READY", which
    // stays up until dismissed - same success/error asymmetry as NTP sync's
    // own popup, see NTP_SYNC_SUCCESS_POPUP_MS), so it auto-dismisses on
    // its own after this long.
    #define MSNGR_ANNOUNCE_POPUP_MS 10000

    // Caps on-screen rows for the Inbox/Bookmarks/Announces lists, same
    // "bound iteration/stack, scrolling still reaches everything past this"
    // shape as MENU_URNS_PATH_MAX_ROWS above. Bookmarks/announces are
    // already hard-capped at MSNGR_MAX_BOOKMARKS/MSNGR_MAX_ANNOUNCES
    // (Messenger.h), well under this; Inbox (MessageStore conversations)
    // can have up to LXMF::MAX_CONVERSATIONS (32), so this is the one that
    // actually bites.
    #define MENU_MSNGR_LIST_MAX_ROWS 16

    // MENU_STATE_MSNGR_PEER - trailing action rows appended after however
    // many message-snippet rows the current peer's thread contributes
    // (messenger_peer_msg_row_count()). Indices are relative offsets added
    // to that row count, not absolute - see draw/confirm handling.
    //
    // The first msngr_preset_count rows are "Send: <preset text>"
    // buttons (0 to MSNGR_MAX_PRESETS of them, user-configurable -
    // RNode Settings > Messenger > Settings > Preset Messages) - dynamic,
    // not fixed compile-time indices the way SEND_HI/BYE/SOS used to be,
    // so they don't get their own #defines here. MSNGR_PEER_FIXED_
    // ACTION_* below are relative to *that*, i.e. the actual row index
    // is msg_rows + msngr_preset_count + MSNGR_PEER_FIXED_ACTION_x - see
    // msngr_peer_row_count() and the draw/confirm handling for exactly
    // where the two pieces join.
    #define MSNGR_PEER_FIXED_ACTION_SEND_CUSTOM 0 // opens MENU_STATE_MSNGR_TEXT_ENTRY
    #define MSNGR_PEER_FIXED_ACTION_PING        1 // opens MENU_STATE_MSNGR_PING_RESULT
    #define MSNGR_PEER_FIXED_ACTION_BOOKMARK    2 // label switches Add/Remove Bookmark
    #define MSNGR_PEER_FIXED_ACTION_CLEAR       3 // opens MENU_STATE_MSNGR_CLEAR_CONFIRM
    #define MSNGR_PEER_FIXED_ACTION_BACK        4
    #define MSNGR_PEER_FIXED_ACTION_COUNT       5
    // Worst-case row count for MENU_STATE_MSNGR_PEER's local draw arrays -
    // message rows + every preset slot filled + all fixed actions, even
    // though msngr_peer_row_count() (the *actual* count for any given
    // peer/preset configuration) is almost always smaller.
    #define MSNGR_PEER_MAX_ROWS (MSNGR_PEER_MAX_MSG_ROWS + MSNGR_MAX_PRESETS + MSNGR_PEER_FIXED_ACTION_COUNT)

    // MENU_STATE_MSNGR_MSG_DETAIL - a message's content, word-wrapped
    // across up to this many rows, plus one DELETE row (opens
    // MENU_STATE_MSNGR_DELETE_CONFIRM) and one BACK row, same "split across
    // several label rows" shape as URNS_PATH_DETAIL's two-row hash above.
    #define MSNGR_MSG_DETAIL_MAX_LINES 7

    // MENU_STATE_MSNGR_TEXT_ENTRY - on-screen keyboard for composing a
    // free-text message, ported from meshtastic_firmware's
    // graphics/VirtualKeyboard (a 4-row/11-col grid designed for real
    // up/down/left/right navigation). RNode boards only ever expose a
    // single rotate-or-tap + press axis (same as every other menu screen
    // in this file), so instead of porting the 2D nav this flattens the
    // grid into one linear cursor, row-major, wrapping key-to-key exactly
    // like menu_clamp_cursor() already does for every list here - rotate/
    // tap steps one key at a time, confirm_select() (encoder click /
    // button long-press, same as everywhere else) presses whichever key
    // is currently highlighted.
    #define MSNGR_KB_ROWS 4
    #define MSNGR_KB_COLS 11
    #define MSNGR_KB_KEY_COUNT (MSNGR_KB_ROWS * MSNGR_KB_COLS)
    // Matches MSNGR_MSG_DETAIL's own display cap (7 lines * 20 chars/line) -
    // no point composing a message longer than what the detail screen can
    // ever show back.
    #define MSNGR_TEXT_ENTRY_MAX_LEN 140

    // Sentinel chars double as both the grid's stored key and the dispatch
    // tag msngr_kb_key_type() below switches on - same trick meshtastic's
    // own LAYOUT uses for \b/\n/space. \x02/\x1b are added for Shift/Back,
    // which meshtastic doesn't need (it uses long-press for case, and a
    // real ESC key instead of a menu to back out of).
    static const char MSNGR_KB_LAYOUT[MSNGR_KB_ROWS][MSNGR_KB_COLS] = {
      {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\b'},
      {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '\n'},
      {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '\x02', ' '},
      {'z', 'x', 'c', 'v', 'b', 'n', 'm', '.', ',', '?', '\x1b'},
    };

    // Cyrillic (ЙЦУКЕН) layout, toggled onto the same grid via a hold on
    // the Shift key (MSNGR_KB_ALT_HOLD_MS) rather than a dedicated cell -
    // the grid is already full at 4x11. Letter cells hold the single-byte
    // glyph codes Fonts/Org_01.h's Cyrillic block was extended with (see
    // its own header comment for the Ё/А-Я/ё/а-я byte ranges), positioned
    // to match a real Russian keyboard 1:1 by row/column (Q->Й, A->Ф,
    // Z->Я, ...) so it's immediately familiar to anyone who's touch-typed
    // one. Meta keys (\b \n \x02 ' ' \x1b) stay at identical grid
    // positions in both layouts so msngr_kb_key_type()'s dispatch and the
    // column layout math don't need to know which layout is active.
    //
    // A real Russian keyboard has 3 more letter positions per row than
    // the ЙЦУКЕН rows below have cells for (the bracket/semicolon/quote
    // keys) - Б, Ё, Ж, Х, Ъ, Э, Ю don't fit into that 26-cell layout as
    // a result. Rather than lose them (or reach them through hidden
    // hold/chord gestures - tried for Ж specifically, reverted once this
    // turned out to fit directly), the top row's digits are replaced
    // with these 7 letters in RU mode instead - alphabetical order, not
    // real-keyboard position, since they no longer correspond to any
    // physical key. '.', ',', '?' are still left as plain Latin
    // punctuation rather than becoming their real-keyboard equivalents
    // (Ю/Б - already covered above anyway) - composing an actual
    // sentence needs a period more than it needs those two letters.
    // Digits themselves are still one hold/chord away via the EN/RU
    // switch (MSNGR_KB_ALT_HOLD_MS) - the 3 leftover cells (7 letters
    // into 10 slots) hold '-'/'/'/'@' instead, none of which exist
    // anywhere else on the grid: '-' is genuinely common in Russian
    // (compound words like кто-то, dates, ranges), '/' for dates and
    // и/или, '@' for addressing/handles.
    static const char MSNGR_KB_LAYOUT_RU[MSNGR_KB_ROWS][MSNGR_KB_COLS] = {
      {'\xA3', '\x80', '\xA8', '\xB7', '\xBC', '\xBF', '\xC0', '-', '/', '@', '\b'}, // Б Ё Ж Х Ъ Э Ю - / @
      {'\xAB', '\xB8', '\xB5', '\xAC', '\xA7', '\xAF', '\xA5', '\xBA', '\xBB', '\xA9', '\n'},
      {'\xB6', '\xBD', '\xA4', '\xA2', '\xB1', '\xB2', '\xB0', '\xAD', '\xA6', '\x02', ' '},
      {'\xC1', '\xB9', '\xB3', '\xAE', '\xAA', '\xB4', '\xBE', '.', ',', '?', '\x1b'},
    };

    #define MSNGR_KB_CHAR      0
    #define MSNGR_KB_BACKSPACE 1
    #define MSNGR_KB_SEND      2
    #define MSNGR_KB_SPACE     3
    #define MSNGR_KB_SHIFT     4
    #define MSNGR_KB_BACK      5

    uint8_t msngr_kb_key_type(char ch) {
      if (ch == '\b') return MSNGR_KB_BACKSPACE;
      if (ch == '\n') return MSNGR_KB_SEND;
      if (ch == ' ')  return MSNGR_KB_SPACE;
      if (ch == '\x02') return MSNGR_KB_SHIFT;
      if (ch == '\x1b') return MSNGR_KB_BACK;
      return MSNGR_KB_CHAR;
    }

    // Hold duration (ms) that means "do the alternate thing" instead of
    // the key's plain action - a deliberate hold past this switches
    // EN/RU on the Shift key, or types the paired punctuation mark on
    // '.'/','/'?' (see msngr_kb_alt_pair()) - short of MSNGR_TEXT_
    // ENTRY's own 3s leave-the-keyboard threshold (menu_encoder_button())
    // and above the 500ms floor menu_button_press() already needs just
    // to recognize a hold at all, so it's reachable as a deliberate
    // gesture on both encoder and button-only boards without colliding
    // with either.
    #define MSNGR_KB_ALT_HOLD_MS 900

    // Applies the currently-active layout's uppercase transform to a
    // literal key char - Latin's is the familiar 'a'-'A' offset; the
    // Cyrillic block in Fonts/Org_01.h was laid out with uppercase
    // exactly 0x21 below lowercase for every letter (Ё/ё included), so
    // one fixed offset covers all of it. Non-letter chars (space,
    // punctuation, meta sentinels) pass through unchanged. Shared by the
    // grid label (draw_menu_msngr_keyboard_disp()), the normal key-press
    // insert, and the encoder chord-insert (menu_encoder_chord_rotate()),
    // so all three stay in sync automatically as layouts are added.
    char msngr_kb_apply_shift(char ch, bool shift_on) {
      if (!shift_on) return ch;
      unsigned char c = (unsigned char)ch;
      if (c >= 'a' && c <= 'z') return (char)(c - 'a' + 'A');
      if (c == 0xA1) return (char)0x80;             // ё -> Ё
      if (c >= 0xA2 && c <= 0xC1) return (char)(c - 0x21); // а-я -> А-Я
      return ch;
    }

    // The grid only has room for one punctuation mark per cell, so each
    // of these is paired with a related mark that shares its cell
    // rather than eating a whole extra key: '.'/':' (both read as a
    // full stop/pause), ','/';' (both clause separators), '?'/'!' (both
    // terminal/emphasis marks), '-'/'+' (both mid-word/number
    // connectors), '/'/'\' (forward/backward slash - same key pairing
    // most real keyboards use for these two), '@'/'#' (both address/tag
    // markers). Returns 0 for any key with no pairing (letters, digits,
    // meta keys) - callers treat that as "no alternate available".
    // Reachable via the encoder chord (press-and-turn, menu_encoder_
    // chord_rotate() - same gesture letters use for a one-shot capital)
    // or a plain hold past MSNGR_KB_ALT_HOLD_MS (msngr_kb_alt_hold_try())
    // - not the ordinary Shift toggle, which has no effect on any of
    // these keys either way (none of them have an "uppercase" form, so
    // msngr_kb_apply_shift() already leaves them unchanged).
    //
    // A hold-alt was tried here for one of the Cyrillic letters missing
    // from the RU grid too ('ш' -> 'ж') before MSNGR_KB_LAYOUT_RU's top
    // row was freed up to just hold all of them directly as real keys -
    // reverted once that turned out to fit. msngr_kb_alt_hold_try()
    // still runs whatever this returns through msngr_kb_apply_shift(),
    // so a future letter-alt would still come out correctly cased for
    // free if one ever gets added back.
    char msngr_kb_alt_pair(char ch) {
      if (ch == '.') return ':';
      if (ch == ',') return ';';
      if (ch == '?') return '!';
      if (ch == '-') return '+';
      if (ch == '/') return '\\';
      if (ch == '@') return '#';
      return 0;
    }

    // Expands msngr_text_entry_buf's internal single-byte Cyrillic codes
    // into real 2-byte UTF-8 (Fonts/Org_01.h's byte-per-glyph remap is a
    // device-local rendering shorthand - Adafruit_GFX can only index
    // glyphs by a single byte, see its own header comment - so it can
    // never leave the device as-is). Called right before the buffer's
    // contents go anywhere else: LXMF send, display-name save. ASCII
    // passes through unchanged. out_cap must leave room for the worst
    // case (every byte a 2-byte Cyrillic code) plus the NUL.
    void msngr_kb_expand_utf8(const char *in, char *out, size_t out_cap) {
      size_t oi = 0;
      for (size_t i = 0; in[i] != 0 && oi + 3 < out_cap; i++) {
        unsigned char b = (unsigned char)in[i];
        uint16_t cp;
        if      (b == 0x80)                cp = 0x0401;             // Ё
        else if (b >= 0x81 && b <= 0xA0)    cp = 0x0410 + (b-0x81);  // А-Я
        else if (b == 0xA1)                 cp = 0x0451;             // ё
        else if (b >= 0xA2 && b <= 0xC1)    cp = 0x0430 + (b-0xA2);  // а-я
        else { out[oi++] = (char)b; continue; }
        out[oi++] = (char)(0xC0 | (cp >> 6));
        out[oi++] = (char)(0x80 | (cp & 0x3F));
      }
      out[oi] = 0;
    }

    // Byte length msngr_kb_expand_utf8() would produce for in, without
    // materializing the expanded string - drives the header's char
    // counter (draw_menu_msngr_keyboard_disp()), which shows actual
    // wire/payload bytes rather than internal character count. That's
    // the number that matters over LoRa airtime, and the one that'll
    // actually get compared against a recipient's own limits - internal
    // character count is just an implementation detail of this device's
    // typing buffer. Same byte-range test as msngr_kb_expand_utf8(): any
    // byte in the internal Cyrillic range (0x80-0xC1, see Fonts/Org_01.h)
    // costs 2 UTF-8 bytes, everything else (ASCII) costs 1.
    size_t msngr_kb_utf8_len(const char *in) {
      size_t n = 0;
      for (size_t i = 0; in[i] != 0; i++) {
        unsigned char b = (unsigned char)in[i];
        n += (b >= 0x80 && b <= 0xC1) ? 2 : 1;
      }
      return n;
    }
  #endif
  #endif

  // Radio submenu (MENU_STATE_URNS_RADIO_LIST/EDIT) - Frequency/Bandwidth/
  // SF/CR/TX Power. NOT gated on HAS_URNS - backed by the same EEPROM
  // fields/functions a connected host's CMD_FREQUENCY/etc + CMD_CONF_SAVE
  // already use (lora_freq/lora_bw/lora_sf/lora_cr/lora_txp,
  // eeprom_conf_save(), Utilities.h), which work independently of the
  // onboard URNS node - see project plan for the Radio menu.
  #define URNS_RADIO_ITEM_FREQ       0
  #define URNS_RADIO_ITEM_BW         1
  #define URNS_RADIO_ITEM_SF         2
  #define URNS_RADIO_ITEM_CR         3
  #define URNS_RADIO_ITEM_TXP        4
  #define URNS_RADIO_ITEM_AUTO_START 5
  #define URNS_RADIO_ITEM_START      6
  #define URNS_RADIO_ITEM_CLEAR      7
  #define URNS_RADIO_ITEM_BACK       8
  #define URNS_RADIO_ITEM_COUNT      9

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

    // Read-only percentage, same battery_percent/battery_ready (Config.h,
    // Power.h::measure_battery()) as HW_ITEM_BATTERY below, but broader -
    // HAS_BATTERY_DIVIDER only covers resistor-divider boards, while PMU
    // boards (T-Beam family etc) compute battery_percent too, just via a
    // fuel gauge instead. No drilldown/edit state, same "plain display row"
    // shape as HW_ITEM_UPTIME below, not HW_ITEM_BATTERY's MENU_STATE_HW_EDIT.
    // Ordered ahead of Battery Voltage (percentage read first, raw voltage
    // as the detail right after it) per user request.
    #if HAS_BATTERY_DIVIDER == true || HAS_PMU == true
      #define HW_ITEM_BATTERY_LEVEL HW_NEXT_A
      #define HW_NEXT_A2            (HW_NEXT_A + 1)
    #else
      #define HW_NEXT_A2 HW_NEXT_A
    #endif

    #if HAS_BATTERY_DIVIDER == true
      #define HW_ITEM_BATTERY HW_NEXT_A2
      #define HW_NEXT_A2B     (HW_NEXT_A2 + 1)
    #else
      #define HW_NEXT_A2B HW_NEXT_A2
    #endif

    // No GPS chip-identification item here (removed - redundant with GNSS
    // chip identity, which only lives on the Diagnostics page now for
    // boards that have one: GNSS_DIAG_ITEM_MODULE. The main GNSS page no
    // longer shows chip identity at all, on any board.)
    #define HW_NEXT_A3 HW_NEXT_A2B

    #if HAS_WIFI == true
      #define HW_ITEM_WIFI_IP  HW_NEXT_A3
      #define HW_ITEM_WIFI_NM  (HW_NEXT_A3 + 1)
      #define HW_ITEM_WIFI_MAC (HW_NEXT_A3 + 2)
      #define HW_NEXT_B        (HW_NEXT_A3 + 3)
    #else
      #define HW_NEXT_B HW_NEXT_A3
    #endif

    // No BT MAC row here (removed per user request) - it duplicated the
    // Bluetooth submenu's own read-only MAC row (BT_ITEM_MAC, below).
    #define HW_NEXT_C HW_NEXT_B

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

    // Heap (MCU_ESP32 also gets PSRAM) diagnostics - not tied to any
    // particular board's wiring, so it shows on every board that reaches
    // the Hardware page at all, on both MCU families: ESP.getFreeHeap()/
    // getPsramSize()/etc (Arduino-ESP32) or dbgHeapTotal()/dbgHeapFree()
    // (Adafruit/Heltec nRF52 core, cores/nRF5/utility/debug.h - always
    // linked, no separate include needed). psramFound() is checked at
    // runtime (not a compile-time PSRAM-enabled guard) so MCU_ESP32 boards
    // without PSRAM wired/enabled just show "N/A" instead of needing their
    // own #if branch here; MCU_NRF52 has no PSRAM concept at all, so its
    // own Memory submenu (MEM_ITEM_*, below) only ever has a Heap row.
    #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
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

    #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
      // Every row is drawn as a bar graph, not text - selecting one (not
      // BACK) drops into MENU_STATE_MEM_DETAIL, a plain text readout of
      // that metric, same "list row opens a submenu" pattern as
      // HW_ITEM_GPIO -> MENU_STATE_GPIO_LIST. MCU_NRF52 has no PSRAM
      // concept at all, so it only ever gets Heap.
      #define MEM_ITEM_HEAP  0
      #if MCU_VARIANT == MCU_ESP32
        #define MEM_ITEM_PSRAM 1
        #define MEM_ITEM_BACK  2
      #else
        #define MEM_ITEM_BACK  1
      #endif
      #define MEM_ITEM_COUNT (MEM_ITEM_BACK + 1)

      // Detail screen - fully read-only, only BACK does anything on
      // confirm. Which metric (Heap vs PSRAM) it's showing is tracked by
      // mem_menu_cursor staying at MEM_ITEM_HEAP/MEM_ITEM_PSRAM while this
      // state is active, same reuse-the-parent-cursor pattern HW_EDIT uses
      // for hw_menu_cursor (HW_ITEM_VOLTAGE vs HW_ITEM_BATTERY). No Min
      // Free row on MCU_NRF52 - newlib's mallinfo() (what dbgHeapUsed(),
      // Adafruit/Heltec core, is built on) has no historical low-water-mark
      // equivalent to ESP.getMinFreeHeap(), and this doesn't track its own
      // running minimum rather than fabricate a number.
      #define MEM_DETAIL_ITEM_TOTAL   0
      #define MEM_DETAIL_ITEM_USED    1
      #define MEM_DETAIL_ITEM_FREE    2
      #if MCU_VARIANT == MCU_ESP32
        #define MEM_DETAIL_ITEM_MINFREE 3
        #define MEM_DETAIL_ITEM_BACK    4
      #else
        #define MEM_DETAIL_ITEM_BACK    3
      #endif
      #define MEM_DETAIL_ITEM_COUNT (MEM_DETAIL_ITEM_BACK + 1)
    #endif
  #endif

  uint8_t menu_state      = MENU_STATE_CLOSED;
  uint8_t menu_cursor     = 0;
  uint8_t menu_edit_field = 0;

  // Main-button double-tap detection for menu_button_press(): a short tap
  // is held pending for MENU_BTN_DOUBLE_TAP_WINDOW ms in case a second one
  // follows (see menu_button_process(), polled from loop()) - if it does,
  // the pair is treated as "go backward" instead of two forward steps.
  // Loosened from 200 to 300 (per user feedback - navigation felt too tight
  // to land a double-tap reliably) - now a bit more forgiving than
  // MeshCore's own comparable MULTI_CLICK_WINDOW_MS (280ms).
  #define MENU_BTN_DOUBLE_TAP_WINDOW 300
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
    #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_WTRACKER_V2 || BOARD_MODEL == BOARD_HELTEC_T1 || BOARD_MODEL == BOARD_HELTEC_T114
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
    // Scaled 2x on T114 only - its panel's much higher pixel density
    // (135x240 vs T096/WTRACKER_V2/T1's 80x160/160x80-class panels) made
    // the plain 1x box look tiny relative to the rest of the UI. Every
    // other board keeps the original 1x sizing.
    #if BOARD_MODEL == BOARD_HELTEC_T114
      #define MENU_STATUS_RECT_SCALE 2
    #else
      #define MENU_STATUS_RECT_SCALE 1
    #endif
    void draw_menu_status_rect(const char *text) {
      #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_WTRACKER_V2 || BOARD_MODEL == BOARD_HELTEC_T1 || BOARD_MODEL == BOARD_HELTEC_T114
        // Only ever reached today via draw_button_hold_overlay() (menu
        // closed) - the menu-open popup path (Sync NTP, Clear Static)
        // needs HAS_WIFI/HAS_ETHERNET, which neither board has. Draws
        // into its own small canvas (menu_popup_canvas, Display.h, kept
        // within drawBitmap()'s region-cache cutoff) rather than
        // menu_canvas - that canvas is sized and positioned for the
        // list-screen case only, not this overlay.
        menu_popup_canvas.setFont(SMALL_FONT);
        menu_popup_canvas.setTextWrap(false);
        // getTextBounds()/print() below both read the current text size,
        // so this single setTextSize() call is what scales tw/th, the
        // glyphs themselves, and (via pad_l/box_h below) the whole box -
        // no separate "doubled" font asset needed.
        menu_popup_canvas.setTextSize(MENU_STATUS_RECT_SCALE);
        menu_popup_canvas.setTextColor(SSD1306_WHITE);

        int16_t x1, y1; uint16_t tw, th;
        menu_popup_canvas.getTextBounds(text, 0, 0, &x1, &y1, &tw, &th);

        const uint16_t pad_l = 3 * MENU_STATUS_RECT_SCALE;
        const uint16_t pad_r = 3 * MENU_STATUS_RECT_SCALE;
        uint16_t box_w = tw + pad_l + pad_r;
        if (box_w > MENU_POPUP_CANVAS_W) box_w = MENU_POPUP_CANVAS_W;
        const uint16_t box_h = 11 * MENU_STATUS_RECT_SCALE;
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
        menu_popup_canvas.setCursor(pad_l - x1, 7 * MENU_STATUS_RECT_SCALE);
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
    #define BUTTON_HOLD_TIER_MESSENGER  5

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
      #if HAS_LXMF == true
        // A shorter, dedicated tier for the emergency Messenger app
        // (Messenger.h) - sits below Settings' own 3s threshold so it
        // doesn't steal that gesture, but above every board's existing
        // "any duration up to 3s" short-click action (BLE toggle etc.,
        // button_event()) so it's still a deliberate hold, not a tap.
        if (held_ms > 1500) return BUTTON_HOLD_TIER_MESSENGER;
      #endif
      return BUTTON_HOLD_TIER_NONE;
    }

    const char *button_hold_tier_text(uint8_t tier) {
      if      (tier == BUTTON_HOLD_TIER_SLEEP)      return "SLEEP";
      else if (tier == BUTTON_HOLD_TIER_SETTINGS)   return "SETTINGS";
      else if (tier == BUTTON_HOLD_TIER_BT_PAIRING) return "BT PAIRING";
      else if (tier == BUTTON_HOLD_TIER_CONSOLE)    return "CONSOLE";
      else if (tier == BUTTON_HOLD_TIER_MESSENGER)  return "MESSENGER";
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
        #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_WTRACKER_V2 || BOARD_MODEL == BOARD_HELTEC_T1 || BOARD_MODEL == BOARD_HELTEC_T114
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
        #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_WTRACKER_V2 || BOARD_MODEL == BOARD_HELTEC_T1 || BOARD_MODEL == BOARD_HELTEC_T114
          menu_status_rect_clear();
        #endif
      }
    }
  #endif

  // Used to be gated behind HAS_WIFI || HAS_ETHERNET || (HAS_GPS && HAS_RTC)
  // - back when the only consumers were WiFi/Ethernet's own Clear Static
  // and Sync GPS (RTC.h/rtc_sync_gps(), which only needs this where HAS_RTC
  // is also true). Now unconditional: the Radio submenu's Start/Stop
  // Radio and Clear Settings actions (menu_confirm_select()'s
  // MENU_STATE_URNS_RADIO_LIST branch) use menu_open_popup() too, and that
  // submenu is present on every HAS_MENU board regardless of WiFi/
  // Ethernet/GPS/RTC - T114 (HAS_GPS, no HAS_RTC, no WiFi/Ethernet) hit a
  // real "ACTION_POPUP_MS not declared" build failure from exactly this
  // gap before this was widened.

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
      // draw_menu_status_rect() already pushes directly to hardware on
      // T096/T114/WTRACKER_V2-style displays (no separate framebuffer to
      // flush) - display.display() only exists on the SSD1306 fallback
      // path (DISPLAY_IS_OLED, Display.h), which is the only one that
      // needs this explicit flush. Never reached on T096 itself (no
      // HAS_WIFI there), only surfaced once WTRACKER_V2 added a real WiFi
      // TFT board to this HAS_WIFI-gated block.
      #if DISPLAY_IS_OLED
        display.display();
      #endif
    }

    // Opens the popup (or updates it if already open) showing `text`,
    // returning to `return_state` once the user dismisses it.
    void menu_open_popup(const char *text, uint8_t return_state) {
      menu_popup_return_state = return_state;
      menu_state = MENU_STATE_STATUS_POPUP;
      // Fresh session - don't erase a footprint left over from wherever
      // the box happened to be the last time the popup was used. Variable
      // name matches whichever declaration draw_menu_status_rect() above
      // actually compiled (panel-coordinate tracking on T096/T114/
      // WTRACKER_V2, canvas-local elsewhere - never reached on T096 itself
      // since it has no HAS_WIFI, only surfaced once WTRACKER_V2 added a
      // real WiFi TFT board to this HAS_WIFI-gated block).
      #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_WTRACKER_V2 || BOARD_MODEL == BOARD_HELTEC_T1 || BOARD_MODEL == BOARD_HELTEC_T114
        menu_popup_prev_panel_valid = false;
      #else
        menu_popup_prev_valid = false;
      #endif
      menu_popup_auto_dismiss_at = 0; // no auto-dismiss unless armed separately, see menu_draw_popup_timed()
      menu_draw_popup(text);
    }

    // Per user request, plain confirmation/result banners auto-dismiss
    // after this long, regardless of outcome - Clear Static IP's "CLEARED"
    // (WiFi/Ethernet), Messenger's "CLEARED"/"DELETED", and the Messenger
    // send-result banner (SENT/NOT READY/UNKNOWN DEST/ERROR,
    // urns_lxmf_send_result_text()). Deliberately not applied to other
    // error/failure popups ("NOT READY" on Announce Node, "UPDATE
    // FAILED", ...), which stay up until dismissed - see
    // menu_draw_popup_timed()'s own comment for that convention.
    #define ACTION_POPUP_MS 5000

    // Popup dismissal on a button press doesn't need the usual short-tap
    // dead zone (MENU_BTN_DOUBLE_TAP_WINDOW-adjacent 200ms, sized to leave
    // room for double-tap disambiguation) - there's no double-tap behavior
    // on this screen to protect against, just genuine input vs. debounce
    // noise (already filtered at the hardware level, button_debounce_delay,
    // Input.h, 25ms). Per user feedback: reusing the 200ms threshold here
    // made a quick tap feel like it needed a deliberate half-second hold to
    // register. This is comfortably above the hardware debounce without
    // requiring a real hold.
    #define MENU_POPUP_DISMISS_MIN_MS 50

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

  #if HAS_NP == true
    uint8_t staged_np_brightness = 0;   // 0-255, scales existing NeoPixel colors
    uint8_t live_np_brightness   = 0;   // same live-preview-baseline role as live_display_brightness
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

  // Radio submenu (top-level item, sits right under URNS on boards that
  // have it) - stages the same lora_freq/lora_bw/lora_sf/lora_cr/lora_txp
  // globals a connected host's CMD_FREQUENCY/etc already read/write, and
  // commits via the same eeprom_conf_save()/eeprom_have_conf() (Utilities.h)
  // a host's CMD_CONF_SAVE already uses. NOT gated on HAS_URNS - see
  // project plan for the Radio menu for why.
  uint8_t urns_radio_menu_cursor = 0;
  uint32_t staged_lora_freq = 0;
  uint32_t staged_lora_bw   = 0;
  uint8_t  staged_lora_sf   = 0;
  uint8_t  staged_lora_cr   = 0;
  uint8_t  staged_lora_txp  = 0;
  bool staged_radio_auto_start_enabled = true;

  #if HAS_URNS == true
    uint8_t urns_menu_cursor = 0;
    bool staged_urns_enabled = true;
    bool staged_urns_transport_enabled = false;
    bool staged_urns_link_mtu_discovery = true;
    bool staged_urns_remote_mgmt_enabled = true;
    bool staged_urns_probe_dest_enabled = false;
    #if HAS_ESPNOW == true
      uint8_t staged_urns_interface_mode = URNS_INTERFACE_LORA_ONLY;
    #endif

    uint8_t urns_paths_menu_cursor = 0;
    uint8_t urns_path_detail_cursor = 0;
    uint8_t urns_free_detail_cursor = URNS_FREE_DETAIL_ITEM_BACK;
    // Cached breakdown, computed once by urns_free_detail_refresh() when
    // MENU_STATE_URNS_FREE_DETAIL is entered - see URNS_ITEM_FREE's own
    // comment for why this can't be recomputed on every draw call.
    size_t urns_free_detail_identity = 0;
    size_t urns_free_detail_announce = 0;
    size_t urns_free_detail_paths    = 0;
    size_t urns_free_detail_messages = 0;
    size_t urns_free_detail_other    = 0;

    void urns_free_detail_refresh() {
      size_t used_b = (size_t)LittleFS.usedBytes();
      urns_free_detail_identity = urns_dir_size_recursive(URNS_IDENTITY_PATH);
      urns_free_detail_announce = urns_dir_size_recursive(URNS_KNOWN_STORE_PATH);
      urns_free_detail_paths    = urns_dir_size_recursive(URNS_PATH_STORE_PATH);
      urns_free_detail_messages = urns_dir_size_recursive(URNS_MESSAGES_PATH);
      size_t accounted = urns_free_detail_identity + urns_free_detail_announce
                        + urns_free_detail_paths + urns_free_detail_messages;
      urns_free_detail_other = (used_b > accounted) ? (used_b - accounted) : 0;
    }
    // Captured (full 16-byte hash, not the 8-hex-char truncated label) when
    // a path row is clicked in MENU_STATE_URNS_PATHS - MENU_STATE_URNS_PATH_
    // DETAIL looks this back up via new_path_table().get() on every draw
    // call rather than caching the DestinationEntry itself, same "always
    // show live state" convention as the list it was opened from.
    RNS::Bytes urns_path_detail_hash;

    // Row count for MENU_STATE_URNS_PATHS: path entries (capped at
    // MENU_URNS_PATH_MAX_ROWS) plus one BACK row, or a single inert
    // "No Paths" row plus BACK when the table's empty - same "always at
    // least one selectable/inert row" shape as ESP-NOW LR's info row.
    // Used identically by both the turn-handler (cursor clamping) and the
    // draw function (row building), so it's one shared source of truth.
    // RNS::Transport::path_table() (a plain std::map, Persistence::PathTable)
    // reads its own local insert call commented out in microReticulum's
    // Transport.cpp - real announce processing inserts into
    // RNS::Transport::new_path_table() instead (a microStore-backed
    // TypedStore, Persistence::NewPathTable), which is the one that's
    // actually live. See project_microreticulum_onboard_node memory - this
    // is what "Path Table reads 0" traced back to, alongside a second real
    // bug in the same store's own init() (fixed in Transport.cpp, same
    // transport_enabled-gate/relative-path pattern already fixed for
    // Identity::_known_store).
    uint8_t urns_path_display_row_count() {
      size_t n = RNS::Transport::new_path_table().size();
      if (n > MENU_URNS_PATH_MAX_ROWS) n = MENU_URNS_PATH_MAX_ROWS;
      if (n == 0) return 2; // "No Paths" + BACK
      return (uint8_t)(n + 1); // paths + BACK
    }

  #if HAS_LXMF == true
    // Messenger app (Messenger.h) - MENU_STATE_MSNGR_* cursor/context state.
    uint8_t msngr_menu_cursor = 0;
    uint8_t msngr_inbox_cursor = 0;
    uint8_t msngr_bookmarks_cursor = 0;
    uint8_t msngr_announces_cursor = 0;
    uint8_t msngr_peer_cursor = 0;
    uint8_t msngr_msg_detail_cursor = 0;
    uint8_t msngr_settings_cursor = 0;
    uint8_t msngr_presets_cursor = 0;
    // Which msngr_presets[] slot MENU_STATE_MSNGR_PRESET_DETAIL is
    // showing Edit/Delete/BACK for - set when a preset row is selected
    // from MENU_STATE_MSNGR_PRESETS, same "remember which one" role
    // msngr_active_message_hash plays for MENU_STATE_MSNGR_MSG_DETAIL.
    uint8_t msngr_preset_detail_index = 0;
    uint8_t msngr_preset_detail_cursor = 0;

    // MENU_STATE_MSNGR_SETTINGS/_EDIT - staged, no-write-until-confirmed
    // values, same "staged, commit on exit" shape as
    // staged_bt_legacy_pairing_enabled etc below. Seeded from the live
    // msngr_* globals (Messenger.h) in menu_stage_from_live().
    uint8_t staged_msngr_max_retries = MSNGR_MAX_RETRIES_DEFAULT;
    uint8_t staged_msngr_retry_delay_s = MSNGR_RETRY_DELAY_DEFAULT;
    bool staged_msngr_announce_at_start = true;
    uint8_t staged_msngr_announce_interval_idx = 0;

    // Which peer MENU_STATE_MSNGR_PEER/MSG_DETAIL are currently showing -
    // set whenever a row is confirmed in Inbox/Bookmarks/Announces (or a
    // message row within MSNGR_PEER itself). Which of those three lists
    // to return to on MSNGR_PEER's own BACK is tracked separately, since
    // all three can lead here.
    RNS::Bytes msngr_active_peer_hash;
    uint8_t msngr_peer_return_state = MENU_STATE_MSNGR_LIST;
    RNS::Bytes msngr_active_message_hash;
    uint8_t msngr_last_send_result = 0xFF; // 0xFF = nothing sent this visit to MSNGR_PEER

    // MENU_STATE_MSNGR_DELETE_CONFIRM / MENU_STATE_MSNGR_CLEAR_CONFIRM -
    // same "default to CANCEL" pattern as fwupd_confirm_cursor above (0 =
    // DELETE/CLEAR, 1 = CANCEL).
    uint8_t msngr_delete_confirm_cursor = 1;
    uint8_t msngr_clear_confirm_cursor = 1;

    // MENU_STATE_MSNGR_TEXT_ENTRY - linear (row-major) cursor into
    // MSNGR_KB_LAYOUT, a persistent Shift toggle (caps-lock style, not
    // meshtastic's one-shot long-press), and the message being composed.
    // Reset (cursor to 0, shift off, buffer cleared) every time the screen
    // is opened fresh from MSNGR_PEER_FIXED_ACTION_SEND_CUSTOM.
    uint8_t msngr_kb_cursor = 0;
    bool msngr_kb_shift_on = false;
    // Latin/Cyrillic layout toggle - lives on the same Shift key as case
    // (see MSNGR_KB_ALT_HOLD_MS), not a separate grid cell (the grid is
    // already full - 4x11 with no spare slot). Reset alongside cursor/shift
    // every time the screen is opened fresh, same as those.
    bool msngr_kb_lang_ru = false;
    char msngr_text_entry_buf[MSNGR_TEXT_ENTRY_MAX_LEN + 1] = {0};

    // The EN/RU switch fires live, the instant a Shift hold crosses
    // MSNGR_KB_ALT_HOLD_MS, rather than waiting for release - polled
    // every loop() tick (menu_button_process() below for the main
    // button, Encoder.h's encoder_process() for the encoder's own
    // button) via msngr_kb_lang_hold_try(), same "fire once while still
    // held" shape as Encoder.h's own enc_btn_hold_beeped. Separate flags
    // per control (rather than one shared flag) because Menu.h and
    // Encoder.h are two different debounce state machines that can't
    // see each other's press-state - menu_confirm_select()'s Shift
    // branch checks/consumes both at release, so whichever control the
    // switch actually happened on doesn't matter there.
    bool msngr_kb_lang_hold_fired_btn = false;
    bool msngr_kb_lang_hold_fired_enc = false;

    // Shared by both pollers above - if held_ms has crossed the
    // threshold, the current hold hasn't already fired this switch, and
    // the grid position it's being measured against is actually the
    // Shift key right now (cursor doesn't move during a plain hold, so
    // this stays true for the whole gesture once checked), performs the
    // switch and latches fired_flag so menu_confirm_select() knows to
    // skip its own release-time toggle. No-ops instead of switching for
    // any other key - holding a letter key isn't a gesture this screen
    // gives meaning to, so it's left alone.
    void msngr_kb_lang_hold_try(unsigned long held_ms, bool &fired_flag) {
      if (menu_state != MENU_STATE_MSNGR_TEXT_ENTRY || fired_flag || held_ms < MSNGR_KB_ALT_HOLD_MS) return;
      uint8_t kb_row = msngr_kb_cursor / MSNGR_KB_COLS;
      uint8_t kb_col = msngr_kb_cursor % MSNGR_KB_COLS;
      char key_ch = (msngr_kb_lang_ru ? MSNGR_KB_LAYOUT_RU : MSNGR_KB_LAYOUT)[kb_row][kb_col];
      if (msngr_kb_key_type(key_ch) != MSNGR_KB_SHIFT) return;
      msngr_kb_lang_ru = !msngr_kb_lang_ru;
      msngr_kb_shift_on = false;
      buzzer_encoder_tick_melody();
      fired_flag = true;
    }

    // MENU_STATE_MSNGR_TEXT_ENTRY is reused for editing the LXMF display
    // name (RNode Settings > Messenger > Settings > Display Name), adding/
    // editing a preset message (RNode Settings > Messenger > Settings >
    // Preset Messages), and composing a message - this tracks which,
    // since the Send key's actual action and the exit/discard target
    // differ. All the actual typing mechanics (grid cursor/shift/buffer
    // above) are identical regardless of purpose, only these three
    // branch on it.
    #define MSNGR_TEXT_ENTRY_PURPOSE_MESSAGE      0
    #define MSNGR_TEXT_ENTRY_PURPOSE_DISPLAY_NAME 1
    #define MSNGR_TEXT_ENTRY_PURPOSE_PRESET        2
    uint8_t msngr_text_entry_purpose = MSNGR_TEXT_ENTRY_PURPOSE_MESSAGE;
    // Which msngr_presets[] slot PURPOSE_PRESET is editing -
    // MSNGR_MAX_PRESETS itself (one past the last real slot) is the
    // sentinel for "adding a new preset" rather than editing an existing
    // one, same "index == count means append" convention
    // messenger_preset_add() itself uses internally.
    uint8_t msngr_preset_edit_index = 0;

    // Same live-while-held shape as msngr_kb_lang_hold_fired_btn/_enc
    // above, for the punctuation-pair gesture (msngr_kb_alt_pair()) on
    // '.'/','/'?' instead of the language switch on Shift - separate
    // flags because a hold can only ever be doing one or the other
    // (whichever key is actually highlighted), but menu_confirm_select()
    // still needs to know which, if either, to skip at release.
    bool msngr_kb_alt_hold_fired_btn = false;
    bool msngr_kb_alt_hold_fired_enc = false;

    // Same "fire once while held" shape as msngr_kb_lang_hold_try(), for
    // the highlighted key's paired punctuation mark instead of a
    // language switch - inserts it directly (respecting the same
    // length cap the normal MSNGR_KB_CHAR path in menu_confirm_select()
    // enforces; also run through msngr_kb_apply_shift() same as any
    // other insert, currently a no-op for punctuation but keeps this
    // correct for free if a letter-alt ever gets paired here again -
    // see msngr_kb_alt_pair()'s own comment) rather than just flipping
    // a mode, since there's nothing else for a hold on a plain
    // character key to *do*. No-ops for any key with no pairing
    // (msngr_kb_alt_pair() returns 0) - letters, digits, space, and
    // every meta key are left entirely to their own existing hold
    // behavior (Shift's language switch, BACK, etc).
    void msngr_kb_alt_hold_try(unsigned long held_ms, bool &fired_flag) {
      if (menu_state != MENU_STATE_MSNGR_TEXT_ENTRY || fired_flag || held_ms < MSNGR_KB_ALT_HOLD_MS) return;
      uint8_t kb_row = msngr_kb_cursor / MSNGR_KB_COLS;
      uint8_t kb_col = msngr_kb_cursor % MSNGR_KB_COLS;
      char key_ch = (msngr_kb_lang_ru ? MSNGR_KB_LAYOUT_RU : MSNGR_KB_LAYOUT)[kb_row][kb_col];
      char alt = msngr_kb_alt_pair(key_ch);
      if (alt == 0) return;
      alt = msngr_kb_apply_shift(alt, msngr_kb_shift_on);
      size_t max_len = (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_DISPLAY_NAME ||
                         msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_PRESET)
        ? MSNGR_NAME_MAX_LEN : MSNGR_TEXT_ENTRY_MAX_LEN;
      size_t text_len = strlen(msngr_text_entry_buf);
      if (text_len < max_len) {
        msngr_text_entry_buf[text_len] = alt;
        msngr_text_entry_buf[text_len + 1] = 0;
        buzzer_encoder_tick_melody();
      }
      fired_flag = true;
    }

    // Hold-to-repeat threshold/rate for DEL - shorter than MSNGR_KB_ALT_
    // HOLD_MS on purpose: unlike Shift/punctuation (which do something
    // *different* on a hold, so need enough delay to not misfire during
    // an ordinary confirm), a held DEL doing the exact same thing it'd
    // do anyway, just repeatedly, is safe to start almost immediately -
    // 500ms is long enough that a normal single backspace tap/click
    // never reaches it. Repeat interval is a plain judgement call
    // (~8/sec) - fast enough to actually clear text, slow enough to
    // still feel countable/controllable one character at a time.
    #define MSNGR_KB_DEL_REPEAT_START_MS    500
    #define MSNGR_KB_DEL_REPEAT_INTERVAL_MS 120

    // Same per-control fired-flag pattern as msngr_kb_lang_hold_fired_*/
    // msngr_kb_alt_hold_fired_* above, plus a per-control "when did
    // this hold's most recent repeat happen" timestamp (0 = hasn't
    // repeated yet this hold) to pace repeats at MSNGR_KB_DEL_REPEAT_
    // INTERVAL_MS apart instead of firing every single loop() tick.
    bool msngr_kb_del_hold_fired_btn = false;
    bool msngr_kb_del_hold_fired_enc = false;
    unsigned long msngr_kb_del_repeat_last_btn = 0;
    unsigned long msngr_kb_del_repeat_last_enc = 0;

    // Same "fire (repeatedly) while held" shape as msngr_kb_lang_hold_
    // try()/msngr_kb_alt_hold_try(), except this one keeps firing at
    // MSNGR_KB_DEL_REPEAT_INTERVAL_MS apart for as long as the hold
    // continues past the start threshold, instead of just once. The
    // very first repeat fires the moment held_ms crosses the start
    // threshold (last_repeat_ms is still 0 then, so the interval check
    // is skipped) - deletes exactly like a normal DEL tap would, just
    // triggered early instead of waiting for release.
    void msngr_kb_del_hold_try(unsigned long held_ms, bool &fired_flag, unsigned long &last_repeat_ms) {
      if (menu_state != MENU_STATE_MSNGR_TEXT_ENTRY || held_ms < MSNGR_KB_DEL_REPEAT_START_MS) return;
      uint8_t kb_row = msngr_kb_cursor / MSNGR_KB_COLS;
      uint8_t kb_col = msngr_kb_cursor % MSNGR_KB_COLS;
      char key_ch = (msngr_kb_lang_ru ? MSNGR_KB_LAYOUT_RU : MSNGR_KB_LAYOUT)[kb_row][kb_col];
      if (msngr_kb_key_type(key_ch) != MSNGR_KB_BACKSPACE) return;
      unsigned long now = millis();
      if (last_repeat_ms != 0 && now - last_repeat_ms < MSNGR_KB_DEL_REPEAT_INTERVAL_MS) return;
      size_t text_len = strlen(msngr_text_entry_buf);
      if (text_len > 0) {
        msngr_text_entry_buf[text_len - 1] = 0;
        buzzer_encoder_tick_melody();
      }
      last_repeat_ms = now;
      fired_flag = true;
    }

    // MENU_STATE_MSNGR_DISCARD_CONFIRM - same "default to CANCEL" pattern
    // as msngr_delete_confirm_cursor/msngr_clear_confirm_cursor above
    // (0 = DISCARD, 1 = CANCEL).
    uint8_t msngr_discard_confirm_cursor = 1;

    // Where leaving MENU_STATE_MSNGR_TEXT_ENTRY (Send/BACK/discard alike)
    // lands, based on why it was opened - shared by menu_msngr_text_
    // entry_leave() below and MENU_STATE_MSNGR_DISCARD_CONFIRM's own
    // DISCARD branch (menu_confirm_select()), which used to duplicate
    // this same two-way ternary rather than call it.
    uint8_t msngr_text_entry_return_state() {
      if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_DISPLAY_NAME) return MENU_STATE_MSNGR_SETTINGS;
      if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_PRESET) return MENU_STATE_MSNGR_PRESETS;
      return MENU_STATE_MSNGR_PEER;
    }

    // Shared exit path for leaving MENU_STATE_MSNGR_TEXT_ENTRY without
    // sending - both the on-grid BACK key (msngr_kb_key_type() dispatch,
    // menu_confirm_select()) and the encoder's own long-press-to-leave
    // (menu_encoder_button(), 3s threshold there instead of the usual
    // 700ms) route through this, so an accidental hold and a deliberate
    // BACK press protect a half-typed message the same way. Skips
    // straight back to the peer screen if nothing's been typed; otherwise
    // opens a DISCARD/CANCEL confirmation instead of silently losing it.
    void menu_msngr_text_entry_leave() {
      uint8_t return_state = msngr_text_entry_return_state();
      if (strlen(msngr_text_entry_buf) == 0) {
        menu_state = return_state;
      } else {
        msngr_discard_confirm_cursor = 1; // default CANCEL
        menu_state = MENU_STATE_MSNGR_DISCARD_CONFIRM;
      }
    }

    // MENU_STATE_MSNGR_PING_RESULT - fixed 2-row screen (status + BACK),
    // default cursor on BACK so a quick click dismisses either a result or
    // an in-flight ping. Row 0 is read-only info, same "selecting it does
    // nothing" shape as MSNGR_MSG_DETAIL's own content lines.
    uint8_t msngr_ping_result_cursor = 1;

    // MENU_STATE_MSNGR_SEND_RESULT - same fixed 2-row shape as
    // MSNGR_PING_RESULT above, default cursor on BACK.
    uint8_t msngr_send_result_cursor = 1;

    // Polled from loop() (RNode_Firmware.ino, alongside messenger_send_
    // process() itself) - auto-returns to MENU_STATE_MSNGR_PEER once a
    // terminal Delivered/No Confirmation/Unknown Destination result has
    // been shown for MSNGR_SEND_RESULT_POPUP_MS, no input needed. Also
    // consumes msngr_send_needs_cache_refresh (Messenger.h) - set whenever
    // messenger_send_lxmf_resolved() actually saves a new outgoing message,
    // whether that happened immediately (identity already known) or later,
    // asynchronously, once a RESOLVING wait completes - either way this is
    // the first point after that save where messenger_refresh_peer_cache()
    // is visible (Menu.h is #include'd after Messenger.h, same layering
    // reason this whole function lives here instead of there).
    void msngr_send_result_process() {
      if (msngr_send_needs_cache_refresh) {
        msngr_send_needs_cache_refresh = false;
        messenger_refresh_peer_cache(msngr_active_peer_hash);
      }
      if ((msngr_send_state == MSNGR_SEND_DELIVERED || msngr_send_state == MSNGR_SEND_TIMEOUT ||
           msngr_send_state == MSNGR_SEND_UNRESOLVED || msngr_send_state == MSNGR_SEND_FAILED) &&
          menu_state == MENU_STATE_MSNGR_SEND_RESULT &&
          millis() - msngr_send_result_at_ms > MSNGR_SEND_RESULT_POPUP_MS) {
        menu_state = MENU_STATE_MSNGR_PEER;
        msngr_send_state = MSNGR_SEND_IDLE;
      }
    }

    uint8_t msngr_inbox_row_count() {
      size_t n = urns_message_store ? urns_message_store->get_conversation_count() : 0;
      if (n > MENU_MSNGR_LIST_MAX_ROWS) n = MENU_MSNGR_LIST_MAX_ROWS;
      if (n == 0) return 2; // "No Messages" + BACK
      return (uint8_t)(n + 1);
    }

    uint8_t msngr_bookmarks_row_count() {
      uint8_t n = msngr_bookmark_count;
      if (n == 0) return 2; // "No Bookmarks" + BACK
      return (uint8_t)(n + 1);
    }

    // One row per configured preset, plus an "Add Preset" row (only
    // while under MSNGR_MAX_PRESETS - unlike bookmarks/announces, presets
    // have their own direct add action right in this list rather than
    // being populated from elsewhere, so there's always something
    // actionable to show even at 0 - no "No Presets" dead label needed),
    // plus BACK.
    uint8_t msngr_presets_row_count() {
      uint8_t n = msngr_preset_count;
      if (msngr_preset_count < MSNGR_MAX_PRESETS) n++; // "Add Preset"
      return (uint8_t)(n + 1); // + BACK
    }

    uint8_t msngr_announces_row_count() {
      uint8_t n = 0;
      for (uint8_t i = 0; i < MSNGR_MAX_ANNOUNCES; i++) if (msngr_announces[i].in_use) n++;
      if (n == 0) return 2; // "No Announces" + BACK
      return (uint8_t)(n + 1);
    }

    // How many message-snippet rows MENU_STATE_MSNGR_PEER shows -
    // the shared source of truth both the rotate-clamp and draw/confirm
    // handlers use, same convention as urns_path_display_row_count()
    // above. Reads msngr_peer_cache_count (Messenger.h), populated once
    // per screen-entry, rather than querying MessageStore directly here -
    // see that cache's own comment for why (this used to read flash on
    // every single call, including from the rotate handler on every
    // encoder detent, which is what actually caused the flash-cache-vs-
    // radio-ISR crash this was rewritten to fix).
    uint8_t msngr_peer_msg_row_count() {
      return msngr_peer_cache_count;
    }

    uint8_t msngr_peer_row_count() {
      return (uint8_t)(msngr_peer_msg_row_count() + msngr_preset_count + MSNGR_PEER_FIXED_ACTION_COUNT);
    }

    // Chars-per-row for MENU_STATE_MSNGR_MSG_DETAIL's word-wrap - tuned for
    // MENU_CONTENT_W/MENU_FONT's generic (non-T096/T114) 120px/Org_01
    // combination, same font every HAS_URNS board uses today.
    #define MSNGR_MSG_DETAIL_CHARS_PER_LINE 20

    // Reads msngr_msg_detail_cache_content (Messenger.h) - same "cache
    // once per screen-entry, don't re-read flash per call" reasoning as
    // msngr_peer_msg_row_count() above.
    uint8_t msngr_msg_detail_row_count() {
      if (!msngr_msg_detail_cache_valid) return 2;
      size_t len = msngr_msg_detail_cache_content.size();
      size_t lines = (len + MSNGR_MSG_DETAIL_CHARS_PER_LINE - 1) / MSNGR_MSG_DETAIL_CHARS_PER_LINE;
      if (lines == 0) lines = 1;
      if (lines > MSNGR_MSG_DETAIL_MAX_LINES) lines = MSNGR_MSG_DETAIL_MAX_LINES;
      return (uint8_t)(lines + 3); // content lines + REPLY + DELETE + BACK
    }
  #endif
  #endif
  #if HAS_ENCODER == true
    bool staged_encoder_enabled = false;
  #endif
  #if HAS_BLE == true
    #if MCU_VARIANT == MCU_ESP32
      bool staged_bt_legacy_pairing_enabled = false;
      bool staged_bt_just_works_enabled = false;
      bool staged_bt_auto_start_enabled = false;
    #endif
    bool staged_bt_battery_service_enabled = false;
    // MENU_STATE_BT_SETTINGS/_EDIT's own cursor - separate from
    // bt_menu_cursor below, which now only tracks position within
    // BT_LIST's own (smaller) item space.
    uint8_t bt_settings_cursor = 0;
  #endif
  #if HAS_BLUETOOTH == true || HAS_BLE == true
    uint8_t bt_menu_cursor = 0;
    #if HAS_BLE == true
      // 0 = FORGET, 1 = CANCEL - same pattern as fwupd_confirm_cursor,
      // defaulting to CANCEL since this wipes every stored bond, not just
      // the least-recently-used one. HAS_BLE-gated (not MCU_VARIANT) since
      // that's what actually has bt_bond_count()/bt_debond_all() - classic
      // HAS_BLUETOOTH (Bluedroid SPP) boards never reach BT_ITEM_UNPAIR.
      uint8_t bt_unpair_confirm_cursor = 1;
    #endif
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
  #endif

  #if HAS_RTC == true || HAS_GPS == true
    // Working copy while inside MENU_STATE_RTC_TZ_EDIT - quarter-hours
    // from UTC, synced fresh from the live value on entry (see
    // get_tz_offset_qh(), Utilities.h) and committed immediately on
    // confirm (tz_conf_save(), Utilities.h) - display-only, nothing to
    // reboot or re-init, same reasoning as Ethernet's Speed field. Shared
    // by both the RTC page's and the GNSS page's own Timezone row (same
    // EEPROM byte, same editor) - see tz_edit_return_state below for how
    // it knows which list to return to.
    int8_t staged_tz_offset_qh = 0;
    // Which list opened MENU_STATE_RTC_TZ_EDIT - MENU_STATE_RTC_LIST or
    // MENU_STATE_GNSS_LIST - so confirm can return to the right one.
    uint8_t tz_edit_return_state = MENU_STATE_RTC_LIST;
  #endif

  #if HAS_GPS == true
    uint8_t gnss_menu_cursor = 0;
    #if HAS_GNSS_DEBUG_MENU == true
      uint8_t gnss_diag_menu_cursor = 0;
      uint8_t gnss_diag_sats_cursor = 0;
    #endif
    // Working copy while inside MENU_STATE_GNSS_EDIT - synced fresh from
    // the live gnss_enabled value on entry (see menu_confirm_select()), not
    // staged at whole-menu-open time, since this commits immediately (and
    // live power-cycles the receiver) on confirm rather than deferring to
    // SAVE & EXIT - same immediate-commit pattern as RTC's own Timezone
    // field above.
    bool staged_gnss_enabled = true;
    #if GNSS_DUTY_CYCLE_CAPABLE == true
      // Working copy while inside MENU_STATE_GNSS_EDIT for the Update
      // Interval row - an index into gnss_update_interval_presets_s
      // (GNSS.h), same immediate-commit-on-confirm pattern as
      // staged_gnss_enabled above.
      uint8_t staged_gnss_interval_index = 0;
    #endif
  #endif

  #if HAS_SENSORS == true
    uint8_t sensors_menu_cursor = 0;
  #endif

  #if MENU_HAS_HW_PAGE == true
    uint8_t hw_menu_cursor = 0;
    #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
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
    // Backlight boards do have a real continuous range, but sweeping the
    // full 0-255 scale one detent at a time is tedious. Present it as a
    // 5-state OFF/LOW/MED/HIGH/MAX pick instead, evenly spaced across the
    // range - same approach as the OLED 3-state picker above.
    static const uint8_t backlight_brightness_values[] = { 0, 64, 128, 191, 255 };

    // Buckets any historical continuous value (e.g. loaded from EEPROM
    // before this board had the 5-state picker) into the nearest state.
    uint8_t backlight_brightness_index(uint8_t val) {
      uint8_t best = 0;
      uint16_t best_dist = 256;
      for (uint8_t i = 0; i < 5; i++) {
        uint16_t dist = abs((int16_t)val - (int16_t)backlight_brightness_values[i]);
        if (dist < best_dist) { best_dist = dist; best = i; }
      }
      return best;
    }

    void format_brightness(uint8_t val, char *buf) {
      static const char *labels[] = { "OFF", "LOW", "MED", "HIGH", "MAX" };
      sprintf(buf, "%s", labels[backlight_brightness_index(val)]);
    }

    void step_brightness(int8_t dir, bool wrap = false) {
      int8_t idx = (int8_t)backlight_brightness_index(staged_display_brightness) + (dir > 0 ? 1 : -1);
      if (wrap) {
        if (idx < 0) idx = 4;
        if (idx > 4) idx = 0;
      } else {
        if (idx < 0) idx = 0;
        if (idx > 4) idx = 4;
      }
      staged_display_brightness = backlight_brightness_values[idx];
    }
  #endif

  // Display Timeout also only offers a curated set of stops rather than a
  // raw seconds dial - same reasoning as the brightness pickers above. See
  // display_timeout_decode_seconds()/display_timeout_codes() (Display.h)
  // for how a single byte reaches durations past 255 seconds while staying
  // backward-compatible with a pre-existing raw value.
  static const char *display_timeout_labels[] = { "OFF", "5s", "10s", "15s", "30s", "1m", "2m", "3m", "5m", "10m", "15m", "30m" };

  void format_timeout(uint8_t val, char *buf) {
    sprintf(buf, "%s", display_timeout_labels[display_timeout_code_index(val)]);
  }

  void step_timeout(int8_t dir, bool wrap = false) {
    int8_t max_idx = (int8_t)DISPLAY_TIMEOUT_CODE_COUNT - 1;
    int8_t idx = (int8_t)display_timeout_code_index(staged_display_timeout) + (dir > 0 ? 1 : -1);
    if (wrap) {
      if (idx < 0) idx = max_idx;
      if (idx > max_idx) idx = 0;
    } else {
      if (idx < 0) idx = 0;
      if (idx > max_idx) idx = max_idx;
    }
    staged_display_timeout = display_timeout_codes[idx];
  }

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

    // Radio submenu steppers - edit the same staged_lora_* fields committed
    // in menu_commit_and_exit() via the existing eeprom_conf_save(). NOT
    // gated on HAS_URNS - see project plan for the Radio menu.

    // 25kHz/detent at rest, ramping with accelerated_step() - sweeping a
    // multi-hundred-MHz range 1Hz at a time isn't usable on an encoder. No
    // firmware-enforced regulatory/range bounds, same as CMD_FREQUENCY's own
    // handler (RNode_Firmware.ino) - this is a "you set what you configure"
    // field exactly as the host path already is.
    void step_urns_radio_freq(int8_t dir, bool wrap = false) {
      int64_t step = (int64_t)accelerated_step() * 25000;
      int64_t v = (int64_t)staged_lora_freq + (dir > 0 ? step : -step);
      if (wrap) {
        if (v < 0)          v = 1000000000;
        if (v > 1000000000) v = 0;
      } else {
        if (v < 0)          v = 0;
        if (v > 1000000000) v = 1000000000;
      }
      staged_lora_freq = (uint32_t)v;
    }

    // Bandwidth has no meaningful continuous range - cycle through the
    // SX126x's standard discrete values instead of raw Hz stepping, same
    // "present a real value set, not free-form" reasoning as OLED
    // step_brightness() above. CMD_BANDWIDTH's own KISS handler still
    // accepts arbitrary Hz (unchanged), this is menu-UI-only.
    void step_urns_radio_bw(int8_t dir, bool wrap = false) {
      static const uint32_t values[] = { 7800, 10400, 15600, 20800, 31250, 41700, 62500, 125000, 250000, 500000 };
      const int8_t count = sizeof(values) / sizeof(values[0]);
      int8_t idx = 0;
      for (int8_t i = 0; i < count; i++) if (values[i] == staged_lora_bw) { idx = i; break; }
      idx += (dir > 0 ? 1 : -1);
      if (wrap) {
        if (idx < 0) idx = count - 1;
        if (idx > count - 1) idx = 0;
      } else {
        if (idx < 0) idx = 0;
        if (idx > count - 1) idx = count - 1;
      }
      staged_lora_bw = values[idx];
    }

    // Bounds match CMD_SF's own handler (RNode_Firmware.ino) so the menu and
    // a connected host never disagree.
    void step_urns_radio_sf(int8_t dir, bool wrap = false) {
      int8_t v = (int8_t)staged_lora_sf + (dir > 0 ? 1 : -1);
      if (wrap) {
        if (v < 5)  v = 12;
        if (v > 12) v = 5;
      } else {
        if (v < 5)  v = 5;
        if (v > 12) v = 12;
      }
      staged_lora_sf = (uint8_t)v;
    }

    // Bounds match CMD_CR's own handler (RNode_Firmware.ino).
    void step_urns_radio_cr(int8_t dir, bool wrap = false) {
      int8_t v = (int8_t)staged_lora_cr + (dir > 0 ? 1 : -1);
      if (wrap) {
        if (v < 5) v = 8;
        if (v > 8) v = 5;
      } else {
        if (v < 5) v = 5;
        if (v > 8) v = 8;
      }
      staged_lora_cr = (uint8_t)v;
    }

    // Ceiling matches CMD_TXPOWER's own handler (RNode_Firmware.ino) via
    // lora_txp_max() (Utilities.h) - shared so the menu and a connected
    // host can't drift apart.
    void step_urns_radio_txp(int8_t dir, bool wrap = false) {
      int8_t max_txp = lora_txp_max();
      int8_t v = (int8_t)staged_lora_txp + (dir > 0 ? 1 : -1);
      if (wrap) {
        if (v < 0)       v = max_txp;
        if (v > max_txp) v = 0;
      } else {
        if (v < 0)       v = 0;
        if (v > max_txp) v = max_txp;
      }
      staged_lora_txp = (uint8_t)v;
    }

    // staged_msngr_* (below) only exists under HAS_LXMF == true (see its
    // own declaration block above) - unlike step_urns_radio_txp() etc.
    // above, this was missing the same guard until a HAS_URNS==true &&
    // HAS_LXMF==false build (heltec32v4pa_urns) actually exercised it.
    // Every other HAS_URNS board also sets HAS_LXMF true, which is why
    // this went uncaught until now.
    #if HAS_LXMF == true
      // Messenger Settings > Retries (MENU_STATE_MSNGR_SETTINGS_EDIT) - fixed
      // 0-5 range (LXMRouter::set_max_delivery_attempts(), LXMRouter.h),
      // always clamped regardless of the wrap param - unlike Frequency/SF/
      // TX Power above, there's no sensible "wrap past the end" behavior for
      // a retry count (5 wrapping to 0 would silently disable retries, the
      // opposite of what turning the encoder further clearly means).
      void step_msngr_retries(int8_t dir, bool wrap = false) {
        int8_t v = (int8_t)staged_msngr_max_retries + (dir > 0 ? 1 : -1);
        if (v < 0) v = 0;
        if (v > 5) v = 5;
        staged_msngr_max_retries = (uint8_t)v;
      }

      // Messenger Settings > Retry Delay (MENU_STATE_MSNGR_SETTINGS_EDIT) -
      // 1-60 second range, 1s steps. No natural small preset set the way
      // Auto Announce has, so a plain clamped stepper like Retries above,
      // not a preset table. Always clamped, never wraps - same reasoning as
      // Retries (wrapping 60->1 would silently make retries near-instant,
      // the opposite of what turning the encoder further past 60 means).
      void step_msngr_retry_delay(int8_t dir, bool wrap = false) {
        int8_t v = (int8_t)staged_msngr_retry_delay_s + (dir > 0 ? 1 : -1);
        if (v < 1) v = 1;
        if (v > 60) v = 60;
        staged_msngr_retry_delay_s = (uint8_t)v;
      }

      // Messenger Settings > Auto Announce (MENU_STATE_MSNGR_SETTINGS_EDIT) -
      // steps through msngr_announce_interval_presets_s's index range
      // (Messenger.h: Off/15m/30m/1h/2h/3h/6h/12h). Same "always clamped,
      // never wraps" reasoning as Retries above - wrapping from 12h back to
      // Off would silently disable auto-announce, the opposite of what
      // turning the encoder further past 12h clearly means.
      void step_msngr_announce_interval(int8_t dir, bool wrap = false) {
        int8_t v = (int8_t)staged_msngr_announce_interval_idx + (dir > 0 ? 1 : -1);
        if (v < 0) v = 0;
        if (v > MSNGR_ANNOUNCE_INTERVAL_PRESET_COUNT - 1) v = MSNGR_ANNOUNCE_INTERVAL_PRESET_COUNT - 1;
        staged_msngr_announce_interval_idx = (uint8_t)v;
      }
    #endif

    // Sane starting values for any field still at its Config.h "never
    // configured" sentinel (0/0/0/0xFF) - seeded the moment the user opens
    // ANY Radio submenu field for editing, so e.g. Frequency starts at a
    // usable 868.000 MHz instead of the encoder having to be cranked up
    // from 0.000 one 25kHz detent at a time. Also what makes Coding Rate's
    // display flip from "Unset" to "4/5" (see radio_unset in the list-view
    // draw code) the moment any other field is touched - extending that
    // same "touch one, the rest fill in" behavior to Bandwidth/SF/TX Power
    // too, per user request. Reuses urns_radio_bringup()'s own seed values
    // (RNode_Firmware.ino) for bw/sf/txp for consistency, except frequency,
    // which the user asked for as a plain round 868.000 MHz rather than
    // that function's 868.825 MHz.
    #define URNS_RADIO_DEFAULT_FREQ 868000000UL
    #define URNS_RADIO_DEFAULT_BW   125000UL
    #define URNS_RADIO_DEFAULT_SF   10
    #define URNS_RADIO_DEFAULT_TXP  17
    void urns_radio_seed_defaults_if_unset() {
      if (staged_lora_freq == 0)   staged_lora_freq = URNS_RADIO_DEFAULT_FREQ;
      if (staged_lora_bw   == 0)   staged_lora_bw   = URNS_RADIO_DEFAULT_BW;
      if (staged_lora_sf   == 0)   staged_lora_sf   = URNS_RADIO_DEFAULT_SF;
      if (staged_lora_txp  == 255) staged_lora_txp  = URNS_RADIO_DEFAULT_TXP;
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

  #endif

  #if HAS_RTC == true || HAS_GPS == true
    // "UTC" for a zero offset, else "+HH:MM"/"-HH:MM" - quarter-hour steps
    // (see TZ_OFFSET_QH_MIN/MAX, Utilities.h) can land on a non-zero minute
    // part (e.g. UTC+05:30), so this always prints both fields rather than
    // special-casing whole hours. Shared by both the RTC and GNSS pages'
    // own Timezone rows.
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
  #endif

  #if HAS_RTC == true
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
    staged_display_timeout    = display_blanking_code;
    staged_display_brightness = display_intensity;
    live_display_brightness   = display_intensity;
    #if HAS_NP == true
      staged_np_brightness = np_intensity;
      live_np_brightness   = np_intensity;
    #endif
    #if HAS_BUZZER == true
      staged_sound_enabled = sound_enabled;
    #endif
    #if HAS_ESPNOW == true
      staged_espnow_enabled = espnow_enabled;
      staged_espnow_mode_v2 = (espnow_mode == ESPNOW_MODE_V2);
      staged_espnow_lr_enabled = espnow_lr_enabled;
    #endif
    #if HAS_URNS == true
      staged_urns_enabled = urns_enabled;
      staged_urns_transport_enabled = urns_transport_enabled;
      staged_urns_link_mtu_discovery = urns_link_mtu_discovery;
      staged_urns_remote_mgmt_enabled = urns_remote_management_enabled;
      staged_urns_probe_dest_enabled = urns_probe_destination_enabled;
      #if HAS_ESPNOW == true
        staged_urns_interface_mode = urns_interface_mode;
      #endif
    #endif
    // Radio submenu - not gated on HAS_URNS, see project plan for the Radio menu.
    staged_lora_freq = lora_freq;
    staged_lora_bw   = lora_bw;
    staged_lora_sf   = (uint8_t)lora_sf;
    staged_lora_cr   = (uint8_t)lora_cr;
    staged_lora_txp  = (uint8_t)lora_txp;
    staged_radio_auto_start_enabled = radio_auto_start_enabled;
    #if HAS_ENCODER == true
      staged_encoder_enabled = encoder_enabled;
    #endif
    #if HAS_BLE == true
      #if MCU_VARIANT == MCU_ESP32
        staged_bt_legacy_pairing_enabled = bt_legacy_pairing_enabled;
        staged_bt_just_works_enabled = bt_just_works_enabled;
        staged_bt_auto_start_enabled = bt_auto_start_enabled;
      #endif
      staged_bt_battery_service_enabled = bt_battery_service_enabled;
    #endif
    #if HAS_LXMF == true
      staged_msngr_max_retries = msngr_max_retries;
      staged_msngr_retry_delay_s = msngr_retry_delay_s;
      staged_msngr_announce_at_start = msngr_announce_at_start;
      staged_msngr_announce_interval_idx = msngr_announce_interval_idx;
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
    #if HAS_LXMF == true
      // Leaving the whole menu (not just backing out of the ping-result
      // screen, which already calls this itself) would otherwise abandon
      // a still-PENDING/HANDSHAKE link with nothing left to ever tear it
      // down - see messenger_ping_process()'s own comment on why the
      // vendored Link has no working timeout watchdog of its own.
      messenger_ping_cancel();
    #endif
    if (staged_display_timeout != display_blanking_code) {
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
    #if HAS_NP == true
      if (staged_np_brightness != live_np_brightness) {
        // Same "compare against the live-preview baseline, not the current
        // value" reasoning as Brightness above - led_set_intensity() may
        // already have applied staged_np_brightness live on confirm.
        led_set_intensity(staged_np_brightness);
        np_int_conf_save(staged_np_brightness);
      }
    #endif
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
    #if HAS_URNS == true
      {
        // Safety net for the URNS_LIST confirm-handler's own gate (which
        // only stops *opening* Probe Destination's edit screen while
        // Transport Mode isn't staged on) - also cover the case where
        // Transport Mode gets staged back off *after* Probe Destination
        // was already staged/saved on in an earlier session. See that
        // gate's own comment for why this combination crashes.
        if (!staged_urns_transport_enabled) { staged_urns_probe_dest_enabled = false; }

        bool urns_enable_changed = (staged_urns_enabled != urns_enabled);
        bool urns_transport_changed = (staged_urns_transport_enabled != urns_transport_enabled);
        bool urns_link_mtu_changed = (staged_urns_link_mtu_discovery != urns_link_mtu_discovery);
        bool urns_remote_mgmt_changed = (staged_urns_remote_mgmt_enabled != urns_remote_management_enabled);
        bool urns_probe_dest_changed = (staged_urns_probe_dest_enabled != urns_probe_destination_enabled);
        #if HAS_ESPNOW == true
          bool urns_interface_changed = (staged_urns_interface_mode != urns_interface_mode);
        #else
          bool urns_interface_changed = false;
        #endif
        if (urns_enable_changed) {
          // Raw physical byte, not through eeprom_addr() - same convention
          // as ADDR_CONF_ESPNOW_MODE/LR above (ADDR_CONF_URNS, ROM.h).
          eeprom_update(ADDR_CONF_URNS, staged_urns_enabled ? URNS_ENABLE_BYTE : URNS_DISABLE_BYTE);
          urns_enabled = staged_urns_enabled;
        }
        if (urns_transport_changed) {
          eeprom_update(ADDR_CONF_URNS_TRANSPORT, staged_urns_transport_enabled ? URNS_TRANSPORT_ENABLE_BYTE : URNS_TRANSPORT_DISABLE_BYTE);
          urns_transport_enabled = staged_urns_transport_enabled;
        }
        if (urns_link_mtu_changed) {
          eeprom_update(ADDR_CONF_URNS_LINK_MTU_DISCOVERY, staged_urns_link_mtu_discovery ? URNS_LINK_MTU_DISCOVERY_ENABLE_BYTE : URNS_LINK_MTU_DISCOVERY_DISABLE_BYTE);
          urns_link_mtu_discovery = staged_urns_link_mtu_discovery;
        }
        if (urns_remote_mgmt_changed) {
          eeprom_update(ADDR_CONF_URNS_REMOTE_MGMT, staged_urns_remote_mgmt_enabled ? URNS_REMOTE_MGMT_ENABLE_BYTE : URNS_REMOTE_MGMT_DISABLE_BYTE);
          urns_remote_management_enabled = staged_urns_remote_mgmt_enabled;
        }
        if (urns_probe_dest_changed) {
          eeprom_update(ADDR_CONF_URNS_PROBE_DEST, staged_urns_probe_dest_enabled ? URNS_PROBE_DEST_ENABLE_BYTE : URNS_PROBE_DEST_DISABLE_BYTE);
          urns_probe_destination_enabled = staged_urns_probe_dest_enabled;
        }
        #if HAS_ESPNOW == true
          if (urns_interface_changed) {
            eeprom_update(ADDR_CONF_URNS_INTERFACE, staged_urns_interface_mode);
            urns_interface_mode = staged_urns_interface_mode;
          }
        #endif
        if (urns_enable_changed || urns_transport_changed || urns_link_mtu_changed ||
            urns_remote_mgmt_changed || urns_probe_dest_changed || urns_interface_changed) { hard_reset(); }
      }
    #endif
    {
      // Radio submenu (Frequency/Bandwidth/SF/CR/TX Power) - not a new
      // EEPROM field, reuses the same lora_freq/bw/sf/cr/txp globals and
      // eeprom_conf_save() (Utilities.h) a connected host's CMD_CONF_SAVE
      // already writes through ADDR_CONF_FREQ/BW/SF/CR/TXP (ROM.h). NOT
      // gated on HAS_URNS - see project plan for the Radio menu for why
      // this reuses rather than adds dedicated URNS_RADIO_* fields.
      bool urns_radio_changed = (staged_lora_freq != lora_freq) || (staged_lora_bw != lora_bw) ||
        (staged_lora_sf != (uint8_t)lora_sf) || (staged_lora_cr != (uint8_t)lora_cr) ||
        (staged_lora_txp != (uint8_t)lora_txp);
      if (urns_radio_changed) {
        lora_freq = staged_lora_freq;
        lora_bw   = staged_lora_bw;
        lora_sf   = staged_lora_sf;
        lora_cr   = staged_lora_cr;
        lora_txp  = staged_lora_txp;
        // eeprom_conf_save() below only writes when radio_online is already
        // true (Utilities.h) - true by the time this runs on a board with a
        // previously-saved config (boot-time validate_status() already
        // started it), but never true on a virgin board with no saved
        // config and no HAS_URNS bring-up (urns_radio_bringup(),
        // RNode_Firmware.ino) to seed it. Without this, the very first Save
        // & Exit on a fresh board would silently no-op the write and reboot
        // straight back into the same unconfigured state.
        if (!radio_online) startRadio();
        eeprom_conf_save();
        hard_reset();
      }
      if (staged_radio_auto_start_enabled != radio_auto_start_enabled) {
        radio_auto_start_conf_save(staged_radio_auto_start_enabled);
      }
    }
    #if HAS_ENCODER == true
      if (staged_encoder_enabled != encoder_enabled) {
        enc_conf_save(staged_encoder_enabled);
      }
    #endif
    #if HAS_BLE == true
      #if MCU_VARIANT == MCU_ESP32
        if (staged_bt_legacy_pairing_enabled != bt_legacy_pairing_enabled) {
          bt_legacy_pairing_conf_save(staged_bt_legacy_pairing_enabled);
        }
        if (staged_bt_just_works_enabled != bt_just_works_enabled) {
          bt_just_works_conf_save(staged_bt_just_works_enabled);
        }
        if (staged_bt_auto_start_enabled != bt_auto_start_enabled) {
          bt_auto_start_conf_save(staged_bt_auto_start_enabled);
        }
      #endif
      if (staged_bt_battery_service_enabled != bt_battery_service_enabled) {
        bt_battery_service_conf_save(staged_bt_battery_service_enabled);
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
        // (wifi_remote_start_sta()'s own comment, Remote.h) - a reboot is
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
      if (wifi_changed) {
        // wifi_remote_init()'s WiFi.mode(WIFI_MODE_NULL) fully deinitializes
        // the shared esp_wifi driver state ESP-NOW rides on top of,
        // deregistering its peer/callbacks along with it - and since
        // espnow_init() (ESPNOW.h) only ever runs once, at boot, ESP-NOW
        // never recovers (confirmed on hardware, see wifi_remote_init()'s
        // own comment, Remote.h - the same reasoning behind that file's
        // separate lighter STA-reconnect-retry path that avoids WiFi.mode()
        // entirely). Calling it here while ESP-NOW is live would silently
        // kill the running ESP-NOW session for the rest of the boot.
        // EEPROM writes above already happened regardless - defer applying
        // them live until the next boot (or ESP-NOW being disabled, which
        // itself always reboots - see espnow_conf_save(), Utilities.h) -
        // instead of leaving the user with no way to change these settings
        // without also being warned they need a reboot for them to take.
        #if HAS_ESPNOW == true
          if (!espnow_ready) { wifi_remote_init(); }
        #else
          wifi_remote_init();
        #endif
      }
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
      }
    #endif
    #if HAS_RTC == true || HAS_GPS == true
      // Same flush-in-progress reasoning as above - reachable from either
      // the RTC or GNSS page's own Timezone row now (tz_edit_return_state).
      if (menu_state == MENU_STATE_RTC_TZ_EDIT) {
        uint8_t live_raw = (uint8_t)(get_tz_offset_qh() + TZ_OFFSET_RAW_ZERO);
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
    #if HAS_LXMF == true
      // Same "don't abandon an in-flight ping" reasoning as
      // menu_commit_and_exit() above - this is the inactivity-timeout exit
      // path, so it's the one most likely to actually catch a ping mid-
      // flight (the user walked away instead of pressing BACK).
      messenger_ping_cancel();
    #endif
    // Brightness gets a live preview the instant it's confirmed (see
    // menu_confirm_select()), unlike every other field - undo that here so
    // an unsaved preview doesn't linger after the menu gives up on it.
    display_intensity = live_display_brightness;
    #if HAS_NP == true
      led_set_intensity(live_np_brightness);
    #endif
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
    if (!menu_is_open()) return;
    #if HAS_GPS == true && HAS_GNSS_DEBUG_MENU == true
      // GNSS Diagnostics/Sats In View are meant to be watched, not
      // interacted with - values update on their own (satellite/duty-cycle
      // state) with no button presses expected, so both this menu's own
      // idle-close and Display.h's generic blanking timeout (which share
      // the same "no real input" clock) would otherwise kick the user back
      // to the main screen mid-observation. display_unblank() resets
      // Display.h's last_unblank_event as a side effect (it's already
      // called unconditionally on every real input event, see its own
      // comment), which is enough to suppress blanking too without
      // Display.h - built before Menu.h - needing to know about
      // MENU_STATE_GNSS_DIAG directly.
      if (menu_state == MENU_STATE_GNSS_DIAG || menu_state == MENU_STATE_GNSS_DIAG_SATS) {
        display_unblank();
        return;
      }
    #endif
    if (millis() - menu_last_activity_ms > (unsigned long)SETTINGS_MENU_TIMEOUT * 1000UL) {
      menu_close_without_saving();
    }
  }

  void menu_encoder_rotate(int8_t dir, bool wrap) {
    menu_last_activity_ms = millis();
    display_unblank();
    if (menu_state == MENU_STATE_STATUS_POPUP) {
      // Not a real navigable screen - any input at all dismisses it,
      // rotation included, rather than the usual per-state handling
      // below.
      buzzer_encoder_tick_melody();
      menu_state = menu_popup_return_state;
      return;
    }
    if (menu_state == MENU_STATE_LIST) {
      buzzer_encoder_tick_melody();
      menu_cursor = menu_clamp_cursor(menu_cursor, dir, MENU_ITEM_COUNT, wrap);
    } else if (menu_state == MENU_STATE_EDIT) {
      buzzer_encoder_tick_melody();
      if (menu_edit_field == MENU_ITEM_DISPLAY_TIMEOUT) {
        step_timeout(dir, wrap);
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
      #if HAS_NP == true
        else if (menu_edit_field == MENU_ITEM_NEOPIXEL_BRIGHTNESS) {
          menu_step_numeric(&staged_np_brightness, dir, wrap);
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
    #if HAS_BLUETOOTH == true || HAS_BLE == true
      else if (menu_state == MENU_STATE_BT_LIST) {
        buzzer_encoder_tick_melody();
        bt_menu_cursor = menu_clamp_cursor(bt_menu_cursor, dir, BT_ITEM_COUNT, wrap);
      }
      #if HAS_BLE == true
        else if (menu_state == MENU_STATE_BT_SETTINGS) {
          buzzer_encoder_tick_melody();
          bt_settings_cursor = menu_clamp_cursor(bt_settings_cursor, dir, BT_SETTINGS_ITEM_COUNT, wrap);
        }
        else if (menu_state == MENU_STATE_BT_SETTINGS_EDIT) {
          buzzer_encoder_tick_melody();
          // Cursor-dispatched, same shape as MSNGR_SETTINGS_EDIT/URNS_EDIT -
          // bt_settings_cursor still points at whichever field was open
          // when this state was entered. All rows here are plain boolean
          // toggles.
          #if MCU_VARIANT == MCU_ESP32
            if (bt_settings_cursor == BT_SETTINGS_ITEM_JUST_WORKS) {
              staged_bt_just_works_enabled = !staged_bt_just_works_enabled;
            } else if (bt_settings_cursor == BT_SETTINGS_ITEM_AUTO_START) {
              staged_bt_auto_start_enabled = !staged_bt_auto_start_enabled;
            } else if (bt_settings_cursor == BT_SETTINGS_ITEM_BATTERY_SERVICE) {
              staged_bt_battery_service_enabled = !staged_bt_battery_service_enabled;
            } else {
              staged_bt_legacy_pairing_enabled = !staged_bt_legacy_pairing_enabled;
            }
          #else // MCU_NRF52 - Battery Service is the only real row here
            staged_bt_battery_service_enabled = !staged_bt_battery_service_enabled;
          #endif
        }
      #endif
      #if HAS_BLE == true
        else if (menu_state == MENU_STATE_BT_UNPAIR_CONFIRM) {
          buzzer_encoder_tick_melody();
          bt_unpair_confirm_cursor = menu_clamp_cursor(bt_unpair_confirm_cursor, dir, 2, wrap);
        }
      #endif
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
      }
    #endif
    #if HAS_RTC == true || HAS_GPS == true
      // Reachable from either the RTC or GNSS page's own Timezone row now
      // (tz_edit_return_state) - not RTC-list-specific.
      else if (menu_state == MENU_STATE_RTC_TZ_EDIT) {
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
        #if GNSS_DUTY_CYCLE_CAPABLE == true
          if (gnss_menu_cursor == GNSS_ITEM_UPDATE_INTERVAL) {
            staged_gnss_interval_index = menu_clamp_cursor(staged_gnss_interval_index, dir, GNSS_UPDATE_INTERVAL_PRESET_COUNT, wrap);
          } else
        #endif
        staged_gnss_enabled = !staged_gnss_enabled;
      }
      #if HAS_GNSS_DEBUG_MENU == true
        else if (menu_state == MENU_STATE_GNSS_DIAG) {
          buzzer_encoder_tick_melody();
          gnss_diag_menu_cursor = menu_clamp_cursor(gnss_diag_menu_cursor, dir, GNSS_DIAG_ITEM_COUNT, wrap);
        } else if (menu_state == MENU_STATE_GNSS_DIAG_SATS) {
          buzzer_encoder_tick_melody();
          uint8_t row_count = gnss_sat_view_count() == 0 ? 2 : (uint8_t)(gnss_sat_view_count() + 1);
          gnss_diag_sats_cursor = menu_clamp_cursor(gnss_diag_sats_cursor, dir, row_count, wrap);
        }
      #endif
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
        else if (espnow_menu_cursor == ESPNOW_ITEM_CHANNEL) step_wifi_channel(dir, wrap);
        else                                                staged_espnow_lr_enabled = !staged_espnow_lr_enabled;
      } else if (menu_state == MENU_STATE_ESPNOW_LR_CONFIRM) {
        // 3-item list (info row/ENABLE/CANCEL) - same tap-to-move/hold-to-
        // select navigation as everywhere else, same pattern as F/W
        // Update's UPDATE/CANCEL (MENU_STATE_FWUPD_CONFIRM).
        buzzer_encoder_tick_melody();
        espnow_lr_confirm_cursor = menu_clamp_cursor(espnow_lr_confirm_cursor, dir, 3, wrap);
      }
    #endif
    #if HAS_URNS == true
      else if (menu_state == MENU_STATE_URNS_LIST) {
        buzzer_encoder_tick_melody();
        urns_menu_cursor = menu_clamp_cursor(urns_menu_cursor, dir, URNS_ITEM_COUNT, wrap);
      } else if (menu_state == MENU_STATE_URNS_EDIT) {
        buzzer_encoder_tick_melody();
        // Cursor-dispatched toggle, same shape as ESP-NOW's MENU_STATE_
        // ESPNOW_EDIT - urns_menu_cursor still points at whichever field
        // was open when this state was entered.
        if (urns_menu_cursor == URNS_ITEM_ENABLED) staged_urns_enabled = !staged_urns_enabled;
        else if (urns_menu_cursor == URNS_ITEM_TRANSPORT) staged_urns_transport_enabled = !staged_urns_transport_enabled;
        #if HAS_ESPNOW == true
        else if (urns_menu_cursor == URNS_ITEM_INTERFACE) staged_urns_interface_mode = menu_clamp_cursor(staged_urns_interface_mode, dir, 3, wrap);
        #endif
        else if (urns_menu_cursor == URNS_ITEM_LINK_MTU_DISCOVERY) staged_urns_link_mtu_discovery = !staged_urns_link_mtu_discovery;
        else if (urns_menu_cursor == URNS_ITEM_REMOTE_MGMT) staged_urns_remote_mgmt_enabled = !staged_urns_remote_mgmt_enabled;
        else                                                staged_urns_probe_dest_enabled = !staged_urns_probe_dest_enabled;
      } else if (menu_state == MENU_STATE_URNS_PATHS) {
        buzzer_encoder_tick_melody();
        urns_paths_menu_cursor = menu_clamp_cursor(urns_paths_menu_cursor, dir, urns_path_display_row_count(), wrap);
      } else if (menu_state == MENU_STATE_URNS_PATH_DETAIL) {
        buzzer_encoder_tick_melody();
        urns_path_detail_cursor = menu_clamp_cursor(urns_path_detail_cursor, dir, URNS_PATH_DETAIL_ITEM_COUNT, wrap);
      } else if (menu_state == MENU_STATE_URNS_FREE_DETAIL) {
        buzzer_encoder_tick_melody();
        urns_free_detail_cursor = menu_clamp_cursor(urns_free_detail_cursor, dir, URNS_FREE_DETAIL_ITEM_COUNT, wrap);
      } else if (menu_state == MENU_STATE_URNS_PATH_HASH_VIEW) {
        // Nothing to move a cursor across - any rotation just dismisses
        // it too, same as a confirm (see menu_confirm_select()).
        buzzer_encoder_tick_melody();
        menu_state = MENU_STATE_URNS_PATH_DETAIL;
      }
      #if HAS_LXMF == true
      else if (menu_state == MENU_STATE_MSNGR_LIST) {
        buzzer_encoder_tick_melody();
        msngr_menu_cursor = menu_clamp_cursor(msngr_menu_cursor, dir, MSNGR_TOP_ITEM_COUNT, wrap);
      } else if (menu_state == MENU_STATE_MSNGR_INBOX) {
        buzzer_encoder_tick_melody();
        msngr_inbox_cursor = menu_clamp_cursor(msngr_inbox_cursor, dir, msngr_inbox_row_count(), wrap);
      } else if (menu_state == MENU_STATE_MSNGR_BOOKMARKS) {
        buzzer_encoder_tick_melody();
        msngr_bookmarks_cursor = menu_clamp_cursor(msngr_bookmarks_cursor, dir, msngr_bookmarks_row_count(), wrap);
      } else if (menu_state == MENU_STATE_MSNGR_ANNOUNCES) {
        buzzer_encoder_tick_melody();
        msngr_announces_cursor = menu_clamp_cursor(msngr_announces_cursor, dir, msngr_announces_row_count(), wrap);
      } else if (menu_state == MENU_STATE_MSNGR_PEER) {
        buzzer_encoder_tick_melody();
        msngr_peer_cursor = menu_clamp_cursor(msngr_peer_cursor, dir, msngr_peer_row_count(), wrap);
      } else if (menu_state == MENU_STATE_MSNGR_MSG_DETAIL) {
        buzzer_encoder_tick_melody();
        msngr_msg_detail_cursor = menu_clamp_cursor(msngr_msg_detail_cursor, dir, msngr_msg_detail_row_count(), wrap);
      } else if (menu_state == MENU_STATE_MSNGR_DELETE_CONFIRM) {
        buzzer_encoder_tick_melody();
        msngr_delete_confirm_cursor = menu_clamp_cursor(msngr_delete_confirm_cursor, dir, 2, wrap);
      } else if (menu_state == MENU_STATE_MSNGR_CLEAR_CONFIRM) {
        buzzer_encoder_tick_melody();
        msngr_clear_confirm_cursor = menu_clamp_cursor(msngr_clear_confirm_cursor, dir, 2, wrap);
      } else if (menu_state == MENU_STATE_MSNGR_TEXT_ENTRY) {
        // Steps one key at a time through the flattened MSNGR_KB_LAYOUT
        // grid, row-major, wrapping at both ends - see its own declaration
        // for why this is a single linear cursor rather than real 2D nav.
        buzzer_encoder_tick_melody();
        msngr_kb_cursor = menu_clamp_cursor(msngr_kb_cursor, dir, MSNGR_KB_KEY_COUNT, wrap);
      } else if (menu_state == MENU_STATE_MSNGR_DISCARD_CONFIRM) {
        buzzer_encoder_tick_melody();
        msngr_discard_confirm_cursor = menu_clamp_cursor(msngr_discard_confirm_cursor, dir, 2, wrap);
      } else if (menu_state == MENU_STATE_MSNGR_PING_RESULT) {
        buzzer_encoder_tick_melody();
        msngr_ping_result_cursor = menu_clamp_cursor(msngr_ping_result_cursor, dir, 2, wrap);
      } else if (menu_state == MENU_STATE_MSNGR_SEND_RESULT) {
        buzzer_encoder_tick_melody();
        msngr_send_result_cursor = menu_clamp_cursor(msngr_send_result_cursor, dir, 2, wrap);
      } else if (menu_state == MENU_STATE_MSNGR_SETTINGS) {
        buzzer_encoder_tick_melody();
        msngr_settings_cursor = menu_clamp_cursor(msngr_settings_cursor, dir, MSNGR_SETTINGS_ITEM_COUNT, wrap);
      } else if (menu_state == MENU_STATE_MSNGR_SETTINGS_EDIT) {
        buzzer_encoder_tick_melody();
        // Cursor-dispatched, same shape as URNS_RADIO_EDIT/URNS_EDIT below -
        // msngr_settings_cursor still points at whichever field was open
        // when this state was entered. Announce at Start is a plain
        // boolean toggle-on-turn, same convention as URNS_EDIT's own
        // boolean fields (URNS_ITEM_ENABLED etc) - OK just confirms/exits,
        // it doesn't flip the value itself.
        if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_RETRIES) step_msngr_retries(dir, wrap);
        else if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_RETRY_DELAY) step_msngr_retry_delay(dir, wrap);
        else if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_ANNOUNCE_START) staged_msngr_announce_at_start = !staged_msngr_announce_at_start;
        else if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_ANNOUNCE_INTERVAL) step_msngr_announce_interval(dir, wrap);
      } else if (menu_state == MENU_STATE_MSNGR_PRESETS) {
        buzzer_encoder_tick_melody();
        msngr_presets_cursor = menu_clamp_cursor(msngr_presets_cursor, dir, msngr_presets_row_count(), wrap);
      } else if (menu_state == MENU_STATE_MSNGR_PRESET_DETAIL) {
        buzzer_encoder_tick_melody();
        msngr_preset_detail_cursor = menu_clamp_cursor(msngr_preset_detail_cursor, dir, 3, wrap);
      }
      #endif
    #endif
      else if (menu_state == MENU_STATE_URNS_RADIO_LIST) {
        buzzer_encoder_tick_melody();
        urns_radio_menu_cursor = menu_clamp_cursor(urns_radio_menu_cursor, dir, URNS_RADIO_ITEM_COUNT, wrap);
      } else if (menu_state == MENU_STATE_URNS_RADIO_EDIT) {
        buzzer_encoder_tick_melody();
        // Cursor-dispatched stepper, same shape as ESP-NOW's MENU_STATE_
        // ESPNOW_EDIT - urns_radio_menu_cursor still points at whichever
        // field was open when this state was entered.
        if (urns_radio_menu_cursor == URNS_RADIO_ITEM_FREQ)      step_urns_radio_freq(dir, wrap);
        else if (urns_radio_menu_cursor == URNS_RADIO_ITEM_BW)   step_urns_radio_bw(dir, wrap);
        else if (urns_radio_menu_cursor == URNS_RADIO_ITEM_SF)   step_urns_radio_sf(dir, wrap);
        else if (urns_radio_menu_cursor == URNS_RADIO_ITEM_CR)   step_urns_radio_cr(dir, wrap);
        else if (urns_radio_menu_cursor == URNS_RADIO_ITEM_AUTO_START)
          staged_radio_auto_start_enabled = !staged_radio_auto_start_enabled;
        else                                                     step_urns_radio_txp(dir, wrap);
      }
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
      #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
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

  // Set whenever a rotation tick arrives while the encoder's push-button
  // is physically held down and actually does something with it (see
  // menu_encoder_chord_rotate() below) - declared unconditionally (not
  // gated on HAS_URNS) so Encoder.h can reset it on every new press
  // without needing its own #if. Consumed by menu_encoder_button() to
  // suppress that press's eventual release action (a plain select or the
  // long-press-leave path) once a chord's already been performed with it.
  bool msngr_kb_chord_used = false;

  // Called instead of menu_encoder_rotate() when a rotation tick arrives
  // while the button is held (Encoder.h's encoder_process()). Only
  // MENU_STATE_MSNGR_TEXT_ENTRY gives the chord any meaning - press-and-
  // turn inserts the highlighted key's uppercase form (letters) or its
  // paired alternate mark (the '.'/','/'?' punctuation cells, see
  // msngr_kb_alt_pair()) without moving the cursor, the same "shifted"
  // gesture meshtastic's own VirtualKeyboard gets from a long-press (not
  // reusable here, since a long hold on this screen already means
  // "leave the keyboard" - see menu_encoder_button() and menu_msngr_
  // text_entry_leave()). Every other screen falls straight through to
  // the normal rotate handling - so holding the button while turning
  // anywhere else keeps behaving exactly as it already did.
  void menu_encoder_chord_rotate(int8_t dir) {
    #if HAS_LXMF == true
      if (menu_state == MENU_STATE_MSNGR_TEXT_ENTRY) {
        msngr_kb_chord_used = true;
        uint8_t kb_row = msngr_kb_cursor / MSNGR_KB_COLS;
        uint8_t kb_col = msngr_kb_cursor % MSNGR_KB_COLS;
        char key_ch = (msngr_kb_lang_ru ? MSNGR_KB_LAYOUT_RU : MSNGR_KB_LAYOUT)[kb_row][kb_col];
        if (msngr_kb_key_type(key_ch) == MSNGR_KB_CHAR) {
          size_t text_len = strlen(msngr_text_entry_buf);
          // Letters get their uppercase form; punctuation has no
          // uppercase (apply_shift() leaves it unchanged), so falls
          // back to its paired alternate mark instead - either way,
          // to_insert == key_ch means "this key has no chord action".
          char shifted = msngr_kb_apply_shift(key_ch, true);
          char to_insert = (shifted != key_ch) ? shifted : msngr_kb_alt_pair(key_ch);
          if (text_len < MSNGR_TEXT_ENTRY_MAX_LEN && to_insert != 0) {
            msngr_text_entry_buf[text_len] = to_insert;
            msngr_text_entry_buf[text_len + 1] = 0;
            buzzer_encoder_tick_melody();
          }
        } else if (msngr_kb_key_type(key_ch) == MSNGR_KB_SHIFT) {
          // Quick alternative to holding Shift past MSNGR_KB_ALT_HOLD_MS
          // (msngr_kb_lang_hold_try()) - press-and-turn switches EN/RU
          // immediately on the first tick, same as chording a letter
          // inserts its capital immediately rather than waiting for a
          // hold. Reuses msngr_kb_chord_used (already set true above)
          // for release-suppression, so there's no separate fired-flag
          // needed the way the hold path requires - a rotated tick while
          // still held plus release only ever means "chord", covered by
          // the exact same is-this-release-the-tail-of-a-chord check
          // menu_encoder_button() already does for every other chord.
          msngr_kb_lang_ru = !msngr_kb_lang_ru;
          msngr_kb_shift_on = false;
          buzzer_encoder_tick_melody();
        }
        return;
      }
    #endif
    menu_encoder_rotate(dir, false);
  }

  void menu_confirm_select(unsigned long duration = 0);

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

  #if HAS_LXMF == true
    // Reached from button_event()'s own dedicated BUTTON_HOLD_TIER_MESSENGER
    // hold duration - lands directly on the Messenger app's top screen
    // rather than the settings top list, same "no-op if console/firmware-
    // update active or device not ready" guard as menu_open_from_closed()
    // above. menu_stage_from_live() is still called (harmless - Messenger
    // has no staged EEPROM fields of its own) so that if the user
    // subsequently backs out into the rest of the settings menu, every
    // other submenu's staged values are already primed same as any other
    // menu entry point.
    void messenger_open_from_closed() {
      if (!console_active && !firmware_update_mode && device_init_done) {
        buzzer_encoder_click_melody();
        menu_stage_from_live();
        menu_state = MENU_STATE_MSNGR_LIST;
        msngr_menu_cursor = 0;
        menu_last_activity_ms = millis();
      }
    }

    // Encoder-only double-click-to-Messenger: two short clicks landing
    // within this window while the menu is closed reach the same
    // destination as the main button's dedicated BUTTON_HOLD_TIER_MESSENGER
    // hold, just via click-count instead of hold-duration - the encoder's
    // own long-press-from-closed is already spoken for (opens Settings).
    // See menu_encoder_button()'s own "no-op (reserved)" comment for why a
    // single short click while closed was free to build this on.
    const unsigned long ENC_DOUBLE_CLICK_WINDOW_MS = 400;
    unsigned long enc_last_short_click_ms = 0;
  #endif

  void menu_encoder_button(unsigned long duration) {
    menu_last_activity_ms = millis();
    display_unblank();

    if (menu_state == MENU_STATE_STATUS_POPUP) {
      // Not a real navigable screen - any input (short click or long
      // press alike) dismisses it, rather than the usual short=confirm/
      // long=commit-and-exit split below (which would otherwise close
      // the *entire* menu on a long press here, not just this popup).
      buzzer_encoder_click_melody();
      menu_state = menu_popup_return_state;
      return;
    }

    #if HAS_LXMF == true
      if (menu_state == MENU_STATE_MSNGR_TEXT_ENTRY && msngr_kb_chord_used) {
        // The hold that's ending just chorded in one or more capital
        // letters (menu_encoder_chord_rotate()) - this release is the
        // tail end of that gesture, not a fresh click, so it shouldn't
        // also select/insert or fall into the long-press-leave path below.
        msngr_kb_chord_used = false;
        return;
      }
    #endif

    unsigned long long_press_threshold = 700;
    #if HAS_LXMF == true
      // Chording needs the button held down while rotating, which can
      // easily run past the normal 700ms threshold on a slow or deliberate
      // turn - a much longer threshold here means an ordinary chord
      // attempt doesn't also risk throwing away a half-typed message via
      // the long-press-leave path below.
      if (menu_state == MENU_STATE_MSNGR_TEXT_ENTRY) long_press_threshold = 3000;
    #endif

    #if HAS_LXMF == true
      if (menu_state == MENU_STATE_CLOSED && duration <= long_press_threshold) {
        unsigned long now = millis();
        if (now - enc_last_short_click_ms <= ENC_DOUBLE_CLICK_WINDOW_MS) {
          enc_last_short_click_ms = 0;
          messenger_open_from_closed();
          return;
        }
        enc_last_short_click_ms = now;
      }
    #endif

    if (duration > long_press_threshold) {
      #if HAS_LXMF == true
        if (menu_state == MENU_STATE_MSNGR_TEXT_ENTRY) {
          buzzer_encoder_click_melody();
          menu_msngr_text_entry_leave();
          return;
        }
      #endif
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

    // Skip the usual click - msngr_kb_lang_hold_try()/msngr_kb_alt_
    // hold_try() already played their own tick the moment the hold
    // actually fired, mid-press; beeping again here on release would be
    // a redundant second sound for the same one gesture. Only relevant
    // to this encoder button's own flags (msngr_kb_..._fired_enc) - the
    // main button's (msngr_kb_..._fired_btn) are menu_button_press()'s
    // concern, released independently.
    #if HAS_LXMF == true
      bool msngr_kb_alt_already_beeped = msngr_kb_lang_hold_fired_enc || msngr_kb_alt_hold_fired_enc || msngr_kb_del_hold_fired_enc;
    #else
      bool msngr_kb_alt_already_beeped = false;
    #endif
    if (menu_state != MENU_STATE_CLOSED && !msngr_kb_alt_already_beeped) buzzer_encoder_click_melody();
    menu_confirm_select(duration);
  }

  // The "confirm/select at the current level" action - shared by the
  // encoder's short-click and the main button's long-press (see
  // menu_button_press()), so both controls behave identically here.
  // duration is the triggering press's hold time (ms) as both callers
  // already had it - unused everywhere except MSNGR_TEXT_ENTRY's Shift
  // key (see MSNGR_KB_ALT_HOLD_MS); every other branch ignores it, same
  // as before this parameter existed. Defaults to 0 at the declaration
  // for any future call site that has no meaningful duration to give it.
  void menu_confirm_select(unsigned long duration) {
    if (menu_state == MENU_STATE_LIST) {
      if (menu_cursor == MENU_ITEM_SAVE_EXIT) {
        menu_commit_and_exit();
      }
      #if HAS_WIFI == true
        else if (menu_cursor == MENU_ITEM_WIFI) {
          wifi_menu_cursor = 0;
          // Settings here are still fully editable and get saved to EEPROM
          // as normal (see menu_commit_and_exit()'s own ESP-NOW guard) -
          // this is purely a heads-up that they won't take effect live
          // while ESP-NOW is running (WiFi.mode(WIFI_MODE_NULL), which
          // wifi_remote_init() needs to apply a change, tears down the
          // shared esp_wifi driver state ESP-NOW depends on - see that
          // guard's own comment). Checks espnow_ready (is ESP-NOW's driver
          // actually live right now), not just the enabled setting -
          // applies regardless of LR mode, since the conflict isn't LR-
          // specific. Dismissed by any input, same as every other
          // MENU_STATE_STATUS_POPUP use - lands in the WiFi list either way.
          #if HAS_ESPNOW == true
            if (espnow_ready) {
              menu_open_popup("WIFI DISABLED (ESP-NOW)", MENU_STATE_WIFI_LIST);
            } else {
              menu_state = MENU_STATE_WIFI_LIST;
            }
          #else
            menu_state = MENU_STATE_WIFI_LIST;
          #endif
        }
      #endif
      #if HAS_BLUETOOTH == true || HAS_BLE == true
        else if (menu_cursor == MENU_ITEM_BLUETOOTH) {
          // The submenu's own draw code reads live BLE stack state (MAC,
          // bt_bond_count() -> NimBLE's ble_store_util_count()/Bluedroid's
          // esp_ble_get_bond_device_num()) unconditionally - calling those
          // with the stack never started (BT_STATE_OFF, e.g. NimBLE's host
          // task/port never initialized) crashes rather than erroring out
          // gracefully. Same enable-first pattern CMD_BT_CTRL's 0x02
          // (enable pairing) sub-command already uses, just applied here
          // too so simply opening the submenu can't crash regardless of
          // current state.
          if (bt_state == BT_STATE_OFF) {
            bt_start();
            bt_conf_save(true);
          }
          menu_state = MENU_STATE_BT_LIST;
          bt_menu_cursor = 0;
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
      #if HAS_LXMF == true
        else if (menu_cursor == MENU_ITEM_MESSENGER) {
          menu_state = MENU_STATE_MSNGR_LIST;
          msngr_menu_cursor = 0;
        }
      #endif
      #if HAS_URNS == true
        else if (menu_cursor == MENU_ITEM_URNS) {
          menu_state = MENU_STATE_URNS_LIST;
          urns_menu_cursor = 0;
        }
      #endif
        else if (menu_cursor == MENU_ITEM_URNS_RADIO) {
          menu_state = MENU_STATE_URNS_RADIO_LIST;
          urns_radio_menu_cursor = 0;
        }
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
      #if HAS_NP == true
        else if (menu_edit_field == MENU_ITEM_NEOPIXEL_BRIGHTNESS) {
          // Same live-preview-on-confirm treatment as Brightness above -
          // EEPROM write stays deferred to menu_commit_and_exit() (see
          // live_np_brightness).
          led_set_intensity(staged_np_brightness);
        }
      #endif
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
          menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
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
    #if HAS_BLUETOOTH == true || HAS_BLE == true
      else if (menu_state == MENU_STATE_BT_LIST) {
        if (bt_menu_cursor == BT_ITEM_BACK) {
          menu_state = MENU_STATE_LIST;
        }
        #if HAS_BLE == true
          else if (bt_menu_cursor == BT_ITEM_SETTINGS) {
            menu_state = MENU_STATE_BT_SETTINGS;
            bt_settings_cursor = 0;
          }
        #endif
        #if HAS_BLE == true
          else if (bt_menu_cursor == BT_ITEM_UNPAIR) {
            bt_unpair_confirm_cursor = 1; // default CANCEL - see its own declaration
            menu_state = MENU_STATE_BT_UNPAIR_CONFIRM;
          }
        #endif
        // MAC/Bonds are read-only - same shape as ESP-NOW's Channel row, no
        // edit state, selecting them does nothing.
      }
      #if HAS_BLE == true
        else if (menu_state == MENU_STATE_BT_SETTINGS) {
          if (bt_settings_cursor == BT_SETTINGS_ITEM_BACK) {
            // No commit here, same as BT_LIST's own Back above - BT
            // settings are part of the shared "RNode Settings" staged-
            // commit tree (Radio/URNS/etc), actually written at that
            // outer tree's own Save & Exit, not per-submenu.
            menu_state = MENU_STATE_BT_LIST;
          } else {
            menu_state = MENU_STATE_BT_SETTINGS_EDIT;
          }
        }
        else if (menu_state == MENU_STATE_BT_SETTINGS_EDIT) {
          menu_state = MENU_STATE_BT_SETTINGS; // confirms staged value, no write yet
        }
      #endif
      #if HAS_BLE == true
        else if (menu_state == MENU_STATE_BT_UNPAIR_CONFIRM) {
          if (bt_unpair_confirm_cursor == 0) { // FORGET
            bt_debond_all();
          }
          // CANCEL: leave existing bonds untouched.
          menu_state = MENU_STATE_BT_LIST;
        }
      #endif
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
          // stays on ETH_LIST, which redraws showing "DHCP"/"N/A" for all
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
            menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
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
          staged_tz_offset_qh = get_tz_offset_qh();
          tz_edit_return_state = MENU_STATE_RTC_LIST;
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
        } else if (gnss_menu_cursor == GNSS_ITEM_TIMEZONE) {
          // Sync fresh from the live value, same immediate-commit
          // reasoning as RTC's own Timezone field.
          staged_tz_offset_qh = get_tz_offset_qh();
          tz_edit_return_state = MENU_STATE_GNSS_LIST;
          menu_state = MENU_STATE_RTC_TZ_EDIT;
        }
        #if HAS_GNSS_DEBUG_MENU == true
          else if (gnss_menu_cursor == GNSS_ITEM_DIAGNOSTICS) {
            gnss_diag_menu_cursor = 0;
            menu_state = MENU_STATE_GNSS_DIAG;
          }
        #endif
        #if BOARD_MODEL != BOARD_HELTEC_T114
          else if (gnss_menu_cursor == GNSS_ITEM_SHOW_BANNER) {
            // One-shot, not a toggle - matches what was actually asked for:
            // pin the panel on screen until a hard reboot, with no menu path
            // back to the RNode/ID+carousel view. Closes the whole settings
            // menu immediately (same discard-and-close used by the idle
            // timeout, menu_timeout_process()) rather than returning to this
            // list - the point is to see the banner right away, not to sit
            // in the menu after triggering it.
            gnss_banner_forced = true;
            menu_close_without_saving();
          }
        #endif
        #if GNSS_DUTY_CYCLE_CAPABLE == true
          else if (gnss_menu_cursor == GNSS_ITEM_UPDATE_INTERVAL) {
            // Sync fresh from the live value (by matching it back to its
            // preset index) - same immediate-commit reasoning as Enabled
            // above, not a whole-submenu staged edit.
            staged_gnss_interval_index = 0;
            for (uint8_t i = 0; i < GNSS_UPDATE_INTERVAL_PRESET_COUNT; i++) {
              if (gnss_update_interval_presets_s[i] == gnss_update_interval_s) { staged_gnss_interval_index = i; break; }
            }
            menu_state = MENU_STATE_GNSS_EDIT;
          }
        #endif
      } else if (menu_state == MENU_STATE_GNSS_EDIT) {
        // Commits + applies live here rather than staging until SAVE &
        // EXIT - a power-saving toggle should apply the instant it's
        // confirmed, same immediate-commit pattern as RTC's Timezone
        // field.
        #if GNSS_DUTY_CYCLE_CAPABLE == true
          if (gnss_menu_cursor == GNSS_ITEM_UPDATE_INTERVAL) {
            uint32_t new_interval_s = gnss_update_interval_presets_s[staged_gnss_interval_index];
            if (new_interval_s != gnss_update_interval_s) {
              gnss_interval_conf_save(staged_gnss_interval_index);
              gnss_update_interval_s = new_interval_s;
            }
          } else
        #endif
        if (staged_gnss_enabled != gnss_enabled) {
          gnss_conf_save(staged_gnss_enabled);
          gnss_set_enabled(staged_gnss_enabled);
        }
        menu_state = MENU_STATE_GNSS_LIST;
      }
      #if HAS_GNSS_DEBUG_MENU == true
        else if (menu_state == MENU_STATE_GNSS_DIAG) {
          if (gnss_diag_menu_cursor == GNSS_DIAG_ITEM_BACK) {
            menu_state = MENU_STATE_GNSS_LIST;
          } else if (gnss_diag_menu_cursor == GNSS_DIAG_ITEM_SATS_VIEW) {
            gnss_diag_sats_cursor = 0;
            menu_state = MENU_STATE_GNSS_DIAG_SATS;
          }
          // Every other row is read-only info - same "only BACK/enterable
          // rows do anything" shape as MENU_STATE_URNS_PATH_DETAIL.
        } else if (menu_state == MENU_STATE_GNSS_DIAG_SATS) {
          // Single fixed list, nothing to drill into further - any confirm
          // on a sat row or BACK both just return, same "flat leaf list"
          // shape as MENU_STATE_MEM_DETAIL.
          menu_state = MENU_STATE_GNSS_DIAG;
        }
      #endif
    #endif
    #if HAS_RTC == true || HAS_GPS == true
      else if (menu_state == MENU_STATE_RTC_TZ_EDIT) {
        // Commits straight to EEPROM here rather than staging until SAVE &
        // EXIT - display-only, nothing to reboot or re-init, same
        // immediate-commit pattern as Set Time/Date above. Returns to
        // whichever list opened it - the RTC page's or the GNSS page's own
        // Timezone row (tz_edit_return_state).
        uint8_t live_raw = (uint8_t)(get_tz_offset_qh() + TZ_OFFSET_RAW_ZERO);
        uint8_t new_raw  = (uint8_t)(staged_tz_offset_qh + TZ_OFFSET_RAW_ZERO);
        if (new_raw != live_raw) { tz_conf_save(new_raw); }
        menu_state = tz_edit_return_state;
      }
    #endif
    #if HAS_ESPNOW == true
      else if (menu_state == MENU_STATE_ESPNOW_LIST) {
        if (espnow_menu_cursor == ESPNOW_ITEM_BACK) {
          menu_state = MENU_STATE_LIST;
        } else if (espnow_menu_cursor == ESPNOW_ITEM_MODE
                   #if HAS_URNS == true
                     && staged_urns_interface_mode == URNS_INTERFACE_LORA_ONLY
                   #endif
                  ) {
          menu_state = MENU_STATE_ESPNOW_EDIT;
        } else if (espnow_menu_cursor == ESPNOW_ITEM_ENABLED ||
                   espnow_menu_cursor == ESPNOW_ITEM_LR ||
                   espnow_menu_cursor == ESPNOW_ITEM_CHANNEL) {
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
    #if HAS_URNS == true
      else if (menu_state == MENU_STATE_URNS_LIST) {
        if (urns_menu_cursor == URNS_ITEM_BACK) {
          menu_state = MENU_STATE_LIST;
        } else if (urns_menu_cursor == URNS_ITEM_PROBE_DEST) {
          // Requires Transport Mode - RNS::Transport::start() (both the
          // vendored C++ port and upstream Python RNS, confirmed against
          // ~/Development/Reticulum/RNS/Transport.py) only ever constructs
          // the actual probe-responder Destination inside its own
          // transport_enabled() branch. Turning this on without Transport
          // Mode also on sets a flag with no real destination behind it -
          // confirmed live on hardware to crash (Interrupt WDT panic,
          // something downstream still touches the never-constructed
          // Destination). Checks the *staged* value, not the live one, so
          // a user can still turn both on together in one menu session
          // before SAVE & EXIT reboots into the new state.
          if (staged_urns_transport_enabled) { menu_state = MENU_STATE_URNS_EDIT; }
        } else if (urns_menu_cursor == URNS_ITEM_ENABLED || urns_menu_cursor == URNS_ITEM_TRANSPORT ||
                   #if HAS_ESPNOW == true
                   urns_menu_cursor == URNS_ITEM_INTERFACE ||
                   #endif
                   urns_menu_cursor == URNS_ITEM_LINK_MTU_DISCOVERY || urns_menu_cursor == URNS_ITEM_REMOTE_MGMT) {
          menu_state = MENU_STATE_URNS_EDIT;
        } else if (urns_menu_cursor == URNS_ITEM_PATHS) {
          menu_state = MENU_STATE_URNS_PATHS;
          urns_paths_menu_cursor = 0;
        } else if (urns_menu_cursor == URNS_ITEM_FREE) {
          urns_free_detail_refresh();
          urns_free_detail_cursor = URNS_FREE_DETAIL_ITEM_BACK;
          menu_state = MENU_STATE_URNS_FREE_DETAIL;
        }
      } else if (menu_state == MENU_STATE_URNS_FREE_DETAIL) {
        // All rows read-only info except BACK - same shape as
        // MENU_STATE_URNS_PATH_DETAIL's Expiry/Hash split above.
        if (urns_free_detail_cursor == URNS_FREE_DETAIL_ITEM_BACK) {
          menu_state = MENU_STATE_URNS_LIST;
        }
      } else if (menu_state == MENU_STATE_URNS_EDIT) {
        // Same deferred-commit reasoning as ESP-NOW's own Enabled field -
        // urns_init()/urns_radio_bringup() are boot-only, so nothing is
        // written here, only staged.
        menu_state = MENU_STATE_URNS_LIST;
      } else if (menu_state == MENU_STATE_URNS_PATHS) {
        uint8_t row_count = urns_path_display_row_count();
        if (urns_paths_menu_cursor == row_count - 1) {
          menu_state = MENU_STATE_URNS_LIST;
        } else if (RNS::Transport::new_path_table().size() > 0) {
          // A real path row, not the inert "No Paths" placeholder (which
          // only ever sits at index 0 when the table's empty - cursor==0
          // falls through to here doing nothing in that case since this
          // whole branch is skipped when size()==0). The list only shows
          // the first 8 hex chars, so re-walk the store up to the
          // selected index to capture its full hash - MENU_STATE_URNS_
          // PATH_DETAIL looks it back up by key on every draw call rather
          // than being handed a copy here.
          RNS::Persistence::NewPathTable& pt = const_cast<RNS::Persistence::NewPathTable&>(RNS::Transport::new_path_table());
          uint8_t i = 0;
          for (auto it = pt.begin(); it != pt.end(); ++it, i++) {
            if (i == urns_paths_menu_cursor) {
              urns_path_detail_hash = (*it).key;
              break;
            }
          }
          urns_path_detail_cursor = 0;
          menu_state = MENU_STATE_URNS_PATH_DETAIL;
        }
      } else if (menu_state == MENU_STATE_URNS_PATH_DETAIL) {
        // Expiry is read-only info - only Hash (opens the full-hash view)
        // and BACK do anything.
        if (urns_path_detail_cursor == URNS_PATH_DETAIL_ITEM_BACK) {
          menu_state = MENU_STATE_URNS_PATHS;
        } else if (urns_path_detail_cursor == URNS_PATH_DETAIL_ITEM_HASH) {
          menu_state = MENU_STATE_URNS_PATH_HASH_VIEW;
        }
      } else if (menu_state == MENU_STATE_URNS_PATH_HASH_VIEW) {
        // A single fixed view, nothing to select - any confirm just
        // dismisses it, same as MENU_STATE_STATUS_POPUP.
        menu_state = MENU_STATE_URNS_PATH_DETAIL;
      }
      #if HAS_LXMF == true
      else if (menu_state == MENU_STATE_MSNGR_LIST) {
        if (msngr_menu_cursor == MSNGR_TOP_ITEM_BACK) {
          menu_state = MENU_STATE_LIST;
        } else if (msngr_menu_cursor == MSNGR_TOP_ITEM_INBOX) {
          menu_state = MENU_STATE_MSNGR_INBOX;
          msngr_inbox_cursor = 0;
        } else if (msngr_menu_cursor == MSNGR_TOP_ITEM_BOOKMARKS) {
          menu_state = MENU_STATE_MSNGR_BOOKMARKS;
          msngr_bookmarks_cursor = 0;
        } else if (msngr_menu_cursor == MSNGR_TOP_ITEM_ANNOUNCES) {
          menu_state = MENU_STATE_MSNGR_ANNOUNCES;
          msngr_announces_cursor = 0;
        } else if (msngr_menu_cursor == MSNGR_TOP_ITEM_ANNOUNCE_NODE) {
          // Announces our own LXMF delivery destination (display name +
          // stamp cost via LXMRouter::announce()'s own app_data build),
          // not urns_destination's separate Phase 1 test destination -
          // see urns_announce()'s own two-part comment (URNS.h) for why
          // those are kept distinct.
          if (urns_ready && urns_lxmf_router) {
            urns_lxmf_router->announce();
            menu_open_popup("ANNOUNCED", MENU_STATE_MSNGR_LIST);
            menu_popup_auto_dismiss_at = millis() + MSNGR_ANNOUNCE_POPUP_MS;
          } else {
            menu_open_popup("NOT READY", MENU_STATE_MSNGR_LIST);
          }
        } else if (msngr_menu_cursor == MSNGR_TOP_ITEM_SETTINGS) {
          menu_state = MENU_STATE_MSNGR_SETTINGS;
          msngr_settings_cursor = 0;
        }
      } else if (menu_state == MENU_STATE_MSNGR_SETTINGS) {
        if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_RETRIES ||
            msngr_settings_cursor == MSNGR_SETTINGS_ITEM_RETRY_DELAY ||
            msngr_settings_cursor == MSNGR_SETTINGS_ITEM_ANNOUNCE_START ||
            msngr_settings_cursor == MSNGR_SETTINGS_ITEM_ANNOUNCE_INTERVAL) {
          menu_state = MENU_STATE_MSNGR_SETTINGS_EDIT;
        } else if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_DISPLAY_NAME) {
          // Pre-populated with the current live name (custom or computed
          // default, whichever urns_lxmf_display_name() resolved to at
          // boot - LXMRouter::display_name() always reflects whichever one
          // is actually active) so opening this always shows a real
          // starting value, never blank - satisfies "default value should
          // still be populated" without needing separate placeholder text.
          msngr_text_entry_purpose = MSNGR_TEXT_ENTRY_PURPOSE_DISPLAY_NAME;
          msngr_text_entry_buf[0] = 0;
          if (urns_lxmf_router) {
            strncpy(msngr_text_entry_buf, urns_lxmf_router->display_name().c_str(), MSNGR_TEXT_ENTRY_MAX_LEN);
            msngr_text_entry_buf[MSNGR_TEXT_ENTRY_MAX_LEN] = 0;
          }
          msngr_kb_cursor = 0;
          msngr_kb_shift_on = false;
          msngr_kb_lang_ru = false;
          menu_state = MENU_STATE_MSNGR_TEXT_ENTRY;
        } else if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_PRESETS) {
          msngr_presets_cursor = 0;
          menu_state = MENU_STATE_MSNGR_PRESETS;
        } else if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_BACK) {
          if (staged_msngr_max_retries != msngr_max_retries) {
            msngr_retries_conf_save(staged_msngr_max_retries);
          }
          if (staged_msngr_retry_delay_s != msngr_retry_delay_s) {
            msngr_retry_delay_conf_save(staged_msngr_retry_delay_s);
          }
          if (staged_msngr_announce_at_start != msngr_announce_at_start) {
            msngr_announce_at_start_conf_save(staged_msngr_announce_at_start);
          }
          if (staged_msngr_announce_interval_idx != msngr_announce_interval_idx) {
            msngr_announce_interval_conf_save(staged_msngr_announce_interval_idx);
          }
          menu_state = MENU_STATE_MSNGR_LIST;
        }
      } else if (menu_state == MENU_STATE_MSNGR_SETTINGS_EDIT) {
        // Confirms the staged value, no write yet - same "OK just returns
        // to the list, actual eeprom_update() happens on the list's own
        // BACK row" shape as MENU_STATE_BT_SETTINGS_EDIT.
        menu_state = MENU_STATE_MSNGR_SETTINGS;
      } else if (menu_state == MENU_STATE_MSNGR_PRESETS) {
        uint8_t row_count = msngr_presets_row_count();
        bool has_add_row = msngr_preset_count < MSNGR_MAX_PRESETS;
        if (msngr_presets_cursor == row_count - 1) {
          menu_state = MENU_STATE_MSNGR_SETTINGS;
        } else if (has_add_row && msngr_presets_cursor == msngr_preset_count) {
          // "Add Preset" row - same reset-and-open sequence Display
          // Name/Reply already use, blank buffer (nothing to pre-fill
          // for a brand new preset) and the append sentinel (see
          // msngr_preset_edit_index's own declaration).
          msngr_text_entry_purpose = MSNGR_TEXT_ENTRY_PURPOSE_PRESET;
          msngr_preset_edit_index = msngr_preset_count;
          msngr_text_entry_buf[0] = 0;
          msngr_kb_cursor = 0;
          msngr_kb_shift_on = false;
          msngr_kb_lang_ru = false;
          menu_state = MENU_STATE_MSNGR_TEXT_ENTRY;
        } else if (msngr_presets_cursor < msngr_preset_count) {
          msngr_preset_detail_index = msngr_presets_cursor;
          msngr_preset_detail_cursor = 0;
          menu_state = MENU_STATE_MSNGR_PRESET_DETAIL;
        }
      } else if (menu_state == MENU_STATE_MSNGR_PRESET_DETAIL) {
        if (msngr_preset_detail_cursor == 0) { // Edit
          msngr_text_entry_purpose = MSNGR_TEXT_ENTRY_PURPOSE_PRESET;
          msngr_preset_edit_index = msngr_preset_detail_index;
          snprintf(msngr_text_entry_buf, MSNGR_TEXT_ENTRY_MAX_LEN + 1, "%s", msngr_presets[msngr_preset_detail_index]);
          msngr_kb_cursor = 0;
          msngr_kb_shift_on = false;
          msngr_kb_lang_ru = false;
          menu_state = MENU_STATE_MSNGR_TEXT_ENTRY;
        } else if (msngr_preset_detail_cursor == 1) { // Delete
          // Immediate, no confirm dialog - unlike MENU_STATE_MSNGR_
          // DELETE_CONFIRM (a received message, permanent data loss), a
          // preset is trivially retypable, same directness bookmark
          // removal already gets away with.
          messenger_preset_delete(msngr_preset_detail_index);
          menu_state = MENU_STATE_MSNGR_PRESETS;
        } else { // BACK
          menu_state = MENU_STATE_MSNGR_PRESETS;
        }
      } else if (menu_state == MENU_STATE_MSNGR_INBOX) {
        uint8_t row_count = msngr_inbox_row_count();
        if (msngr_inbox_cursor == row_count - 1) {
          menu_state = MENU_STATE_MSNGR_LIST;
        } else if (urns_message_store && urns_message_store->get_conversation_count() > 0) {
          std::vector<RNS::Bytes> convs = urns_message_store->get_conversations();
          if (msngr_inbox_cursor < convs.size()) {
            msngr_active_peer_hash = convs[msngr_inbox_cursor];
            urns_message_store->mark_conversation_read(msngr_active_peer_hash);
            msngr_peer_return_state = MENU_STATE_MSNGR_INBOX;
            msngr_peer_cursor = 0;
            msngr_last_send_result = 0xFF;
            messenger_refresh_peer_cache(msngr_active_peer_hash);
            menu_state = MENU_STATE_MSNGR_PEER;
          }
        }
      } else if (menu_state == MENU_STATE_MSNGR_BOOKMARKS) {
        uint8_t row_count = msngr_bookmarks_row_count();
        if (msngr_bookmarks_cursor == row_count - 1) {
          menu_state = MENU_STATE_MSNGR_LIST;
        } else if (msngr_bookmark_count > 0) {
          uint8_t vis = 0;
          for (uint8_t i = 0; i < MSNGR_MAX_BOOKMARKS; i++) {
            if (!msngr_bookmarks[i].in_use) continue;
            if (vis == msngr_bookmarks_cursor) {
              msngr_active_peer_hash = RNS::Bytes(msngr_bookmarks[i].hash, LXMF::PEER_HASH_SIZE);
              msngr_peer_return_state = MENU_STATE_MSNGR_BOOKMARKS;
              msngr_peer_cursor = 0;
              msngr_last_send_result = 0xFF;
              messenger_refresh_peer_cache(msngr_active_peer_hash);
              menu_state = MENU_STATE_MSNGR_PEER;
              break;
            }
            vis++;
          }
        }
      } else if (menu_state == MENU_STATE_MSNGR_ANNOUNCES) {
        uint8_t row_count = msngr_announces_row_count();
        if (msngr_announces_cursor == row_count - 1) {
          menu_state = MENU_STATE_MSNGR_LIST;
        } else {
          uint8_t vis = 0;
          for (uint8_t i = 0; i < MSNGR_MAX_ANNOUNCES; i++) {
            if (!msngr_announces[i].in_use) continue;
            if (vis == msngr_announces_cursor) {
              msngr_active_peer_hash = RNS::Bytes(msngr_announces[i].hash, LXMF::PEER_HASH_SIZE);
              msngr_peer_return_state = MENU_STATE_MSNGR_ANNOUNCES;
              msngr_peer_cursor = 0;
              msngr_last_send_result = 0xFF;
              messenger_refresh_peer_cache(msngr_active_peer_hash);
              menu_state = MENU_STATE_MSNGR_PEER;
              break;
            }
            vis++;
          }
        }
      } else if (menu_state == MENU_STATE_MSNGR_PEER) {
        uint8_t msg_rows = msngr_peer_msg_row_count();
        if (msngr_peer_cursor < msg_rows) {
          // Reads msngr_peer_cache (Messenger.h), not MessageStore
          // directly - see that cache's own comment for why.
          RNS::Bytes msg_hash(msngr_peer_cache[msngr_peer_cursor].hash, LXMF::MESSAGE_HASH_SIZE);
          messenger_refresh_msg_detail_cache(msg_hash);
          msngr_active_message_hash = msg_hash;
          msngr_msg_detail_cursor = 0;
          menu_state = MENU_STATE_MSNGR_MSG_DETAIL;
        } else {
          uint8_t action = msngr_peer_cursor - msg_rows;
          if (action < msngr_preset_count) {
            msngr_last_send_result = messenger_send_lxmf(msngr_active_peer_hash, msngr_presets[action]);
            if (msngr_last_send_result == URNS_LXMF_SEND_OK || msngr_last_send_result == URNS_LXMF_SEND_RESOLVING) {
              // A new (outgoing) message was just saved (OK) - or identity/
              // path is still being resolved and nothing exists to show yet
              // (RESOLVING, see messenger_send_lxmf()'s own comment,
              // Messenger.h) - either way MENU_STATE_MSNGR_SEND_RESULT's own
              // draw code reads msngr_send_state live and shows the right
              // status text ("Resolving..."/"Sending..."/etc). The cache
              // refresh itself is polled (msngr_send_result_process() below)
              // rather than called here directly, since the RESOLVING path
              // only actually saves a message later, asynchronously, from
              // inside Messenger.h where this function isn't visible yet.
              msngr_send_result_cursor = 1; // default BACK - see its own declaration
              menu_state = MENU_STATE_MSNGR_SEND_RESULT;
            } else {
              menu_open_popup(urns_lxmf_send_result_text(msngr_last_send_result), MENU_STATE_MSNGR_PEER);
              menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
            }
          } else {
            uint8_t fixed_action = action - msngr_preset_count;
            if (fixed_action == MSNGR_PEER_FIXED_ACTION_BACK) {
              menu_state = msngr_peer_return_state;
            } else if (fixed_action == MSNGR_PEER_FIXED_ACTION_BOOKMARK) {
              if (messenger_bookmark_find(msngr_active_peer_hash) >= 0) {
                messenger_bookmark_remove(msngr_active_peer_hash);
              } else {
                messenger_bookmark_add(msngr_active_peer_hash, messenger_peer_display_name(msngr_active_peer_hash));
              }
            } else if (fixed_action == MSNGR_PEER_FIXED_ACTION_CLEAR) {
              msngr_clear_confirm_cursor = 1; // default CANCEL - see its own declaration
              menu_state = MENU_STATE_MSNGR_CLEAR_CONFIRM;
            } else if (fixed_action == MSNGR_PEER_FIXED_ACTION_PING) {
              messenger_ping_start(msngr_active_peer_hash);
              msngr_ping_result_cursor = 1; // default BACK - see its own declaration
              menu_state = MENU_STATE_MSNGR_PING_RESULT;
            } else if (fixed_action == MSNGR_PEER_FIXED_ACTION_SEND_CUSTOM) {
              // Explicit reset, not a reliance on MSNGR_TEXT_ENTRY_
              // PURPOSE_MESSAGE just happening to be the compile-time
              // default (0) - now that DISPLAY_NAME/PRESET entry points
              // both set this away from MESSAGE, this screen needs to
              // set it back explicitly too, or a stale purpose from
              // whichever of those was opened most recently would leak
              // into this compose session.
              msngr_text_entry_purpose = MSNGR_TEXT_ENTRY_PURPOSE_MESSAGE;
              msngr_kb_cursor = 0;
              msngr_kb_shift_on = false;
              msngr_kb_lang_ru = false;
              msngr_text_entry_buf[0] = 0;
              menu_state = MENU_STATE_MSNGR_TEXT_ENTRY;
            }
          }
        }
      } else if (menu_state == MENU_STATE_MSNGR_MSG_DETAIL) {
        // Content lines are read-only - only the trailing REPLY/DELETE/
        // BACK rows do anything.
        uint8_t row_count = msngr_msg_detail_row_count();
        if (msngr_msg_detail_cursor == row_count - 1) {
          menu_state = MENU_STATE_MSNGR_PEER;
        } else if (msngr_msg_detail_cursor == row_count - 2) {
          msngr_delete_confirm_cursor = 1; // default CANCEL - see its own declaration
          menu_state = MENU_STATE_MSNGR_DELETE_CONFIRM;
        } else if (msngr_msg_detail_cursor == row_count - 3) {
          // Same reset sequence MSNGR_PEER_FIXED_ACTION_SEND_CUSTOM uses to
          // open the keyboard fresh - msngr_active_peer_hash is already
          // this message's own peer (set on entering MENU_STATE_MSNGR_
          // PEER, never touched by MSG_DETAIL), so the composed reply
          // goes to the right destination without needing to look
          // anything up again here. Purpose reset explicitly, not left
          // to the compile-time default - see MSNGR_PEER_FIXED_ACTION_
          // SEND_CUSTOM's own matching comment on the peer screen.
          msngr_text_entry_purpose = MSNGR_TEXT_ENTRY_PURPOSE_MESSAGE;
          msngr_kb_cursor = 0;
          msngr_kb_shift_on = false;
          msngr_kb_lang_ru = false;
          msngr_text_entry_buf[0] = 0;
          menu_state = MENU_STATE_MSNGR_TEXT_ENTRY;
        }
      } else if (menu_state == MENU_STATE_MSNGR_DELETE_CONFIRM) {
        if (msngr_delete_confirm_cursor == 0) { // DELETE
          // Masked for the whole delete+cache-refresh sequence - both do
          // LittleFS I/O (MessageStore/microStore), and a real crash was
          // confirmed live where the DIO0 RX interrupt fired mid-flash-op
          // (ESP-IDF suspends the scheduler and disables interrupts/cache
          // briefly during any flash read/write) and tried to do SPI from
          // true ISR context, hitting FreeRTOS's own hard assert on taking
          // a blocking semaphore with the scheduler suspended - not a hang,
          // an immediate abort(). See feedback_dio0_isr_does_spi_work /
          // feedback_sx126x_tx_rx_spi_mutex_race memory - same root class
          // of hazard (handleDio0Rise() does blocking SPI in real ISR
          // context), a new trigger (flash I/O, not just TX).
          LoRa->maskDio0();
          if (urns_message_store) urns_message_store->delete_message(msngr_active_message_hash);
          // The message list this peer's screen shows just shrank by one -
          // refresh the cache and land the cursor back at the top of it
          // rather than risking a stale index into a now-shorter list.
          messenger_refresh_peer_cache(msngr_active_peer_hash);
          LoRa->unmaskDio0();
          msngr_peer_cursor = 0;
          menu_open_popup("DELETED", MENU_STATE_MSNGR_PEER);
          menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
        } else { // CANCEL
          menu_state = MENU_STATE_MSNGR_MSG_DETAIL;
        }
      } else if (menu_state == MENU_STATE_MSNGR_CLEAR_CONFIRM) {
        if (msngr_clear_confirm_cursor == 0) { // CLEAR
          // See the DELETE branch's own comment just above - same hazard,
          // and the one actually confirmed to crash live (delete_conversation
          // does much more LittleFS I/O per call than delete_message, so it
          // was far more likely to hit the race).
          LoRa->maskDio0();
          if (urns_message_store) urns_message_store->delete_conversation(msngr_active_peer_hash);
          messenger_refresh_peer_cache(msngr_active_peer_hash);
          LoRa->unmaskDio0();
          msngr_peer_cursor = 0;
          menu_open_popup("CLEARED", MENU_STATE_MSNGR_PEER);
          menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
        } else { // CANCEL
          menu_state = MENU_STATE_MSNGR_PEER;
        }
      } else if (menu_state == MENU_STATE_MSNGR_TEXT_ENTRY) {
        // "Presses" whichever key msngr_kb_cursor is currently highlighting -
        // insert/space/backspace mutate msngr_text_entry_buf in place (always
        // appending/trimming at the end, no mid-string edit point, same
        // simplification meshtastic's own VirtualKeyboard makes). Shift is a
        // persistent toggle here rather than meshtastic's one-shot long-press,
        // since confirm_select() is already spoken for as "press this key".
        uint8_t kb_row = msngr_kb_cursor / MSNGR_KB_COLS;
        uint8_t kb_col = msngr_kb_cursor % MSNGR_KB_COLS;
        char key_ch = (msngr_kb_lang_ru ? MSNGR_KB_LAYOUT_RU : MSNGR_KB_LAYOUT)[kb_row][kb_col];
        uint8_t key_type = msngr_kb_key_type(key_ch);
        size_t text_len = strlen(msngr_text_entry_buf);

        if (key_type == MSNGR_KB_CHAR || key_type == MSNGR_KB_SPACE) {
          if (msngr_kb_alt_hold_fired_btn || msngr_kb_alt_hold_fired_enc) {
            // Already inserted the paired punctuation mark live, mid-hold
            // (msngr_kb_alt_hold_try(), see its own comment) - this
            // release is just the tail end of that gesture, not a fresh
            // press, so it shouldn't also insert the key's own plain
            // character on top of it.
            msngr_kb_alt_hold_fired_btn = false;
            msngr_kb_alt_hold_fired_enc = false;
          } else {
            // Display Name uses the same practical cap as bookmark/announce
            // names elsewhere in this file (MSNGR_NAME_MAX_LEN=31) - the
            // protocol's own single-announce-packet ceiling is much higher
            // (~274-280 bytes, derived from Type::Reticulum::MTU/HEADER_
            // MAXSIZE/IFAC_MIN_SIZE minus the announce's fixed identity/
            // signature/ratchet fields), but that's not a sane UI limit for
            // a name field. Message composing keeps the higher MSNGR_TEXT_
            // ENTRY_MAX_LEN (140) unchanged.
            size_t max_len = (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_DISPLAY_NAME ||
                               msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_PRESET)
              ? MSNGR_NAME_MAX_LEN : MSNGR_TEXT_ENTRY_MAX_LEN;
            if (text_len < max_len) {
              char c = (key_type == MSNGR_KB_SPACE) ? ' ' : msngr_kb_apply_shift(key_ch, msngr_kb_shift_on);
              msngr_text_entry_buf[text_len] = c;
              msngr_text_entry_buf[text_len + 1] = 0;
            }
          }
        } else if (key_type == MSNGR_KB_BACKSPACE) {
          if (msngr_kb_del_hold_fired_btn || msngr_kb_del_hold_fired_enc) {
            // Already deleted (at least once, maybe several times) live,
            // mid-hold (msngr_kb_del_hold_try()) - this release is just
            // the tail end of that gesture, not a fresh press, so it
            // shouldn't also delete one more character on top of it.
            msngr_kb_del_hold_fired_btn = false;
            msngr_kb_del_hold_fired_enc = false;
          } else if (text_len > 0) {
            msngr_text_entry_buf[text_len - 1] = 0;
          }
        } else if (key_type == MSNGR_KB_SHIFT) {
          // A quick press still just toggles case (unchanged). A
          // deliberate hold past MSNGR_KB_ALT_HOLD_MS switches layout
          // instead - but that already happened live, mid-hold (see
          // msngr_kb_lang_hold_try(), polled from menu_button_process()/
          // Encoder.h's encoder_process()), not here. This release is
          // just the tail end of that gesture, so it only needs to
          // consume whichever flag fired and skip toggling case - the
          // duration check below is a defensive fallback for a release
          // that somehow crossed the threshold without a live poll
          // catching it first, which shouldn't normally happen since
          // polling runs every loop() tick, far more often than a
          // release can occur.
          if (msngr_kb_lang_hold_fired_btn || msngr_kb_lang_hold_fired_enc) {
            msngr_kb_lang_hold_fired_btn = false;
            msngr_kb_lang_hold_fired_enc = false;
          } else if (duration >= MSNGR_KB_ALT_HOLD_MS) {
            msngr_kb_lang_ru = !msngr_kb_lang_ru;
            msngr_kb_shift_on = false;
          } else {
            msngr_kb_shift_on = !msngr_kb_shift_on;
          }
        } else if (key_type == MSNGR_KB_BACK) {
          menu_msngr_text_entry_leave();
        } else if (key_type == MSNGR_KB_SEND) {
          if (text_len > 0 && msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_DISPLAY_NAME) {
            // No result screen needed - unlike an LXMF send, this can't
            // fail in a way worth reporting (a local file write), so it's
            // save-and-return rather than save-and-show-status. Expanded
            // to real UTF-8 first (msngr_kb_expand_utf8()) - the announce
            // this name goes out in is read by other Reticulum clients,
            // not just this device's own Org_01 glyph table.
            char name_utf8[MSNGR_NAME_MAX_LEN * 2 + 1];
            msngr_kb_expand_utf8(msngr_text_entry_buf, name_utf8, sizeof(name_utf8));
            msngr_display_name_conf_save(name_utf8);
            msngr_text_entry_buf[0] = 0;
            menu_state = MENU_STATE_MSNGR_SETTINGS;
          } else if (text_len > 0 && msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_PRESET) {
            // No UTF-8 expansion needed here unlike the display-name/
            // message branches - this text never leaves the device, same
            // reasoning bookmark names already get away with storing as
            // typed (Messenger.h). msngr_preset_edit_index == msngr_
            // preset_count (set when MENU_STATE_MSNGR_PRESETS' own "Add
            // Preset" row opened this screen) means append a new one;
            // anything less is an existing slot being edited in place -
            // same "index == count means append" sentinel messenger_
            // preset_add() itself uses.
            if (msngr_preset_edit_index >= msngr_preset_count) {
              messenger_preset_add(msngr_text_entry_buf);
            } else {
              messenger_preset_update(msngr_preset_edit_index, msngr_text_entry_buf);
            }
            msngr_text_entry_buf[0] = 0;
            menu_state = MENU_STATE_MSNGR_PRESETS;
          } else if (text_len > 0) {
            // Only actually clear the composed text on a confirmed send -
            // a failure leaves it in place so the user can retry instead
            // of having to retype it. Same MENU_STATE_MSNGR_SEND_RESULT
            // hand-off as the preset Send: Hi/Bye/SOS actions - see that
            // branch's own comment. Expanded to real UTF-8 first, same
            // reasoning as the display-name save above.
            char msg_utf8[MSNGR_TEXT_ENTRY_MAX_LEN * 2 + 1];
            msngr_kb_expand_utf8(msngr_text_entry_buf, msg_utf8, sizeof(msg_utf8));
            msngr_last_send_result = messenger_send_lxmf(msngr_active_peer_hash, msg_utf8);
            if (msngr_last_send_result == URNS_LXMF_SEND_OK || msngr_last_send_result == URNS_LXMF_SEND_RESOLVING) {
              // RESOLVING already has its own copy of this text (msngr_send_
              // pending_content, Messenger.h) independent of this buffer -
              // clear it here same as OK, since the send has meaningfully
              // started either way (see the preset-Send branch's own comment
              // above for why cache refresh isn't called directly here).
              msngr_text_entry_buf[0] = 0;
              msngr_send_result_cursor = 1; // default BACK - see its own declaration
              menu_state = MENU_STATE_MSNGR_SEND_RESULT;
            } else {
              menu_open_popup(urns_lxmf_send_result_text(msngr_last_send_result), MENU_STATE_MSNGR_TEXT_ENTRY);
              menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
            }
          }
        }
      } else if (menu_state == MENU_STATE_MSNGR_DISCARD_CONFIRM) {
        if (msngr_discard_confirm_cursor == 0) { // DISCARD
          msngr_text_entry_buf[0] = 0;
          menu_state = msngr_text_entry_return_state();
        } else { // CANCEL - resume typing, buffer/cursor/shift untouched
          menu_state = MENU_STATE_MSNGR_TEXT_ENTRY;
        }
      } else if (menu_state == MENU_STATE_MSNGR_PING_RESULT) {
        // Row 0 (status) is read-only - only BACK does anything, and it
        // doubles as Cancel while a ping's still in flight.
        if (msngr_ping_result_cursor == 1) {
          messenger_ping_cancel();
          menu_state = MENU_STATE_MSNGR_PEER;
        }
      } else if (menu_state == MENU_STATE_MSNGR_SEND_RESULT) {
        // Row 0 (status) is read-only - only BACK does anything. If the
        // packet's already gone out (PENDING/DELIVERED/TIMEOUT) there's
        // nothing to tear down, same as Ping - this just stops watching
        // for this send's proof so a late-arriving one doesn't affect
        // whatever the screen shows next time it's opened for a different
        // send. If still RESOLVING (waiting on identity/path -
        // messenger_send_lxmf()/_process(), Messenger.h), setting state
        // back to IDLE here doubles as a real cancel: nothing's been sent
        // yet in that case, and messenger_send_process() only acts on
        // MSNGR_SEND_RESOLVING, so the parked message is simply abandoned
        // rather than firing off later without the user watching.
        if (msngr_send_result_cursor == 1) {
          msngr_send_state = MSNGR_SEND_IDLE;
          menu_state = MENU_STATE_MSNGR_PEER;
        }
      }
      #endif
    #endif
      else if (menu_state == MENU_STATE_URNS_RADIO_LIST) {
        if (urns_radio_menu_cursor == URNS_RADIO_ITEM_BACK) {
          menu_state = MENU_STATE_LIST;
        } else if (urns_radio_menu_cursor == URNS_RADIO_ITEM_START) {
          // Fire-and-forget action, not a staged field - acts on the live
          // lora_* globals directly (same ones startRadio() itself reads),
          // not the staged_lora_* copies above, since those only take
          // effect after menu_commit_and_exit()'s eeprom_conf_save() +
          // hard_reset(). Mirrors the host's own CMD_RADIO_STATE 0x00/0x01
          // handlers (RNode_Firmware.ino) - same stopRadio()/startRadio()
          // calls, just triggered from the on-device menu instead of a KISS
          // command. Label itself flips Start/Stop based on radio_online -
          // see the list-view draw code above.
          if (radio_online) {
            stopRadio();
            menu_open_popup("STOPPED", MENU_STATE_URNS_RADIO_LIST);
          } else if (console_active) {
            menu_open_popup("HOST ATTACHED", MENU_STATE_URNS_RADIO_LIST);
          } else {
            bool started = startRadio();
            if (started)          menu_open_popup("STARTED", MENU_STATE_URNS_RADIO_LIST);
            else if (radio_locked) menu_open_popup("UNSET", MENU_STATE_URNS_RADIO_LIST);
            else                   menu_open_popup("FAILED", MENU_STATE_URNS_RADIO_LIST);
          }
          menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
        } else if (urns_radio_menu_cursor == URNS_RADIO_ITEM_CLEAR) {
          // Immediate action, not staged/deferred to the whole menu's own
          // SAVE & EXIT (unlike e.g. WIFI_ITEM_CLEAR above) -
          // eeprom_conf_delete() (Utilities.h) is a single, complete write
          // (ADDR_CONF_OK -> 0x00) with nothing else left to commit
          // afterwards, so there's no reason to make the user Save & Exit
          // (and take another reboot) just to make a clear stick. Also
          // resets live/staged lora_* back to Config.h's own "never
          // configured" values so the list immediately reads "Unset"
          // without needing a reboot to see it take effect.
          if (radio_online) stopRadio();
          lora_freq = 0; lora_bw = 0; lora_sf = 0; lora_cr = 5; lora_txp = 255;
          staged_lora_freq = 0; staged_lora_bw = 0; staged_lora_sf = 0; staged_lora_cr = 5; staged_lora_txp = 255;
          eeprom_conf_delete();
          menu_open_popup("CLEARED", MENU_STATE_URNS_RADIO_LIST);
          menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
        } else {
          urns_radio_seed_defaults_if_unset();
          menu_state = MENU_STATE_URNS_RADIO_EDIT;
        }
      } else if (menu_state == MENU_STATE_URNS_RADIO_EDIT) {
        // Same deferred-commit reasoning as URNS_EDIT above - nothing's
        // written here, only staged; menu_commit_and_exit() applies it and
        // reboots via the existing eeprom_conf_save() (Utilities.h).
        menu_state = MENU_STATE_URNS_RADIO_LIST;
      }
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
        #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
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
      #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
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
          // See OTA.h's ota_handle_install() comment - ota_verify_and_set_
          // boot() itself isn't masked (shared with the pre-radio-init
          // recovery path), so this caller masks around it.
          LoRa->maskDio0();
          bool verified = target && ota_verify_and_set_boot(target);
          LoRa->unmaskDio0();
          if (verified) {
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
  // same as an encoder short-click. 200-499ms is a dead zone (no-op) so an
  // imprecise press doesn't do either by accident. Short-tap boundary
  // loosened from 150 to 200 alongside MENU_BTN_DOUBLE_TAP_WINDOW (per user
  // feedback - navigation felt too tight to land reliably).
  void menu_button_press(unsigned long duration) {
    menu_last_activity_ms = millis();
    display_unblank();
    if (menu_state == MENU_STATE_STATUS_POPUP) {
      // Not a real navigable screen - a single short tap dismisses it
      // immediately, no need for the usual double-tap-pending wait
      // (there's nothing to go "back" from here) or to wait for a long
      // press. See MENU_POPUP_DISMISS_MIN_MS's own comment for why this
      // doesn't reuse the normal 200ms short-tap dead zone.
      if (duration >= MENU_POPUP_DISMISS_MIN_MS) {
        menu_btn_pending = false;
        buzzer_encoder_click_melody();
        menu_state = menu_popup_return_state;
      }
      return;
    }
    if (duration < 200) {
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
      // Skip the usual click if the hold already beeped for itself -
      // see menu_encoder_button()'s own matching comment (same reasoning,
      // just this control's own _btn flags instead of _enc).
      #if HAS_LXMF == true
        bool msngr_kb_alt_already_beeped = msngr_kb_lang_hold_fired_btn || msngr_kb_alt_hold_fired_btn || msngr_kb_del_hold_fired_btn;
      #else
        bool msngr_kb_alt_already_beeped = false;
      #endif
      if (menu_state != MENU_STATE_CLOSED && !msngr_kb_alt_already_beeped) buzzer_encoder_click_melody();
      menu_confirm_select(duration);
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
    #if HAS_LXMF == true
      // Live EN/RU switch, punctuation/letter-alternate, and DEL-repeat
      // on a held main button - see msngr_kb_lang_hold_try()/msngr_kb_alt_
      // hold_try()/msngr_kb_del_hold_try()'s own comments. Encoder
      // boards get the same behavior from their encoder's own button
      // via Encoder.h's encoder_process(), polled there instead since
      // button_pressed()/button_down_last (Input.h) only ever reflect
      // this main button, not the encoder's separate physical button.
      if (button_pressed()) {
        unsigned long held_ms = millis() - button_down_last;
        msngr_kb_lang_hold_try(held_ms, msngr_kb_lang_hold_fired_btn);
        msngr_kb_alt_hold_try(held_ms, msngr_kb_alt_hold_fired_btn);
        msngr_kb_del_hold_try(held_ms, msngr_kb_del_hold_fired_btn, msngr_kb_del_repeat_last_btn);
      } else {
        msngr_kb_lang_hold_fired_btn = false;
        msngr_kb_alt_hold_fired_btn = false;
        msngr_kb_del_hold_fired_btn = false;
        msngr_kb_del_repeat_last_btn = 0;
      }
    #endif
  }

  // Org_01 glyphs sit 4px above and 1px below the setCursor() baseline, so
  // the highlight rect must start a few px above the row's text baseline,
  // not right at it, or the glyph tops get clipped by the rect's own top
  // edge. Shows up to 4 rows at a time, scrolling to keep the cursor
  // visible - lists have grown past 4 items and will likely keep growing.
  // icons/icon_widths are parallel arrays of length count, entries nullptr/0
  // for rows with no icon - both left at their default (nullptr) by every
  // call site except draw_settings_menu_disp()'s top-level list.
  // icon_dx is a per-row nudge (px, from MENU_ROW_ICON_X) for glyphs that
  // sit visually off-center in their reserved column just from being
  // narrower than MENU_ROW_ICON_COL_W and left-anchored, e.g. bm_menu_icon_urns
  // (7px, the narrowest) - default nullptr/0, same opt-in pattern as icons.
  // icon_col_shared: true (default) reserves MENU_ROW_TEXT_X_ICONS for every
  // row once any icons[] table is passed at all - right for lists where
  // most/every row has its own icon (RNODE SETTINGS, MESSENGER's top
  // screen), so a future icon-less row would still land in the same
  // column. false shifts only the individual rows that actually have an
  // icon (icons[i] set) and leaves the rest at the plain x=8 the list used
  // before any of its rows had one - for a list like GNSS where a single
  // row (Show GNSS Banner) is the exception, not the rule.
  // text_dx is a per-row nudge (px) added on top of whatever text_x the
  // rules above already picked - unlike icon_dx (which only re-centers a
  // narrow icon inside its already-reserved column), this widens the gap
  // itself for one row without touching MENU_ROW_ICON_COL_W/
  // MENU_ROW_TEXT_X_ICONS and so without shifting every other row in the
  // list too - e.g. MESSENGER's Inbox row, whose bm_menu_icon_inbox glyph
  // (Graphics.h) reads visually wider than MENU_ICON_W_INBOX gives it
  // credit for. Default nullptr/0, same opt-in pattern as icon_dx.
  void draw_menu_list_disp(const char *title, const char **labels, char valbufs[][24], uint8_t count, uint8_t cursor, const uint8_t **icons = nullptr, const uint8_t *icon_widths = nullptr, const int8_t *icon_dx = nullptr, bool icon_col_shared = true, const int8_t *text_dx = nullptr, const uint8_t **right_icons = nullptr, const uint8_t *right_icon_widths = nullptr) {
    MENU_GFX.setFont(MENU_FONT);
    MENU_GFX.setTextSize(1);
    MENU_GFX.setTextColor(SSD1306_WHITE);
    MENU_GFX.setCursor(6, MENU_HEADER_TEXT_Y);
    MENU_GFX.print(title);
    MENU_GFX.drawFastHLine(4, MENU_HEADER_HLINE_Y, MENU_CONTENT_W, SSD1306_WHITE);

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
      bool selected = (i == cursor);
      if (selected) {
        MENU_GFX.fillRect(4, row_top, MENU_CONTENT_W, row_h - 1, SSD1306_WHITE);
        MENU_GFX.setTextColor(SSD1306_BLACK);
      } else {
        MENU_GFX.setTextColor(SSD1306_WHITE);
      }

      uint8_t text_x = 8;
      const uint8_t *row_icon = nullptr;
      uint8_t row_icon_w = 0;
      uint8_t icon_x = MENU_ROW_ICON_X;
      if (icons && icon_col_shared) {
        // Explicit per-list table (RNODE SETTINGS, MESSENGER's top screen)
        // - column reserved uniformly across every row so labels stay
        // aligned whether or not that particular row has art.
        text_x = MENU_ROW_TEXT_X_ICONS;
        if (icons[i]) {
          row_icon = icons[i];
          row_icon_w = icon_widths[i];
          icon_x += icon_dx ? icon_dx[i] : 0;
        }
      } else if (icons && icons[i]) {
        // icon_col_shared == false - a single-icon exception (GNSS's Show
        // GNSS Banner row): only this row's own label shifts to make room
        // for its icon, every other row keeps the plain x=8 the list used
        // before this row existed.
        text_x = MENU_ROW_TEXT_X_ICONS;
        row_icon = icons[i];
        row_icon_w = icon_widths[i];
        icon_x += icon_dx ? icon_dx[i] : 0;
      } else if (strcmp(labels[i], "BACK") == 0) {
        // No explicit table (every plain submenu list) - BACK still gets
        // bm_menu_icon_back automatically, so none of its ~20 call sites
        // need to opt in individually. Indented just enough for its own
        // narrow (9px) glyph rather than the wide shared column above,
        // which would collide with this list's own longer labels/inline
        // values (e.g. URNS's "Remote Management" + ON/OFF) - see
        // MENU_BACK_TEXT_X's own comment.
        row_icon = bm_menu_icon_back;
        row_icon_w = MENU_ICON_W_BACK;
        text_x = MENU_BACK_TEXT_X;
      }
      if (text_dx) text_x += text_dx[i];
      if (row_icon) {
        // Vertically centered in the row rather than top-aligned - on the
        // 11px-row boards row_h - MENU_ICON_H (10px) is only 1px of slack,
        // so this rounds down to the same top-aligned position it always
        // was there. T114's much taller 20px row (bigger Tamsyn6x12 font)
        // is why this can't just stay a fixed offset: 10px of slack pinned
        // the icon visibly to the top-left of the row, confirmed on real
        // hardware.
        int16_t icon_y = row_top + (row_h - MENU_ICON_H) / 2 + MENU_ICON_Y_NUDGE;
        uint16_t fg = selected ? SSD1306_BLACK : SSD1306_WHITE;
        uint16_t bg = selected ? SSD1306_WHITE : SSD1306_BLACK;
        MENU_GFX.drawBitmap(icon_x, icon_y, row_icon, row_icon_w, MENU_ICON_H, fg, bg);
      }
      MENU_GFX.setCursor(text_x, y);
      MENU_GFX.print(labels[i]);

      if (valbufs[i][0] != 0) {
        int16_t x1, y1; uint16_t w, h;
        MENU_GFX.getTextBounds(valbufs[i], 0, 0, &x1, &y1, &w, &h);
        uint8_t val_right = 4 + MENU_CONTENT_W - 2;
        // MENU_STATE_MSNGR_PEER's incoming messages (Menu.h) - reserve room
        // for a trailing direction icon by pulling the value's right edge
        // in first, same "shrink the text side, not the icon" idiom the
        // left-hand icon column above uses.
        if (right_icons && right_icons[i]) val_right -= (right_icon_widths[i] + MENU_ROW_ICON_GAP);
        MENU_GFX.setCursor(val_right - w, y);
        MENU_GFX.print(valbufs[i]);
        if (right_icons && right_icons[i]) {
          int16_t icon_y = row_top + (row_h - MENU_ICON_H) / 2 + MENU_ICON_Y_NUDGE;
          uint16_t fg = selected ? SSD1306_BLACK : SSD1306_WHITE;
          uint16_t bg = selected ? SSD1306_WHITE : SSD1306_BLACK;
          MENU_GFX.drawBitmap(4 + MENU_CONTENT_W - 2 - right_icon_widths[i], icon_y, right_icons[i], right_icon_widths[i], MENU_ICON_H, fg, bg);
        }
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

  #if HAS_URNS == true
    // MENU_STATE_URNS_PATH_HASH_VIEW - the full 32-char path hash, two
    // plain centered lines, no field captions and no BACK row (any input
    // just dismisses it, see menu_confirm_select()/menu_encoder_rotate())
    // - the point of this screen is to be nothing but the hash, easy to
    // read at a glance instead of squeezed into a label+value list row.
    void draw_menu_urns_path_hash_disp() {
      MENU_GFX.setFont(MENU_FONT);
      MENU_GFX.setTextSize(1);
      MENU_GFX.setTextColor(SSD1306_WHITE);

      std::string full_hex = urns_path_detail_hash.toHex();
      std::string line1 = full_hex.size() >= 16 ? full_hex.substr(0, 16) : full_hex;
      std::string line2 = full_hex.size() > 16 ? full_hex.substr(16, 16) : "";

      int16_t x1, y1; uint16_t w1, h1, w2, h2;
      MENU_GFX.getTextBounds(line1.c_str(), 0, 0, &x1, &y1, &w1, &h1);
      MENU_GFX.getTextBounds(line2.c_str(), 0, 0, &x1, &y1, &w2, &h2);

      int16_t cx = MENU_GFX.width() / 2;
      int16_t cy = MENU_GFX.height() / 2;
      MENU_GFX.setCursor(cx - (int16_t)w1 / 2, cy - 6);
      MENU_GFX.print(line1.c_str());
      MENU_GFX.setCursor(cx - (int16_t)w2 / 2, cy + 8);
      MENU_GFX.print(line2.c_str());
    }

  #if HAS_LXMF == true
    // MENU_STATE_MSNGR_TEXT_ENTRY's on-screen keyboard - a real grid (not
    // a draw_menu_list_disp() vertical list), so it gets its own draw
    // function, same as draw_menu_memory_disp() above. Header/footer reuse
    // draw_menu_list_disp()'s own fixed offsets so this screen still looks
    // like part of the same menu; only the middle content (input preview +
    // key grid) is bespoke. Column/row math is a first pass tuned by eye
    // for the generic 128x64/Org_01 combination (same font every HAS_URNS
    // board uses today, see MSNGR_MSG_DETAIL_CHARS_PER_LINE's own comment)
    // - expect this to need live on-hardware nudging like every other
    // pixel-level layout in this file.
    void draw_menu_msngr_keyboard_disp() {
      MENU_GFX.setFont(MENU_FONT);
      MENU_GFX.setTextSize(1);
      MENU_GFX.setTextColor(SSD1306_WHITE);
      // 1px below the usual MENU_HEADER_TEXT_Y baseline every other
      // screen's title uses - the EN/RU indicator box below needs that
      // extra pixel of headroom above its own text or it clips against
      // the very top of the canvas. Local to this screen only; every
      // other menu keeps the shared constant unchanged.
      const int16_t header_y = MENU_HEADER_TEXT_Y + 1;
      MENU_GFX.setCursor(6, header_y);
      MENU_GFX.print(msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_DISPLAY_NAME ? "Name" :
                     msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_PRESET ? "Preset" : "Send Msg");

      // Byte count, right-aligned on the same title line - "(N bytes)",
      // the actual UTF-8 payload size this buffer would send/save as
      // (msngr_kb_utf8_len()) - the number that costs LoRa airtime, not
      // this device's internal 1-byte-per-glyph typing representation.
      // No "/max" denominator - the actual typing cap enforced at input
      // time (msngr_kb_key_type()'s DEL/typing handlers, MSNGR_NAME_MAX_LEN
      // or MSNGR_TEXT_ENTRY_MAX_LEN) is character-based, not byte-based,
      // so pairing it with a byte count here just produced a confusing
      // pair of numbers that don't share a unit.
      //
      // The EN/RU tag itself is drawn as its own filled/inverted square
      // (dark caption on a bright box) rather than plain text - there's
      // no room on the grid to show the active layout (the Shift key's
      // own label is already busy with '^'/'^^', see MSNGR_KB_ALT_HOLD_MS),
      // so this is the one place on the whole screen that has to stand
      // in as the layout indicator, and it reads faster set off in its
      // own box than blended into the surrounding text. Sized off
      // getTextBounds()'s actual measured glyph box (not a guessed
      // padding constant) so it stays correct if the label or font ever
      // changes.
      {
        char count_buf[24];
        snprintf(count_buf, sizeof(count_buf), " (%u bytes)", (unsigned)msngr_kb_utf8_len(msngr_text_entry_buf));
        const char *lang_label = msngr_kb_lang_ru ? "RU" : "EN";

        int16_t lx1, ly1; uint16_t lang_w, lang_h;
        MENU_GFX.getTextBounds(lang_label, 0, header_y, &lx1, &ly1, &lang_w, &lang_h);
        int16_t cx1, cy1; uint16_t count_w, count_h;
        MENU_GFX.getTextBounds(count_buf, 0, 0, &cx1, &cy1, &count_w, &count_h);

        const int16_t pad_x = 2, pad_y = 1;
        const int16_t lang_box_w = (int16_t)lang_w + pad_x * 2;
        const int16_t lang_box_h = (int16_t)lang_h + pad_y * 2;
        const int16_t lang_box_x = 4 + MENU_CONTENT_W - lang_box_w - (int16_t)count_w;
        const int16_t lang_box_y = ly1 - pad_y;

        MENU_GFX.fillRect(lang_box_x, lang_box_y, lang_box_w, lang_box_h, SSD1306_WHITE);
        MENU_GFX.setTextColor(SSD1306_BLACK);
        MENU_GFX.setCursor(lang_box_x + pad_x, header_y);
        MENU_GFX.print(lang_label);

        MENU_GFX.setTextColor(SSD1306_WHITE);
        MENU_GFX.setCursor(lang_box_x + lang_box_w, header_y);
        MENU_GFX.print(count_buf);
      }

      // No header separator line here anymore - the input box's own top
      // border (box_y below, MENU_HEADER_HLINE_Y + 1) sat directly under
      // where this used to draw, so the two 1px lines stacked into a
      // visibly 2px-thick double border. The box border alone is enough
      // separation from the title row above.
      //
      // Input preview - tail of what's typed so far (leading "..." if it
      // doesn't all fit), with a caret after the last character. Same
      // "show the tail, not the head" idea as MSNGR_MSG_DETAIL's word-wrap,
      // just single-line since there's no vertical room to spare here.
      // box_y tracks MENU_HEADER_HLINE_Y (1px below it, same as before it moved).
      const int16_t box_x = 4, box_y = MENU_HEADER_HLINE_Y + 1, box_w = MENU_CONTENT_W, box_h = 9;
      MENU_GFX.drawRect(box_x, box_y, box_w, box_h, SSD1306_WHITE);
      {
        std::string shown(msngr_text_entry_buf);
        size_t full_len = shown.size();
        int16_t x1, y1; uint16_t tw, th;
        MENU_GFX.getTextBounds(shown.c_str(), 0, 0, &x1, &y1, &tw, &th);
        const int16_t max_w = box_w - 6;
        while (tw > (uint16_t)max_w && !shown.empty()) {
          shown.erase(0, 1);
          std::string probe = "..." + shown;
          MENU_GFX.getTextBounds(probe.c_str(), 0, 0, &x1, &y1, &tw, &th);
        }
        if (shown.size() < full_len) shown = "..." + shown;
        MENU_GFX.getTextBounds(shown.c_str(), 0, 0, &x1, &y1, &tw, &th);
        MENU_GFX.setCursor(box_x + 2, box_y + 6); // nudged up 1px, per user request on real hardware
        MENU_GFX.print(shown.c_str());
        int16_t caret_x = box_x + 2 + (int16_t)tw + 1;
        if (caret_x < box_x + box_w - 1) MENU_GFX.drawFastVLine(caret_x, box_y + 1, box_h - 2, SSD1306_WHITE);
      }

      // Keyboard grid - reserve extra width for the last (action) column,
      // sized to the widest action label ("SPACE"), split the rest evenly
      // across the other 10 columns, and hand any leftover pixels to the
      // first few columns - same approach meshtastic's own
      // VirtualKeyboard::draw() uses, just against Adafruit_GFX instead of
      // OLEDDisplay.
      const int16_t grid_top = box_y + box_h + 1;
      const int16_t grid_bottom = MENU_LIST_FOOTER_HLINE_Y;
      const uint8_t row_h = (uint8_t)((grid_bottom - grid_top) / MSNGR_KB_ROWS);

      int16_t x1, y1; uint16_t last_col_w, th;
      MENU_GFX.getTextBounds("SPACE", 0, 0, &x1, &y1, &last_col_w, &th);
      last_col_w += 4;
      const uint8_t left_cols = MSNGR_KB_COLS - 1;
      int16_t usable_w = MENU_CONTENT_W - last_col_w;
      if (usable_w < left_cols) usable_w = left_cols;
      int16_t cell_w = usable_w / left_cols;
      int16_t leftover = usable_w - cell_w * left_cols;

      int16_t col_x[MSNGR_KB_COLS], col_w[MSNGR_KB_COLS];
      int16_t running_x = box_x;
      for (uint8_t c = 0; c < left_cols; c++) {
        int16_t cw = cell_w + (c < leftover ? 1 : 0);
        col_x[c] = running_x;
        col_w[c] = cw;
        running_x += cw;
      }
      col_x[left_cols] = running_x;
      col_w[left_cols] = last_col_w;

      const uint8_t cur_row = msngr_kb_cursor / MSNGR_KB_COLS;
      const uint8_t cur_col = msngr_kb_cursor % MSNGR_KB_COLS;

      for (uint8_t r = 0; r < MSNGR_KB_ROWS; r++) {
        for (uint8_t c = 0; c < MSNGR_KB_COLS; c++) {
          char ch = (msngr_kb_lang_ru ? MSNGR_KB_LAYOUT_RU : MSNGR_KB_LAYOUT)[r][c];
          uint8_t type = msngr_kb_key_type(ch);
          int16_t kx = col_x[c];
          int16_t ky = grid_top + r * row_h;
          int16_t kw = col_w[c];

          char label_buf[2];
          const char *label;
          switch (type) {
            case MSNGR_KB_BACKSPACE: label = "DEL"; break;
            case MSNGR_KB_SEND:      label = "SEND"; break;
            case MSNGR_KB_SPACE:     label = "SPACE"; break;
            case MSNGR_KB_BACK:      label = "BACK"; break;
            case MSNGR_KB_SHIFT:     label = msngr_kb_shift_on ? "^^" : "^"; break;
            default: {
              char c2 = msngr_kb_apply_shift(ch, msngr_kb_shift_on);
              label_buf[0] = c2; label_buf[1] = 0;
              label = label_buf;
              break;
            }
          }

          bool selected = (r == cur_row && c == cur_col);
          if (selected) {
            MENU_GFX.fillRect(kx, ky, kw, row_h - 1, SSD1306_WHITE);
            MENU_GFX.setTextColor(SSD1306_BLACK);
          } else {
            MENU_GFX.setTextColor(SSD1306_WHITE);
          }

          uint16_t lw, lh;
          int16_t lx1, ly1;
          MENU_GFX.getTextBounds(label, 0, 0, &lx1, &ly1, &lw, &lh);
          int16_t label_x = kx + (kw - (int16_t)lw + 1) / 2;
          if (label_x < kx) label_x = kx;
          int16_t label_y = ky + row_h - 3; // nudged up 1px, per user request on real hardware - fits the key boxes better
          MENU_GFX.setCursor(label_x, label_y);
          MENU_GFX.print(label);
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
  #endif

  #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
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
      MENU_GFX.setCursor(6, MENU_HEADER_TEXT_Y);
      MENU_GFX.print("MEMORY");
      MENU_GFX.drawFastHLine(4, MENU_HEADER_HLINE_Y, MENU_CONTENT_W, SSD1306_WHITE);

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
        uint32_t total = 0, free_b = 0;
        bool have_reading = true;
        if (i == MEM_ITEM_HEAP) {
          MENU_GFX.print("Heap");
          #if MCU_VARIANT == MCU_ESP32
            total = ESP.getHeapSize();
            free_b = ESP.getFreeHeap();
          #else // MCU_NRF52 - cores/nRF5/utility/debug.h, always linked
            total = dbgHeapTotal();
            free_b = dbgHeapFree();
          #endif
        }
        #if MCU_VARIANT == MCU_ESP32
          else { // MEM_ITEM_PSRAM
            MENU_GFX.print("PSRAM");
            if (psramFound()) {
              total = ESP.getPsramSize();
              free_b = ESP.getFreePsram();
            } else {
              have_reading = false;
            }
          }
        #endif

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

        #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_WTRACKER_V2 || BOARD_MODEL == BOARD_HELTEC_T1 || BOARD_MODEL == BOARD_HELTEC_T114
          // Heap bar geometry/tier for the push-time colourizer (drawBitmap,
          // Display.h) to pick up - see heap_bar_lit's own declaration
          // comment for why this can't just be computed there instead.
          // Border stays plain white regardless of tier (colourizer only
          // recolours the fillRect'd interior, not the drawRect outline) -
          // per user request. Selected-row highlight (fg==BLACK above)
          // naturally overrides this with no extra code: draw_menu_bar_meter
          // then draws the whole bar with cleared (0) bits against the
          // filled-white row background, and the colourizer only ever
          // recolours set (1) bits, so a selected Heap row shows the normal
          // inverted look instead of a colour clash.
          if (i == MEM_ITEM_HEAP) {
            heap_bar_x = bar_x; heap_bar_y = bar_y; heap_bar_w = bar_w; heap_bar_h = bar_h;
            if (!have_reading) {
              heap_bar_lit = 0;
            } else {
              float free_pct = total ? (float)free_b / total * 100.0f : 0;
              if      (free_pct > 40) heap_bar_lit = 3; // green - plenty free
              else if (free_pct > 15) heap_bar_lit = 2; // yellow - medium
              else                    heap_bar_lit = 1; // red - low free memory
            }
          }
        #endif
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
    MENU_GFX.setCursor(6, MENU_HEADER_TEXT_Y);
    MENU_GFX.print(title);
    MENU_GFX.drawFastHLine(4, MENU_EDIT_HEADER_HLINE_Y, MENU_CONTENT_W, SSD1306_WHITE);

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
      MENU_GFX.setCursor(6, MENU_HEADER_TEXT_Y);
      MENU_GFX.print(title);
      MENU_GFX.drawFastHLine(4, MENU_EDIT_HEADER_HLINE_Y, 120, SSD1306_WHITE);

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
      MENU_GFX.drawFastHLine(4, MENU_EDIT_FOOTER_HLINE_Y, 120, SSD1306_WHITE);
      MENU_GFX.setCursor(6, MENU_EDIT_FOOTER_TEXT_Y);
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
      MENU_GFX.setCursor(6, MENU_HEADER_TEXT_Y);
      // "UTC" suffix - the RTC list's own Time/Date rows show local
      // (Timezone-shifted) time, but this editor always reads/writes the
      // RTC in UTC, same as CMD_TIME/rtc_sync_ntp() - worth being explicit
      // about here since it'd otherwise be the one screen on this page
      // that doesn't match what's shown everywhere else.
      MENU_GFX.print("SET TIME/DATE UTC");
      MENU_GFX.drawFastHLine(4, MENU_EDIT_HEADER_HLINE_Y, 120, SSD1306_WHITE);

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
      MENU_GFX.drawFastHLine(4, MENU_EDIT_FOOTER_HLINE_Y, 120, SSD1306_WHITE);
      MENU_GFX.setCursor(6, MENU_EDIT_FOOTER_TEXT_Y);
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
      MENU_GFX.setCursor(6, MENU_HEADER_TEXT_Y);
      MENU_GFX.print(title);
      MENU_GFX.drawFastHLine(4, MENU_EDIT_HEADER_HLINE_Y, 120, SSD1306_WHITE);

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
      MENU_GFX.drawFastHLine(4, MENU_EDIT_FOOTER_HLINE_Y, 120, SSD1306_WHITE);
      MENU_GFX.setCursor(6, MENU_EDIT_FOOTER_TEXT_Y);
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
    #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_WTRACKER_V2 || BOARD_MODEL == BOARD_HELTEC_T1 || BOARD_MODEL == BOARD_HELTEC_T114
      // Unlike the other boards (whose display.clearDisplay() call in
      // update_display() wipes the whole panel buffer before getting
      // here every cycle), nothing clears menu_canvas on its own - it's
      // just an off-screen buffer that draw calls accumulate into.
      // Without this, switching screens (e.g. list -> edit) or the
      // cursor moving leaves stale pixels from the previous draw behind,
      // since fillRect/print only ever touch the specific pixels the
      // new content needs, never the ones it doesn't.
      menu_canvas.fillScreen(SSD1306_BLACK);
      // Same staleness hazard as the canvas pixels above, but for the
      // colourizer's stored Heap-bar geometry/tier (heap_bar_lit,
      // Display.h): it's a plain global that persists across screens, not
      // something the mono-pixel diff can catch. Left set after navigating
      // away from MENU_STATE_MEM_LIST, any other screen's first content row
      // happening to draw "on" pixels in that same rectangle (bar_x+1.. at
      // MENU_LIST_TOP_Y+1..) would get spuriously tinted. Reset unconditionally
      // every cycle; draw_menu_memory_disp() below re-sets it to the current
      // value only when actually showing that screen.
      heap_bar_lit = 0;
    #endif
    MENU_GFX.setTextWrap(false);

    if (menu_state == MENU_STATE_LIST) {
      const char *labels[MENU_ITEM_COUNT];
      char valbufs[MENU_ITEM_COUNT][24];
      // Left-column glyphs for the handful of items that have one (Graphics.h
      // bm_menu_icon_*) - every other slot stays nullptr/0, which
      // draw_menu_list_disp() treats as "no icon" but still reserves the
      // column for, so labels stay aligned whether or not their row has art.
      const uint8_t *icons[MENU_ITEM_COUNT] = { nullptr };
      uint8_t icon_widths[MENU_ITEM_COUNT] = { 0 };
      int8_t icon_dx[MENU_ITEM_COUNT] = { 0 };

      labels[MENU_ITEM_DISPLAY_TIMEOUT] = "Display Timeout";
      format_timeout(staged_display_timeout, valbufs[MENU_ITEM_DISPLAY_TIMEOUT]);
      icons[MENU_ITEM_DISPLAY_TIMEOUT] = bm_menu_icon_display;
      icon_widths[MENU_ITEM_DISPLAY_TIMEOUT] = MENU_ICON_W_DISPLAY;

      labels[MENU_ITEM_DISPLAY_BRIGHTNESS] = "Brightness";
      format_brightness(staged_display_brightness, valbufs[MENU_ITEM_DISPLAY_BRIGHTNESS]);
      icons[MENU_ITEM_DISPLAY_BRIGHTNESS] = bm_menu_icon_brightness;
      icon_widths[MENU_ITEM_DISPLAY_BRIGHTNESS] = MENU_ICON_W_BRIGHTNESS;
      icon_dx[MENU_ITEM_DISPLAY_BRIGHTNESS] = 1;

      labels[MENU_ITEM_ORIENTATION] = "Orientation";
      format_orientation(staged_display_rotation, valbufs[MENU_ITEM_ORIENTATION]);
      icons[MENU_ITEM_ORIENTATION] = bm_menu_icon_rotation;
      icon_widths[MENU_ITEM_ORIENTATION] = MENU_ICON_W_ROTATION;

      #if HAS_BUZZER == true
        labels[MENU_ITEM_SOUND] = "Sound";
        sprintf(valbufs[MENU_ITEM_SOUND], staged_sound_enabled ? "ON" : "OFF");
        icons[MENU_ITEM_SOUND] = bm_menu_icon_sound;
        icon_widths[MENU_ITEM_SOUND] = MENU_ICON_W_SOUND;
      #endif

      #if HAS_NP == true
        labels[MENU_ITEM_NEOPIXEL_BRIGHTNESS] = "LED Brightness";
        sprintf(valbufs[MENU_ITEM_NEOPIXEL_BRIGHTNESS], "%u", staged_np_brightness);
        icons[MENU_ITEM_NEOPIXEL_BRIGHTNESS] = bm_menu_icon_led_brightness;
        icon_widths[MENU_ITEM_NEOPIXEL_BRIGHTNESS] = MENU_ICON_W_LED_BRIGHTNESS;
        icon_dx[MENU_ITEM_NEOPIXEL_BRIGHTNESS] = 1;
      #endif

      #if HAS_ENCODER == true
        labels[MENU_ITEM_ENCODER] = "Encoder";
        sprintf(valbufs[MENU_ITEM_ENCODER], staged_encoder_enabled ? "ON" : "OFF");
        icons[MENU_ITEM_ENCODER] = bm_menu_icon_encoder;
        icon_widths[MENU_ITEM_ENCODER] = MENU_ICON_W_ENCODER;
      #endif

      #if HAS_LXMF == true
        labels[MENU_ITEM_MESSENGER] = "Messenger";
        sprintf(valbufs[MENU_ITEM_MESSENGER], ">"); // opens a submenu, not an inline value
        icons[MENU_ITEM_MESSENGER] = bm_menu_icon_messenger;
        icon_widths[MENU_ITEM_MESSENGER] = MENU_ICON_W_MESSENGER;
      #endif

      #if HAS_URNS == true
        labels[MENU_ITEM_URNS] = "URNS";
        sprintf(valbufs[MENU_ITEM_URNS], ">"); // opens a submenu, not an inline value
        icons[MENU_ITEM_URNS] = bm_menu_icon_urns;
        icon_widths[MENU_ITEM_URNS] = MENU_ICON_W_URNS;
        icon_dx[MENU_ITEM_URNS] = 2; // narrowest icon (7px) - left-anchored, looks off vs. the others without this
      #endif

      labels[MENU_ITEM_URNS_RADIO] = "Radio";
      sprintf(valbufs[MENU_ITEM_URNS_RADIO], ">"); // opens a submenu, not an inline value
      icons[MENU_ITEM_URNS_RADIO] = bm_menu_icon_radio;
      icon_widths[MENU_ITEM_URNS_RADIO] = MENU_ICON_W_RADIO;

      #if HAS_ESPNOW == true
        labels[MENU_ITEM_ESPNOW] = "ESP-NOW";
        sprintf(valbufs[MENU_ITEM_ESPNOW], ">"); // opens a submenu, not an inline value
        icons[MENU_ITEM_ESPNOW] = bm_menu_icon_espnow;
        icon_widths[MENU_ITEM_ESPNOW] = MENU_ICON_W_ESPNOW;
      #endif

      #if HAS_WIFI == true
        labels[MENU_ITEM_WIFI] = "WiFi";
        sprintf(valbufs[MENU_ITEM_WIFI], ">"); // opens a submenu, not an inline value
        icons[MENU_ITEM_WIFI] = bm_menu_icon_wifi;
        icon_widths[MENU_ITEM_WIFI] = MENU_ICON_W_WIFI;
      #endif

      #if HAS_BLUETOOTH == true || HAS_BLE == true
        labels[MENU_ITEM_BLUETOOTH] = "Bluetooth";
        sprintf(valbufs[MENU_ITEM_BLUETOOTH], ">"); // opens a submenu, not an inline value
        icons[MENU_ITEM_BLUETOOTH] = bm_menu_icon_bt_legacy_pairing;
        icon_widths[MENU_ITEM_BLUETOOTH] = MENU_ICON_W_BT_LEGACY_PAIRING;
      #endif

      #if HAS_ETHERNET == true
        labels[MENU_ITEM_ETHERNET] = "Ethernet";
        sprintf(valbufs[MENU_ITEM_ETHERNET], ">"); // opens a submenu, not an inline value
        icons[MENU_ITEM_ETHERNET] = bm_menu_icon_ethernet;
        icon_widths[MENU_ITEM_ETHERNET] = MENU_ICON_W_ETHERNET;
      #endif

      #if HAS_RTC == true
        labels[MENU_ITEM_RTC] = "RTC";
        sprintf(valbufs[MENU_ITEM_RTC], ">"); // opens a submenu, not an inline value
        icons[MENU_ITEM_RTC] = bm_menu_icon_rtc;
        icon_widths[MENU_ITEM_RTC] = MENU_ICON_W_RTC;
      #endif

      #if HAS_GPS == true
        labels[MENU_ITEM_GNSS] = "GNSS";
        sprintf(valbufs[MENU_ITEM_GNSS], ">"); // opens a submenu, not an inline value
        icons[MENU_ITEM_GNSS] = bm_menu_icon_gnss;
        icon_widths[MENU_ITEM_GNSS] = MENU_ICON_W_GNSS;
      #endif

      #if HAS_SENSORS == true
        labels[MENU_ITEM_SENSORS] = "Sensors";
        sprintf(valbufs[MENU_ITEM_SENSORS], ">"); // opens a submenu, not an inline value
        icons[MENU_ITEM_SENSORS] = bm_menu_icon_sensors;
        icon_widths[MENU_ITEM_SENSORS] = MENU_ICON_W_SENSORS;
      #endif

      #if MENU_HAS_HW_PAGE == true
        labels[MENU_ITEM_HARDWARE] = "Hardware";
        sprintf(valbufs[MENU_ITEM_HARDWARE], ">"); // opens a submenu, not an inline value
        icons[MENU_ITEM_HARDWARE] = bm_menu_icon_hardware;
        icon_widths[MENU_ITEM_HARDWARE] = MENU_ICON_W_HARDWARE;
      #endif

      #if HAS_OTA == true
        labels[MENU_ITEM_FW_UPDATE] = "F/W Update";
        sprintf(valbufs[MENU_ITEM_FW_UPDATE], ">"); // opens a submenu, not an inline value
        icons[MENU_ITEM_FW_UPDATE] = bm_menu_icon_fwupdate;
        icon_widths[MENU_ITEM_FW_UPDATE] = MENU_ICON_W_FWUPDATE;
      #endif

      labels[MENU_ITEM_SAVE_EXIT] = "SAVE & EXIT";
      valbufs[MENU_ITEM_SAVE_EXIT][0] = 0;
      icons[MENU_ITEM_SAVE_EXIT] = bm_menu_icon_saveexit;
      icon_widths[MENU_ITEM_SAVE_EXIT] = MENU_ICON_W_SAVEEXIT;

      // Same bt_dh[14]/bt_dh[15] 4-digit ID as bt_devname's "RNode XXXX"
      // BLE name and the boot splash carousel (Display.h) - bt_dh is
      // populated at boot on any ESP32 board or HAS_BLUETOOTH/HAS_BLE
      // nRF52 board regardless of whether Bluetooth itself is enabled
      // (see bt_init()'s own call-site gate, RNode_Firmware.ino), so this
      // is available by the time the menu can ever be opened.
      char settings_title[24];
      sprintf(settings_title, "RNODE %02X%02X SETTINGS", bt_dh[14], bt_dh[15]);
      draw_menu_list_disp(settings_title, labels, valbufs, MENU_ITEM_COUNT, menu_cursor, icons, icon_widths, icon_dx);

    } else if (menu_state == MENU_STATE_EDIT) {
      char valbuf[8];
      const char *title = "";
      if (menu_edit_field == MENU_ITEM_DISPLAY_TIMEOUT) {
        title = "DISPLAY TIMEOUT";
        format_timeout(staged_display_timeout, valbuf);
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
      #if HAS_NP == true
        else if (menu_edit_field == MENU_ITEM_NEOPIXEL_BRIGHTNESS) {
          title = "LED BRIGHTNESS";
          sprintf(valbuf, "%u", staged_np_brightness);
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

        // "N/A" rather than "DHCP" when unset - unlike IP/Netmask, there's
        // no DHCP client running once IP/NM are static, so an unset
        // Gateway/DNS just means "not configured," not "provided by DHCP."
        labels[WIFI_ITEM_GATEWAY] = "Gateway";
        if (staged_wifi_gw[0]==0 && staged_wifi_gw[1]==0 && staged_wifi_gw[2]==0 && staged_wifi_gw[3]==0) sprintf(valbufs[WIFI_ITEM_GATEWAY], "N/A");
        else format_addr_octets(staged_wifi_gw, valbufs[WIFI_ITEM_GATEWAY]);

        labels[WIFI_ITEM_DNS] = "DNS";
        if (staged_wifi_dns[0]==0 && staged_wifi_dns[1]==0 && staged_wifi_dns[2]==0 && staged_wifi_dns[3]==0) sprintf(valbufs[WIFI_ITEM_DNS], "N/A");
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
    #if HAS_BLUETOOTH == true || HAS_BLE == true
      else if (menu_state == MENU_STATE_BT_LIST) {
        const char *labels[BT_ITEM_COUNT];
        char valbufs[BT_ITEM_COUNT][24];
        const uint8_t *icons[BT_ITEM_COUNT] = { nullptr };
        uint8_t icon_widths[BT_ITEM_COUNT] = { 0 };

        #if HAS_BLE == true
          labels[BT_ITEM_SETTINGS] = "Settings";
          valbufs[BT_ITEM_SETTINGS][0] = 0;
          icons[BT_ITEM_SETTINGS] = bm_menu_icon_msngr_settings;
          icon_widths[BT_ITEM_SETTINGS] = MENU_ICON_W_MSNGR_SETTINGS;
        #endif

        labels[BT_ITEM_MAC] = "MAC";
        #if MCU_VARIANT == MCU_ESP32
          {
            uint8_t mac[6];
            esp_read_mac(mac, ESP_MAC_BT);
            sprintf(valbufs[BT_ITEM_MAC], "%02X:%02X:%02X:%02X:%02X:%02X",
              mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
          }
        #elif MCU_VARIANT == MCU_NRF52
          // Bluefruit.getAddr() only returns a real address once the BLE
          // stack has actually started (bt_start()/bt_ready, Bluetooth.h) -
          // unlike ESP32's esp_read_mac(), there's no hardware-burned MAC
          // readable before that.
          if (bt_ready) {
            ble_gap_addr_t gap_addr = Bluefruit.getAddr();
            sprintf(valbufs[BT_ITEM_MAC], "%02X:%02X:%02X:%02X:%02X:%02X",
              gap_addr.addr[5], gap_addr.addr[4], gap_addr.addr[3],
              gap_addr.addr[2], gap_addr.addr[1], gap_addr.addr[0]);
          } else {
            sprintf(valbufs[BT_ITEM_MAC], "N/A");
          }
        #endif

        #if HAS_BLE == true
          labels[BT_ITEM_BONDS] = "Bonds";
          #if MCU_VARIANT == MCU_ESP32
            // bt_bond_count() -> ble_store_util_count() -> ble_hs_lock() derefs
            // NimBLE host state that only exists once the host has actually
            // started (see bt_security_setup()'s own comment on this exact
            // crash). menu_confirm_select()'s bt_start() call on entering this
            // list can silently no-op (BT_START_MIN_UPTIME_MS window, or
            // SerialBT.begin() itself failing) and still land here with
            // bt_state unchanged, so this can't assume the stack is up.
            if (bt_state != BT_STATE_OFF) {
              sprintf(valbufs[BT_ITEM_BONDS], "%d", bt_bond_count());
            } else {
              sprintf(valbufs[BT_ITEM_BONDS], "N/A");
            }
          #else
            // MCU_NRF52 - Bluefruit's bond directory lives on InternalFS,
            // populated by bond_init() inside Bluefruit.begin() (bt_setup_hw())
            // regardless of the BLE stack's current on/off state, so
            // bt_bond_count() is safe to call any time the stack has been
            // set up at all (bt_ready), not just while bt_state != OFF.
            //
            // Unlike NimBLE's ble_store_util_count() above, this walks real
            // LittleFS directory entries on InternalFS - the same 24KB
            // filesystem/mutex Bluefruit's own SEC_INFO_REQUEST handler
            // (bonding.cpp, invoked from the SoftDevice's task on every real
            // reconnect) needs to load that bond's keys. update_display()
            // (RNode_Firmware.ino) redraws this screen on every single
            // loop() iteration while it's open, not just on change - calling
            // bt_bond_count() unthrottled there means hundreds of
            // Adafruit_LittleFS mutex acquisitions per second for as long as
            // this screen stays open, which can starve a concurrent
            // reconnect's own bond lookup long enough to blow the central's
            // connection supervision timeout - observed live as BLE
            // connect/disconnect flapping until the central gave up, while
            // this exact screen happened to be open. Same class of bug as
            // URNS_ITEM_FREE's own LittleFS.usedBytes()/totalBytes() hazard
            // (project_urns_partition_growth memory) - cache and refresh at
            // most once a second.
            static int cached_bond_count = 0;
            static unsigned long bt_bond_count_last_ms = 0;
            unsigned long bt_bond_count_now_ms = millis();
            if (bt_ready) {
              if (bt_bond_count_now_ms - bt_bond_count_last_ms >= 1000 || bt_bond_count_last_ms == 0) {
                cached_bond_count = bt_bond_count();
                bt_bond_count_last_ms = bt_bond_count_now_ms;
              }
              sprintf(valbufs[BT_ITEM_BONDS], "%d", cached_bond_count);
            } else {
              sprintf(valbufs[BT_ITEM_BONDS], "N/A");
            }
          #endif

          labels[BT_ITEM_UNPAIR] = "Forget Bonds";
          sprintf(valbufs[BT_ITEM_UNPAIR], ">"); // opens a confirm dialog, not an inline value
        #endif

        labels[BT_ITEM_BACK] = "BACK";
        valbufs[BT_ITEM_BACK][0] = 0;
        // Explicit here (not auto-detected) since this list already builds
        // its own icons[] table for Settings above - same reasoning as
        // MSNGR_LIST's own BACK row.
        icons[BT_ITEM_BACK] = bm_menu_icon_back;
        icon_widths[BT_ITEM_BACK] = MENU_ICON_W_BACK;

        // icon_col_shared=false - only Settings (the one row with an icon)
        // gets the wider indent; MAC/Bonds/Forget Bonds stay at the plain
        // x=8 the list used before Settings existed. BACK still auto-gets
        // its own icon+narrow position via the explicit icons[BT_ITEM_BACK]
        // entry above, same as every other plain submenu list.
        draw_menu_list_disp("BLUETOOTH", labels, valbufs, BT_ITEM_COUNT, bt_menu_cursor, icons, icon_widths, nullptr, false);
      }
      #if HAS_BLE == true
        else if (menu_state == MENU_STATE_BT_SETTINGS) {
          const char *labels[BT_SETTINGS_ITEM_COUNT];
          char valbufs[BT_SETTINGS_ITEM_COUNT][24];

          #if MCU_VARIANT == MCU_ESP32
            labels[BT_SETTINGS_ITEM_LEGACY_PAIRING] = "Legacy Pairing";
            sprintf(valbufs[BT_SETTINGS_ITEM_LEGACY_PAIRING], staged_bt_legacy_pairing_enabled ? "ON" : "OFF");

            labels[BT_SETTINGS_ITEM_JUST_WORKS] = "Just Works";
            sprintf(valbufs[BT_SETTINGS_ITEM_JUST_WORKS], staged_bt_just_works_enabled ? "ON" : "OFF");

            labels[BT_SETTINGS_ITEM_AUTO_START] = "Auto Start";
            sprintf(valbufs[BT_SETTINGS_ITEM_AUTO_START], staged_bt_auto_start_enabled ? "ON" : "OFF");
          #endif

          labels[BT_SETTINGS_ITEM_BATTERY_SERVICE] = "Battery Service";
          sprintf(valbufs[BT_SETTINGS_ITEM_BATTERY_SERVICE], staged_bt_battery_service_enabled ? "ON" : "OFF");

          labels[BT_SETTINGS_ITEM_BACK] = "BACK";
          valbufs[BT_SETTINGS_ITEM_BACK][0] = 0;

          draw_menu_list_disp("BLUETOOTH SETTINGS", labels, valbufs, BT_SETTINGS_ITEM_COUNT, bt_settings_cursor);
        }
        else if (menu_state == MENU_STATE_BT_SETTINGS_EDIT) {
          #if MCU_VARIANT == MCU_ESP32
            if (bt_settings_cursor == BT_SETTINGS_ITEM_JUST_WORKS) {
              draw_menu_edit_disp("JUST WORKS", staged_bt_just_works_enabled ? "ON" : "OFF");
            } else if (bt_settings_cursor == BT_SETTINGS_ITEM_AUTO_START) {
              draw_menu_edit_disp("AUTO START", staged_bt_auto_start_enabled ? "ON" : "OFF");
            } else if (bt_settings_cursor == BT_SETTINGS_ITEM_BATTERY_SERVICE) {
              draw_menu_edit_disp("BATTERY SERVICE", staged_bt_battery_service_enabled ? "ON" : "OFF");
            } else {
              draw_menu_edit_disp("LEGACY PAIRING", staged_bt_legacy_pairing_enabled ? "ON" : "OFF");
            }
          #else // MCU_NRF52 - Battery Service is the only real row here
            draw_menu_edit_disp("BATTERY SERVICE", staged_bt_battery_service_enabled ? "ON" : "OFF");
          #endif
        }
      #endif
      #if HAS_BLE == true
        else if (menu_state == MENU_STATE_BT_UNPAIR_CONFIRM) {
          // Plain 2-item list, same pattern as F/W Update's UPDATE/CANCEL
          // (MENU_STATE_FWUPD_CONFIRM).
          const char *labels[2] = { "FORGET", "CANCEL" };
          char valbufs[2][24];
          valbufs[0][0] = 0;
          valbufs[1][0] = 0;
          draw_menu_list_disp("FORGET BONDS?", labels, valbufs, 2, bt_unpair_confirm_cursor);
        }
      #endif
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

        // "N/A" rather than "DHCP" when unset - unlike IP/Netmask, there's
        // no DHCP client running once IP/NM are static, so an unset
        // Gateway/DNS just means "not configured," not "provided by DHCP."
        labels[ETH_ITEM_GATEWAY] = "Gateway";
        {
          uint8_t gw_octets[4];
          if (addr4_read(ADDR_CONF_ETH_GW, gw_octets)) format_addr_octets(gw_octets, valbufs[ETH_ITEM_GATEWAY]);
          else                                             sprintf(valbufs[ETH_ITEM_GATEWAY], "N/A");
        }

        labels[ETH_ITEM_DNS] = "DNS";
        {
          uint8_t dns_octets[4];
          if (addr4_read(ADDR_CONF_ETH_DNS, dns_octets)) format_addr_octets(dns_octets, valbufs[ETH_ITEM_DNS]);
          else                                              sprintf(valbufs[ETH_ITEM_DNS], "N/A");
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
        // itself) stays strictly UTC; apply_tz_offset() (Utilities.h) only
        // shifts this display copy of the epoch.
        uint32_t epoch = apply_tz_offset(rtc_get_unixtime());
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
        format_tz_offset(get_tz_offset_qh(), valbufs[RTC_ITEM_TIMEZONE]);

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
      }
    #endif
    #if HAS_GPS == true
      else if (menu_state == MENU_STATE_GNSS_LIST) {
        const char *labels[GNSS_ITEM_COUNT];
        char valbufs[GNSS_ITEM_COUNT][24];
        // Same "reserve the column, leave every other row iconless" pattern
        // as the top-level RNODE SETTINGS list (draw_settings_menu_disp())
        // - only Show GNSS Banner below actually sets one.
        const uint8_t *icons[GNSS_ITEM_COUNT] = { nullptr };
        uint8_t icon_widths[GNSS_ITEM_COUNT] = { 0 };
        int8_t icon_dx[GNSS_ITEM_COUNT] = { 0 };

        labels[GNSS_ITEM_ENABLED] = "Enabled";
        sprintf(valbufs[GNSS_ITEM_ENABLED], gnss_enabled ? "ON" : "OFF");

        #if GNSS_DUTY_CYCLE_CAPABLE == true
          labels[GNSS_ITEM_UPDATE_INTERVAL] = "Poll Interval";
          if (gnss_update_interval_s == GNSS_UPDATE_INTERVAL_CONTINUOUS) sprintf(valbufs[GNSS_ITEM_UPDATE_INTERVAL], "Cont.");
          else if (gnss_update_interval_s < 3600)                        sprintf(valbufs[GNSS_ITEM_UPDATE_INTERVAL], "%u min", (unsigned)(gnss_update_interval_s / 60));
          else                                                            sprintf(valbufs[GNSS_ITEM_UPDATE_INTERVAL], "%u hour", (unsigned)(gnss_update_interval_s / 3600));
        #endif

        // Chip identity no longer shown here at all (per user request) -
        // boards with Diagnostics (HAS_GNSS_DEBUG_MENU) still expose it
        // there (GNSS_DIAG_ITEM_MODULE below). Universal across every
        // HAS_GPS board now - gnss_pstate_text() (GNSS.h) already returns
        // a correct static "ACTIVE"/"OFF" on boards that can't actually
        // duty-cycle, since gnss_duty_cycle_update() never leaves ACTIVE
        // there.
        labels[GNSS_ITEM_DUTY_STATE] = "Duty State";
        sprintf(valbufs[GNSS_ITEM_DUTY_STATE], "%s", gnss_pstate_text());

        labels[GNSS_ITEM_FIX] = "Fix";
        if (gnss_has_fix()) {
          // Latitude/Longitude/Altitude/Fix all latch their last known
          // value forever (TinyGPSLocation::isValid() never resets - see
          // gnss_location_age_ms(), GNSS.h) - while duty-cycling is
          // active, an age suffix distinguishes a live fix from a stale
          // one held over from before the last sleep, so the page doesn't
          // read as broken during SLEEP/early SEARCHING.
          #if GNSS_DUTY_CYCLE_CAPABLE == true
            if (gnss_update_interval_s != GNSS_UPDATE_INTERVAL_CONTINUOUS) {
              uint32_t gnss_fix_age_s = gnss_location_age_ms() / 1000;
              if (gnss_fix_age_s < 60) sprintf(valbufs[GNSS_ITEM_FIX], "YES (%us ago)", (unsigned)gnss_fix_age_s);
              else                      sprintf(valbufs[GNSS_ITEM_FIX], "YES (%um ago)", (unsigned)(gnss_fix_age_s / 60));
            } else
          #endif
          sprintf(valbufs[GNSS_ITEM_FIX], "YES");
        } else {
          sprintf(valbufs[GNSS_ITEM_FIX], "No Fix");
        }

        labels[GNSS_ITEM_SATELLITES] = "Sats Used";
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

        labels[GNSS_ITEM_TIMEZONE] = "Timezone";
        format_tz_offset(get_tz_offset_qh(), valbufs[GNSS_ITEM_TIMEZONE]);

        #if HAS_GNSS_DEBUG_MENU == true
          labels[GNSS_ITEM_DIAGNOSTICS] = "Diagnostics";
          sprintf(valbufs[GNSS_ITEM_DIAGNOSTICS], ">"); // opens a submenu, not an inline value
        #endif

        #if BOARD_MODEL != BOARD_HELTEC_T114
          labels[GNSS_ITEM_SHOW_BANNER] = "Show GNSS Banner";
          valbufs[GNSS_ITEM_SHOW_BANNER][0] = 0;
          icons[GNSS_ITEM_SHOW_BANNER] = bm_menu_icon_display;
          icon_widths[GNSS_ITEM_SHOW_BANNER] = MENU_ICON_W_DISPLAY;
        #endif

        labels[GNSS_ITEM_BACK] = "BACK";
        valbufs[GNSS_ITEM_BACK][0] = 0;

        draw_menu_list_disp("GNSS", labels, valbufs, GNSS_ITEM_COUNT, gnss_menu_cursor, icons, icon_widths, icon_dx, false);
      } else if (menu_state == MENU_STATE_GNSS_EDIT) {
        #if GNSS_DUTY_CYCLE_CAPABLE == true
          if (gnss_menu_cursor == GNSS_ITEM_UPDATE_INTERVAL) {
            uint32_t staged_interval_s = gnss_update_interval_presets_s[staged_gnss_interval_index];
            char interval_valbuf[24];
            if (staged_interval_s == GNSS_UPDATE_INTERVAL_CONTINUOUS) sprintf(interval_valbuf, "Continuous");
            else if (staged_interval_s < 3600)                        sprintf(interval_valbuf, "%u min", (unsigned)(staged_interval_s / 60));
            else                                                       sprintf(interval_valbuf, "%u hour", (unsigned)(staged_interval_s / 3600));
            draw_menu_edit_disp("POLL INTERVAL", interval_valbuf);
          } else
        #endif
        draw_menu_edit_disp("GNSS ENABLED", staged_gnss_enabled ? "ON" : "OFF");
      }
      #if HAS_GNSS_DEBUG_MENU == true
        else if (menu_state == MENU_STATE_GNSS_DIAG) {
          const char *labels[GNSS_DIAG_ITEM_COUNT];
          char valbufs[GNSS_DIAG_ITEM_COUNT][24];

          labels[GNSS_DIAG_ITEM_MODULE] = "Module";
          sprintf(valbufs[GNSS_DIAG_ITEM_MODULE], "%s", gnss_chip_name());

          labels[GNSS_DIAG_ITEM_FIX_QUALITY] = "Fix Quality";
          sprintf(valbufs[GNSS_DIAG_ITEM_FIX_QUALITY], "%s", gnss_fix_quality_text());

          labels[GNSS_DIAG_ITEM_FIX_MODE] = "Fix Mode";
          sprintf(valbufs[GNSS_DIAG_ITEM_FIX_MODE], "%s", gnss_fix_mode_text());

          labels[GNSS_DIAG_ITEM_HDOP] = "HDOP";
          if (gnss_hdop_valid()) sprintf(valbufs[GNSS_DIAG_ITEM_HDOP], "%.1f %s", gnss_hdop(), gnss_hdop_band_text());
          else                   sprintf(valbufs[GNSS_DIAG_ITEM_HDOP], "N/A");

          labels[GNSS_DIAG_ITEM_PDOP] = "PDOP";
          if (gnss_pdop_valid()) sprintf(valbufs[GNSS_DIAG_ITEM_PDOP], "%.1f", gnss_pdop());
          else                   sprintf(valbufs[GNSS_DIAG_ITEM_PDOP], "N/A");

          labels[GNSS_DIAG_ITEM_VDOP] = "VDOP";
          if (gnss_vdop_valid()) sprintf(valbufs[GNSS_DIAG_ITEM_VDOP], "%.1f", gnss_vdop());
          else                   sprintf(valbufs[GNSS_DIAG_ITEM_VDOP], "N/A");

          labels[GNSS_DIAG_ITEM_SPEED] = "Speed";
          if (gnss_speed_valid()) sprintf(valbufs[GNSS_DIAG_ITEM_SPEED], "%.1f km/h", gnss_speed_kmph());
          else                    sprintf(valbufs[GNSS_DIAG_ITEM_SPEED], "N/A");

          labels[GNSS_DIAG_ITEM_COURSE] = "Course";
          if (gnss_course_valid()) sprintf(valbufs[GNSS_DIAG_ITEM_COURSE], "%.0f %s", gnss_course_deg(), gnss_course_cardinal());
          else                     sprintf(valbufs[GNSS_DIAG_ITEM_COURSE], "N/A");

          labels[GNSS_DIAG_ITEM_DATE] = "Date";
          if (gnss_date_valid()) sprintf(valbufs[GNSS_DIAG_ITEM_DATE], "%04u-%02u-%02u", gnss_date_year(), gnss_date_month(), gnss_date_day());
          else                   sprintf(valbufs[GNSS_DIAG_ITEM_DATE], "N/A");

          labels[GNSS_DIAG_ITEM_SATS_USED] = "Sats Used";
          sprintf(valbufs[GNSS_DIAG_ITEM_SATS_USED], "%u", (unsigned)gnss_satellite_count());

          labels[GNSS_DIAG_ITEM_SATS_VIEW] = "Sats In View";
          sprintf(valbufs[GNSS_DIAG_ITEM_SATS_VIEW], "%u >", (unsigned)gnss_sat_view_count()); // count preview + drill-down indicator

          labels[GNSS_DIAG_ITEM_CHK_PASSED] = "Chk Passed";
          sprintf(valbufs[GNSS_DIAG_ITEM_CHK_PASSED], "%lu", (unsigned long)gnss_checksum_passed());

          labels[GNSS_DIAG_ITEM_CHK_FAILED] = "Chk Failed";
          sprintf(valbufs[GNSS_DIAG_ITEM_CHK_FAILED], "%lu", (unsigned long)gnss_checksum_failed());

          labels[GNSS_DIAG_ITEM_CHK_RATE] = "Chk Pass Rate";
          sprintf(valbufs[GNSS_DIAG_ITEM_CHK_RATE], "%u%%", (unsigned)gnss_checksum_pass_rate_pct());

          labels[GNSS_DIAG_ITEM_CHARS] = "Chars RX";
          sprintf(valbufs[GNSS_DIAG_ITEM_CHARS], "%lu", (unsigned long)gnss_chars_processed());

          #if GNSS_DUTY_CYCLE_CAPABLE == true
            labels[GNSS_DIAG_ITEM_DUTY_STATE] = "Duty State";
            sprintf(valbufs[GNSS_DIAG_ITEM_DUTY_STATE], "%s", gnss_pstate_text());

            labels[GNSS_DIAG_ITEM_LOCK_COUNT] = "Lock Count";
            sprintf(valbufs[GNSS_DIAG_ITEM_LOCK_COUNT], "%u", (unsigned)gnss_duty_lock_count);

            labels[GNSS_DIAG_ITEM_FAIL_COUNT] = "Consec Fails";
            sprintf(valbufs[GNSS_DIAG_ITEM_FAIL_COUNT], "%u", (unsigned)gnss_duty_consecutive_failures);

            labels[GNSS_DIAG_ITEM_PREDICTED] = "Pred. Lock";
            if (gnss_duty_predicted_lock_ms == 0) sprintf(valbufs[GNSS_DIAG_ITEM_PREDICTED], "N/A");
            else                                  sprintf(valbufs[GNSS_DIAG_ITEM_PREDICTED], "%lus", (unsigned long)gnss_duty_predicted_lock_s());

            labels[GNSS_DIAG_ITEM_WAKE_COUNTDOWN] = "Wake In";
            if (gnss_pstate == GNSS_PSTATE_SLEEP) sprintf(valbufs[GNSS_DIAG_ITEM_WAKE_COUNTDOWN], "%lus", (unsigned long)gnss_duty_wake_countdown_s());
            else                                  sprintf(valbufs[GNSS_DIAG_ITEM_WAKE_COUNTDOWN], "--");
          #endif

          labels[GNSS_DIAG_ITEM_BACK] = "BACK";
          valbufs[GNSS_DIAG_ITEM_BACK][0] = 0;

          draw_menu_list_disp("GNSS DIAGNOSTICS", labels, valbufs, GNSS_DIAG_ITEM_COUNT, gnss_diag_menu_cursor);
        } else if (menu_state == MENU_STATE_GNSS_DIAG_SATS) {
          uint8_t sat_count = gnss_sat_view_count();
          uint8_t row_count = sat_count == 0 ? 2 : (uint8_t)(sat_count + 1); // "No Data" + BACK, or sats + BACK

          const char *labels[GNSS_SAT_VIEW_MAX + 1];
          char label_bufs[GNSS_SAT_VIEW_MAX][8];
          char valbufs[GNSS_SAT_VIEW_MAX + 1][24];

          if (sat_count == 0) {
            labels[0] = "No Data";
            valbufs[0][0] = 0;
          } else {
            for (uint8_t i = 0; i < sat_count; i++) {
              snprintf(label_bufs[i], sizeof(label_bufs[i]), "Sat %u", (unsigned)gnss_sat_view_prn(i));
              labels[i] = label_bufs[i];
              sprintf(valbufs[i], "El%u Az%u %udB", (unsigned)gnss_sat_view_elevation(i), (unsigned)gnss_sat_view_azimuth(i), (unsigned)gnss_sat_view_snr(i));
            }
          }
          labels[row_count - 1] = "BACK";
          valbufs[row_count - 1][0] = 0;

          draw_menu_list_disp("SATS IN VIEW", labels, valbufs, row_count, gnss_diag_sats_cursor);
        }
      #endif
    #endif
    #if HAS_RTC == true || HAS_GPS == true
      else if (menu_state == MENU_STATE_RTC_TZ_EDIT) {
        char valbuf[8];
        format_tz_offset(staged_tz_offset_qh, valbuf);
        draw_menu_edit_disp("TIMEZONE", valbuf);
      }
    #endif
    #if HAS_ESPNOW == true
      else if (menu_state == MENU_STATE_ESPNOW_LIST) {
        const char *labels[ESPNOW_ITEM_COUNT];
        char valbufs[ESPNOW_ITEM_COUNT][24];

        labels[ESPNOW_ITEM_ENABLED] = "Enabled";
        sprintf(valbufs[ESPNOW_ITEM_ENABLED], staged_espnow_enabled ? "ON" : "OFF");

        labels[ESPNOW_ITEM_MODE] = "Version";
        #if HAS_URNS == true
          // v1's per-packet seq/more header is meaningless to
          // UrnsEspNowInterface (URNS.h), which speaks unfragmented v2
          // directly - whenever URNS shares this channel the whole device
          // is forced to v2 at boot (RNode_Firmware.ino), so this row shows
          // that override instead of pretending the field is still free to
          // pick (see menu_confirm_select()'s matching gate on entering
          // MENU_STATE_ESPNOW_EDIT for this item).
          if (staged_urns_interface_mode != URNS_INTERFACE_LORA_ONLY) {
            sprintf(valbufs[ESPNOW_ITEM_MODE], "v2.0 (URNS)");
          } else
        #endif
          sprintf(valbufs[ESPNOW_ITEM_MODE], staged_espnow_mode_v2 ? "v2.0" : "v1.0");

        labels[ESPNOW_ITEM_LR] = "LR Mode";
        sprintf(valbufs[ESPNOW_ITEM_LR], staged_espnow_lr_enabled ? "ON" : "OFF");

        labels[ESPNOW_ITEM_CHANNEL] = "Channel";
        sprintf(valbufs[ESPNOW_ITEM_CHANNEL], "%u", staged_wifi_channel);

        labels[ESPNOW_ITEM_BACK] = "BACK";
        valbufs[ESPNOW_ITEM_BACK][0] = 0;

        draw_menu_list_disp("ESP-NOW", labels, valbufs, ESPNOW_ITEM_COUNT, espnow_menu_cursor);
      } else if (menu_state == MENU_STATE_ESPNOW_EDIT) {
        if (espnow_menu_cursor == ESPNOW_ITEM_ENABLED) {
          draw_menu_edit_disp("ESP-NOW ENABLED", staged_espnow_enabled ? "ON" : "OFF");
        } else if (espnow_menu_cursor == ESPNOW_ITEM_MODE) {
          draw_menu_edit_disp("ESP-NOW VERSION", staged_espnow_mode_v2 ? "v2.0" : "v1.0");
        } else if (espnow_menu_cursor == ESPNOW_ITEM_CHANNEL) {
          char valbuf[8];
          sprintf(valbuf, "%u", staged_wifi_channel);
          draw_menu_edit_disp("ESP-NOW CHANNEL", valbuf);
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
    #if HAS_URNS == true
      else if (menu_state == MENU_STATE_URNS_LIST) {
        const char *labels[URNS_ITEM_COUNT];
        char valbufs[URNS_ITEM_COUNT][24];

        labels[URNS_ITEM_ENABLED] = "Enabled";
        sprintf(valbufs[URNS_ITEM_ENABLED], staged_urns_enabled ? "ON" : "OFF");

        labels[URNS_ITEM_TRANSPORT] = "Transport Mode";
        sprintf(valbufs[URNS_ITEM_TRANSPORT], staged_urns_transport_enabled ? "ON" : "OFF");

        #if HAS_ESPNOW == true
          labels[URNS_ITEM_INTERFACE] = "Interface";
          switch (staged_urns_interface_mode) {
            case URNS_INTERFACE_ESPNOW_ONLY: sprintf(valbufs[URNS_ITEM_INTERFACE], "ESP-NOW"); break;
            case URNS_INTERFACE_BOTH:        sprintf(valbufs[URNS_ITEM_INTERFACE], "Both"); break;
            default:                         sprintf(valbufs[URNS_ITEM_INTERFACE], "LoRa"); break;
          }
        #endif

        labels[URNS_ITEM_LINK_MTU_DISCOVERY] = "Link MTU Discovery";
        sprintf(valbufs[URNS_ITEM_LINK_MTU_DISCOVERY], staged_urns_link_mtu_discovery ? "ON" : "OFF");

        labels[URNS_ITEM_REMOTE_MGMT] = "Remote Management";
        sprintf(valbufs[URNS_ITEM_REMOTE_MGMT], staged_urns_remote_mgmt_enabled ? "ON" : "OFF");

        labels[URNS_ITEM_PROBE_DEST] = "Probe Destination";
        // Inert (see the confirm-handler's own gate/comment) unless
        // Transport Mode is staged on too - shown as N/A rather than a
        // silent dead-end row, same "explain why, don't just ignore the
        // press" reasoning as every other gated field in this menu.
        if (staged_urns_transport_enabled) {
          sprintf(valbufs[URNS_ITEM_PROBE_DEST], staged_urns_probe_dest_enabled ? "ON" : "OFF");
        } else {
          sprintf(valbufs[URNS_ITEM_PROBE_DEST], "N/A");
        }

        labels[URNS_ITEM_PATHS] = "Path Table";
        sprintf(valbufs[URNS_ITEM_PATHS], "%u", (unsigned)RNS::Transport::new_path_table().size());

        labels[URNS_ITEM_FREE] = "Free";
        {
          // update_display() (RNode_Firmware.ino) calls draw_settings_menu_disp()
          // on every loop() iteration while this screen is open, not just on
          // change - esp_littlefs_info() (what usedBytes()/totalBytes() both
          // call, each independently) walks the whole filesystem's block
          // allocation to compute this, which was cheap on the original
          // 512KB urns partition but not on the grown ~6.9MB one (see
          // project_urns_partition_growth memory) - calling it unthrottled
          // stalled loopTask long enough to trip the task watchdog, the
          // same failure mode as the LittleFS-full crash earlier. Cache it
          // and only refresh once a second.
          static size_t cached_free_b = 0;
          static unsigned long last_check_ms = 0;
          unsigned long now_ms = millis();
          if (now_ms - last_check_ms >= 1000 || last_check_ms == 0) {
            size_t total_b = (size_t)LittleFS.totalBytes();
            size_t used_b  = (size_t)LittleFS.usedBytes();
            cached_free_b = (total_b > used_b) ? (total_b - used_b) : 0;
            last_check_ms = now_ms;
          }
          if (cached_free_b >= 1024 * 1024) sprintf(valbufs[URNS_ITEM_FREE], "%.1fMB", cached_free_b / (1024.0 * 1024.0));
          else                              sprintf(valbufs[URNS_ITEM_FREE], "%.1fKB", cached_free_b / 1024.0);
        }

        labels[URNS_ITEM_BACK] = "BACK";
        valbufs[URNS_ITEM_BACK][0] = 0;

        draw_menu_list_disp("URNS", labels, valbufs, URNS_ITEM_COUNT, urns_menu_cursor);
      } else if (menu_state == MENU_STATE_URNS_FREE_DETAIL) {
        // Static snapshot from urns_free_detail_refresh() (called once on
        // entry, menu_confirm_select()) - NOT recomputed here, same
        // throttling reasoning as URNS_ITEM_FREE's own cache above, except
        // here there's no periodic refresh at all since walking every
        // bucket's directory is a heavier operation than a single
        // esp_littlefs_info() call.
        const char *labels[URNS_FREE_DETAIL_ITEM_COUNT];
        char valbufs[URNS_FREE_DETAIL_ITEM_COUNT][24];
        size_t vals[URNS_FREE_DETAIL_ITEM_COUNT - 1] = {
          urns_free_detail_identity, urns_free_detail_announce,
          urns_free_detail_paths, urns_free_detail_messages, urns_free_detail_other
        };
        const char *names[URNS_FREE_DETAIL_ITEM_COUNT - 1] = {
          "Identity", "Announce", "Paths", "Messages", "Other"
        };
        for (uint8_t i = 0; i < URNS_FREE_DETAIL_ITEM_COUNT - 1; i++) {
          labels[i] = names[i];
          size_t v = vals[i];
          // Identity is a single small key file (bytes, not KB-scale) -
          // "0.1KB" rounds away almost all the precision that's actually
          // available for it, so show raw bytes below 1KB for every
          // bucket rather than special-casing just Identity.
          if (v >= 1024 * 1024)   sprintf(valbufs[i], "%.1fMB", v / (1024.0 * 1024.0));
          else if (v >= 1024)     sprintf(valbufs[i], "%.1fKB", v / 1024.0);
          else                    sprintf(valbufs[i], "%zuB", v);
        }
        labels[URNS_FREE_DETAIL_ITEM_BACK] = "BACK";
        valbufs[URNS_FREE_DETAIL_ITEM_BACK][0] = 0;

        draw_menu_list_disp("URNS FREE", labels, valbufs, URNS_FREE_DETAIL_ITEM_COUNT, urns_free_detail_cursor);
      } else if (menu_state == MENU_STATE_URNS_EDIT) {
        if (urns_menu_cursor == URNS_ITEM_ENABLED) {
          draw_menu_edit_disp("URNS ENABLED", staged_urns_enabled ? "ON" : "OFF");
        } else if (urns_menu_cursor == URNS_ITEM_TRANSPORT) {
          draw_menu_edit_disp("TRANSPORT MODE", staged_urns_transport_enabled ? "ON" : "OFF");
        #if HAS_ESPNOW == true
        } else if (urns_menu_cursor == URNS_ITEM_INTERFACE) {
          const char *v = staged_urns_interface_mode == URNS_INTERFACE_ESPNOW_ONLY ? "ESP-NOW" :
                          staged_urns_interface_mode == URNS_INTERFACE_BOTH ? "BOTH" : "LORA";
          draw_menu_edit_disp("URNS INTERFACE", v);
        #endif
        } else if (urns_menu_cursor == URNS_ITEM_LINK_MTU_DISCOVERY) {
          draw_menu_edit_disp("LINK MTU DISCOVERY", staged_urns_link_mtu_discovery ? "ON" : "OFF");
        } else if (urns_menu_cursor == URNS_ITEM_REMOTE_MGMT) {
          draw_menu_edit_disp("REMOTE MANAGEMENT", staged_urns_remote_mgmt_enabled ? "ON" : "OFF");
        } else {
          draw_menu_edit_disp("PROBE DESTINATION", staged_urns_probe_dest_enabled ? "ON" : "OFF");
        }
      } else if (menu_state == MENU_STATE_URNS_PATHS) {
        // Built fresh every draw call, same "recompute live state each
        // frame" convention as draw_menu_memory_disp()'s heap/PSRAM
        // figures - the path table changes as announces arrive, and a
        // stale snapshot would be actively misleading on a "live network
        // state" screen like this one.
        // new_path_table() returns a const&, but TypedStore's begin()/end()
        // aren't const-qualified (neither is the BasicFileStore/HeapStore
        // they wrap) even though iteration here is read-only - the
        // const_cast is scoped to this one read-only display loop, not a
        // library patch (unlike the Transport.cpp/Identity.cpp fixes,
        // this isn't a bug, just an API that doesn't expose a const
        // iteration path).
        RNS::Persistence::NewPathTable& pt = const_cast<RNS::Persistence::NewPathTable&>(RNS::Transport::new_path_table());
        uint8_t row_count = urns_path_display_row_count();

        const char *labels[MENU_URNS_PATH_MAX_ROWS + 1];
        char label_bufs[MENU_URNS_PATH_MAX_ROWS][9];   // 8 hex chars + NUL
        char valbufs[MENU_URNS_PATH_MAX_ROWS + 1][24];

        if (pt.size() == 0) {
          labels[0] = "No Paths";
          valbufs[0][0] = 0;
        } else {
          uint8_t i = 0;
          for (auto it = pt.begin(); it != pt.end() && i < MENU_URNS_PATH_MAX_ROWS; ++it, i++) {
            auto entry = *it;
            snprintf(label_bufs[i], sizeof(label_bufs[i]), "%s", entry.key.toHex().substr(0, 8).c_str());
            labels[i] = label_bufs[i];
            uint8_t hops = entry.value._hops;
            sprintf(valbufs[i], "%u hop%s", hops, hops == 1 ? "" : "s");
          }
        }
        labels[row_count - 1] = "BACK";
        valbufs[row_count - 1][0] = 0;

        draw_menu_list_disp("PATH TABLE", labels, valbufs, row_count, urns_paths_menu_cursor);
      } else if (menu_state == MENU_STATE_URNS_PATH_DETAIL) {
        // Looked up fresh by the captured full hash every draw call (not
        // handed a snapshot at click time) - same "always show live
        // state" convention as the list. If the entry expired/got culled
        // between clicking and viewing (DESTINATION_TIMEOUT etc.), get()
        // just returns false and Expiry shows that plainly instead of
        // whatever stale data happened to be sitting around.
        RNS::Persistence::NewPathTable& pt = const_cast<RNS::Persistence::NewPathTable&>(RNS::Transport::new_path_table());
        RNS::Persistence::DestinationEntry entry;
        bool found = pt.get(urns_path_detail_hash, entry);

        const char *labels[URNS_PATH_DETAIL_ITEM_COUNT];
        char valbufs[URNS_PATH_DETAIL_ITEM_COUNT][24];

        // Preview only (16 of 32 hex chars) - the Hash row itself is
        // enterable and opens MENU_STATE_URNS_PATH_HASH_VIEW for the full
        // value.
        std::string full_hex = urns_path_detail_hash.toHex();
        labels[URNS_PATH_DETAIL_ITEM_HASH] = "Hash";
        snprintf(valbufs[URNS_PATH_DETAIL_ITEM_HASH], 24, "%s", full_hex.substr(0, 16).c_str());

        labels[URNS_PATH_DETAIL_ITEM_EXPIRY] = "Expiry";
        if (!found) {
          sprintf(valbufs[URNS_PATH_DETAIL_ITEM_EXPIRY], "Expired");
        } else {
          double remaining = entry._expires - RNS::Utilities::OS::time();
          if (remaining <= 0) {
            sprintf(valbufs[URNS_PATH_DETAIL_ITEM_EXPIRY], "Expired");
          } else {
            unsigned long rem_s = (unsigned long)remaining;
            if (rem_s < 60) sprintf(valbufs[URNS_PATH_DETAIL_ITEM_EXPIRY], "%lus", rem_s);
            else if (rem_s < 3600) sprintf(valbufs[URNS_PATH_DETAIL_ITEM_EXPIRY], "%lum", rem_s / 60);
            else sprintf(valbufs[URNS_PATH_DETAIL_ITEM_EXPIRY], "%luh", rem_s / 3600);
          }
        }

        labels[URNS_PATH_DETAIL_ITEM_BACK] = "BACK";
        valbufs[URNS_PATH_DETAIL_ITEM_BACK][0] = 0;

        draw_menu_list_disp("PATH DETAIL", labels, valbufs, URNS_PATH_DETAIL_ITEM_COUNT, urns_path_detail_cursor);
      } else if (menu_state == MENU_STATE_URNS_PATH_HASH_VIEW) {
        draw_menu_urns_path_hash_disp();
      }
      #if HAS_LXMF == true
      else if (menu_state == MENU_STATE_MSNGR_LIST) {
        const char *labels[MSNGR_TOP_ITEM_COUNT];
        char valbufs[MSNGR_TOP_ITEM_COUNT][24];
        const uint8_t *icons[MSNGR_TOP_ITEM_COUNT] = { nullptr };
        uint8_t icon_widths[MSNGR_TOP_ITEM_COUNT] = { 0 };
        int8_t text_dx[MSNGR_TOP_ITEM_COUNT] = { 0 };

        labels[MSNGR_TOP_ITEM_INBOX] = "Inbox";
        sprintf(valbufs[MSNGR_TOP_ITEM_INBOX], "%u", (unsigned)(urns_message_store ? urns_message_store->get_unread_count() : 0));
        icons[MSNGR_TOP_ITEM_INBOX] = bm_menu_icon_inbox;
        icon_widths[MSNGR_TOP_ITEM_INBOX] = MENU_ICON_W_INBOX;
        // bm_menu_icon_inbox (Graphics.h) reads visually wider than
        // MENU_ICON_W_INBOX credits it for - widen just this row's gap
        // rather than MENU_ROW_ICON_COL_W/MENU_ROW_TEXT_X_ICONS, which
        // would shift every other row (and every other icon-bearing list
        // sharing those constants) too.
        text_dx[MSNGR_TOP_ITEM_INBOX] = 8;

        labels[MSNGR_TOP_ITEM_BOOKMARKS] = "Bookmarks";
        sprintf(valbufs[MSNGR_TOP_ITEM_BOOKMARKS], "%u", (unsigned)msngr_bookmark_count);
        icons[MSNGR_TOP_ITEM_BOOKMARKS] = bm_menu_icon_msngr_bookmarks;
        icon_widths[MSNGR_TOP_ITEM_BOOKMARKS] = MENU_ICON_W_MSNGR_BOOKMARKS;

        labels[MSNGR_TOP_ITEM_ANNOUNCES] = "Announces";
        {
          uint8_t n = 0;
          for (uint8_t i = 0; i < MSNGR_MAX_ANNOUNCES; i++) if (msngr_announces[i].in_use) n++;
          sprintf(valbufs[MSNGR_TOP_ITEM_ANNOUNCES], "%u", (unsigned)n);
        }
        icons[MSNGR_TOP_ITEM_ANNOUNCES] = bm_menu_icon_msngr_announces;
        icon_widths[MSNGR_TOP_ITEM_ANNOUNCES] = MENU_ICON_W_MSNGR_ANNOUNCES;

        labels[MSNGR_TOP_ITEM_ANNOUNCE_NODE] = "Announce Node";
        valbufs[MSNGR_TOP_ITEM_ANNOUNCE_NODE][0] = 0;
        icons[MSNGR_TOP_ITEM_ANNOUNCE_NODE] = bm_menu_icon_announce_node;
        icon_widths[MSNGR_TOP_ITEM_ANNOUNCE_NODE] = MENU_ICON_W_ANNOUNCE_NODE;

        labels[MSNGR_TOP_ITEM_SETTINGS] = "Settings";
        valbufs[MSNGR_TOP_ITEM_SETTINGS][0] = 0;
        icons[MSNGR_TOP_ITEM_SETTINGS] = bm_menu_icon_msngr_settings;
        icon_widths[MSNGR_TOP_ITEM_SETTINGS] = MENU_ICON_W_MSNGR_SETTINGS;

        labels[MSNGR_TOP_ITEM_BACK] = "BACK";
        valbufs[MSNGR_TOP_ITEM_BACK][0] = 0;
        // Explicit here (not auto-detected) since this list already builds
        // its own icons[] table for Inbox/Announce Node above - see
        // draw_menu_list_disp()'s own comment on why passing any table at
        // all switches a list over to the wide shared column instead of
        // BACK's usual narrow auto-indent.
        icons[MSNGR_TOP_ITEM_BACK] = bm_menu_icon_back;
        icon_widths[MSNGR_TOP_ITEM_BACK] = MENU_ICON_W_BACK;

        // icon_col_shared=false - only Inbox/Bookmarks/Announces/Announce
        // Node/Settings (the rows with icons) get the wider indent; BACK
        // stays at the plain x=8 via its own auto-icon path, same as
        // BT_LIST's own call.
        draw_menu_list_disp("MESSENGER", labels, valbufs, MSNGR_TOP_ITEM_COUNT, msngr_menu_cursor, icons, icon_widths, nullptr, false, text_dx);
      } else if (menu_state == MENU_STATE_MSNGR_INBOX) {
        uint8_t row_count = msngr_inbox_row_count();
        const char *labels[MENU_MSNGR_LIST_MAX_ROWS + 1];
        char label_bufs[MENU_MSNGR_LIST_MAX_ROWS][MSNGR_NAME_MAX_LEN + 1];
        char valbufs[MENU_MSNGR_LIST_MAX_ROWS + 1][24];

        size_t conv_count = urns_message_store ? urns_message_store->get_conversation_count() : 0;
        if (conv_count == 0) {
          labels[0] = "No Messages";
          valbufs[0][0] = 0;
        } else {
          std::vector<RNS::Bytes> convs = urns_message_store->get_conversations();
          uint8_t n = (uint8_t)convs.size();
          if (n > MENU_MSNGR_LIST_MAX_ROWS) n = MENU_MSNGR_LIST_MAX_ROWS;
          for (uint8_t i = 0; i < n; i++) {
            snprintf(label_bufs[i], sizeof(label_bufs[i]), "%s", messenger_peer_display_name(convs[i]).c_str());
            labels[i] = label_bufs[i];
            LXMF::MessageStore::ConversationInfo info = urns_message_store->get_conversation_info(convs[i]);
            if (info.unread_count > 0) sprintf(valbufs[i], "(%u)", (unsigned)info.unread_count);
            else valbufs[i][0] = 0;
          }
        }
        labels[row_count - 1] = "BACK";
        valbufs[row_count - 1][0] = 0;

        draw_menu_list_disp("INBOX", labels, valbufs, row_count, msngr_inbox_cursor);
      } else if (menu_state == MENU_STATE_MSNGR_BOOKMARKS) {
        uint8_t row_count = msngr_bookmarks_row_count();
        const char *labels[MSNGR_MAX_BOOKMARKS + 1];
        char valbufs[MSNGR_MAX_BOOKMARKS + 1][24];

        if (msngr_bookmark_count == 0) {
          labels[0] = "No Bookmarks";
          valbufs[0][0] = 0;
        } else {
          uint8_t vis = 0;
          for (uint8_t i = 0; i < MSNGR_MAX_BOOKMARKS; i++) {
            if (!msngr_bookmarks[i].in_use) continue;
            // Points directly at the persistent global entry's own name
            // buffer (not a stack-local copy) - safe since msngr_bookmarks
            // outlives this draw call, same "labels can point at storage
            // that isn't a string literal" shape as PATH TABLE's hash rows,
            // just not needing a temporary buffer here since the source is
            // already stable.
            labels[vis] = msngr_bookmarks[i].name;
            valbufs[vis][0] = 0;
            vis++;
          }
        }
        labels[row_count - 1] = "BACK";
        valbufs[row_count - 1][0] = 0;

        draw_menu_list_disp("BOOKMARKS", labels, valbufs, row_count, msngr_bookmarks_cursor);
      } else if (menu_state == MENU_STATE_MSNGR_ANNOUNCES) {
        uint8_t row_count = msngr_announces_row_count();
        const char *labels[MSNGR_MAX_ANNOUNCES + 1];
        char valbufs[MSNGR_MAX_ANNOUNCES + 1][24];
        // msngr_announces[].name is genuine UTF-8 off the wire (see
        // msngr_kb_decode_utf8()'s own comment, Messenger.h) - unlike
        // every other place this list of names gets read, this draw
        // path doesn't go through messenger_peer_display_name() (this
        // *is* one of that function's own sources), so it has to decode
        // for itself. Needs its own backing buffer since labels[] just
        // holds pointers - can't decode in place over msngr_announces
        // itself without corrupting the very UTF-8 messenger_peer_
        // display_name() still needs to read on its own next call.
        char name_decoded[MSNGR_MAX_ANNOUNCES][MSNGR_NAME_MAX_LEN + 1];

        uint8_t any = 0;
        for (uint8_t i = 0; i < MSNGR_MAX_ANNOUNCES; i++) if (msngr_announces[i].in_use) any++;

        if (any == 0) {
          labels[0] = "No Announces";
          valbufs[0][0] = 0;
        } else {
          uint8_t vis = 0;
          for (uint8_t i = 0; i < MSNGR_MAX_ANNOUNCES; i++) {
            if (!msngr_announces[i].in_use) continue;
            if (msngr_announces[i].name[0]) {
              msngr_kb_decode_utf8(msngr_announces[i].name, name_decoded[vis], sizeof(name_decoded[vis]));
              labels[vis] = name_decoded[vis];
            } else {
              labels[vis] = "(unnamed)";
            }
            unsigned long ago_s = (millis() - msngr_announces[i].last_heard_ms) / 1000;
            if (ago_s < 60) sprintf(valbufs[vis], "%lus", ago_s);
            else if (ago_s < 3600) sprintf(valbufs[vis], "%lum", ago_s / 60);
            else sprintf(valbufs[vis], "%luh", ago_s / 3600);
            vis++;
          }
        }
        labels[row_count - 1] = "BACK";
        valbufs[row_count - 1][0] = 0;

        draw_menu_list_disp("ANNOUNCES", labels, valbufs, row_count, msngr_announces_cursor);
      } else if (menu_state == MENU_STATE_MSNGR_PEER) {
        // Picks up a message that arrived for this peer while the screen
        // is sitting open - see that function's own comment for why this
        // is safe to call every redraw when messenger_refresh_peer_cache()
        // itself is not.
        messenger_refresh_peer_cache_if_stale(msngr_active_peer_hash);

        uint8_t msg_rows = msngr_peer_msg_row_count();
        uint8_t row_count = msngr_peer_row_count();
        const char *labels[MSNGR_PEER_MAX_ROWS];
        char label_bufs[MSNGR_PEER_MAX_MSG_ROWS][24];
        // "Send: <preset text>" labels - dynamic text now (msngr_presets[]
        // is user-configurable), not the string literals "Send: Hi"/"Send:
        // Bye"/"Send: SOS" used to be, so each needs its own backing
        // buffer the same way label_bufs[] already does for message rows.
        char preset_label_bufs[MSNGR_MAX_PRESETS][24];
        char valbufs[MSNGR_PEER_MAX_ROWS][24];
        const uint8_t *icons[MSNGR_PEER_MAX_ROWS] = { nullptr };
        uint8_t icon_widths[MSNGR_PEER_MAX_ROWS] = { 0 };
        const uint8_t *right_icons[MSNGR_PEER_MAX_ROWS] = { nullptr };
        uint8_t right_icon_widths[MSNGR_PEER_MAX_ROWS] = { 0 };

        // Reads msngr_peer_cache (Messenger.h, populated once on screen
        // entry/after a send), not MessageStore directly - see that
        // cache's own comment for why (this used to read flash on every
        // redraw here, which is what actually caused a real crash - a
        // LoRa DIO0 interrupt landing inside the resulting flash-cache-
        // disabled window hit an SPI-bus-arbitration assert).
        //
        // draw_menu_list_disp() left-aligns labels[] and right-aligns
        // valbufs[] - reused here (instead of a real bubble layout) to
        // put incoming messages on the left and outgoing ones on the
        // right, so direction reads at a glance without needing the old
        // "<"/">" text prefix. bm_menu_icon_msngr_msg_incoming sits left
        // of the label (the plain left-icon column) and
        // bm_menu_icon_msngr_msg_outgoing sits right of the value (the
        // right_icons column draw_menu_list_disp() grew for this) - per
        // user request, reversed from this feature's first pass (outgoing
        // left/incoming right), to match the "yours on the right" reading
        // convention most chat UIs use.
        for (uint8_t i = 0; i < msg_rows; i++) {
          if (msngr_peer_cache[i].incoming) {
            snprintf(label_bufs[i], sizeof(label_bufs[i]), "%s", msngr_peer_cache[i].snippet);
            labels[i] = label_bufs[i];
            valbufs[i][0] = 0;
            icons[i] = bm_menu_icon_msngr_msg_incoming;
            icon_widths[i] = MENU_ICON_W_MSNGR_MSG_INCOMING;
          } else {
            labels[i] = "";
            snprintf(valbufs[i], 24, "%s", msngr_peer_cache[i].snippet);
            right_icons[i] = bm_menu_icon_msngr_msg_outgoing;
            right_icon_widths[i] = MENU_ICON_W_MSNGR_MSG_OUTGOING;
          }
        }

        uint8_t base = msg_rows;
        for (uint8_t i = 0; i < msngr_preset_count; i++) {
          snprintf(preset_label_bufs[i], sizeof(preset_label_bufs[i]), "Send: %s", msngr_presets[i]);
          labels[base + i] = preset_label_bufs[i];
          valbufs[base + i][0] = 0;
        }

        uint8_t fixed_base = base + msngr_preset_count;
        labels[fixed_base + MSNGR_PEER_FIXED_ACTION_SEND_CUSTOM] = "Compose message"; valbufs[fixed_base + MSNGR_PEER_FIXED_ACTION_SEND_CUSTOM][0] = 0;
        icons[fixed_base + MSNGR_PEER_FIXED_ACTION_SEND_CUSTOM] = bm_menu_icon_msngr_compose;
        icon_widths[fixed_base + MSNGR_PEER_FIXED_ACTION_SEND_CUSTOM] = MENU_ICON_W_MSNGR_COMPOSE;
        labels[fixed_base + MSNGR_PEER_FIXED_ACTION_PING] = "Ping"; valbufs[fixed_base + MSNGR_PEER_FIXED_ACTION_PING][0] = 0;
        icons[fixed_base + MSNGR_PEER_FIXED_ACTION_PING] = bm_menu_icon_msngr_ping;
        icon_widths[fixed_base + MSNGR_PEER_FIXED_ACTION_PING] = MENU_ICON_W_MSNGR_PING;

        bool is_bookmarked = messenger_bookmark_find(msngr_active_peer_hash) >= 0;
        labels[fixed_base + MSNGR_PEER_FIXED_ACTION_BOOKMARK] = is_bookmarked ? "Remove Bookmark" : "Add Bookmark";
        valbufs[fixed_base + MSNGR_PEER_FIXED_ACTION_BOOKMARK][0] = 0;

        labels[fixed_base + MSNGR_PEER_FIXED_ACTION_CLEAR] = "Clear Conversation";
        valbufs[fixed_base + MSNGR_PEER_FIXED_ACTION_CLEAR][0] = 0;
        icons[fixed_base + MSNGR_PEER_FIXED_ACTION_CLEAR] = bm_menu_icon_msngr_delete;
        icon_widths[fixed_base + MSNGR_PEER_FIXED_ACTION_CLEAR] = MENU_ICON_W_MSNGR_DELETE;

        labels[fixed_base + MSNGR_PEER_FIXED_ACTION_BACK] = "BACK";
        valbufs[fixed_base + MSNGR_PEER_FIXED_ACTION_BACK][0] = 0;

        char title[24];
        snprintf(title, sizeof(title), "%s", messenger_peer_display_name(msngr_active_peer_hash).c_str());
        // icon_col_shared=false - only the message rows and the Compose
        // message/Ping/Clear Conversation action rows above opted into
        // icons[]/right_icons[], the remaining action rows stay at the
        // plain x=8 they always used.
        draw_menu_list_disp(title, labels, valbufs, row_count, msngr_peer_cursor, icons, icon_widths, nullptr, false, nullptr, right_icons, right_icon_widths);
      } else if (menu_state == MENU_STATE_MSNGR_MSG_DETAIL) {
        uint8_t row_count = msngr_msg_detail_row_count();
        // +3, not +1 - content lines plus the trailing Reply, Delete and
        // BACK rows.
        const char *labels[MSNGR_MSG_DETAIL_MAX_LINES + 3];
        char label_bufs[MSNGR_MSG_DETAIL_MAX_LINES][MSNGR_MSG_DETAIL_CHARS_PER_LINE + 1];
        char valbufs[MSNGR_MSG_DETAIL_MAX_LINES + 3][24];
        const uint8_t *icons[MSNGR_MSG_DETAIL_MAX_LINES + 3] = { nullptr };
        uint8_t icon_widths[MSNGR_MSG_DETAIL_MAX_LINES + 3] = { 0 };

        // Reads msngr_msg_detail_cache_* (Messenger.h, populated once
        // when the message row was selected) - same "don't read flash
        // from the render path" reasoning as MENU_STATE_MSNGR_PEER above.
        const std::string &content = msngr_msg_detail_cache_content;

        uint8_t lines = row_count - 3; // content lines - Reply, Delete, BACK are appended after
        for (uint8_t i = 0; i < lines; i++) {
          size_t start = (size_t)i * MSNGR_MSG_DETAIL_CHARS_PER_LINE;
          if (start < content.size()) {
            snprintf(label_bufs[i], sizeof(label_bufs[i]), "%s", content.substr(start, MSNGR_MSG_DETAIL_CHARS_PER_LINE).c_str());
          } else {
            label_bufs[i][0] = 0;
          }
          labels[i] = label_bufs[i];
          valbufs[i][0] = 0;
        }

        labels[lines] = "Reply";
        valbufs[lines][0] = 0;
        icons[lines] = bm_menu_icon_msngr_reply;
        icon_widths[lines] = MENU_ICON_W_MSNGR_REPLY;
        labels[lines + 1] = "Delete";
        valbufs[lines + 1][0] = 0;
        icons[lines + 1] = bm_menu_icon_msngr_delete;
        icon_widths[lines + 1] = MENU_ICON_W_MSNGR_DELETE;
        labels[lines + 2] = "BACK";
        valbufs[lines + 2][0] = 0;

        // Local (Timezone-shifted) time+date, same apply_tz_offset()
        // convention as the RTC list's own Time/Date rows - time first,
        // per user request, then "RCVD"/DD.MM.YY (not "RECEIVED"/YYYY-MM-DD)
        // so "SENT 14:32:05 07.09.26" still fits this board's ~20-char
        // title width. msngr_civil_from_days() above is the date math,
        // shared with RTC.h's own rtc_civil_from_days() but not tied to
        // this board having an RTC chip. msngr_msg_detail_cache_timestamp
        // is 0 when the message metadata failed to load, so the caption
        // falls back to the plain word rather than "00:00:00 01.01.70".
        char title[24];
        if (msngr_msg_detail_cache_timestamp > 0) {
          uint32_t epoch = apply_tz_offset((uint32_t)msngr_msg_detail_cache_timestamp);
          int32_t days = (int32_t)(epoch / 86400UL);
          uint32_t rem = epoch % 86400UL;
          uint8_t hh = (uint8_t)(rem / 3600); rem %= 3600;
          uint8_t mi = (uint8_t)(rem / 60);
          uint8_t ss = (uint8_t)(rem % 60);
          int32_t yy; uint32_t mo, dd;
          msngr_civil_from_days(days, yy, mo, dd);
          snprintf(title, sizeof(title), "%s %02u:%02u:%02u %02u.%02u.%02u", msngr_msg_detail_cache_incoming ? "RCVD" : "SENT", hh, mi, ss, dd, mo, (unsigned)(yy % 100));
        } else {
          snprintf(title, sizeof(title), "%s", msngr_msg_detail_cache_incoming ? "RCVD" : "SENT");
        }

        // icon_col_shared=false - only the Reply/Delete rows above opted
        // into icons[], BACK keeps its own auto-icon path and the
        // content lines stay at the plain x=8 they always used.
        draw_menu_list_disp(title, labels, valbufs, row_count, msngr_msg_detail_cursor, icons, icon_widths, nullptr, false);
      } else if (menu_state == MENU_STATE_MSNGR_DELETE_CONFIRM) {
        // Plain 2-item list, same draw_menu_list_disp() as everywhere else -
        // same pattern as F/W Update's UPDATE/CANCEL (MENU_STATE_FWUPD_CONFIRM).
        const char *labels[2] = { "DELETE", "CANCEL" };
        char valbufs[2][24];
        valbufs[0][0] = 0;
        valbufs[1][0] = 0;
        draw_menu_list_disp("DELETE MSG?", labels, valbufs, 2, msngr_delete_confirm_cursor);
      } else if (menu_state == MENU_STATE_MSNGR_CLEAR_CONFIRM) {
        const char *labels[2] = { "CLEAR", "CANCEL" };
        char valbufs[2][24];
        valbufs[0][0] = 0;
        valbufs[1][0] = 0;
        draw_menu_list_disp("CLEAR ALL?", labels, valbufs, 2, msngr_clear_confirm_cursor);
      } else if (menu_state == MENU_STATE_MSNGR_TEXT_ENTRY) {
        draw_menu_msngr_keyboard_disp();
      } else if (menu_state == MENU_STATE_MSNGR_DISCARD_CONFIRM) {
        const char *labels[2] = { "DISCARD", "CANCEL" };
        char valbufs[2][24];
        valbufs[0][0] = 0;
        valbufs[1][0] = 0;
        draw_menu_list_disp("DISCARD MSG?", labels, valbufs, 2, msngr_discard_confirm_cursor);
      } else if (menu_state == MENU_STATE_MSNGR_PING_RESULT) {
        // Reads msngr_ping_state/msngr_ping_rtt fresh on every redraw -
        // messenger_ping_process() (RNode_Firmware.ino's loop()) is what
        // actually advances them, same "read live state, don't poll from
        // here" split as every other MSNGR screen's draw code.
        const char *labels[2];
        char valbufs[2][24];
        char status_buf[24];
        switch (msngr_ping_state) {
          case MSNGR_PING_RESOLVING:    snprintf(status_buf, sizeof(status_buf), "Resolving..."); break;
          case MSNGR_PING_ESTABLISHING: snprintf(status_buf, sizeof(status_buf), "Pinging..."); break;
          case MSNGR_PING_SUCCESS:      snprintf(status_buf, sizeof(status_buf), "RTT: %.0f ms", msngr_ping_rtt * 1000.0); break;
          case MSNGR_PING_TIMEOUT:      snprintf(status_buf, sizeof(status_buf), "Timed Out"); break;
          case MSNGR_PING_NO_IDENTITY:  snprintf(status_buf, sizeof(status_buf), "Not Ready"); break;
          case MSNGR_PING_FAILED:       snprintf(status_buf, sizeof(status_buf), "Link Failed"); break;
          default:                      snprintf(status_buf, sizeof(status_buf), "..."); break;
        }
        labels[0] = status_buf;
        valbufs[0][0] = 0;
        labels[1] = "BACK";
        valbufs[1][0] = 0;

        char title[24];
        snprintf(title, sizeof(title), "PING: %s", messenger_peer_display_name(msngr_active_peer_hash).c_str());
        draw_menu_list_disp(title, labels, valbufs, 2, msngr_ping_result_cursor);
      } else if (menu_state == MENU_STATE_MSNGR_SEND_RESULT) {
        // Reads msngr_send_state fresh on every redraw - messenger_send_
        // process()/messenger_on_delivered() (Messenger.h) are what
        // actually advance it, same "read live state, don't poll from
        // here" split as MSNGR_PING_RESULT above. Terminal states
        // (Delivered/No Confirmation) auto-return to MENU_STATE_MSNGR_PEER
        // after MSNGR_SEND_RESULT_POPUP_MS (msngr_send_result_process(),
        // this file) - unlike Ping, no manual BACK needed to see the
        // outcome and move on, matching the Announce/GPS-Sync/NTP-Sync
        // popups' own auto-dismiss convention.
        const char *labels[2];
        char valbufs[2][24];
        char status_buf[24];
        switch (msngr_send_state) {
          case MSNGR_SEND_RESOLVING:  snprintf(status_buf, sizeof(status_buf), "Resolving..."); break;
          case MSNGR_SEND_PENDING: {
            // msngr_send_method - which method LXMessage::pack() actually
            // resolved this send to (see its own declaration, Messenger.h) -
            // OPPORTUNISTIC vs the silent DIRECT upgrade for anything over
            // LORA_ENCRYPTED_PACKET_MDU, not just always "Sending...".
            const char *method_name = (msngr_send_method == LXMF::Type::Message::DIRECT) ? "Direct" : "Opportunistic";
            // Only show the attempt count once a retry has actually
            // started (msngr_send_attempt > 1, set by messenger_send_
            // process()'s live poll of the router's own delivery_
            // attempts() - see msngr_send_attempt's own declaration,
            // Messenger.h) - keeps the common first-try case uncluttered.
            // Drops the "Sending" prefix in that case to leave room for
            // the attempt count within status_buf's 24-byte budget.
            if (msngr_send_attempt > 1) {
              snprintf(status_buf, sizeof(status_buf), "%s (%u/%u)", method_name, (unsigned)msngr_send_attempt, (unsigned)msngr_max_retries);
            } else {
              snprintf(status_buf, sizeof(status_buf), "Sending %s", method_name);
            }
            break;
          }
          case MSNGR_SEND_DELIVERED:  snprintf(status_buf, sizeof(status_buf), "Delivered"); break;
          case MSNGR_SEND_TIMEOUT:    snprintf(status_buf, sizeof(status_buf), "No Confirmation"); break;
          case MSNGR_SEND_UNRESOLVED: snprintf(status_buf, sizeof(status_buf), "Unknown Destination"); break;
          // Router-confirmed failure (messenger_on_failed(), Messenger.h) -
          // exhausted delivery_attempts() and gave up for good. Distinct
          // from MSNGR_SEND_TIMEOUT (this screen's own blind guess-timeout).
          case MSNGR_SEND_FAILED:     snprintf(status_buf, sizeof(status_buf), "Delivery Failed"); break;
          default:                    snprintf(status_buf, sizeof(status_buf), "..."); break;
        }
        labels[0] = status_buf;
        valbufs[0][0] = 0;
        labels[1] = "BACK";
        valbufs[1][0] = 0;

        char title[24];
        snprintf(title, sizeof(title), "SEND: %s", messenger_peer_display_name(msngr_active_peer_hash).c_str());
        draw_menu_list_disp(title, labels, valbufs, 2, msngr_send_result_cursor);
      } else if (menu_state == MENU_STATE_MSNGR_SETTINGS) {
        const char *labels[MSNGR_SETTINGS_ITEM_COUNT];
        char valbufs[MSNGR_SETTINGS_ITEM_COUNT][24];

        labels[MSNGR_SETTINGS_ITEM_RETRIES] = "Retries";
        sprintf(valbufs[MSNGR_SETTINGS_ITEM_RETRIES], "%u", (unsigned)staged_msngr_max_retries);

        labels[MSNGR_SETTINGS_ITEM_RETRY_DELAY] = "Retry Delay";
        sprintf(valbufs[MSNGR_SETTINGS_ITEM_RETRY_DELAY], "%us", (unsigned)staged_msngr_retry_delay_s);

        labels[MSNGR_SETTINGS_ITEM_ANNOUNCE_START] = "Announce at Start";
        sprintf(valbufs[MSNGR_SETTINGS_ITEM_ANNOUNCE_START], staged_msngr_announce_at_start ? "ON" : "OFF");

        labels[MSNGR_SETTINGS_ITEM_ANNOUNCE_INTERVAL] = "Auto Announce";
        sprintf(valbufs[MSNGR_SETTINGS_ITEM_ANNOUNCE_INTERVAL], msngr_announce_interval_labels[staged_msngr_announce_interval_idx]);

        labels[MSNGR_SETTINGS_ITEM_DISPLAY_NAME] = "Name";
        // Not staged (opens the on-screen keyboard directly, not MSNGR_
        // SETTINGS_EDIT - see that item's own OK-button comment), so this
        // shows the live name, not a staged copy. Explicitly truncated to
        // a fixed 10-char + "..." ellipsis (per user request) rather than
        // relying on draw_menu_list_disp()'s own generic value-column
        // eliding, for a consistent look regardless of font/pixel width.
        #define MSNGR_SETTINGS_NAME_VALUE_SHOWN_LEN 10
        valbufs[MSNGR_SETTINGS_ITEM_DISPLAY_NAME][0] = 0;
        if (urns_lxmf_router) {
          std::string name = urns_lxmf_router->display_name();
          if (name.length() > MSNGR_SETTINGS_NAME_VALUE_SHOWN_LEN) {
            snprintf(valbufs[MSNGR_SETTINGS_ITEM_DISPLAY_NAME], sizeof(valbufs[MSNGR_SETTINGS_ITEM_DISPLAY_NAME]), "%.*s...", MSNGR_SETTINGS_NAME_VALUE_SHOWN_LEN, name.c_str());
          } else {
            snprintf(valbufs[MSNGR_SETTINGS_ITEM_DISPLAY_NAME], sizeof(valbufs[MSNGR_SETTINGS_ITEM_DISPLAY_NAME]), "%s", name.c_str());
          }
        }

        labels[MSNGR_SETTINGS_ITEM_PRESETS] = "Preset Messages";
        sprintf(valbufs[MSNGR_SETTINGS_ITEM_PRESETS], "%u", (unsigned)msngr_preset_count);

        labels[MSNGR_SETTINGS_ITEM_BACK] = "BACK";
        valbufs[MSNGR_SETTINGS_ITEM_BACK][0] = 0;

        draw_menu_list_disp("MESSENGER SETTINGS", labels, valbufs, MSNGR_SETTINGS_ITEM_COUNT, msngr_settings_cursor);
      } else if (menu_state == MENU_STATE_MSNGR_SETTINGS_EDIT) {
        char valbuf[24];
        if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_RETRIES) {
          sprintf(valbuf, "%u", (unsigned)staged_msngr_max_retries);
          draw_menu_edit_disp("RETRIES", valbuf);
        } else if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_RETRY_DELAY) {
          sprintf(valbuf, "%us", (unsigned)staged_msngr_retry_delay_s);
          draw_menu_edit_disp("RETRY DELAY", valbuf);
        } else if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_ANNOUNCE_START) {
          sprintf(valbuf, staged_msngr_announce_at_start ? "ON" : "OFF");
          draw_menu_edit_disp("ANNOUNCE AT START", valbuf);
        } else {
          sprintf(valbuf, msngr_announce_interval_labels[staged_msngr_announce_interval_idx]);
          draw_menu_edit_disp("AUTO ANNOUNCE", valbuf);
        }
      } else if (menu_state == MENU_STATE_MSNGR_PRESETS) {
        uint8_t row_count = msngr_presets_row_count();
        const char *labels[MSNGR_MAX_PRESETS + 2]; // presets + Add Preset + BACK
        char valbufs[MSNGR_MAX_PRESETS + 2][24];

        for (uint8_t i = 0; i < msngr_preset_count; i++) {
          // Points directly at the persistent global entry's own text
          // buffer, same "no temporary copy needed" reasoning
          // MENU_STATE_MSNGR_BOOKMARKS' own draw code already uses for
          // msngr_bookmarks[i].name.
          labels[i] = msngr_presets[i];
          valbufs[i][0] = 0;
        }
        uint8_t next = msngr_preset_count;
        if (msngr_preset_count < MSNGR_MAX_PRESETS) {
          labels[next] = "Add Preset";
          valbufs[next][0] = 0;
          next++;
        }
        labels[row_count - 1] = "BACK";
        valbufs[row_count - 1][0] = 0;

        draw_menu_list_disp("PRESET MESSAGES", labels, valbufs, row_count, msngr_presets_cursor);
      } else if (menu_state == MENU_STATE_MSNGR_PRESET_DETAIL) {
        const char *labels[3] = { "Edit", "Delete", "BACK" };
        char valbufs[3][24] = { {0}, {0}, {0} };
        // Title is the preset's own text (e.g. "Hi") - same "show what
        // you're actually looking at" idea as MSNGR_MSG_DETAIL's RCVD/
        // SENT-plus-timestamp title, just simpler since there's no
        // timestamp/direction for a preset.
        draw_menu_list_disp(msngr_presets[msngr_preset_detail_index], labels, valbufs, 3, msngr_preset_detail_cursor);
      }
      #endif
    #endif
      else if (menu_state == MENU_STATE_URNS_RADIO_LIST) {
        const char *labels[URNS_RADIO_ITEM_COUNT];
        char valbufs[URNS_RADIO_ITEM_COUNT][24];

        // Config.h's default initializers (freq/bw/sf=0, txp=0xFF) are what
        // every one of these fields reads as before eeprom_conf_load() ever
        // runs - i.e. genuinely never configured, not just "coincidentally
        // zero". Coding Rate's own unconfigured default is 5, which is also
        // a legal CR (4/5), so it can't be told apart from a real value on
        // its own - only shown as unset when the other four agree nothing's
        // been saved yet.
        bool radio_unset = (staged_lora_freq == 0 && staged_lora_bw == 0 &&
          staged_lora_sf == 0 && staged_lora_txp == 255);

        labels[URNS_RADIO_ITEM_FREQ] = "Frequency";
        if (staged_lora_freq == 0) sprintf(valbufs[URNS_RADIO_ITEM_FREQ], "Unset");
        else sprintf(valbufs[URNS_RADIO_ITEM_FREQ], "%lu.%03lu", (unsigned long)(staged_lora_freq / 1000000), (unsigned long)((staged_lora_freq / 1000) % 1000));

        labels[URNS_RADIO_ITEM_BW] = "Bandwidth";
        if (staged_lora_bw == 0)         sprintf(valbufs[URNS_RADIO_ITEM_BW], "Unset");
        else if (staged_lora_bw >= 1000) sprintf(valbufs[URNS_RADIO_ITEM_BW], "%lukHz", (unsigned long)(staged_lora_bw / 1000));
        else                             sprintf(valbufs[URNS_RADIO_ITEM_BW], "%luHz", (unsigned long)staged_lora_bw);

        labels[URNS_RADIO_ITEM_SF] = "Spreading Factor";
        if (staged_lora_sf == 0) sprintf(valbufs[URNS_RADIO_ITEM_SF], "Unset");
        else sprintf(valbufs[URNS_RADIO_ITEM_SF], "%u", staged_lora_sf);

        labels[URNS_RADIO_ITEM_CR] = "Coding Rate";
        if (radio_unset) sprintf(valbufs[URNS_RADIO_ITEM_CR], "Unset");
        else sprintf(valbufs[URNS_RADIO_ITEM_CR], "4/%u", staged_lora_cr);

        labels[URNS_RADIO_ITEM_TXP] = "TX Power";
        if (staged_lora_txp == 255) sprintf(valbufs[URNS_RADIO_ITEM_TXP], "Unset");
        else sprintf(valbufs[URNS_RADIO_ITEM_TXP], "%udBm", staged_lora_txp);

        labels[URNS_RADIO_ITEM_AUTO_START] = "Auto Start";
        sprintf(valbufs[URNS_RADIO_ITEM_AUTO_START], staged_radio_auto_start_enabled ? "ON" : "OFF");

        labels[URNS_RADIO_ITEM_START] = radio_online ? "Stop Radio" : "Start Radio";
        valbufs[URNS_RADIO_ITEM_START][0] = 0;

        labels[URNS_RADIO_ITEM_CLEAR] = "Clear Settings";
        valbufs[URNS_RADIO_ITEM_CLEAR][0] = 0;

        labels[URNS_RADIO_ITEM_BACK] = "BACK";
        valbufs[URNS_RADIO_ITEM_BACK][0] = 0;

        draw_menu_list_disp("RADIO", labels, valbufs, URNS_RADIO_ITEM_COUNT, urns_radio_menu_cursor);
      } else if (menu_state == MENU_STATE_URNS_RADIO_EDIT) {
        char valbuf[24];
        if (urns_radio_menu_cursor == URNS_RADIO_ITEM_FREQ) {
          sprintf(valbuf, "%lu.%03lu MHz", (unsigned long)(staged_lora_freq / 1000000), (unsigned long)((staged_lora_freq / 1000) % 1000));
          draw_menu_edit_disp("FREQUENCY", valbuf);
        } else if (urns_radio_menu_cursor == URNS_RADIO_ITEM_BW) {
          if (staged_lora_bw >= 1000) sprintf(valbuf, "%lu kHz", (unsigned long)(staged_lora_bw / 1000));
          else                        sprintf(valbuf, "%lu Hz", (unsigned long)staged_lora_bw);
          draw_menu_edit_disp("BANDWIDTH", valbuf);
        } else if (urns_radio_menu_cursor == URNS_RADIO_ITEM_SF) {
          sprintf(valbuf, "%u", staged_lora_sf);
          draw_menu_edit_disp("SPREADING FACTOR", valbuf);
        } else if (urns_radio_menu_cursor == URNS_RADIO_ITEM_CR) {
          sprintf(valbuf, "4/%u", staged_lora_cr);
          draw_menu_edit_disp("CODING RATE", valbuf);
        } else if (urns_radio_menu_cursor == URNS_RADIO_ITEM_AUTO_START) {
          draw_menu_edit_disp("AUTO START", staged_radio_auto_start_enabled ? "ON" : "OFF");
        } else {
          sprintf(valbuf, "%u dBm", staged_lora_txp);
          draw_menu_edit_disp("TX POWER", valbuf);
        }
      }
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

        #if HAS_BATTERY_DIVIDER == true || HAS_PMU == true
          labels[HW_ITEM_BATTERY_LEVEL] = "Battery Level";
          if (battery_ready) sprintf(valbufs[HW_ITEM_BATTERY_LEVEL], "%.0f%%", battery_percent);
          else                sprintf(valbufs[HW_ITEM_BATTERY_LEVEL], "N/A");
        #endif

        #if HAS_BATTERY_DIVIDER == true
          labels[HW_ITEM_BATTERY] = "Battery Voltage";
          if (battery_ready) sprintf(valbufs[HW_ITEM_BATTERY], "%.2fV", battery_voltage);
          else                sprintf(valbufs[HW_ITEM_BATTERY], "N/A");
        #endif

        // No GNSS Chip row here anymore - redundant with GNSS chip
        // identity on the GNSS page itself, see HW_NEXT_A3's own comment.

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

        #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
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
      #if MCU_VARIANT == MCU_ESP32 || MCU_VARIANT == MCU_NRF52
        else if (menu_state == MENU_STATE_MEM_LIST) {
          draw_menu_memory_disp();
        } else if (menu_state == MENU_STATE_MEM_DETAIL) {
          const char *labels[MEM_DETAIL_ITEM_COUNT];
          char valbufs[MEM_DETAIL_ITEM_COUNT][24];

          #if MCU_VARIANT == MCU_ESP32
            uint32_t min_free = 0;
          #endif
          uint32_t total = 0, free_b = 0;
          bool have_reading = true;
          if (mem_menu_cursor == MEM_ITEM_HEAP) {
            #if MCU_VARIANT == MCU_ESP32
              total    = ESP.getHeapSize();
              free_b   = ESP.getFreeHeap();
              min_free = ESP.getMinFreeHeap();
            #else // MCU_NRF52 - cores/nRF5/utility/debug.h, always linked
              total  = dbgHeapTotal();
              free_b = dbgHeapFree();
            #endif
          }
          #if MCU_VARIANT == MCU_ESP32
            else { // MEM_ITEM_PSRAM
              if (psramFound()) {
                total    = ESP.getPsramSize();
                free_b   = ESP.getFreePsram();
                min_free = ESP.getMinFreePsram();
              } else {
                have_reading = false;
              }
            }
          #endif

          labels[MEM_DETAIL_ITEM_TOTAL]   = "Total";
          labels[MEM_DETAIL_ITEM_USED]    = "Used";
          labels[MEM_DETAIL_ITEM_FREE]    = "Free";
          #if MCU_VARIANT == MCU_ESP32
            labels[MEM_DETAIL_ITEM_MINFREE] = "Min Free";
          #endif
          if (have_reading) {
            sprintf(valbufs[MEM_DETAIL_ITEM_TOTAL],   "%.1fKB", total / 1024.0);
            sprintf(valbufs[MEM_DETAIL_ITEM_USED],    "%.1fKB", (total - free_b) / 1024.0);
            sprintf(valbufs[MEM_DETAIL_ITEM_FREE],    "%.1fKB", free_b / 1024.0);
            #if MCU_VARIANT == MCU_ESP32
              sprintf(valbufs[MEM_DETAIL_ITEM_MINFREE], "%.1fKB", min_free / 1024.0);
            #endif
          } else {
            sprintf(valbufs[MEM_DETAIL_ITEM_TOTAL],   "N/A");
            sprintf(valbufs[MEM_DETAIL_ITEM_USED],    "N/A");
            sprintf(valbufs[MEM_DETAIL_ITEM_FREE],    "N/A");
            #if MCU_VARIANT == MCU_ESP32
              sprintf(valbufs[MEM_DETAIL_ITEM_MINFREE], "N/A");
            #endif
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
        // BUILD_NUMBER==0 means this build didn't come through the
        // Makefile/platformio.ini's git-commit-count injection at all (see
        // BUILD_NUMBER's own fallback, Boards.h) - unknown, not a real
        // build 0, so the suffix is omitted rather than showing a
        // misleading ".0".
        if (BUILD_NUMBER != 0) snprintf(valbufs[FWUPD_ITEM_CURRENT], 24, "%s.%d", ota_current_version().c_str(), BUILD_NUMBER);
        else                     snprintf(valbufs[FWUPD_ITEM_CURRENT], 24, "%s", ota_current_version().c_str());

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
