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

  // Used by the IP-octet/datetime editors (draw_menu_addr_edit_disp(),
  // draw_menu_datetime_edit_disp()) and as BOARD_HELTEC_T114's own
  // MENU_FONT (see below) - every other menu screen stays on SMALL_FONT/
  // Org_01 unless it's one of these two exceptions.
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
    #define MENU_CONTENT_X 4 // left edge of the content/border box - see MENU_CONTENT_W's own comment
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
    #define MENU_CONTENT_X 4 // left edge of the content/border box - see MENU_CONTENT_W's own comment
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
    // Full 128px physical SSD1306 width, not 120 - per user request, every
    // menu screen's horizontal separators/borders/selection highlight
    // should reach the actual screen edges on this small OLED, not leave a
    // 4px unused margin on each side. MENU_CONTENT_X=0 (below) is this
    // board group's own left-edge counterpart - T096/T114/WTRACKER_V2 keep
    // their existing 4px margin (MENU_CONTENT_X=4 there) untouched.
    #define MENU_CONTENT_W 128
    #define MENU_CONTENT_X 0
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
  // 5/6 formerly MENU_STATE_WIFI_TEXT_EDIT/_TEXT_CONFIRM (character-wheel
  // SSID/PSK entry) - removed, WiFi SSID/PSK entry now reuses Messenger's
  // on-screen keyboard (MENU_STATE_MSNGR_TEXT_ENTRY, MSNGR_TEXT_ENTRY_
  // PURPOSE_WIFI_SSID/_WIFI_PSK). Left unassigned rather than renumbering
  // every state after it.
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
  #define MENU_STATE_FWUPD_CONFIRM  23  // UPDATE/CANCEL list before Update actually runs - same pattern as MENU_STATE_MSNGR_DISCARD_CONFIRM
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
  #define MENU_STATE_MSNGR_PING_RESULT 43 // live status + BACK, opened from MSNGR_PEER_FIXED_ACTION_PING (Messenger.h's messenger_ping_start()), auto-dismisses on Success (msngr_ping_result_process(), polled from loop(), reuses MSNGR_SEND_RESULT_POPUP_MS) - Timeout/No Identity/Failed still need manual BACK, same as MSNGR_SEND_RESULT's own error states
  #define MENU_STATE_URNS_PATH_HASH_VIEW 44 // full path hash, two plain lines, no captions - opened from MENU_STATE_URNS_PATH_DETAIL's Hash row, dismissed by any input
  #define MENU_STATE_URNS_FREE_DETAIL 45 // urns partition usage broken down by data type (HAS_URNS boards), opened from MENU_STATE_URNS_LIST's Free row - read-only, computed once on entry (never in a draw path - see project_urns_partition_growth memory)
  #define MENU_STATE_MSNGR_SEND_RESULT 46 // live status (Sending.../Delivered/No Confirmation) + BACK, opened from MENU_STATE_MSNGR_PEER's Send Hi/Bye/SOS and MENU_STATE_MSNGR_TEXT_ENTRY's Send key - same "live status + BACK" shape as MENU_STATE_MSNGR_PING_RESULT, auto-dismisses on Delivered/Sent to Node (msngr_send_result_process(), polled from loop())
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
  #if HAS_BLE_HID_HOST == true
    #define MENU_STATE_BLEKBD_LIST 60 // BLE Keyboard submenu - Enabled/Status/Scan for Keyboard/Forget Keyboard/Back, opened from MENU_STATE_BT_LIST's BT_ITEM_KEYBOARD row
    #define MENU_STATE_BLEKBD_EDIT 61 // editing the Enabled field - the only editable row in MENU_STATE_BLEKBD_LIST, same shared-EDIT-state shape as MENU_STATE_BT_SETTINGS/_EDIT
    #define MENU_STATE_BLEKBD_SCAN 62 // live-populated list of nearby HID-advertising keyboards (blekbd_discovered[], BLEKeyboardHost.h) - same scrollable-list shape as MENU_STATE_MSNGR_ANNOUNCES
    #define MENU_STATE_BLEKBD_PAIR_CONFIRM 63 // PAIR/CANCEL list before esp_hidh_dev_open() actually fires against the selected MENU_STATE_BLEKBD_SCAN row - same pattern as MENU_STATE_FWUPD_CONFIRM
    #define MENU_STATE_BLEKBD_PAIRING 64 // live status (Pairing.../Paired!/Failed) + BACK while waiting for ESP_HIDH_OPEN_EVENT, auto-dismisses on success - same "live status + BACK" shape as MENU_STATE_MSNGR_SEND_RESULT, polled via blekbd_pair_result_process() from loop()
    #define MENU_STATE_BLEKBD_FORGET_CONFIRM 65 // FORGET/CANCEL list before the stored peer's EEPROM entry is erased and its bond deleted (ble_store_util_delete_peer(), NOT bt_debond_all()) - same pattern as MENU_STATE_BT_UNPAIR_CONFIRM
    #define MENU_STATE_MSNGR_CHAT 66 // leaner BLE-keyboard-only chat view for an LXMF conversation - no title/footer chrome, last 5 messages (msngr_peer_cache) above a persistent compose box, Enter sends directly via msngr_chat_do_send(), opened from MENU_STATE_MSNGR_PEER's MSNGR_PEER_FIXED_ACTION_CHAT row
  #endif
  #define MENU_STATE_URNS_PATH_DELETE_CONFIRM 67 // DELETE/CANCEL list before a single path-table entry is actually removed (HAS_URNS boards) - same pattern as MENU_STATE_MSNGR_DELETE_CONFIRM, opened from MENU_STATE_URNS_PATH_DETAIL's Delete Path row
  #define MENU_STATE_URNS_IDENTITIES 68 // fixed list of this node's own identity hash + registered destinations (HAS_URNS boards) - read-only, rows open the shared MENU_STATE_URNS_PATH_HASH_VIEW, opened from MENU_STATE_URNS_LIST's Identities row
  #define MENU_STATE_HW_REBOOT_CONFIRM 69 // REBOOT/CANCEL list before hard_reset() actually runs - same pattern as MENU_STATE_FWUPD_CONFIRM, opened from MENU_STATE_HW_LIST's Reboot row
  #define MENU_STATE_URNS_IDENTITY_KEY_VIEW 70 // urns_identity's raw 64-byte private key, base32-encoded (urns_identity_key_encode(), IdentityTransfer.h), read-only, no header/footer chrome, dismissed by any input - scaled-up sibling of MENU_STATE_URNS_PATH_HASH_VIEW sized/wrapped for a full identity key instead of a truncated hash, opened from MENU_STATE_URNS_KEYS's Display Identity Key row (and returns there, not straight to MENU_STATE_URNS_LIST). Available whenever vault_enabled is true regardless of HAS_LXMF - unlike MENU_STATE_MSNGR_TEXT_ENTRY's identity-restore purpose, this is pure read-only display with no keyboard dependency.
  #define MENU_STATE_URNS_KEYS 71 // small submenu (Display Identity Key always, Restore Identity on HAS_LXMF boards, BACK) for manual paper-backup key management - opened from MENU_STATE_URNS_LIST's Keys row (URNS_ITEM_KEYS), itself only reachable when vault_enabled is true (see that item's own comment) - HAS_URNS boards, own submenu, only BACK/its rows do anything, same shape as MENU_STATE_URNS_FREE_DETAIL.
  #define MENU_STATE_URNS_PATH_PURGE_CONFIRM 72 // PURGE/CANCEL list before every entry in RNS::Transport::new_path_table() is removed at once (HAS_URNS boards) - same pattern as MENU_STATE_URNS_PATH_DELETE_CONFIRM, just for the whole table instead of one entry, opened from MENU_STATE_URNS_FREE_DETAIL's Paths row
  #if HAS_LXMF == true
    #define MENU_STATE_URNS_MSG_PURGE_CONFIRM 73 // PURGE/CANCEL list before every stored LXMF message is wiped at once (MessageStore::clear_all()) - same pattern as MENU_STATE_URNS_PATH_PURGE_CONFIRM, opened from MENU_STATE_URNS_FREE_DETAIL's Messages row
  #endif

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
    #define FWUPD_ITEM_UPDATE  2  // opens MENU_STATE_FWUPD_CONFIRM - a plain UPDATE/CANCEL list, same pattern as Messenger's DISCARD/CANCEL (MENU_STATE_MSNGR_DISCARD_CONFIRM)
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
      #if HAS_BLE_HID_HOST == true
        // Opens MENU_STATE_BLEKBD_LIST - separate submenu, not folded into
        // BT_SETTINGS, since it has its own multi-screen scan/pair/forget
        // flow rather than a single toggle.
        #define BT_ITEM_KEYBOARD (BT_ITEM_UNPAIR + 1)
        #define BT_ITEM_BACK (BT_ITEM_KEYBOARD + 1)
      #else
        #define BT_ITEM_BACK (BT_ITEM_UNPAIR + 1)
      #endif
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

  #if HAS_BLE_HID_HOST == true
    #define BLEKBD_ITEM_ENABLED 0 // toggle - staged like BT_SETTINGS_ITEM_BATTERY_SERVICE, opens MENU_STATE_BLEKBD_EDIT
    #define BLEKBD_ITEM_STATUS  1 // read-only - "Disabled" / "Not Paired" / paired keyboard's name (or address if never reconnected since boot)
    #define BLEKBD_ITEM_SCAN    2 // opens MENU_STATE_BLEKBD_SCAN; no-op with a brief popup hint if Enabled is currently off
    #define BLEKBD_ITEM_FORGET  3 // opens MENU_STATE_BLEKBD_FORGET_CONFIRM; no-op if no peer stored
    #define BLEKBD_ITEM_BACK    4
    #define BLEKBD_ITEM_COUNT   5
  #endif

  // Widened to HAS_WIFI too (not just HAS_URNS) - this block is pure
  // #defines/state down through MSNGR_TOP_ITEM_* (genuinely URNS/LXMF-
  // only, stays nested in its own #if HAS_LXMF==true below) and then the
  // shared on-screen-keyboard grid tables/purpose enum (MSNGR_KB_ROWS
  // etc., widened to their own "HAS_LXMF==true || HAS_WIFI==true" guard
  // further down) - WiFi SSID/PSK entry reuses those regardless of
  // whether this board also has HAS_URNS.
  #if HAS_URNS == true || HAS_WIFI == true
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
    // Editable, but NOT a staged/commit-on-exit bool like every other item
    // above - confirming this row immediately runs the full PIN-entry flow
    // (vault_enroll_flow()/vault_disable_flow(), VaultUnlock.h/Vault.h),
    // since it has real, immediate side effects (encrypting or decrypting
    // the identity on disk) that don't fit the "flip a staged bool, apply
    // everything on SAVE & EXIT" model the boot-only settings above use -
    // there's no meaningful "staged but not yet saved" state for "the
    // identity is now encrypted under a PIN the user just entered." See
    // MENU_STATE_URNS_EDIT's own confirm-select handling for
    // URNS_ITEM_VAULT.
    #define URNS_ITEM_VAULT (URNS_ITEM_PROBE_DEST + 1)
    // Opens MENU_STATE_URNS_PATHS - a read-only, scrollable dump of
    // RNS::Transport's live path table (destination hash + hop count),
    // same "own submenu, only BACK does anything" shape as SENSORS_LIST.
    #define URNS_ITEM_PATHS (URNS_ITEM_VAULT + 1)
    // Opens MENU_STATE_URNS_IDENTITIES - a fixed, read-only list of this
    // node's own identities and registered destinations (Transport ID and
    // Probe, plus lxmf.delivery on HAS_LXMF boards), same "own submenu,
    // only BACK does anything, rows open the shared full-hash view" shape as
    // URNS_ITEM_PATHS/URNS_PATH_DETAIL. Not a generic RNS::Transport::
    // destinations() dump - Destination has no public accessor for the
    // app_name/aspects string that would be needed to label an arbitrary
    // entry (see its private _name field, Destination.h), so this lists
    // exactly the destinations this firmware itself constructs (URNS.h/
    // LXMRouter.cpp) instead.
    #define URNS_ITEM_IDENTITIES (URNS_ITEM_PATHS + 1)
    // Opens MENU_STATE_URNS_KEYS - a small submenu (Display Identity Key/
    // Restore Identity/BACK, see that state's own comment) for manually
    // reading or restoring urns_identity's raw key as a paper backup, not
    // reachable at all unless vault_enabled - see URNS_ITEM_VAULT_ONLY_
    // FIRST/_LAST below and their use at the cursor-clamp/draw-compaction
    // sites.
    #define URNS_ITEM_KEYS (URNS_ITEM_IDENTITIES + 1)
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
    #define URNS_ITEM_FREE (URNS_ITEM_KEYS + 1)
    #define URNS_ITEM_BACK (URNS_ITEM_FREE + 1)
    #define URNS_ITEM_COUNT (URNS_ITEM_BACK + 1)

    // URNS_ITEM_KEYS is hidden whenever vault_enabled is false - see the
    // clamp-skip loop in menu_encoder_rotate() and the label-array
    // compaction in MENU_STATE_URNS_LIST's draw block for how this is
    // actually enforced. Kept as a (single-item) range rather than a flat
    // equality check purely so both of those call sites can stay
    // unchanged if a second vault-only row is ever added here later.
    #define URNS_ITEM_VAULT_ONLY_FIRST URNS_ITEM_KEYS
    #define URNS_ITEM_VAULT_ONLY_LAST  URNS_ITEM_KEYS

    // MENU_STATE_URNS_KEYS rows - Display is always compiled (no HAS_LXMF-
    // gated keyboard dependency, see MENU_STATE_URNS_IDENTITY_KEY_VIEW's
    // own comment); Restore only exists on HAS_LXMF boards, since it
    // reuses Messenger's own on-screen keyboard (Menu.h's MSNGR_KB_*) -
    // boards with HAS_URNS but not HAS_LXMF (e.g. MeshPoE-S3) still get
    // Display, and fall back to the existing KISS CMD_IDENTITY_IMPORT
    // flow (IdentityTransfer.h) for restore instead. No runtime hiding
    // needed within this submenu itself (unlike URNS_ITEM_KEYS above) -
    // the whole submenu is already unreachable unless vault_enabled.
    #define URNS_KEYS_ITEM_DISPLAY 0
    #if HAS_LXMF == true
      #define URNS_KEYS_ITEM_RESTORE (URNS_KEYS_ITEM_DISPLAY + 1)
      #define URNS_KEYS_ITEM_BACK (URNS_KEYS_ITEM_RESTORE + 1)
    #else
      #define URNS_KEYS_ITEM_BACK (URNS_KEYS_ITEM_DISPLAY + 1)
    #endif
    #define URNS_KEYS_ITEM_COUNT (URNS_KEYS_ITEM_BACK + 1)

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
    // Recomputes urns_free_detail_refresh()'s cached byte counts in place
    // (screen stays open, cursor untouched) - added because Purge Paths/
    // Purge Messages don't actually shrink their bucket's on-disk size
    // right away: both are backed by microStore's append-only log
    // (FileStore.h), so a delete/clear_all() just appends a tombstone
    // record rather than truncating anything - urns_dir_size_recursive()
    // (URNS.h) keeps reporting the pre-purge size until that segment file
    // is next compacted, which happens on its own schedule
    // (RNS::Transport::cull_new_path_table()/MessageStore's own
    // compaction), not synchronously with the purge. The refresh right
    // after each purge (MENU_STATE_URNS_PATH_PURGE_CONFIRM/MENU_STATE_URNS_
    // MSG_PURGE_CONFIRM above) only re-reads whatever's on disk at that
    // moment, so it can't show the drop either - this row exists so the
    // user can manually re-check once compaction has actually run.
    #define URNS_FREE_DETAIL_ITEM_REFRESH  5
    #define URNS_FREE_DETAIL_ITEM_BACK     6
    #define URNS_FREE_DETAIL_ITEM_COUNT    7

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
    // RNS::Persistence::DestinationEntry's own _expires timestamp. Delete
    // Path opens MENU_STATE_URNS_PATH_DELETE_CONFIRM - a DELETE/CANCEL
    // dialog, same pattern as MENU_STATE_MSNGR_DELETE_CONFIRM - before
    // actually removing the entry from RNS::Transport::new_path_table()
    // via RNS::Transport::remove_path().
    #define URNS_PATH_DETAIL_ITEM_HASH   0
    #define URNS_PATH_DETAIL_ITEM_EXPIRY 1
    #define URNS_PATH_DETAIL_ITEM_DELETE 2
    #define URNS_PATH_DETAIL_ITEM_BACK   3
    #define URNS_PATH_DETAIL_ITEM_COUNT  4

    // MENU_STATE_URNS_IDENTITIES rows - see URNS_ITEM_IDENTITIES' own
    // comment for why this is a fixed list rather than a generic
    // RNS::Transport::destinations() dump. Two independent root
    // RNS::Identity keypairs feed everything else here, each persisted to
    // its own file on the urns LittleFS partition:
    //   - Node Identity (urns_identity, /urns/identity) - this node's own
    //     application identity. lxmf.delivery is a Destination built from
    //     it plus an app_name/aspects pair (LXMF delivery). It used to also
    //     back a second, general-purpose "rnode.onboard" Destination
    //     (urns_destination) - that was only ever a Tier-1 bring-up
    //     smoke-test beacon (its one caller, a boot-time urns_destination.
    //     announce("URNS-TEST"), was removed once LXMF replaced it as the
    //     real over-the-air proof the onboard node worked - see
    //     urns_announce_lxmf()'s own comment, URNS.h) - nothing ever
    //     registered a request/packet handler on it, so it was removed
    //     outright rather than kept as a dead but still-displayed row.
    //   - Transport Identity (RNS::Transport::identity(),
    //     /urns/transport_identity, loaded/created unconditionally inside
    //     Transport::start() - the same on-device counterpart to a plain
    //     rnsd's own storage/transport_identity) - Probe is a Destination
    //     built from *this* one instead, not Node Identity.
    // Display order: Node ID, Transport ID, LXMF ID, Probe ID, BACK.
    #define URNS_ID_ITEM_NODE_IDENTITY 0
    // Unlike Node ID, this is always valid the moment this screen is
    // reachable (Transport::start() runs unconditionally from
    // urns_reticulum.start(), regardless of Transport Mode) - no N/A case.
    #define URNS_ID_ITEM_TRANSPORT_IDENTITY (URNS_ID_ITEM_NODE_IDENTITY + 1)
    #if HAS_LXMF == true
      #define URNS_ID_ITEM_LXMF_DEST (URNS_ID_ITEM_TRANSPORT_IDENTITY + 1)
      #define URNS_ID_NEXT_A (URNS_ID_ITEM_LXMF_DEST + 1)
    #else
      #define URNS_ID_NEXT_A (URNS_ID_ITEM_TRANSPORT_IDENTITY + 1)
    #endif
    // Only exists at the RNS::Transport level while RNode Settings > URNS >
    // Probe Destination is actually active (which itself requires Transport
    // Mode on - see that row's own N/A comment) - row is always shown, same
    // "explain why, don't hide it" reasoning as everywhere else in this
    // menu, but its value falls back to N/A when RNS::Transport::
    // probe_destination() is a default-constructed (Type::NONE) Destination.
    #define URNS_ID_ITEM_PROBE_DEST URNS_ID_NEXT_A
    #define URNS_ID_NEXT_B (URNS_ID_ITEM_PROBE_DEST + 1)
    #define URNS_ID_ITEM_BACK  URNS_ID_NEXT_B
    #define URNS_ID_ITEM_COUNT (URNS_ID_ITEM_BACK + 1)

  #if HAS_LXMF == true
    // Messenger app (Messenger.h) - MENU_STATE_MSNGR_LIST's own item rows.
    #define MSNGR_TOP_ITEM_INBOX         0
    #define MSNGR_TOP_ITEM_BOOKMARKS     1
    #define MSNGR_TOP_ITEM_ANNOUNCES     2
    #define MSNGR_TOP_ITEM_ANNOUNCE_NODE 3 // sends our own LXMF delivery destination announce
    #define MSNGR_TOP_ITEM_SYNC_PROP     4 // manually syncs from the active propagation node bookmark (messenger_prop_node_set_active(), Messenger.h) - see msngr_sync_popup_process() below for the live "Syncing.../Synced N Msgs/Sync Failed" popup
    #define MSNGR_TOP_ITEM_SETTINGS      5 // opens MENU_STATE_MSNGR_SETTINGS
    #define MSNGR_TOP_ITEM_BACK          6
    #define MSNGR_TOP_ITEM_COUNT         7

    // MENU_STATE_MSNGR_SETTINGS - just Retries + Back today, but its own
    // item-index space (mirrors MSNGR_TOP_ITEM_* above) so more Messenger
    // settings can be added later without renumbering the top screen.
    #define MSNGR_SETTINGS_ITEM_RETRIES           0
    #define MSNGR_SETTINGS_ITEM_RETRY_DELAY       1
    #define MSNGR_SETTINGS_ITEM_ANNOUNCE_START    2
    #define MSNGR_SETTINGS_ITEM_ANNOUNCE_INTERVAL 3
    #define MSNGR_SETTINGS_ITEM_PROP_ON_FAIL      4 // ON/OFF - LXMRouter::set_fallback_to_propagation() (Messenger.h)
    #define MSNGR_SETTINGS_ITEM_SYNC_INTERVAL     5 // stepped preset incl. Off - msngr_sync_interval_presets_s[] (Messenger.h)
    #define MSNGR_SETTINGS_ITEM_SYNC_LIMIT        6 // stepped 0-254, 0=unlimited (Messenger.h)
    #define MSNGR_SETTINGS_ITEM_STAMP_COST        7 // stepped 0-255, 0=disabled (Messenger.h)
    #define MSNGR_SETTINGS_ITEM_DISPLAY_NAME      8 // opens MENU_STATE_MSNGR_TEXT_ENTRY (reused from the message composer), not MSNGR_SETTINGS_EDIT
    #define MSNGR_SETTINGS_ITEM_PRESETS           9 // opens MENU_STATE_MSNGR_PRESETS
    #define MSNGR_SETTINGS_ITEM_BACK              10
    #define MSNGR_SETTINGS_ITEM_COUNT             11

    // "ANNOUNCED" has nothing to acknowledge (unlike "NOT READY", which
    // stays up until dismissed - same success/error asymmetry as NTP sync's
    // own popup, see NTP_SYNC_SUCCESS_POPUP_MS), so it auto-dismisses on
    // its own after this long.
    #define MSNGR_ANNOUNCE_POPUP_MS 10000

    // MSNGR_TOP_ITEM_SYNC_PROP's own "Synced N Msg(s)" result popup -
    // same auto-dismiss treatment as ACTION_POPUP_MS's other result
    // banners (there's nothing to acknowledge on success). "Sync Failed"
    // and "Prop Not Set" deliberately don't use this - same "errors stay
    // up until dismissed" asymmetry as MSNGR_ANNOUNCE_POPUP_MS's own
    // "NOT READY" comment above.
    #define MSNGR_SYNC_POPUP_MS ACTION_POPUP_MS

    // True from the moment MSNGR_TOP_ITEM_SYNC_PROP actually kicks off a
    // sync (LXMRouter::request_messages_from_propagation_node()) until
    // msngr_sync_popup_process() below observes it reach PR_COMPLETE/
    // PR_FAILED and updates the still-open "Syncing..." popup with the
    // real result - polled from loop() (RNode_Firmware.ino) the same way
    // msngr_send_result_process() already is, since a propagation sync is
    // a multi-second path-request/link/fetch sequence, not something that
    // finishes within the single confirm_select() call that starts it.
    // Left true (and silently dropped once seen) if the user manually
    // dismisses the "Syncing..." popup before it resolves - see that
    // function's own comment for why forcing the popup back open in that
    // case would be more surprising than just not reporting the result.
    bool msngr_sync_popup_pending = false;

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
    // Toggles the peer's outbound send method - label switches Send Direct
    // (OPPORTUNISTIC, silently upgraded to DIRECT for oversized content)/
    // Send Propagated (skips straight to the active propagation node,
    // no direct attempt at all) - messenger_toggle_delivery_mode()
    // (Messenger.h). Persisted per-bookmark; session-only for a peer
    // that isn't bookmarked - see that function's own comment.
    #define MSNGR_PEER_FIXED_ACTION_DELIVERY_MODE 2
    #define MSNGR_PEER_FIXED_ACTION_BOOKMARK    3 // label switches Add/Remove Bookmark
    // Opens MENU_STATE_URNS_PATH_HASH_VIEW (reused as-is) against
    // msngr_active_peer_hash - same full two-line hex view MSNGR_PEER_PROP_
    // ACTION_SHOW_HASH already uses for a Propagation-type bookmark.
    #define MSNGR_PEER_FIXED_ACTION_SHOW_HASH   4
    #define MSNGR_PEER_FIXED_ACTION_CLEAR       (MSNGR_PEER_FIXED_ACTION_SHOW_HASH + 1) // opens MENU_STATE_MSNGR_CLEAR_CONFIRM
    #if HAS_BLE_HID_HOST == true
      // Opens MENU_STATE_MSNGR_CHAT - leaner BLE-keyboard-only compose view
      // (no on-screen grid, no title/footer chrome) - only meaningful on a
      // board that can actually have a physical keyboard paired.
      #define MSNGR_PEER_FIXED_ACTION_CHAT (MSNGR_PEER_FIXED_ACTION_CLEAR + 1)
      #define MSNGR_PEER_FIXED_ACTION_BACK (MSNGR_PEER_FIXED_ACTION_CHAT + 1)
    #else
      #define MSNGR_PEER_FIXED_ACTION_BACK (MSNGR_PEER_FIXED_ACTION_CLEAR + 1)
    #endif
    #define MSNGR_PEER_FIXED_ACTION_COUNT (MSNGR_PEER_FIXED_ACTION_BACK + 1)

    // MENU_STATE_MSNGR_PEER's entire action set when msngr_active_peer_hash
    // is a Propagation-type bookmark instead of an LXMF one
    // (messenger_bookmark_is_prop_node(), Messenger.h) - Compose/Clear
    // Conversation/preset Send buttons all assume an LXMF delivery
    // destination, which a propagation node isn't (it answers on "lxmf"/
    // "propagation", not "lxmf"/"delivery" - no conversation ever exists
    // to show either, so message rows/presets are skipped entirely too,
    // not just these two). Replaces MSNGR_PEER_FIXED_ACTION_* wholesale
    // rather than coexisting with it - see msngr_peer_row_count() and the
    // draw/confirm handling's own is_prop_node branch for where the two
    // action sets fork. Ping is the one fixed action with its own
    // Propagation-side counterpart (MSNGR_PEER_PROP_ACTION_PING below)
    // rather than being dropped outright - see messenger_ping_start()'s
    // is_prop argument (Messenger.h) for how it targets "propagation"
    // instead of "delivery".
    #define MSNGR_PEER_PROP_ACTION_SYNC       0 // manually syncs against THIS bookmark's node specifically, making it active first if it wasn't already - msngr_prop_sync_start()
    // Opens MENU_STATE_MSNGR_PING_RESULT (reused as-is), same bare
    // Link-handshake-as-pong mechanism MSNGR_PEER_FIXED_ACTION_PING uses
    // against an LXMF peer's lxmf.delivery destination - messenger_ping_
    // start(hash, true) (Messenger.h) just builds the Link against this
    // bookmark's lxmf.propagation destination instead, since a real
    // propagation node has to accept link requests there too (it's how
    // sync/propagated-delivery already work - see that function's own
    // comment for why this is expected to work against any reference-
    // implementation propagation node).
    #define MSNGR_PEER_PROP_ACTION_PING       1
    #define MSNGR_PEER_PROP_ACTION_SHOW_HASH  2 // opens MENU_STATE_URNS_PATH_HASH_VIEW (reused as-is) against this bookmark's hash, same full two-line hex view URNS Path Table's own Hash row uses
    #define MSNGR_PEER_PROP_ACTION_SET_ACTIVE 3 // label switches Set/Unset Active - messenger_prop_node_set_active()/_clear_active()
    #define MSNGR_PEER_PROP_ACTION_RENAME     4 // opens MENU_STATE_MSNGR_TEXT_ENTRY (MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_RENAME) - messenger_bookmark_rename(), the only way a propagation node ever gets a friendly display name (see that function's own comment, Messenger.h)
    #define MSNGR_PEER_PROP_ACTION_REMOVE     5 // messenger_bookmark_remove() - same action MSNGR_PEER_FIXED_ACTION_BOOKMARK's "Remove Bookmark" wording does for an LXMF peer, just unconditional here (a Propagation-type bookmark is always bookmarked - that's the only way one exists)
    #define MSNGR_PEER_PROP_ACTION_BACK       6
    #define MSNGR_PEER_PROP_ACTION_COUNT      7

    // Worst-case row count for MENU_STATE_MSNGR_PEER's local draw arrays -
    // message rows + every preset slot filled + all fixed actions, even
    // though msngr_peer_row_count() (the *actual* count for any given
    // peer/preset configuration) is almost always smaller.
    #define MSNGR_PEER_MAX_ROWS (MSNGR_PEER_MAX_MSG_ROWS + MSNGR_MAX_PRESETS + MSNGR_PEER_FIXED_ACTION_COUNT)

    // Marquee-scroll timing for MENU_STATE_MSNGR_PEER's currently-selected
    // message row (msngr_peer_scroll_*, Messenger.h) - see that draw
    // block's own comment (this file, below) for the full mechanism.
    #define MSNGR_PEER_SCROLL_STEP_MS 250
    #define MSNGR_PEER_SCROLL_WINDOW  23 // label_bufs[]/valbufs[] are char[24] (draw_menu_list_disp()'s own fixed signature)
    // Per user request, a fully-scrolled marquee doesn't just stop at the
    // end - it holds there for this long, then loops back to the start and
    // scrolls again, for as long as the row stays selected/visible.
    #define MSNGR_PEER_SCROLL_LOOP_PAUSE_MS 2000
    // Per user request, a fresh loop back to the start also pauses here
    // before actually scrolling again, so the beginning of the message is
    // actually readable rather than immediately sliding away.
    #define MSNGR_PEER_SCROLL_START_PAUSE_MS 1000

    // MENU_STATE_MSNGR_MSG_DETAIL - a message's content, word-wrapped
    // across up to this many rows, plus one DELETE row (opens
    // MENU_STATE_MSNGR_DELETE_CONFIRM) and one BACK row, same "split across
    // several label rows" shape as URNS_PATH_DETAIL's two-row hash above.
    #define MSNGR_MSG_DETAIL_MAX_LINES 7
  #endif

  // Shared on-screen keyboard mechanics (grid layouts, cursor/shift state,
  // insert/backspace primitives) - reused by both the LXMF Messenger app
  // (compose/display-name/preset/bookmark/identity-restore purposes) and
  // WiFi SSID/PSK entry (MSNGR_TEXT_ENTRY_PURPOSE_WIFI_SSID/_WIFI_PSK,
  // Menu.h's WIFI_ITEM_SSID/PSK), hence available on either HAS_LXMF or
  // HAS_WIFI boards rather than gated to HAS_LXMF alone like the rest of
  // the Messenger app around it.
  #if HAS_LXMF == true || HAS_WIFI == true
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

    // MENU_STATE_MSNGR_TEXT_ENTRY is reused for editing the LXMF display
    // name (RNode Settings > Messenger > Settings > Display Name), adding/
    // editing a preset message (RNode Settings > Messenger > Settings >
    // Preset Messages), composing a message, typing a bookmark's raw
    // destination hash (RNode Settings > Messenger > Bookmarks > Add by
    // Hash), and renaming a Propagation-type bookmark (RNode Settings >
    // Messenger > Bookmarks > <a Propagation-type bookmark> > Rename) -
    // this tracks which, since the Send key's actual action, the
    // exit/discard target, and (for BOOKMARK_HASH) the active keyboard
    // layout itself all differ. All the actual typing mechanics (grid
    // cursor/shift/buffer, declared further down) are identical regardless
    // of purpose, only specific branches care. Declared up here, before
    // MSNGR_KB_LAYOUT below, so msngr_kb_active_layout()/msngr_kb_active_
    // rows() (and every hold-gesture poller that needs to know which
    // layout is live right now) can reference it.
    #define MSNGR_TEXT_ENTRY_PURPOSE_MESSAGE         0
    #define MSNGR_TEXT_ENTRY_PURPOSE_DISPLAY_NAME    1
    #define MSNGR_TEXT_ENTRY_PURPOSE_PRESET          2
    #define MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH   3 // MSNGR_BOOKMARKS' "Add by Hash" row - typing an arbitrary peer's destination hash directly, not learned from an announce/message
    #define MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_RENAME 4 // MSNGR_PEER_PROP_ACTION_RENAME - a friendly name for a Propagation-type bookmark, which (unlike an LXMF peer) never gets one automatically
    #define MSNGR_TEXT_ENTRY_PURPOSE_IDENTITY_RESTORE 5 // URNS_KEYS_ITEM_RESTORE - typing the raw 64-byte identity private key as its VAULT_IDENTITY_KEY_BASE32_LEN-char Base32 encoding (IdentityTransfer.h), using its own dedicated MSNGR_KB_LAYOUT_BASE32 grid (not the hex one - identity keys aren't hex)
    #define MSNGR_TEXT_ENTRY_PURPOSE_WIFI_SSID       6 // WIFI_ITEM_SSID (MENU_STATE_WIFI_LIST) - reuses this same on-screen keyboard for WiFi SSID entry instead of the old character-wheel dialog, available whenever HAS_WIFI is true regardless of HAS_LXMF (see the widened-guard comments throughout this file's keyboard mechanics)
    #define MSNGR_TEXT_ENTRY_PURPOSE_WIFI_PSK        7 // WIFI_ITEM_PSK (MENU_STATE_WIFI_LIST) - same as WIFI_SSID above, for the passphrase field
    uint8_t msngr_text_entry_purpose = MSNGR_TEXT_ENTRY_PURPOSE_MESSAGE;

    // MENU_STATE_MSNGR_TEXT_ENTRY - linear (row-major) cursor into
    // MSNGR_KB_LAYOUT, a persistent Shift toggle (caps-lock style, not
    // meshtastic's one-shot long-press), and the message/credential being
    // composed. Reset (cursor to 0, shift off, buffer cleared) every time
    // the screen is opened fresh. Declared up here, alongside msngr_text_
    // entry_purpose, rather than down with the rest of the keyboard
    // mechanics (msngr_kb_active_layout() etc., further below) - Chat's
    // own compose functions (msngr_chat_insert_char() etc., HAS_BLE_HID_
    // HOST-only, below) and the WiFi SSID/PSK vars (staged_wifi_ssid/psk,
    // also below) both need msngr_text_entry_buf declared before their own
    // point in the file, and those two land on opposite sides of where
    // the rest of the keyboard mechanics itself has to sit (see that
    // block's own comment on why it can't move any earlier than
    // staged_wifi_ssid/psk) - so the buffer/cursor state alone has to
    // live earlier than all three.
    uint8_t msngr_kb_cursor = 0;
    bool msngr_kb_shift_on = false;
    // Latin/Cyrillic layout toggle - lives on the same Shift key as case
    // (see MSNGR_KB_ALT_HOLD_MS), not a separate grid cell (the grid is
    // already full - 4x11 with no spare slot). Reset alongside cursor/shift
    // every time the screen is opened fresh, same as those.
    bool msngr_kb_lang_ru = false;
    char msngr_text_entry_buf[MSNGR_TEXT_ENTRY_MAX_LEN + 1] = {0};

    // MSNGR_TEXT_ENTRY_PURPOSE_WIFI_PSK only - draw_msngr_compose_box()
    // masks every character with '*' except the one just typed, which
    // stays in plain text for this long before masking too (same "briefly
    // show the last digit" convention phone PIN entry uses). 0 (its reset
    // value - both here and after every backspace, see msngr_kb_do_
    // backspace()) means "nothing recently typed", so a freshly-opened
    // screen with a preloaded saved password shows fully masked from the
    // very first frame, never briefly revealing it.
    #define MSNGR_KB_PSK_REVEAL_MS 800
    unsigned long msngr_kb_last_insert_ms = 0;

    // Which kind of bookmark MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH's SAVE
    // key will create - toggled via the Type cell on MSNGR_KB_LAYOUT_HEX's
    // own row 2 (MSNGR_KB_TYPE_TOGGLE below). Reset to LXMF every time the
    // Add by Hash screen is (re-)opened (MENU_STATE_MSNGR_BOOKMARKS' row-
    // select handler), same as msngr_kb_cursor/shift/lang_ru are.
    // MSNGR_BOOKMARK_TYPE_LXMF/_PROPAGATION (Messenger.h, HAS_LXMF-only) -
    // only meaningful for MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH, itself
    // only ever reachable on a HAS_LXMF board, but the variable is declared
    // unconditionally here (this whole region is shared with HAS_WIFI) so
    // every non-purpose-specific keyboard helper can reference it without
    // its own guard - 0 is MSNGR_BOOKMARK_TYPE_LXMF's own value.
    uint8_t msngr_kb_bookmark_type = 0;

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

    // MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH's own restricted grid - a
    // destination hash is only ever 0-9/A-F, so the full EN/RU keyboard is
    // unnecessary friction to navigate for a 32-char hash (confirmed
    // annoying on hardware before this existed). Reuses the exact same
    // per-cell pixel grid every other layout gets - draw_menu_msngr_
    // keyboard_disp()'s col_x/col_w split (10 narrow digit-width columns +
    // 1 wide action column) is shared unchanged by every row here too, so
    // '0'-'9' lines up with 'A'-'F' column-for-column (both start at
    // col_x[0]) and DEL/SAVE/BACK all sit in the exact same rightmost
    // column - not custom-fitted per row.
    //
    // Row 0: 10 digits + DEL, same shape as every other layout's own rows.
    // Row 1: 6 hex letters (cols 0-5, unused past that) + SAVE in the
    // shared action column (col 10) - "SAVE", not "SEND", since this
    // bookmarks a hash locally, nothing goes out over the air
    // (draw_menu_msngr_keyboard_disp()'s own hex_mode label override).
    // Row 2: a Type toggle (col 0, spans the same 10 narrow columns '0'-'9'/
    // 'A'-'F' sit in above it - see draw_menu_msngr_keyboard_disp()'s own
    // MSNGR_KB_TYPE_TOGGLE handling) + BACK in the action column - not
    // sharing row 1's narrow cells (illegible) and not left out of the
    // grid entirely either (unreachable on a tap-only board otherwise -
    // menu_button_press() has no long-hold-to-leave path, only "hold to
    // press the highlighted key" the same as every other key, so BACK
    // needs a real cell here same as everywhere else) - an extra row
    // spent on what was originally just a single button, on request,
    // rather than trying to squeeze BACK in elsewhere. Type picks which
    // kind of bookmark SAVE (row 1) creates - LXMF (default) or
    // Propagation (RNode Settings > Messenger > Bookmarks > <a
    // Propagation-type bookmark> > Set/Unset Active) - see
    // MSNGR_BOOKMARK_TYPE_LXMF/_PROPAGATION (Messenger.h).
    //
    // '\0' cells are genuinely unused - draw_menu_msngr_keyboard_disp()
    // skips drawing them entirely (MSNGR_KB_NONE below) and msngr_kb_
    // cursor_rc()/msngr_kb_active_key_count() build cursor navigation by
    // scanning for non-'\0' cells, so they're never reachable either -
    // not dead placeholder buttons, just blank canvas.
    #define MSNGR_KB_HEX_ROWS 3
    static const char MSNGR_KB_LAYOUT_HEX[MSNGR_KB_HEX_ROWS][MSNGR_KB_COLS] = {
      {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '\b'},
      {'A', 'B', 'C', 'D', 'E', 'F', '\0', '\0', '\0', '\0', '\n'},
      {'\x04', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\x1b'},
    };

    // MSNGR_TEXT_ENTRY_PURPOSE_IDENTITY_RESTORE's own grid - the identity
    // key is Base32 (RFC 4648: A-Z, 2-7, '=' padding - see IdentityTransfer.h's
    // identity_key_to_base32()/identity_key_from_base32()), not hex, so
    // MSNGR_KB_LAYOUT_HEX's 0-9/A-F alphabet doesn't cover it. Unlike the
    // hex grid, this uses the full standard MSNGR_KB_ROWS (4) - 33 symbol
    // cells (26 letters + 6 digits + '=') don't fit hex's compact 3-row/
    // mostly-empty-row-2 shape - so msngr_kb_active_rows() does NOT
    // special-case this purpose (falls through to its own MSNGR_KB_ROWS
    // default), only msngr_kb_active_layout()/_cursor_rc()/_active_key_
    // count() do, same shape as the hex purpose gets but through its own
    // table/order/count below instead of reusing hex's.
    static const char MSNGR_KB_LAYOUT_BASE32[MSNGR_KB_ROWS][MSNGR_KB_COLS] = {
      {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', '\b'},
      {'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', '\n'},
      {'U', 'V', 'W', 'X', 'Y', 'Z', '2', '3', '4', '5', '\x1b'},
      {'6', '7', '=', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'},
    };

    // Same "explicit per-cursor-value (row,col) table, not division/
    // modulo" reasoning as MSNGR_KB_HEX_ORDER above - MSNGR_KB_LAYOUT_
    // BASE32 has real cells scattered unevenly (row 3 only has 3), so a
    // flat division would let the cursor land on blank cells division
    // alone can't skip. Reading order: A-J, DEL, K-T, SAVE, U-5, BACK,
    // 6/7/=.
    static const uint8_t MSNGR_KB_BASE32_ORDER[36][2] = {
      {0,0}, {0,1}, {0,2}, {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {0,9}, // A-J
      {0,10}, // DEL
      {1,0}, {1,1}, {1,2}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7}, {1,8}, {1,9}, // K-T
      {1,10}, // SAVE
      {2,0}, {2,1}, {2,2}, {2,3}, {2,4}, {2,5}, {2,6}, {2,7}, {2,8}, {2,9}, // U-Z,2-5
      {2,10}, // BACK
      {3,0}, {3,1}, {3,2}, // 6, 7, =
    };
    #define MSNGR_KB_BASE32_KEY_COUNT 36

    // MSNGR_TEXT_ENTRY_PURPOSE_WIFI_SSID/_WIFI_PSK's own alternate page -
    // toggled onto the same grid via the same hold-Shift gesture EN/RU
    // uses everywhere else (msngr_kb_active_layout()'s own WIFI branch
    // repurposes msngr_kb_lang_ru as a Letters/Symbols flag for these two
    // purposes specifically), not a dedicated cell - same reasoning
    // MSNGR_KB_LAYOUT_RU gets away with reusing Shift instead of a
    // separate key. A full, uniform MSNGR_KB_ROWS x MSNGR_KB_COLS
    // rectangle like every layout except HEX/BASE32, so msngr_kb_active_
    // rows()/_cursor_rc()/_active_key_count() need no special-casing for
    // it (plain division/modulo already works).
    //
    // Digits aren't repeated here - MSNGR_KB_LAYOUT's own row 0 already
    // has '1'-'9'/'0' directly reachable without switching pages at all,
    // same as every other purpose. That leaves all 39 non-meta cells free
    // for punctuation - the old character-wheel dialog this on-screen
    // keyboard replaces for WiFi SSID/PSK entry supported this exact set
    // (WHEEL_SYMS, 32 symbols), reading-order left-to-right/top-to-bottom,
    // same convention as every other layout table here. Meta keys stay at
    // the same fixed positions as MSNGR_KB_LAYOUT/_RU (Backspace [0][10],
    // Send [1][10], Shift/toggle [2][9], Space [2][10], Back [3][10]) -
    // row 3's 7 trailing cells are genuinely unused (MSNGR_KB_NONE, same
    // convention as MSNGR_KB_LAYOUT_HEX/_BASE32's own blank cells).
    static const char MSNGR_KB_LAYOUT_SYMBOLS[MSNGR_KB_ROWS][MSNGR_KB_COLS] = {
      {'!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '\b'},
      {'+', ',', '-', '.', '/', ':', ';', '<', '=', '>', '\n'},
      {'?', '@', '[', '\\', ']', '^', '_', '`', '{', '\x02', ' '},
      {'|', '}', '~', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\x1b'},
    };

    #define MSNGR_KB_CHAR      0
    #define MSNGR_KB_BACKSPACE 1
    #define MSNGR_KB_SEND      2
    #define MSNGR_KB_SPACE     3
    #define MSNGR_KB_SHIFT     4
    #define MSNGR_KB_BACK      5
    #define MSNGR_KB_NONE      6 // unused cell (MSNGR_KB_LAYOUT_HEX's '\0' entries) - never reachable by cursor navigation (msngr_kb_cursor_rc()/msngr_kb_active_key_count() both skip these) or drawn (draw_menu_msngr_keyboard_disp() skips them too), but msngr_kb_key_type() still needs a safe classification for '\0' so nothing ever mistakes it for a literal character to insert.
    #define MSNGR_KB_TYPE_TOGGLE 7 // MSNGR_KB_LAYOUT_HEX row 2's Type cell ('\x04') - cycles msngr_kb_bookmark_type between MSNGR_BOOKMARK_TYPE_LXMF/_PROPAGATION on press, same "press to change" shape as Shift, not a left/right stepper (this screen's rotation is spoken for as cursor movement, same as every other MSNGR_TEXT_ENTRY purpose).

    uint8_t msngr_kb_key_type(char ch) {
      if (ch == '\0') return MSNGR_KB_NONE;
      if (ch == '\b') return MSNGR_KB_BACKSPACE;
      if (ch == '\n') return MSNGR_KB_SEND;
      if (ch == ' ')  return MSNGR_KB_SPACE;
      if (ch == '\x02') return MSNGR_KB_SHIFT;
      if (ch == '\x1b') return MSNGR_KB_BACK;
      if (ch == '\x04') return MSNGR_KB_TYPE_TOGGLE;
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

    // Plain action row, needs no hardware capability at all (same reasoning
    // as Node Uptime just above) - opens MENU_STATE_HW_REBOOT_CONFIRM, a
    // REBOOT/CANCEL dialog (same pattern as MENU_STATE_FWUPD_CONFIRM/
    // MENU_STATE_MSNGR_DELETE_CONFIRM) before actually calling hard_reset()
    // (Utilities.h).
    #define HW_ITEM_REBOOT HW_NEXT_F
    #define HW_NEXT_G      (HW_NEXT_F + 1)

    #define HW_ITEM_BACK  HW_NEXT_G
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

    #if HAS_BLE_HID_HOST == true
      // Ambient "keyboard connected/disconnected" notice - same draw_menu_
      // status_rect() primitive draw_button_hold_overlay() just used above,
      // superimposed directly on top of WHATEVER is currently on screen
      // (idle main screen, an open Settings/Messenger submenu, Chat, ...)
      // and redrawn every cycle for as long as it's armed - not routed
      // through the menu-open popup machinery (MENU_STATE_STATUS_POPUP),
      // which is a real state transition (see blekbd_notice_process()'s own
      // comment, this file, below) draw_menu_status_rect() itself has
      // already proven safe to call regardless of menu-open state (that's
      // exactly what MENU_STATE_STATUS_POPUP's own draw case does, via
      // menu_draw_popup()). 0 = inactive. Per user request: shown
      // everywhere, not just while idle - a keyboard dropping/reconnecting
      // mid-navigation is exactly when this is most useful to notice.
      unsigned long blekbd_notice_until_ms = 0;
      char blekbd_notice_text[20] = {0};

      // Called every cycle from update_display() (Display.h), both the
      // menu-open and menu-closed paths - same "redraw every cycle or the
      // next ordinary refresh wipes it" reason as draw_button_hold_
      // overlay()'s own comment.
      void draw_blekbd_notice_overlay() {
        if (blekbd_notice_until_ms == 0 || (int32_t)(millis() - blekbd_notice_until_ms) >= 0) {
          if (blekbd_notice_until_ms != 0) {
            blekbd_notice_until_ms = 0;
            #if BOARD_MODEL == BOARD_HELTEC_T096 || BOARD_MODEL == BOARD_HELTEC_WTRACKER_V2 || BOARD_MODEL == BOARD_HELTEC_T1 || BOARD_MODEL == BOARD_HELTEC_T114
              menu_status_rect_clear();
            #endif
          }
          return;
        }
        draw_menu_status_rect(blekbd_notice_text);
      }
    #endif
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
    // 0 = DELETE, 1 = CANCEL - same list-with-cursor pattern as
    // msngr_delete_confirm_cursor, defaulting to CANCEL for the same reason.
    uint8_t urns_path_delete_confirm_cursor = 1;
    // 0 = PURGE, 1 = CANCEL - same shape as urns_path_delete_confirm_cursor,
    // just for MENU_STATE_URNS_FREE_DETAIL's Paths row wiping every entry
    // in the path table at once instead of one entry at a time.
    uint8_t urns_path_purge_confirm_cursor = 1;
    #if HAS_LXMF == true
      // Same shape again, for MENU_STATE_URNS_FREE_DETAIL's Messages row -
      // wipes every stored LXMF conversation at once (MessageStore::
      // clear_all()).
      uint8_t urns_msg_purge_confirm_cursor = 1;
    #endif
    uint8_t urns_identities_cursor = 0;
    uint8_t urns_keys_cursor = 0;
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
    // show live state" convention as the list it was opened from. Also the
    // hash MENU_STATE_URNS_PATH_HASH_VIEW's draw_menu_urns_path_hash_disp()
    // actually renders - reused as-is (not renamed/duplicated) by
    // MSNGR_PEER_PROP_ACTION_SHOW_HASH below, which is the same "just show
    // the full hash, dismiss on any input" need against a different hash.
    RNS::Bytes urns_path_detail_hash;

    // Where MENU_STATE_URNS_PATH_HASH_VIEW returns to on dismiss - set at
    // each entry point right before switching menu_state to it, same
    // "explicit return target" shape as menu_open_popup()'s own return_
    // state parameter. Defaults to its original sole caller (URNS Path
    // Detail's Hash row) so nothing else needs to set it if a future call
    // site forgets to.
    uint8_t menu_hash_view_return_state = MENU_STATE_URNS_PATH_DETAIL;

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

    // Cache for MENU_STATE_URNS_PATHS - populated by urns_path_cache_
    // refresh() below, NOT rebuilt on every redraw the way this screen
    // originally worked. NewPathTable is a microStore TypedStore
    // (Persistence::NewPathTable) over a flash-backed FileStore -
    // TypedStore::iterator::operator++/* triggers a lazy per-entry value
    // load (TypedStore.h's own load()), i.e. a real flash read, for every
    // row it visits. draw_settings_menu_disp() runs every loop() iteration
    // (see urns_dir_size_recursive()'s own comment, URNS.h) - walking up to
    // MENU_URNS_PATH_MAX_ROWS (24) flash-backed entries from inside that
    // per-frame draw path is exactly the same "expensive call from a
    // redraw path" mistake Messenger's own msngr_peer_cache was already
    // introduced to fix (see that struct's own comment, Messenger.h), just
    // never caught here since MENU_STATE_URNS_PATH_DETAIL's single get()
    // lookup never showed the symptom - this is what made Path Table's top
    // level sluggish (and its button input feel bad, since the same
    // loop() iteration that's stalled on flash reads is also the one
    // polling the encoder/buttons) while Path Detail stayed fine.
    struct UrnsPathCacheRow {
      RNS::Bytes hash; // full key, not just the 8-hex-char label - lets the
                        // confirm handler below hand this straight to
                        // MENU_STATE_URNS_PATH_DETAIL without re-walking
                        // the store a second time to find it by cursor
                        // index, which is the other place this same cost
                        // used to get paid.
      char label[9];    // 8 hex chars + NUL
      uint8_t hops;
    };
    UrnsPathCacheRow urns_path_cache[MENU_URNS_PATH_MAX_ROWS];
    uint8_t urns_path_cache_count = 0;
    // new_path_table().size() as of the last refresh - TypedStore::size()
    // just returns the underlying store's live record count (no per-entry
    // flash read, unlike begin()/++/*), so comparing against it is a cheap
    // way to notice the table changed (an announce arrived, an entry
    // expired/got culled, Delete Path ran) without paying iteration cost
    // on every redraw. (size_t)-1 sentinel forces the very first call to
    // always refresh. Same imprecision Messenger's own msngr_peer_cache_
    // message_count already accepts - a same-count churn (one entry
    // replaced by another between refreshes) goes unnoticed until the
    // count next changes, which is fine for a mostly-static path table.
    size_t urns_path_cache_table_size = (size_t)-1;

    void urns_path_cache_refresh() {
      RNS::Persistence::NewPathTable& pt = const_cast<RNS::Persistence::NewPathTable&>(RNS::Transport::new_path_table());
      urns_path_cache_count = 0;
      for (auto it = pt.begin(); it != pt.end() && urns_path_cache_count < MENU_URNS_PATH_MAX_ROWS; ++it) {
        auto entry = *it;
        UrnsPathCacheRow &row = urns_path_cache[urns_path_cache_count];
        row.hash = entry.key;
        snprintf(row.label, sizeof(row.label), "%s", entry.key.toHex(true).substr(0, 8).c_str());
        row.hops = entry.value._hops;
        urns_path_cache_count++;
      }
      urns_path_cache_table_size = pt.size();
    }

    void urns_path_cache_refresh_if_stale() {
      if (RNS::Transport::new_path_table().size() != urns_path_cache_table_size) urns_path_cache_refresh();
    }

    // Wipes every entry in RNS::Transport::new_path_table() at once -
    // opened from MENU_STATE_URNS_FREE_DETAIL's Paths row via MENU_STATE_
    // URNS_PATH_PURGE_CONFIRM. Walks the live store directly rather than
    // urns_path_cache above, which only ever holds up to MENU_URNS_PATH_
    // MAX_ROWS entries for display - a purge has to reach every real
    // entry, not just the ones currently visible on MENU_STATE_URNS_PATHS.
    // The per-entry flash reads that walk costs are exactly what made
    // doing this from a draw path sluggish (urns_path_cache's own comment)
    // - fine here since this only ever runs once, synchronously, on an
    // explicit user confirmation.
    void urns_purge_path_table() {
      RNS::Persistence::NewPathTable& pt = const_cast<RNS::Persistence::NewPathTable&>(RNS::Transport::new_path_table());
      std::vector<RNS::Bytes> hashes;
      for (auto it = pt.begin(); it != pt.end(); ++it) hashes.push_back((*it).key);
      if (!hashes.empty()) {
        // Same DIO0-masking reasoning as MENU_STATE_URNS_PATH_DELETE_
        // CONFIRM's single remove_path() call just below - remove_paths()
        // is the same TypedStore::remove() flash I/O, just looped.
        LoRa->maskDio0();
        RNS::Transport::remove_paths(hashes);
        LoRa->unmaskDio0();
      }
      urns_path_cache_refresh();
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
    bool staged_msngr_propagate_on_fail = false;
    uint8_t staged_msngr_sync_interval_idx = 0;
    uint8_t staged_msngr_sync_limit = MSNGR_SYNC_LIMIT_DEFAULT;
    uint8_t staged_msngr_stamp_cost = MSNGR_STAMP_COST_DEFAULT;

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
  #endif


  #if HAS_LXMF == true
    // Which msngr_presets[] slot PURPOSE_PRESET is editing -
    // MSNGR_MAX_PRESETS itself (one past the last real slot) is the
    // sentinel for "adding a new preset" rather than editing an existing
    // one, same "index == count means append" convention
    // messenger_preset_add() itself uses internally.
    uint8_t msngr_preset_edit_index = 0;
  #endif


  #if HAS_LXMF == true
    // MENU_STATE_MSNGR_PING_RESULT - fixed 2-row screen (status + BACK),
    // default cursor on BACK so a quick click dismisses either a result or
    // an in-flight ping. Row 0 is read-only info, same "selecting it does
    // nothing" shape as MSNGR_MSG_DETAIL's own content lines.
    uint8_t msngr_ping_result_cursor = 1;

    // MENU_STATE_MSNGR_SEND_RESULT - 2-row screen (status + BACK) normally,
    // growing to 4 (status + Retry + Retry via Prop + BACK) only once the
    // send has reached a terminal failure state (TIMEOUT/UNRESOLVED/FAILED)
    // AND the router's own single-in-flight outbound queue (LXMRouter.cpp's
    // process_outbound() comment) has actually finished with the original
    // message - msngr_send_result_queue_busy()'s own comment below. Neither
    // Retry row exists before then, not just hidden-but-inert, so they
    // can't be selected while a Retry would just queue uselessly behind a
    // still-retrying earlier send. Default cursor on BACK (last row,
    // whichever index that currently is).
    uint8_t msngr_send_result_cursor = 1;
    // Row count as of the last redraw (msngr_send_result_row_count() below) -
    // lets the draw code notice a busy->free transition happening while this
    // screen is still open (the router's background retry cycle exhausting
    // between one frame and the next, with no key press involved) and reset
    // the cursor before row 1's meaning silently flips from BACK to Retry
    // underneath it - see that reset's own comment, Menu.h draw code.
    uint8_t msngr_send_result_last_row_count = 2;

    // True once the currently-tracked send has hit a terminal failure -
    // the one point Menu.h's draw/input code for MENU_STATE_MSNGR_SEND_
    // RESULT needs to agree on row count (2 vs 4) and BACK's index (1 vs
    // 3) with.
    bool msngr_send_result_failed() {
      return msngr_send_state == MSNGR_SEND_TIMEOUT ||
             msngr_send_state == MSNGR_SEND_UNRESOLVED ||
             msngr_send_state == MSNGR_SEND_FAILED;
    }

    // Whether LXMRouter's single-in-flight outbound queue is occupied at
    // all right now - by anything, not just whatever this screen happens
    // to be tracking. Retry/Retry via Prop would just queue uselessly
    // behind it either way (LXMRouter.cpp's process_outbound() only ever
    // services the front of the queue), so this is deliberately NOT scoped
    // to "is it specifically my tracked message" - an earlier version of
    // this checked pending_outbound_attempts_for(msngr_send_message_hash)
    // instead, which only recognized the queue as busy while this screen's
    // own tracked send was the one actively at the front. That missed the
    // case where a second send gets started (or the same send is retried)
    // while an *earlier* one is still retrying in the background: the new
    // send just sits queued behind it, its own hash never matches the
    // front, and the check read "free" even though nothing had actually
    // been attempted yet. draw_menu_list_disp()'s own footer_override
    // fallback uses this same global signal, so the live "Retry N/M in Xs"
    // status is visible everywhere the queue is occupied, not just here.
    bool msngr_send_result_queue_busy() {
      return urns_lxmf_router && urns_lxmf_router->pending_outbound_count() > 0;
    }

    // Single source of truth for this screen's row count - same
    // msngr_XXX_row_count() convention already used for Inbox/Bookmarks/
    // Peer/Presets (see their own call sites in the draw/input code below).
    uint8_t msngr_send_result_row_count() {
      if (!msngr_send_result_failed()) return 2;
      return msngr_send_result_queue_busy() ? 2 : 4;
    }
  #endif


  #if HAS_LXMF == true
    #if HAS_BLE_HID_HOST == true
      // MENU_STATE_MSNGR_CHAT's own text-editing cursor - a byte offset
      // into msngr_text_entry_buf (0..strlen(buf)), separate from the on-
      // screen keyboard's msngr_kb_insert_char()/_do_backspace() above,
      // which only ever operate at the end of the buffer (no cursor concept
      // at all - nothing in that grid-based flow needs one). Chat has a
      // physical keyboard's own Left/Right keys available, so real in-place
      // editing (insert/delete anywhere, not just append/trim-the-tail)
      // is worth supporting there specifically. Reset to 0 (buffer's start,
      // same as "empty") wherever msngr_text_entry_buf itself gets reset
      // for Chat - MSNGR_PEER_FIXED_ACTION_CHAT's own entry point and
      // msngr_chat_do_send() after a successful send (both below).
      uint8_t msngr_chat_cursor = 0;

      // Persisted horizontal scroll position for draw_msngr_compose_box()'s
      // cursor-aware path (this file, below) - a byte offset into msngr_
      // text_entry_buf, kept across redraws so the visible slice only
      // slides when msngr_chat_cursor actually moves out of it (minimal-
      // scroll, same idiom every real text field uses), not recomputed
      // from scratch every frame. Reset at the same two points as msngr_
      // chat_cursor itself, immediately below.
      size_t msngr_chat_compose_win_start = 0;

      // Mirrors msngr_kb_insert_char()'s own length cap (MSNGR_TEXT_ENTRY_
      // MAX_LEN - Chat's msngr_text_entry_purpose is always MESSAGE, never
      // the other three purposes that function has to branch on), but
      // inserts at msngr_chat_cursor instead of always appending, shifting
      // everything from the cursor onward one slot to the right first.
      void msngr_chat_insert_char(char c) {
        size_t text_len = strlen(msngr_text_entry_buf);
        if (text_len >= MSNGR_TEXT_ENTRY_MAX_LEN) return;
        if (msngr_chat_cursor > text_len) msngr_chat_cursor = (uint8_t)text_len;
        for (size_t i = text_len + 1; i > msngr_chat_cursor; i--) {
          msngr_text_entry_buf[i] = msngr_text_entry_buf[i - 1];
        }
        msngr_text_entry_buf[msngr_chat_cursor] = c;
        msngr_chat_cursor++;
      }

      // Deletes the character immediately before the cursor (standard
      // Backspace semantics) and shifts the remainder left - unlike msngr_
      // kb_do_backspace() above, which only ever trims the buffer's own
      // last character. No-ops at the start of the buffer (cursor == 0),
      // same "nothing to delete" convention as that function's own no-op
      // on an already-empty buffer.
      void msngr_chat_backspace() {
        if (msngr_chat_cursor == 0) return;
        size_t text_len = strlen(msngr_text_entry_buf);
        for (size_t i = msngr_chat_cursor - 1; i < text_len; i++) {
          msngr_text_entry_buf[i] = msngr_text_entry_buf[i + 1];
        }
        msngr_chat_cursor--;
      }

      void msngr_chat_cursor_left() {
        if (msngr_chat_cursor > 0) msngr_chat_cursor--;
      }

      void msngr_chat_cursor_right() {
        size_t text_len = strlen(msngr_text_entry_buf);
        if (msngr_chat_cursor < text_len) msngr_chat_cursor++;
      }

      // MENU_STATE_MSNGR_CHAT's message-list browsing - Up/Down/PgUp/PgDn/
      // Home/End (blekbd_key_event(), below) move between the compose box
      // (msngr_chat_sel == 0xFF, Messenger.h - the default/normal focus)
      // and a selected message in the history (msngr_chat_cache, Messenger.
      // h). Selecting a message is what makes Backspace open the DELETE
      // MESSAGE? mini-dialog below instead of editing compose text; typing/
      // Left/Right always return focus to the compose box first (their own
      // dispatch, below).
      #define MSNGR_CHAT_PAGE_ROWS 4 // PgUp/PgDn step - leaves 1 row as a reference/overlap, per user request
      // Per user request, browsing auto-deselects back to the compose box
      // after this long with no navigation activity - same effect as
      // pressing Esc. Checked every redraw (Chat's own draw block, below).
      #define MSNGR_CHAT_SEL_TIMEOUT_MS 10000

      // True while the DELETE MESSAGE? confirm mini-dialog (draw block,
      // below) is up - intercepts Enter/Esc/Backspace exclusively while
      // set, same "nothing else does anything" idiom MENU_STATE_STATUS_
      // POPUP uses elsewhere, just Chat-local instead of a real menu_state
      // change (Chat's own menu_state never leaves MENU_STATE_MSNGR_CHAT
      // for this - simpler than reusing MENU_STATE_MSNGR_DELETE_CONFIRM,
      // which is a cursor-based DELETE/CANCEL list screen built around
      // menu_confirm_select() - a different interaction shape than "Enter
      // confirms, Esc/Backspace cancel" with no list to navigate).
      bool msngr_chat_delete_confirm_pending = false;

      void msngr_chat_enter_browsing() {
        if (msngr_chat_sel != 0xFF) return;
        // Selects the bottom-most (most recent) currently-visible row -
        // the natural first stop coming from the compose box, which sits
        // visually right below it.
        if (msngr_chat_cache_count > 0) msngr_chat_sel = msngr_chat_cache_count - 1;
      }

      void msngr_chat_exit_browsing() {
        msngr_chat_sel = 0xFF;
      }

      void msngr_chat_nav_up() {
        msngr_chat_sel_last_activity_ms = millis();
        if (msngr_chat_sel == 0xFF) { msngr_chat_enter_browsing(); return; }
        if (msngr_chat_sel > 0) { msngr_chat_sel--; return; }
        if (msngr_chat_window_start > 0) {
          messenger_refresh_chat_window(msngr_active_peer_hash, msngr_chat_window_start - 1);
          msngr_chat_sel = 0;
        }
      }

      void msngr_chat_nav_down() {
        msngr_chat_sel_last_activity_ms = millis();
        if (msngr_chat_sel == 0xFF) { msngr_chat_enter_browsing(); return; }
        if ((size_t)(msngr_chat_sel + 1) < msngr_chat_cache_count) { msngr_chat_sel++; return; }
        if (msngr_chat_window_start + msngr_chat_cache_count < msngr_chat_total_count) {
          messenger_refresh_chat_window(msngr_active_peer_hash, msngr_chat_window_start + 1);
          msngr_chat_sel = msngr_chat_cache_count > 0 ? (uint8_t)(msngr_chat_cache_count - 1) : 0;
        }
      }

      void msngr_chat_nav_page_up() {
        msngr_chat_sel_last_activity_ms = millis();
        msngr_chat_enter_browsing();
        size_t new_start = (msngr_chat_window_start > MSNGR_CHAT_PAGE_ROWS) ? (msngr_chat_window_start - MSNGR_CHAT_PAGE_ROWS) : 0;
        messenger_refresh_chat_window(msngr_active_peer_hash, new_start);
        msngr_chat_sel = 0;
      }

      void msngr_chat_nav_page_down() {
        msngr_chat_sel_last_activity_ms = millis();
        msngr_chat_enter_browsing();
        messenger_refresh_chat_window(msngr_active_peer_hash, msngr_chat_window_start + MSNGR_CHAT_PAGE_ROWS);
        msngr_chat_sel = msngr_chat_cache_count > 0 ? (uint8_t)(msngr_chat_cache_count - 1) : 0;
      }

      void msngr_chat_nav_home() {
        msngr_chat_sel_last_activity_ms = millis();
        messenger_refresh_chat_window(msngr_active_peer_hash, 0);
        msngr_chat_sel = 0;
      }

      void msngr_chat_nav_end() {
        msngr_chat_sel_last_activity_ms = millis();
        messenger_refresh_chat_window(msngr_active_peer_hash, (size_t)-1);
        msngr_chat_sel = msngr_chat_cache_count > 0 ? (uint8_t)(msngr_chat_cache_count - 1) : (uint8_t)0xFF;
      }

    #endif
  #endif


  #if HAS_LXMF == true
    #if HAS_BLE_HID_HOST == true
      // Set by msngr_chat_do_send() right after a successful (OK/
      // RESOLVING) send, consumed by msngr_chat_send_watch_process()
      // below once msngr_send_state (Messenger.h) actually reaches a
      // terminal outcome (Delivered/No Confirmation/Unknown Destination/
      // Delivery Failed) - the second, later popup this drives. Same
      // "don't yank the user into a popup for a screen they've since
      // navigated away from" judgment call msngr_sync_popup_process()'s
      // own comment already makes for its analogous case.
      bool msngr_chat_watching_send = false;

      // MENU_STATE_MSNGR_CHAT's own Enter-to-send - a leaner sibling of
      // msngr_kb_do_send() above rather than a reuse of it: Chat only ever
      // sends real messages (msngr_text_entry_purpose is always MESSAGE
      // there), so it doesn't need that function's other 3 purpose
      // branches, and it wants popup-only feedback (menu_open_popup(),
      // fixed ~1.5s auto-dismiss back into Chat) instead of the full
      // MENU_STATE_MSNGR_SEND_RESULT screen with its own Resolving/
      // Delivered/Failed state tracking - confirmed as the wanted shape.
      // This first popup is just the immediate "SENT"/"RESOLVING"/error
      // echo - msngr_chat_send_watch_process() below fires a second one
      // later, once real delivery confirmation (or a terminal failure)
      // actually arrives. History refresh needs no new plumbing: msngr_
      // send_result_process() (below) already refreshes msngr_peer_cache
      // whenever msngr_send_needs_cache_refresh is set, unconditionally on
      // menu_state - the sent message shows up in Chat's own 5-line view
      // on its own once the send actually lands (immediately for OK,
      // asynchronously for RESOLVING).
      void msngr_chat_do_send() {
        size_t text_len = strlen(msngr_text_entry_buf);
        if (text_len == 0) return;
        char msg_utf8[MSNGR_TEXT_ENTRY_MAX_LEN * 2 + 1];
        msngr_kb_expand_utf8(msngr_text_entry_buf, msg_utf8, sizeof(msg_utf8));
        msngr_last_send_result = messenger_send_lxmf(msngr_active_peer_hash, msg_utf8);
        if (msngr_last_send_result == URNS_LXMF_SEND_OK || msngr_last_send_result == URNS_LXMF_SEND_RESOLVING) {
          msngr_text_entry_buf[0] = 0;
          msngr_chat_cursor = 0;
          msngr_chat_compose_win_start = 0;
          msngr_chat_watching_send = true;
        }
        menu_open_popup(urns_lxmf_send_result_text(msngr_last_send_result), MENU_STATE_MSNGR_CHAT);
        menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
      }

      // Polled from loop() (RNode_Firmware.ino) right alongside msngr_
      // send_result_process() - that function only ever acts while sitting
      // on MENU_STATE_MSNGR_SEND_RESULT, so it does nothing useful for
      // Chat's own send flow, which never opens that screen. Watches the
      // exact same msngr_send_state (Messenger.h) transitions that
      // screen's own draw block turns into "Delivered"/"No Confirmation"/
      // "Unknown Destination"/"Delivery Failed" text, and shows that as a
      // second popup here instead - only while still actually looking at
      // Chat (or the first "SENT"/"RESOLVING" popup that's about to return
      // to it - see below), matching msngr_sync_popup_process()'s own
      // precedent for not interrupting whatever the user's since moved on
      // to.
      //
      // FIXED: the "still looking at Chat" check used to be a plain
      // menu_state == MENU_STATE_MSNGR_CHAT, which silently dropped the
      // Delivered popup entirely on a fast/local link - msngr_chat_do_
      // send()'s own first popup (SENT/RESOLVING) holds menu_state at
      // MENU_STATE_STATUS_POPUP for up to ACTION_POPUP_MS (5s) before
      // returning to MENU_STATE_MSNGR_CHAT, and real delivery confirmation
      // regularly lands well within that window - msngr_chat_watching_send
      // was already consumed (cleared) the instant that happened, so by the
      // time the first popup auto-dismissed back to Chat there was nothing
      // left to show. Now also accepts "still on that first popup, which
      // will return to Chat" as in-scope - menu_open_popup() below just
      // replaces its text in place (SENT -> DELIVERED) and restarts its
      // auto-dismiss timer, rather than waiting to be replaced.
      void msngr_chat_send_watch_process() {
        if (!msngr_chat_watching_send) return;
        if (msngr_send_state != MSNGR_SEND_DELIVERED && msngr_send_state != MSNGR_SEND_TIMEOUT &&
            msngr_send_state != MSNGR_SEND_UNRESOLVED && msngr_send_state != MSNGR_SEND_FAILED) return;
        msngr_chat_watching_send = false;
        const char *text =
          (msngr_send_state == MSNGR_SEND_DELIVERED)  ? "DELIVERED" :
          (msngr_send_state == MSNGR_SEND_TIMEOUT)    ? "NO CONFIRMATION" :
          (msngr_send_state == MSNGR_SEND_UNRESOLVED) ? "UNKNOWN DESTINATION" : "DELIVERY FAILED";
        msngr_send_state = MSNGR_SEND_IDLE;
        bool still_in_chat_flow = menu_state == MENU_STATE_MSNGR_CHAT ||
          (menu_state == MENU_STATE_STATUS_POPUP && menu_popup_return_state == MENU_STATE_MSNGR_CHAT);
        if (still_in_chat_flow) {
          menu_open_popup(text, MENU_STATE_MSNGR_CHAT);
          menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
        }
      }
    #endif

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
      // The Retry/Retry-via-Prop rows only exist once msngr_send_result_
      // failed() is true AND the router's own outbound queue has actually
      // finished with the original message (msngr_send_result_row_count()'s
      // own comment - 2 rows otherwise, growing to 4 once both are true) -
      // reset the cursor onto BACK's new index the moment that changes, so
      // a cursor left on BACK's old index doesn't land on the wrong row
      // (Retry/Retry via Prop, or off the end of a shrunk list) after the
      // transition. This also covers the background-retry-exhausts-while-
      // this-screen-is-still-open case (row count going 2->4 while
      // msngr_send_result_failed() stays true throughout, so a plain
      // failed-ness comparison alone couldn't see it - this used to just
      // compare msngr_send_result_failed() before/after, which also meant
      // this row count's own growth from 3 to 4 rows, when Retry via Prop
      // was added, left this resetting onto the wrong index (2, BACK's old
      // 3-row position) instead of today's 3). Tracked via last-seen row
      // count so this only fires once per transition, not every poll - a
      // manual cursor move onto Retry while already failed must survive
      // later polls here.
      {
        uint8_t row_count = msngr_send_result_row_count();
        if (row_count != msngr_send_result_last_row_count) {
          msngr_send_result_cursor = row_count - 1;
          msngr_send_result_last_row_count = row_count;
        }
      }
      // Per user request: error/uncertain outcomes (TIMEOUT/UNRESOLVED/
      // FAILED) no longer auto-dismiss - MSNGR_SEND_TIMEOUT in particular
      // is just this UI's own patience window running out, not a router-
      // confirmed failure (see its own declaration), so a PROPAGATED send
      // that's still genuinely succeeding in the background (a slow stamp
      // grind, a slow multi-hop resource transfer) could silently pop back
      // to the peer screen with the error easy to miss entirely. Only
      // genuine success (Delivered/Sent to Node) still auto-returns -
      // every error state now stays up until the user presses BACK
      // (msngr_send_result_cursor's own confirm handler, above).
      if ((msngr_send_state == MSNGR_SEND_DELIVERED || msngr_send_state == MSNGR_SEND_SENT_TO_NODE) &&
          menu_state == MENU_STATE_MSNGR_SEND_RESULT &&
          millis() - msngr_send_result_at_ms > MSNGR_SEND_RESULT_POPUP_MS) {
        menu_state = MENU_STATE_MSNGR_PEER;
        msngr_send_state = MSNGR_SEND_IDLE;
      }
    }

    // Per user request: same success-only auto-dismiss as MSNGR_SEND_
    // RESULT above, reusing its MSNGR_SEND_RESULT_POPUP_MS timeout -
    // TIMEOUT/NO_IDENTITY/FAILED still require manual BACK (msngr_ping_
    // result_cursor's own confirm handler), same "don't silently drop an
    // error the user hasn't seen yet" reasoning as MSNGR_SEND_RESULT's own
    // error states.
    void msngr_ping_result_process() {
      if (msngr_ping_state == MSNGR_PING_SUCCESS &&
          menu_state == MENU_STATE_MSNGR_PING_RESULT &&
          millis() - msngr_ping_result_at_ms > MSNGR_SEND_RESULT_POPUP_MS) {
        // Same cleanup as a manual BACK press on this row (messenger_ping_
        // cancel(), Messenger.h) - a no-op on the actual link teardown at
        // this point (already handled by messenger_ping_process()'s own
        // msngr_ping_teardown_pending poll, well before this timeout could
        // fire), just resets the tracking state.
        messenger_ping_cancel();
        menu_state = MENU_STATE_MSNGR_PEER;
      }
    }

    // Polled from loop() (RNode_Firmware.ino, alongside msngr_send_result_
    // process() itself) - watches a manual sync (MSNGR_TOP_ITEM_SYNC_PROP
    // or MSNGR_PEER_PROP_ACTION_SYNC, both via msngr_prop_sync_start()
    // below) through to completion and updates the still-open "Syncing..."
    // popup with the real result. Only touches the screen if it's still
    // showing that popup - if the user already dismissed it (button press
    // while "Syncing..." was up, which this popup allows same as any
    // other), silently drops the result instead of yanking them back into
    // a popup for a screen they've since navigated away from.
    void msngr_sync_popup_process() {
      if (!msngr_sync_popup_pending || !urns_lxmf_router) return;

      LXMF::LXMRouter::PropagationSyncState state = urns_lxmf_router->get_sync_state();
      if (state != LXMF::LXMRouter::PR_COMPLETE && state != LXMF::LXMRouter::PR_FAILED) return;

      msngr_sync_popup_pending = false;
      if (menu_state != MENU_STATE_STATUS_POPUP) return;

      if (state == LXMF::LXMRouter::PR_COMPLETE) {
        char buf[24];
        snprintf(buf, sizeof(buf), "Synced %u Msg%s", (unsigned)msngr_sync_last_count, msngr_sync_last_count == 1 ? "" : "s");
        menu_draw_popup_timed(buf, MSNGR_SYNC_POPUP_MS);
      } else { // PR_FAILED - stays up until dismissed, same as "NOT READY"
        menu_draw_popup("Sync Failed");
      }
    }

    // Shared by MSNGR_TOP_ITEM_SYNC_PROP (Messenger main menu) and
    // MSNGR_PEER_PROP_ACTION_SYNC (a Propagation-type bookmark's own peer
    // screen) - kicks off a manual sync against `hash` specifically.
    // LXMRouter only ever tracks one outbound propagation node at a time,
    // so if `hash` isn't already the active one this makes it active
    // first (messenger_prop_node_set_active()) - there's no way to sync a
    // *different* node without that, and pressing Sync on a specific
    // bookmark is a reasonable, expected way to switch which one is
    // active. Caller is responsible for the "no hash at all" case (empty/
    // Prop Not Set) - this function always has a real hash to work with.
    void msngr_prop_sync_start(const RNS::Bytes &hash, uint8_t return_state) {
      if (!urns_lxmf_router) {
        menu_open_popup("NOT READY", return_state);
        return;
      }
      if (!messenger_prop_node_is_active(hash)) {
        messenger_prop_node_set_active(hash);
      }
      LXMF::LXMRouter::PropagationSyncState state = urns_lxmf_router->get_sync_state();
      if (state != LXMF::LXMRouter::PR_IDLE && state != LXMF::LXMRouter::PR_COMPLETE && state != LXMF::LXMRouter::PR_FAILED) {
        menu_open_popup("Already Syncing", return_state);
        menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
        return;
      }
      msngr_sync_last_count = 0;
      urns_lxmf_router->request_messages_from_propagation_node();
      msngr_sync_popup_pending = true;
      menu_open_popup("Syncing...", return_state);
    }

    uint8_t msngr_inbox_row_count() {
      size_t n = urns_message_store ? urns_message_store->get_conversation_count() : 0;
      if (n > MENU_MSNGR_LIST_MAX_ROWS) n = MENU_MSNGR_LIST_MAX_ROWS;
      if (n == 0) return 2; // "No Messages" + BACK
      return (uint8_t)(n + 1);
    }

    // One row per saved bookmark, plus an "Add by Hash" row (hidden once
    // MSNGR_MAX_BOOKMARKS is full), plus BACK - same "always something
    // actionable, no dead placeholder label" shape as msngr_presets_row_
    // count()'s own "Add Preset" row.
    uint8_t msngr_bookmarks_row_count() {
      uint8_t n = msngr_bookmark_count;
      if (msngr_bookmark_count < MSNGR_MAX_BOOKMARKS) n++; // "Add by Hash"
      return (uint8_t)(n + 1); // + BACK
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
      if (messenger_bookmark_is_prop_node(msngr_active_peer_hash)) return MSNGR_PEER_PROP_ACTION_COUNT;
      return (uint8_t)(msngr_peer_msg_row_count() + msngr_preset_count + MSNGR_PEER_FIXED_ACTION_COUNT);
    }

    // Real word-wrap - breaks at the last space that still fits, falling
    // back to a hard character break only when a single word itself
    // exceeds a whole line. Measures actual pixel width via
    // getTextBounds() instead of assuming a fixed chars-per-line count
    // (what both MSG_DETAIL and MSG_VIEW used to do here, independently,
    // both tuned to 23): Org_01's real glyph widths vary enough that a
    // flat count wraps some lines a character or two early even though the
    // whole line still fits on screen - e.g. "Pong! [00:32:14, Hops: 1]"
    // (25 chars, ~110px wide) broke into two lines on this 128px-wide
    // screen purely because 25 > 23, despite ~18px of unused width still
    // left on the line. No amount of re-tuning the constant fixes that in
    // general (a wider message can always exist that still fits); actually
    // measuring the candidate line's pixel width removes the guesswork
    // entirely. Shared by MENU_STATE_MSNGR_MSG_DETAIL's content-preview
    // rows and the full-screen MSG_VIEW below (msngr_msg_view_wrap()) -
    // caller must have already set MENU_GFX's font/size (every
    // draw_*_disp() in this file does this at entry; msngr_msg_detail_
    // refresh_wrap_cache() below sets it explicitly itself, since it can
    // run outside a draw call).
    uint8_t msngr_wrap_text(const std::string &content, std::string out_lines[], uint8_t max_lines, int16_t max_w) {
      uint8_t n = 0;
      size_t pos = 0;
      size_t len = content.size();
      int16_t bx, by; uint16_t bw, bh;
      while (pos < len && n < max_lines) {
        while (pos < len && content[pos] == ' ') pos++;
        if (pos >= len) break;
        size_t remaining = len - pos;
        MENU_GFX.getTextBounds(content.substr(pos, remaining).c_str(), 0, 0, &bx, &by, &bw, &bh);
        if ((int16_t)bw <= max_w) {
          out_lines[n++] = content.substr(pos, remaining);
          break;
        }
        // Binary search the longest prefix of the remaining text that
        // still fits max_w, then back off to the last space inside it
        // (real word-wrap); a single word wider than max_w on its own
        // falls back to a hard break at that same longest-fitting length.
        size_t lo = 1, hi = remaining, fit_len = 1;
        while (lo <= hi) {
          size_t mid = lo + (hi - lo) / 2;
          MENU_GFX.getTextBounds(content.substr(pos, mid).c_str(), 0, 0, &bx, &by, &bw, &bh);
          if ((int16_t)bw <= max_w) { fit_len = mid; lo = mid + 1; }
          else if (mid == 0) break;
          else hi = mid - 1;
        }
        size_t last_space = content.rfind(' ', pos + fit_len - 1);
        if (last_space != std::string::npos && last_space > pos) {
          out_lines[n++] = content.substr(pos, last_space - pos);
          pos = last_space + 1;
        } else {
          out_lines[n++] = content.substr(pos, fit_len);
          pos += fit_len;
        }
      }
      return n;
    }

    // MENU_STATE_MSNGR_MSG_DETAIL's content-preview rows - msngr_wrap_
    // text() above, computed once per screen-entry into msngr_msg_detail_
    // wrapped_lines/_count (msngr_msg_detail_refresh_wrap_cache(), called
    // right after messenger_refresh_msg_detail_cache() at every entry
    // point below) rather than re-measured on every draw or estimated from
    // a char-count guess - same "cache once per screen-entry, don't
    // recompute from flash-backed content on every call" discipline
    // msngr_peer_cache/msg_detail_cache themselves already follow
    // elsewhere in this file. Caching also keeps the row count
    // (msngr_msg_detail_row_count() below) and the actually-drawn text
    // (draw_settings_menu_disp()'s own MSG_DETAIL block) in exact
    // agreement, since both just read this same array.
    std::string msngr_msg_detail_wrapped_lines[MSNGR_MSG_DETAIL_MAX_LINES];
    uint8_t msngr_msg_detail_wrapped_count = 0;
    void msngr_msg_detail_refresh_wrap_cache() {
      MENU_GFX.setFont(MENU_FONT);
      MENU_GFX.setTextSize(1);
      msngr_msg_detail_wrapped_count = msngr_wrap_text(msngr_msg_detail_cache_content, msngr_msg_detail_wrapped_lines, MSNGR_MSG_DETAIL_MAX_LINES, MENU_CONTENT_W);
    }

    uint8_t msngr_msg_detail_row_count() {
      if (!msngr_msg_detail_cache_valid) return 2;
      // content lines + [PLAY/STOP, voice messages only] + REPLY + DELETE + FULL MESSAGE + BACK
      return (uint8_t)(msngr_msg_detail_wrapped_count + 4 + (msngr_msg_detail_has_audio ? 1 : 0));
    }

    // Full-screen, ornament-free single-message view, real word-wrap
    // (shares msngr_wrap_text() with MENU_STATE_MSNGR_MSG_DETAIL's own
    // content-preview rows above - unlike that screen's cached array,
    // this one re-wraps on every draw since it's the only reader).
    // Shared by two entry points, per user request ("it should actually
    // use the same function"): MENU_STATE_MSNGR_MSG_DETAIL's own "Full
    // Message" row (menu_confirm_select(), below) and MENU_STATE_MSNGR_
    // CHAT's Enter-while-browsing (blekbd_key_event(), further below,
    // HAS_BLE_HID_HOST boards only) - msngr_msg_view_return_state records
    // which of the two to land back on when the view closes. Deliberately
    // NOT gated behind HAS_BLE_HID_HOST (unlike the CHAT entry point) -
    // MSG_DETAIL is core Messenger functionality, reachable via a real
    // encoder/single button on every HAS_LXMF board, keyboard or not.
    // Reuses msngr_msg_detail_cache_content (Messenger.h, messenger_
    // refresh_msg_detail_cache()) for the actual fetch either way - the
    // same full-content cache MSG_DETAIL itself already populates, not
    // msngr_chat_cache[]'s own capped-length snippet.
    bool msngr_msg_view_active = false;
    // Which wrapped line is at the top of the screen - Up/Down (keyboard),
    // encoder rotation, and single-button short-tap/double-tap (matching
    // MENU_BTN_DOUBLE_TAP_WINDOW's own forward/backward split, same as any
    // other list-nav screen - see menu_encoder_rotate()'s own msngr_msg_
    // view_active branch) all move this, clamped in the draw block once
    // the real wrapped line count for the current message is known.
    uint8_t msngr_msg_view_scroll_line = 0;
    // MENU_STATE_MSNGR_MSG_DETAIL or MENU_STATE_MSNGR_CHAT - whichever
    // opened the view, so Esc/long-press/encoder-long-click closes back to
    // the right screen (menu_encoder_rotate()/menu_confirm_select()/
    // menu_encoder_button(), below).
    uint8_t msngr_msg_view_return_state = MENU_STATE_MSNGR_MSG_DETAIL;

    // Org_01 at MENU_CONTENT_W=128, minus the scrollbar's own reserved
    // ~7px on the right (4px track + 1px gap + 1px margin,
    // draw_msngr_msg_view_scrollbar() below) - per user request, this
    // space is always reserved regardless of whether a given message
    // actually needs to scroll, so the wrap width never changes message
    // to message.
    //
    // Unlike MSNGR_MSG_DETAIL_MAX_LINES' own bounded preview (a handful of
    // rows with a "Full Message" link out, by design), this is the actual
    // full-message reader, so it must be able to wrap arbitrarily long
    // messages in full rather than silently dropping anything past a fixed
    // line count (a real bug this used to have: a fixed 12-line cap here
    // cut off any message whose wrapped length exceeded it, on top of the
    // separate decode-buffer truncation fixed in Messenger.h's
    // messenger_refresh_msg_detail_cache()). msngr_wrap_text_all() below
    // returns a vector sized to however many lines the message actually
    // needs; the only remaining ceiling is uint8_t's own 255-line range,
    // which every piece of this screen's scroll/scrollbar math already
    // assumes (draw_msngr_full_message_view() below).
    std::vector<std::string> msngr_wrap_text_all(const std::string &content, int16_t max_w) {
      std::vector<std::string> out;
      size_t pos = 0;
      size_t len = content.size();
      int16_t bx, by; uint16_t bw, bh;
      while (pos < len && out.size() < 255) {
        while (pos < len && content[pos] == ' ') pos++;
        if (pos >= len) break;
        size_t remaining = len - pos;
        MENU_GFX.getTextBounds(content.substr(pos, remaining).c_str(), 0, 0, &bx, &by, &bw, &bh);
        if ((int16_t)bw <= max_w) {
          out.push_back(content.substr(pos, remaining));
          break;
        }
        size_t lo = 1, hi = remaining, fit_len = 1;
        while (lo <= hi) {
          size_t mid = lo + (hi - lo) / 2;
          MENU_GFX.getTextBounds(content.substr(pos, mid).c_str(), 0, 0, &bx, &by, &bw, &bh);
          if ((int16_t)bw <= max_w) { fit_len = mid; lo = mid + 1; }
          else if (mid == 0) break;
          else hi = mid - 1;
        }
        size_t last_space = content.rfind(' ', pos + fit_len - 1);
        if (last_space != std::string::npos && last_space > pos) {
          out.push_back(content.substr(pos, last_space - pos));
          pos = last_space + 1;
        } else {
          out.push_back(content.substr(pos, fit_len));
          pos += fit_len;
        }
      }
      return out;
    }

    // msngr_wrap_text_all() above, at MENU_CONTENT_W minus the scrollbar's
    // own reserved 7px (this screen's own comment above).
    std::vector<std::string> msngr_msg_view_wrap(const std::string &content) {
      return msngr_wrap_text_all(content, MENU_CONTENT_W - 7);
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
  #if HAS_BLE_HID_HOST == true
    // MENU_STATE_BLEKBD_* cursor/context state (BLEKeyboardHost.h drives
    // the actual scan/pair/persist logic).
    uint8_t blekbd_menu_cursor = 0;
    bool staged_blekbd_enabled = false;
    uint8_t blekbd_scan_cursor = 0;
    // 0 = PAIR/FORGET, 1 = CANCEL - same defaulting-to-CANCEL convention as
    // bt_unpair_confirm_cursor above.
    uint8_t blekbd_pair_confirm_cursor = 1;
    uint8_t blekbd_forget_confirm_cursor = 1;
    // Row 0 (status) is read-only, row 1 is BACK - same shape as
    // msngr_send_result_cursor.
    uint8_t blekbd_pairing_cursor = 1;
    // Stashed from the selected MENU_STATE_BLEKBD_SCAN row while
    // MENU_STATE_BLEKBD_PAIR_CONFIRM is open, consumed by
    // blekbd_connect_start() if the user confirms PAIR.
    uint8_t blekbd_pending_addr[6];
    uint8_t blekbd_pending_addr_type;
    char blekbd_pending_name[BLEKBD_NAME_MAX_LEN + 1] = {0};

    // Same shape as msngr_announces_row_count().
    uint8_t blekbd_scan_row_count() {
      uint8_t n = 0;
      for (uint8_t i = 0; i < BLEKBD_MAX_DISCOVERED; i++) if (blekbd_discovered[i].in_use) n++;
      if (n == 0) return 2; // "Scanning..." + BACK
      return (uint8_t)(n + 1);
    }

    // Definition for the declaration in BLEKeyboardHost.h - lives here since
    // it touches menu_state/MENU_STATE_BLEKBD_*, not yet declared at that
    // file's own point in the include order (same reason msngr_send_result_
    // process() lives in Menu.h instead of Messenger.h). Polled from loop()
    // (RNode_Firmware.ino) right after blekbd_loop() - advances
    // MENU_STATE_BLEKBD_PAIRING once a result is available. Only touches
    // menu_state while that screen is actually open, so it can't interfere
    // with unrelated menu navigation.
    void blekbd_pair_result_process() {
      if (menu_state != MENU_STATE_BLEKBD_PAIRING) return;
      if (blekbd_pair_result == BLEKBD_PAIR_OK && (int32_t)(millis() - blekbd_pair_result_at_ms) > BLEKBD_PAIR_RESULT_POPUP_MS) {
        blekbd_pair_result = BLEKBD_PAIR_NONE;
        menu_state = MENU_STATE_BLEKBD_LIST;
      }
      // FAILED stays up until the user presses BACK from the menu itself -
      // same "errors wait for real input" convention used elsewhere in Menu.h.
    }

    // Definition for the declaration in BLEKeyboardHost.h - same reason as
    // blekbd_pair_result_process() above. Polled from loop() right
    // alongside it. Per user request, arms the notice unconditionally,
    // regardless of what's currently on screen (Settings, Messenger, Chat,
    // idle main screen, ...) - a keyboard dropping/reconnecting is exactly
    // as worth noticing mid-navigation as while idle, and the overlay
    // itself (draw_blekbd_notice_overlay(), above) is a pure superimposed
    // redraw that never touches menu_state/navigation, so there's nothing
    // to "hijack" the way the very first implementation attempt (menu_open_
    // popup()/MENU_STATE_STATUS_POPUP, see draw_blekbd_notice_overlay()'s
    // own comment) actually did.
    void blekbd_notice_process() {
      if (blekbd_connected_popup_pending) {
        blekbd_connected_popup_pending = false;
        snprintf(blekbd_notice_text, sizeof(blekbd_notice_text), "KBD CONNECTED");
        blekbd_notice_until_ms = millis() + ACTION_POPUP_MS;
      }
      if (blekbd_disconnected_popup_pending) {
        blekbd_disconnected_popup_pending = false;
        snprintf(blekbd_notice_text, sizeof(blekbd_notice_text), "KBD DISCONNECTED");
        blekbd_notice_until_ms = millis() + ACTION_POPUP_MS;
      }
    }

    // Definition for the declaration in BLEKeyboardHost.h - same reason as
    // blekbd_pair_result_process() above. Fires repeated presses of
    // whatever key is currently held (blekbd_held_active/_char,
    // BLEKeyboardHost.h - Backspace, Up/Down/Left/Right, or any plain
    // typed character), matching the on-screen keyboard's own DEL hold-
    // to-repeat timing exactly (MSNGR_KB_DEL_REPEAT_START_MS/_INTERVAL_MS).
    //
    // Two different scopes, per user request:
    //   - Backspace and plain typed characters only repeat while actually
    //     composing/editing text (MENU_STATE_MSNGR_TEXT_ENTRY/_CHAT) -
    //     letting Backspace repeat elsewhere would cascade through
    //     multiple menu levels via blekbd_backspace_as_back() on a single
    //     held press, which nobody asked for.
    //   - Up/Down/Left/Right never have that risk (they only ever move
    //     within the CURRENT screen/level - list cursor, or a value up/
    //     down/left/right in an EDIT-style submenu - never jump between
    //     menu levels), so their repeat is allowed in ANY open menu
    //     screen, covering general RNode Settings list/edit navigation as
    //     well as composing (where Left/Right already move the Chat text
    //     cursor, Up/Down already browse Chat's own message list).
    // Enter/Escape/shortcuts never set blekbd_held_active at all
    // (BLEKeyboardHost.h's own comment on it) so this never fires for
    // those regardless of state.
    void blekbd_key_repeat_process() {
      if (!blekbd_held_active) return;
      bool composing = (menu_state == MENU_STATE_MSNGR_TEXT_ENTRY || menu_state == MENU_STATE_MSNGR_CHAT);
      bool nav_repeat_key = (blekbd_held_char == BLEKBD_CH_UP || blekbd_held_char == BLEKBD_CH_DOWN ||
                              blekbd_held_char == BLEKBD_CH_LEFT || blekbd_held_char == BLEKBD_CH_RIGHT);
      if (!composing && !(nav_repeat_key && menu_state != MENU_STATE_CLOSED)) return;
      unsigned long now = millis();
      if ((int32_t)(now - blekbd_held_press_ms) < BLEKBD_KEY_REPEAT_START_MS) return;
      if (blekbd_held_last_repeat_ms != 0 && (int32_t)(now - blekbd_held_last_repeat_ms) < BLEKBD_KEY_REPEAT_INTERVAL_MS) return;
      blekbd_key_queue_push(blekbd_held_char);
      blekbd_held_last_repeat_ms = now;
    }
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

  #endif

  // Shared on-screen keyboard mechanics, positioned after staged_wifi_ssid/
  // psk (above) since msngr_kb_do_send() writes directly into them for the
  // WIFI_SSID/WIFI_PSK purposes - HAS_WIFI boards need this code too, and
  // HAS_URNS is not implied by HAS_WIFI (unlike HAS_LXMF, which always does
  // imply HAS_URNS - see this file's own comment on staged_msngr_* further
  // up), so this can't stay nested under the #if HAS_URNS==true wrapper it
  // originally lived in. Kept as one contiguous block, in original relative
  // order, rather than re-split across several in-place HAS_URNS-nested sites.
  // Shared on-screen keyboard grid-navigation helpers - see the wider
  // comment on this same widened-guard pattern further up this file
  // (MSNGR_KB_ROWS/_COLS etc). msngr_kb_cursor/_shift_on/_lang_ru/msngr_
  // text_entry_buf themselves are declared earlier (alongside msngr_text_
  // entry_purpose) - see that declaration's own comment for why.
  #if HAS_LXMF == true || HAS_WIFI == true
    // Centralizes which layout table is "live" right now - the restricted
    // hex grid for MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH, the Base32 grid
    // for _IDENTITY_RESTORE, EN/RU otherwise - so every call site that
    // indexes [kb_row][kb_col] does it through here instead of duplicating
    // the purpose check. Returns a row-array pointer (decays to the same
    // char(*)[MSNGR_KB_COLS] type no matter which table's actual row
    // count is, since only MSNGR_KB_COLS - shared by every table - affects
    // that pointer type) so callers don't need to know or care that
    // MSNGR_KB_LAYOUT_HEX has fewer rows.
    const char (*msngr_kb_active_layout())[MSNGR_KB_COLS] {
      if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH) return MSNGR_KB_LAYOUT_HEX;
      if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_IDENTITY_RESTORE) return MSNGR_KB_LAYOUT_BASE32;
      // WiFi SSID/PSK reuse msngr_kb_lang_ru as a Letters/Symbols toggle
      // instead of EN/RU (Cyrillic is meaningless for WiFi credentials,
      // and WPA passphrases commonly need punctuation the plain EN grid
      // doesn't have room for) - same hold-Shift gesture, same flag,
      // different meaning for these two purposes only.
      if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_WIFI_SSID ||
          msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_WIFI_PSK)
        return msngr_kb_lang_ru ? MSNGR_KB_LAYOUT_SYMBOLS : MSNGR_KB_LAYOUT;
      return msngr_kb_lang_ru ? MSNGR_KB_LAYOUT_RU : MSNGR_KB_LAYOUT;
    }

    // Row count of whichever layout msngr_kb_active_layout() would return
    // right now - MSNGR_KB_HEX_ROWS for hash entry, MSNGR_KB_ROWS for
    // everything else, IDENTITY_RESTORE included (MSNGR_KB_LAYOUT_BASE32
    // uses the full standard row count, unlike hex's compact 3-row grid -
    // see that table's own comment). Bounds the keyboard grid's own
    // row-drawing loop so hash entry never draws (or leaves visible
    // click-through space for) the extra row a straight MSNGR_KB_ROWS
    // would otherwise leave underneath it - cursor navigation itself is
    // bounded separately, by msngr_kb_active_key_count() below, since
    // neither MSNGR_KB_LAYOUT_HEX's nor MSNGR_KB_LAYOUT_BASE32's real
    // cells are evenly spread across every row.
    uint8_t msngr_kb_active_rows() {
      return msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH ? MSNGR_KB_HEX_ROWS : MSNGR_KB_ROWS;
    }

    // Every other layout is a uniform MSNGR_KB_ROWS x MSNGR_KB_COLS
    // rectangle, so a linear cursor maps to (row, col) with plain
    // division/modulo, which also happens to visit cells in reading order
    // (left-to-right, top-to-bottom). MSNGR_KB_LAYOUT_HEX's own reading
    // order would visit DEL (row 0's last cell) before 'A'-'F' (row 1),
    // which isn't the tab order that's actually wanted - digits, then
    // letters, then DEL/SAVE/BACK last - so hash entry gets an explicit
    // per-cursor-value (row, col) table instead of computing one.
    static const uint8_t MSNGR_KB_HEX_ORDER[20][2] = {
      {0,0}, {0,1}, {0,2}, {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {0,9}, // 0-9
      {1,0}, {1,1}, {1,2}, {1,3}, {1,4}, {1,5}, // A-F
      {0,10}, // DEL
      {1,10}, // SAVE
      {2,0},  // Type (LXMF/Propagation)
      {2,10}, // BACK
    };
    #define MSNGR_KB_HEX_KEY_COUNT 20

    // Maps a linear cursor value to (row, col) for whichever layout is
    // active - MSNGR_KB_HEX_ORDER for hash entry, MSNGR_KB_BASE32_ORDER
    // for identity restore, plain division/modulo for every other
    // (uniform-grid) purpose. Shared by every call site that used to do
    // the division/modulo itself, so none of them need their own purpose
    // check.
    void msngr_kb_cursor_rc(uint8_t cursor, uint8_t &row, uint8_t &col) {
      if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH) {
        row = MSNGR_KB_HEX_ORDER[cursor][0];
        col = MSNGR_KB_HEX_ORDER[cursor][1];
        return;
      }
      if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_IDENTITY_RESTORE) {
        row = MSNGR_KB_BASE32_ORDER[cursor][0];
        col = MSNGR_KB_BASE32_ORDER[cursor][1];
        return;
      }
      row = cursor / MSNGR_KB_COLS;
      col = cursor % MSNGR_KB_COLS;
    }

    // Total selectable cells for whichever layout is active right now -
    // bounds cursor navigation (menu_encoder_rotate()'s MENU_STATE_MSNGR_
    // TEXT_ENTRY branch). MSNGR_KB_KEY_COUNT (rows*cols) for every normal
    // purpose; MSNGR_KB_HEX_KEY_COUNT/MSNGR_KB_BASE32_KEY_COUNT (their own
    // order tables' lengths) for hash entry/identity restore - not
    // msngr_kb_active_rows()*MSNGR_KB_COLS, which would let the cursor
    // wander onto cells those order tables don't have entries for.
    uint8_t msngr_kb_active_key_count() {
      if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH) return MSNGR_KB_HEX_KEY_COUNT;
      if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_IDENTITY_RESTORE) return MSNGR_KB_BASE32_KEY_COUNT;
      return MSNGR_KB_KEY_COUNT;
    }

    // Real 2D grid navigation for a BLE keyboard's Left/Right/Up/Down
    // (blekbd_key_event(), below) - per user request, distinct from the
    // single linear cursor step menu_encoder_rotate()'s own MENU_STATE_
    // MSNGR_TEXT_ENTRY branch does (dir=+-1 on msngr_kb_cursor directly),
    // which for MSNGR_KB_LAYOUT_HEX's deliberately non-reading-order tab
    // sequence (MSNGR_KB_HEX_ORDER's own comment) doesn't reliably
    // correspond to "move right" or "move down" at all - e.g. one linear
    // step from '9' (row 0) lands on 'A' (row 1), neither a same-row nor
    // a same-column move. A real encoder/the single button still use the
    // linear step (unchanged - rotation only, no separate Left/Right
    // input to give it), so this is BLE-keyboard-specific.
    //
    // msngr_kb_nav_col: moves to the nearest valid cell strictly left/
    // right of the current one WITHIN THE SAME ROW ONLY, clamped at the
    // row's own first/last real cell (never spills into an adjacent row,
    // unlike msngr_kb_nav_row below). O(active key count) linear scan -
    // at most 44 cells (normal 4x11 layout), negligible.
    void msngr_kb_nav_col(int8_t dir) {
      uint8_t row, col;
      msngr_kb_cursor_rc(msngr_kb_cursor, row, col);
      uint8_t count = msngr_kb_active_key_count();
      bool found = false;
      int16_t best_col = 0;
      uint8_t best_cursor = msngr_kb_cursor;
      for (uint8_t i = 0; i < count; i++) {
        uint8_t r, c;
        msngr_kb_cursor_rc(i, r, c);
        if (r != row) continue;
        if (dir < 0) {
          if ((int16_t)c < (int16_t)col && (!found || (int16_t)c > best_col)) { best_col = c; best_cursor = i; found = true; }
        } else {
          if ((int16_t)c > (int16_t)col && (!found || (int16_t)c < best_col)) { best_col = c; best_cursor = i; found = true; }
        }
      }
      if (found) msngr_kb_cursor = best_cursor;
    }

    // msngr_kb_nav_row: moves to the row above/below, landing on whichever
    // cell in that row is closest to the current column (exact match
    // preferred) - rows don't all have the same active columns (MSNGR_KB_
    // LAYOUT_HEX especially: row 2 only has cells at columns 0 and 10),
    // so this is a nearest-column search, not a fixed offset. Clamped at
    // the grid's own first/last row - no wrap, same as this screen's
    // existing Up/Down (menu_encoder_rotate(dir, false)'s own wrap=false).
    void msngr_kb_nav_row(int8_t dir) {
      uint8_t row, col;
      msngr_kb_cursor_rc(msngr_kb_cursor, row, col);
      int16_t target_row = (int16_t)row + dir;
      if (target_row < 0 || target_row >= (int16_t)msngr_kb_active_rows()) return;
      uint8_t count = msngr_kb_active_key_count();
      bool found = false;
      int16_t best_dist = 0;
      uint8_t best_cursor = msngr_kb_cursor;
      for (uint8_t i = 0; i < count; i++) {
        uint8_t r, c;
        msngr_kb_cursor_rc(i, r, c);
        if (r != (uint8_t)target_row) continue;
        int16_t dist = (int16_t)c - (int16_t)col;
        if (dist < 0) dist = -dist;
        if (!found || dist < best_dist) { best_dist = dist; best_cursor = i; found = true; }
      }
      if (found) msngr_kb_cursor = best_cursor;
    }

    // Shared by the on-screen keyboard's own confirm gesture (menu_
    // confirm_select()'s MSNGR_KB_TYPE_TOGGLE branch) and a BLE keyboard's
    // Space (while the highlight is on this cell)/Tab (from anywhere on
    // this screen) shortcuts (blekbd_key_event(), below) - all three
    // "press" the Type cell the same way. MSNGR_KB_TYPE_TOGGLE only ever
    // appears on MSNGR_KB_LAYOUT_HEX, itself only reachable via
    // MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH - a HAS_LXMF-only purpose -
    // so this whole function is HAS_LXMF-only too, unlike the rest of the
    // keyboard mechanics around it.
    #if HAS_LXMF == true
    void msngr_kb_toggle_bookmark_type() {
      msngr_kb_bookmark_type = (msngr_kb_bookmark_type == MSNGR_BOOKMARK_TYPE_PROPAGATION)
        ? MSNGR_BOOKMARK_TYPE_LXMF : MSNGR_BOOKMARK_TYPE_PROPAGATION;
    }
    #endif

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
      uint8_t kb_row, kb_col;
      msngr_kb_cursor_rc(msngr_kb_cursor, kb_row, kb_col);
      char key_ch = msngr_kb_active_layout()[kb_row][kb_col];
      if (msngr_kb_key_type(key_ch) != MSNGR_KB_SHIFT) return;
      msngr_kb_lang_ru = !msngr_kb_lang_ru;
      msngr_kb_shift_on = false;
      buzzer_encoder_tick_melody();
      fired_flag = true;
    }
  #endif
  #if HAS_LXMF == true || HAS_WIFI == true
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
      uint8_t kb_row, kb_col;
      msngr_kb_cursor_rc(msngr_kb_cursor, kb_row, kb_col);
      char key_ch = msngr_kb_active_layout()[kb_row][kb_col];
      char alt = msngr_kb_alt_pair(key_ch);
      if (alt == 0) return;
      alt = msngr_kb_apply_shift(alt, msngr_kb_shift_on);
      // WiFi purposes never actually reach here - the Symbols grid gives
      // every punctuation mark its own cell, so msngr_kb_alt_pair() has
      // nothing to pair for any key WIFI_SSID/_WIFI_PSK ever highlight -
      // but the cap still has to resolve to *something* on a HAS_WIFI-only
      // build with no HAS_LXMF purposes to branch on.
      size_t max_len = MSNGR_TEXT_ENTRY_MAX_LEN;
      #if HAS_LXMF == true
        max_len = msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH
          ? (size_t)(LXMF::PEER_HASH_SIZE * 2)
          : msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_IDENTITY_RESTORE
          ? (size_t)VAULT_IDENTITY_KEY_BASE32_LEN
          : (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_DISPLAY_NAME ||
             msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_PRESET ||
             msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_RENAME)
            ? MSNGR_NAME_MAX_LEN : MSNGR_TEXT_ENTRY_MAX_LEN;
      #endif
      if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_WIFI_SSID ||
          msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_WIFI_PSK) max_len = 32;
      size_t text_len = strlen(msngr_text_entry_buf);
      if (text_len < max_len) {
        msngr_text_entry_buf[text_len] = alt;
        msngr_text_entry_buf[text_len + 1] = 0;
        msngr_kb_last_insert_ms = millis();
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
      uint8_t kb_row, kb_col;
      msngr_kb_cursor_rc(msngr_kb_cursor, kb_row, kb_col);
      char key_ch = msngr_kb_active_layout()[kb_row][kb_col];
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
    // Which screen opened this confirm dialog (MENU_STATE_MSNGR_TEXT_ENTRY
    // or, once MENU_STATE_MSNGR_CHAT exists, that instead) - CANCEL returns
    // here. Set by menu_msngr_text_entry_leave() right before it opens this
    // dialog; DISCARD doesn't need this at all (it already goes through
    // msngr_text_entry_return_state() below, which is purpose-based and
    // already correct for both callers).
    uint8_t msngr_discard_confirm_return_state = MENU_STATE_MSNGR_TEXT_ENTRY;

    // Where leaving MENU_STATE_MSNGR_TEXT_ENTRY (Send/BACK/discard alike)
    // lands, based on why it was opened - shared by menu_msngr_text_
    // entry_leave() below and MENU_STATE_MSNGR_DISCARD_CONFIRM's own
    // DISCARD branch (menu_confirm_select()), which used to duplicate
    // this same two-way ternary rather than call it.
    uint8_t msngr_text_entry_return_state() {
      if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_DISPLAY_NAME) return MENU_STATE_MSNGR_SETTINGS;
      if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_PRESET) return MENU_STATE_MSNGR_PRESETS;
      if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH) return MENU_STATE_MSNGR_BOOKMARKS;
      if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_IDENTITY_RESTORE) return MENU_STATE_URNS_KEYS;
      if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_WIFI_SSID ||
          msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_WIFI_PSK) return MENU_STATE_WIFI_LIST;
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
        msngr_discard_confirm_return_state = menu_state; // CANCEL comes back here - see this var's own declaration
        menu_state = MENU_STATE_MSNGR_DISCARD_CONFIRM;
      }
    }
  #endif
  #if HAS_LXMF == true || HAS_WIFI == true
    // Appends an already-shift-resolved character to msngr_text_entry_buf,
    // respecting the same per-purpose length cap the on-screen keyboard's
    // own MSNGR_KB_CHAR/_SPACE dispatch enforces (menu_confirm_select()) -
    // extracted so the BLE keyboard path (blekbd_key_event(), below) can
    // reuse it instead of duplicating the cap logic. Silently drops the
    // character once the cap is hit, same as the on-screen keyboard always
    // has - no truncation warning, just stops accepting more input.
    void msngr_kb_insert_char(char c) {
      size_t max_len = MSNGR_TEXT_ENTRY_MAX_LEN;
      #if HAS_LXMF == true
        max_len = msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH
          ? (size_t)(LXMF::PEER_HASH_SIZE * 2)
          : msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_IDENTITY_RESTORE
          ? (size_t)VAULT_IDENTITY_KEY_BASE32_LEN
          : (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_DISPLAY_NAME ||
             msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_PRESET ||
             msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_RENAME)
            ? MSNGR_NAME_MAX_LEN : MSNGR_TEXT_ENTRY_MAX_LEN;
      #endif
      if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_WIFI_SSID ||
          msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_WIFI_PSK) max_len = 32;
      size_t text_len = strlen(msngr_text_entry_buf);
      if (text_len < max_len) {
        msngr_text_entry_buf[text_len] = c;
        msngr_text_entry_buf[text_len + 1] = 0;
        msngr_kb_last_insert_ms = millis(); // MSNGR_TEXT_ENTRY_PURPOSE_WIFI_PSK's brief reveal window - harmless no-op read for every other purpose
      }
    }

    // Trims the last character off msngr_text_entry_buf - extracted from
    // the on-screen keyboard's own MSNGR_KB_BACKSPACE dispatch (menu_
    // confirm_select()) for the same reuse reason as msngr_kb_insert_char()
    // above. No-ops on an already-empty buffer.
    void msngr_kb_do_backspace() {
      size_t text_len = strlen(msngr_text_entry_buf);
      if (text_len > 0) {
        msngr_text_entry_buf[text_len - 1] = 0;
        // Nothing left to reveal - the new last character (if any) wasn't
        // just typed, so PSK masking shouldn't briefly show it either.
        msngr_kb_last_insert_ms = 0;
      }
    }
  #endif
  #if HAS_LXMF == true || HAS_WIFI == true
    // Whatever's currently in msngr_text_entry_buf, dispatched by purpose
    // (message/display-name/bookmark-hash/preset/WiFi SSID+PSK) -
    // extracted verbatim from the on-screen keyboard's own MSNGR_KB_SEND
    // dispatch (menu_confirm_select()) so the BLE keyboard's Enter key can
    // trigger the exact same save/send flow instead of re-implementing
    // several separate purpose-specific paths. The WIFI_SSID/WIFI_PSK
    // branch is unconditional (no HAS_LXMF dependency - raw credential
    // bytes into staged_wifi_ssid/psk, no UTF-8 expansion since these
    // never go out as LXMF display text); every other purpose calls
    // Messenger.h/LXMF::/RNS:: functions that only exist under HAS_LXMF.
    void msngr_kb_do_send() {
      size_t text_len = strlen(msngr_text_entry_buf);
      if (text_len > 0 && (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_WIFI_SSID ||
                            msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_WIFI_PSK)) {
        char *dst = (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_WIFI_SSID) ? staged_wifi_ssid : staged_wifi_psk;
        strncpy(dst, msngr_text_entry_buf, 32); dst[32] = 0;
        msngr_text_entry_buf[0] = 0;
        menu_state = MENU_STATE_WIFI_LIST;
      }
      #if HAS_LXMF == true
      else if (text_len > 0 && msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_DISPLAY_NAME) {
        // No result screen needed - unlike an LXMF send, this can't fail
        // in a way worth reporting (a local file write), so it's save-
        // and-return rather than save-and-show-status. Expanded to real
        // UTF-8 first (msngr_kb_expand_utf8()) - the announce this name
        // goes out in is read by other Reticulum clients, not just this
        // device's own Org_01 glyph table.
        char name_utf8[MSNGR_NAME_MAX_LEN * 2 + 1];
        msngr_kb_expand_utf8(msngr_text_entry_buf, name_utf8, sizeof(name_utf8));
        msngr_display_name_conf_save(name_utf8);
        msngr_text_entry_buf[0] = 0;
        menu_state = MENU_STATE_MSNGR_SETTINGS;
      } else if (text_len > 0 && msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH) {
        // Manually-entered destination hash - no identity/keys yet (those
        // only ever arrive via an announce), but bookmarking it is enough:
        // opening the resulting peer screen and sending already runs
        // through messenger_send_process()'s existing Identity::recall()/
        // Transport::request_path() pending-send path (Messenger.h) the
        // same way replying to an unknown sender does, so no separate
        // "request keys" step is needed here - Send just resolves once an
        // announce comes back.
        uint8_t raw_hash[LXMF::PEER_HASH_SIZE];
        if (messenger_hash_from_hex(msngr_text_entry_buf, raw_hash, LXMF::PEER_HASH_SIZE)) {
          RNS::Bytes hash(raw_hash, LXMF::PEER_HASH_SIZE);
          if (messenger_bookmark_find(hash) < 0) {
            // Empty name, not messenger_peer_display_name(hash) - at this
            // point nothing is known about the peer yet, so that would
            // just resolve to the truncated-hex fallback and pin it as the
            // bookmark's name forever (messenger_peer_display_name()'s own
            // bookmark-name check short-circuits before ever reaching its
            // live Identity::recall_app_data() backfill check below, once
            // the bookmark has ANY non-empty name stored) - confirmed on
            // hardware: a hash-only bookmark's name never self-healed
            // until Remove+re-Add cleared the pinned hex and let the live
            // check run. Leaving it empty here keeps every future lookup
            // falling through to that live check until a real name
            // actually resolves.
            messenger_bookmark_add(hash, "", msngr_kb_bookmark_type);
          }
          msngr_text_entry_buf[0] = 0;
          menu_state = MENU_STATE_MSNGR_BOOKMARKS;
        } else {
          menu_open_popup("INVALID HASH", MENU_STATE_MSNGR_TEXT_ENTRY);
          menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
        }
      } else if (text_len > 0 && msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_IDENTITY_RESTORE) {
        // Manually-typed identity restore - VAULT_IDENTITY_KEY_BASE32_LEN
        // Base32 chars decode to the raw 64-byte private key (see
        // URNS_KEYS_ITEM_RESTORE's own comment). Validate first, THEN
        // confirm: this way a user who cancels the hold-gesture lands
        // back on the entry screen with their fully-typed buffer still
        // intact rather than having to retype the whole key, and an
        // invalid string never reaches the destructive-confirm step at
        // all. Reuses vault_identity_confirm()/vault_identity_commit()
        // (IdentityTransfer.h) exactly as the existing KISS CMD_IDENTITY_
        // IMPORT flow (vault_identity_import_flow()) already does for
        // this same "replace the identity" moment, so both entry paths
        // share one safety bar.
        uint8_t raw_key[VAULT_IDENTITY_KEYSIZE_BYTES];
        if (!identity_key_from_base32(msngr_text_entry_buf, raw_key)) {
          menu_open_popup("INVALID KEY", MENU_STATE_MSNGR_TEXT_ENTRY);
          menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
        } else {
          bool confirmed = vault_identity_confirm("Replace Identity?", "Current key is lost.", "Cannot be undone.");
          if (confirmed) {
            RNS::Bytes identity_plain(raw_key, VAULT_IDENTITY_KEYSIZE_BYTES);
            bool committed = vault_identity_commit(identity_plain);
            RNS::secure_zero(identity_plain);
            msngr_text_entry_buf[0] = 0;
            if (committed) {
              // Same "live session state is all built around the OLD
              // identity, a clean reboot is required" reasoning vault_
              // identity_import_flow() (IdentityTransfer.h) already
              // documents for the KISS path.
              vault_unlock_draw("Restored", "Restarting...");
              vault_wdt_safe_delay(1500);
              hard_reset();
            } else {
              menu_open_popup("ERROR", MENU_STATE_MSNGR_TEXT_ENTRY);
            }
          }
          // Cancelled: stay on MENU_STATE_MSNGR_TEXT_ENTRY with the typed
          // buffer intact - same "default to not losing the user's typing"
          // reasoning as every other purpose's own failure path above.
        }
        memset(raw_key, 0, sizeof(raw_key));
      } else if (text_len > 0 && msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_RENAME) {
        // No UTF-8 expansion, same reasoning PRESET below gets away with
        // skipping it - this name never goes out over the air (unlike
        // Display Name's announce), it's purely local, so it's stored
        // exactly as typed, same as every other bookmark name.
        messenger_bookmark_rename(msngr_active_peer_hash, msngr_text_entry_buf);
        msngr_text_entry_buf[0] = 0;
        menu_state = MENU_STATE_MSNGR_PEER;
      } else if (text_len > 0 && msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_PRESET) {
        // No UTF-8 expansion needed here unlike the display-name/message
        // branches - this text never leaves the device, same reasoning
        // bookmark names already get away with storing as typed
        // (Messenger.h). msngr_preset_edit_index == msngr_preset_count
        // (set when MENU_STATE_MSNGR_PRESETS' own "Add Preset" row opened
        // this screen) means append a new one; anything less is an
        // existing slot being edited in place - same "index == count
        // means append" sentinel messenger_preset_add() itself uses.
        if (msngr_preset_edit_index >= msngr_preset_count) {
          messenger_preset_add(msngr_text_entry_buf);
        } else {
          messenger_preset_update(msngr_preset_edit_index, msngr_text_entry_buf);
        }
        msngr_text_entry_buf[0] = 0;
        menu_state = MENU_STATE_MSNGR_PRESETS;
      } else if (text_len > 0) {
        // Only actually clear the composed text on a confirmed send - a
        // failure leaves it in place so the user can retry instead of
        // having to retype it. Same MENU_STATE_MSNGR_SEND_RESULT hand-off
        // as the preset Send: Hi/Bye/SOS actions - see that branch's own
        // comment. Expanded to real UTF-8 first, same reasoning as the
        // display-name save above.
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
          msngr_send_result_cursor = 1; // default BACK (2-row, fresh send never starts failed) - see its own declaration
          menu_state = MENU_STATE_MSNGR_SEND_RESULT;
        } else {
          menu_open_popup(urns_lxmf_send_result_text(msngr_last_send_result), MENU_STATE_MSNGR_TEXT_ENTRY);
          menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
        }
      }
      #endif
    }
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
    // 0 = REBOOT, 1 = CANCEL - same list-with-cursor pattern as
    // fwupd_confirm_cursor, defaulting to CANCEL for the same reason.
    uint8_t hw_reboot_confirm_cursor = 1;
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
    // Messenger's msngr_discard_confirm_cursor (DISCARD/CANCEL), but
    // defaults to CANCEL (1) rather than the primary action, since this
    // one reboots and reflashes the device instead of just discarding
    // some typed text.
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

  #if HAS_BLE_HID_HOST == true
    // BLE keyboard Brightness Up/Down keys (BLEKBD_CH_BRIGHTNESS_UP/_DOWN,
    // BLEKeyboardHost.h) - unlike step_brightness() above (which only ever
    // touches staged_display_brightness, deferred to menu_commit_and_exit()
    // like every other settings-menu field), this is a direct hardware-
    // button-style shortcut: steps and applies display_intensity (the LIVE
    // value the render path actually reads) immediately. Per user request,
    // a pure runtime adjustment, not persisted to EEPROM at all - reverts
    // to whatever's actually saved on the next boot, unlike the CMD_DISP_
    // INT KISS handler's own instant-set-AND-persist shape for the same
    // field, which this deliberately does NOT mirror. Reuses the same
    // discrete step tables (oled_brightness_index()/OLED_BRIGHTNESS_* or
    // backlight_brightness_index()/backlight_brightness_values, above)
    // rather than a raw 0-255 sweep, for the same "no perceptually useful
    // continuous range on OLED, tedious one-detent-at-a-time sweep on
    // backlight boards" reasons those exist - but on OLED specifically,
    // clamped to DIM/BRIGHT only (never OFF, index 0) per user request:
    // these are brightness keys, not a screen power toggle, so they should
    // never leave the display effectively switched off as a side effect.
    void blekbd_step_display_brightness(int8_t dir) {
      #if DISPLAY_IS_OLED == true
        static const uint8_t values[] = { OLED_BRIGHTNESS_OFF, OLED_BRIGHTNESS_DIM, OLED_BRIGHTNESS_BRIGHT };
        int8_t idx = (int8_t)oled_brightness_index(display_intensity) + (dir > 0 ? 1 : -1);
        if (idx < 1) idx = 1;
        if (idx > 2) idx = 2;
        display_intensity = values[idx];
        // Per user request - a brief on-screen confirmation, same overlay
        // primitive the KBD CONNECTED/DISCONNECTED notice uses (draw_
        // blekbd_notice_overlay(), above - superimposed on top of
        // whatever's currently on screen, not a real menu_state change).
        snprintf(blekbd_notice_text, sizeof(blekbd_notice_text), idx == 2 ? "BRIGHT" : "DIM");
      #else
        int8_t idx = (int8_t)backlight_brightness_index(display_intensity) + (dir > 0 ? 1 : -1);
        if (idx < 0) idx = 0;
        if (idx > 4) idx = 4;
        display_intensity = backlight_brightness_values[idx];
        format_brightness(display_intensity, blekbd_notice_text);
      #endif
      blekbd_notice_until_ms = millis() + ACTION_POPUP_MS;
    }

    #if HAS_BUZZER == true
      // BLE keyboard Volume Up/Down keys (BLEKBD_CH_SOUND_ENABLE/_DISABLE,
      // BLEKeyboardHost.h) - repurposed to enable/disable the buzzer, per
      // user request (this device has no analog volume for these to
      // otherwise adjust). Same "direct hardware-button shortcut, live
      // effect, no staged EEPROM session" shape as blekbd_step_display_
      // brightness() above - sound_enabled is checked directly by every
      // buzzer call (Utilities.h), so this takes effect immediately with
      // no further wiring needed.
      void blekbd_set_sound_enabled(bool enabled) {
        sound_enabled = enabled;
        snprintf(blekbd_notice_text, sizeof(blekbd_notice_text), enabled ? "SOUND ON" : "SOUND OFF");
        blekbd_notice_until_ms = millis() + ACTION_POPUP_MS;
      }
    #endif

    // On/off switch for MENU_STATE_MSNGR_CHAT's own "other rows" marquee
    // (draw block, below) - per user request, the cursor-highlighted
    // row's own scroll is always on regardless of this flag, both here
    // and in MSNGR_PEER's own marquee (which has no "other rows" concept
    // at all - only ever the one selected row - so it never reads this
    // flag in the first place). Only a BLE keyboard's Play/Pause key
    // (below) ever changes it, so unlike MSNGR_PEER's marquee code this
    // can safely live inside the HAS_BLE_HID_HOST guard - MENU_STATE_
    // MSNGR_CHAT only exists on boards that have one anyway.
    bool blekbd_marquee_enabled = true;
    void blekbd_toggle_marquee() {
      blekbd_marquee_enabled = !blekbd_marquee_enabled;
      snprintf(blekbd_notice_text, sizeof(blekbd_notice_text), blekbd_marquee_enabled ? "MARQUEE ON" : "MARQUEE OFF");
      blekbd_notice_until_ms = millis() + ACTION_POPUP_MS;
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

      // Messenger Settings > Periodic Sync (MENU_STATE_MSNGR_SETTINGS_EDIT) -
      // steps through msngr_sync_interval_presets_s's index range
      // (Messenger.h: Off/15m/30m/1h/2h/6h/12h/24h). Same "always clamped,
      // never wraps" reasoning as Auto Announce above.
      void step_msngr_sync_interval(int8_t dir, bool wrap = false) {
        int8_t v = (int8_t)staged_msngr_sync_interval_idx + (dir > 0 ? 1 : -1);
        if (v < 0) v = 0;
        if (v > MSNGR_SYNC_INTERVAL_PRESET_COUNT - 1) v = MSNGR_SYNC_INTERVAL_PRESET_COUNT - 1;
        staged_msngr_sync_interval_idx = (uint8_t)v;
      }

      // Messenger Settings > Sync Limit (MENU_STATE_MSNGR_SETTINGS_EDIT) -
      // 0-254 range (see ADDR_CONF_MSNGR_SYNC_LIMIT's own comment, ROM.h,
      // for why 255 is deliberately excluded), 1 message per step. Always
      // clamped, never wraps - same reasoning as Retries (wrapping 254
      // back to 0 would silently mean "unlimited", the opposite of what
      // turning the encoder further past 254 means).
      void step_msngr_sync_limit(int8_t dir, bool wrap = false) {
        int16_t v = (int16_t)staged_msngr_sync_limit + (dir > 0 ? 1 : -1);
        if (v < 0) v = 0;
        if (v > MSNGR_SYNC_LIMIT_MAX) v = MSNGR_SYNC_LIMIT_MAX;
        staged_msngr_sync_limit = (uint8_t)v;
      }

      // Messenger Settings > Required Stamp Cost (MENU_STATE_MSNGR_
      // SETTINGS_EDIT) - 0-254 range (see ADDR_CONF_MSNGR_STAMP_COST's own
      // comment, ROM.h, for why 255 is deliberately excluded, same as Sync
      // Limit above), 1 bit per step. Always clamped, never wraps -
      // wrapping 254 back to 0 would silently disable enforcement, the
      // opposite of what turning the encoder further past 254 means.
      void step_msngr_stamp_cost(int8_t dir, bool wrap = false) {
        int16_t v = (int16_t)staged_msngr_stamp_cost + (dir > 0 ? 1 : -1);
        if (v < 0) v = 0;
        if (v > MSNGR_STAMP_COST_MAX) v = MSNGR_STAMP_COST_MAX;
        staged_msngr_stamp_cost = (uint8_t)v;
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
    #if HAS_BLE_HID_HOST == true
      staged_blekbd_enabled = blekbd_enabled;
    #endif
    #if HAS_LXMF == true
      staged_msngr_max_retries = msngr_max_retries;
      staged_msngr_retry_delay_s = msngr_retry_delay_s;
      staged_msngr_announce_at_start = msngr_announce_at_start;
      staged_msngr_announce_interval_idx = msngr_announce_interval_idx;
      staged_msngr_propagate_on_fail = msngr_propagate_on_fail;
      staged_msngr_sync_interval_idx = msngr_sync_interval_idx;
      staged_msngr_sync_limit = msngr_sync_limit;
      staged_msngr_stamp_cost = msngr_stamp_cost;
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
    #if HAS_BLE_HID_HOST == true
      if (staged_blekbd_enabled != blekbd_enabled) {
        bool blekbd_now_disabled = !staged_blekbd_enabled;
        blekbd_enabled_conf_save(staged_blekbd_enabled);
        if (blekbd_now_disabled) blekbd_stop();
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
    #if HAS_BLE_HID_HOST == true
      // Same exemption, same reasoning as GNSS Diagnostics above - a paused
      // thought mid-message (composing with a physical keyboard, easy to
      // sit for a while between keystrokes) shouldn't get kicked back to
      // the main screen just because nothing's been pressed in a bit; this
      // is a live conversation view, not a screen this codebase's usual
      // "walked away from the settings menu" idle-close assumption fits.
      if (menu_state == MENU_STATE_MSNGR_CHAT) {
        display_unblank();
        return;
      }
    #endif
    // Same exemption, same reasoning as GNSS Diagnostics/MSNGR_CHAT above -
    // confirmed live: a PROPAGATED send genuinely in flight (proof-of-work
    // stamp grind, then the resource transfer/PN confirmation wait) can
    // easily run past SETTINGS_MENU_TIMEOUT (127s) with the user just
    // watching the screen, not touching any input - this idle-close was
    // silently kicking the user back to the home screen mid-operation,
    // with no status shown at all (distinct from, and in addition to,
    // msngr_send_result_process()'s own fix for error states no longer
    // auto-dismissing - that one only ever applied once a terminal result
    // existed to show; this covers the still-PENDING wait beforehand).
    // MSNGR_PING_RESULT included for the same reason, even though a ping
    // rarely runs long enough to hit this in practice.
    if (menu_state == MENU_STATE_MSNGR_SEND_RESULT || menu_state == MENU_STATE_MSNGR_PING_RESULT) {
      display_unblank();
      return;
    }
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
    #if HAS_LXMF == true
      // Full Message view is open, on top of whatever menu_state it was
      // opened from - per user request, rotation (real encoder, or the
      // single-button's own short-tap/double-tap forward/backward split,
      // menu_button_press()/menu_button_process() below, which already
      // calls this with dir=+1/-1 exactly like any other list-nav screen)
      // scrolls the message one line at a time instead of whatever the
      // underlying menu_state's own branch further down would do.
      // Clamped against the real wrapped line count in the draw block,
      // not here - msngr_msg_view_active's own declaration has the full
      // reasoning for why this lives outside HAS_BLE_HID_HOST.
      if (msngr_msg_view_active) {
        buzzer_encoder_tick_melody();
        if (dir > 0) msngr_msg_view_scroll_line++;
        else if (msngr_msg_view_scroll_line > 0) msngr_msg_view_scroll_line--;
        return;
      }
    #endif
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
        // Skip over URNS_ITEM_KEYS while vault_enabled is false - it's
        // absent from the drawn list (see the draw-block compaction
        // below), so the cursor must never rest on it either. Bounded to
        // at most 1 extra step since neither the first (URNS_ITEM_ENABLED)
        // nor last (URNS_ITEM_BACK) row is ever hidden.
        while (!vault_enabled && urns_menu_cursor >= URNS_ITEM_VAULT_ONLY_FIRST && urns_menu_cursor <= URNS_ITEM_VAULT_ONLY_LAST) {
          urns_menu_cursor = menu_clamp_cursor(urns_menu_cursor, dir, URNS_ITEM_COUNT, wrap);
        }
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
      } else if (menu_state == MENU_STATE_URNS_PATH_DELETE_CONFIRM) {
        buzzer_encoder_tick_melody();
        urns_path_delete_confirm_cursor = menu_clamp_cursor(urns_path_delete_confirm_cursor, dir, 2, wrap);
      } else if (menu_state == MENU_STATE_URNS_PATH_PURGE_CONFIRM) {
        buzzer_encoder_tick_melody();
        urns_path_purge_confirm_cursor = menu_clamp_cursor(urns_path_purge_confirm_cursor, dir, 2, wrap);
      }
      #if HAS_LXMF == true
        else if (menu_state == MENU_STATE_URNS_MSG_PURGE_CONFIRM) {
          buzzer_encoder_tick_melody();
          urns_msg_purge_confirm_cursor = menu_clamp_cursor(urns_msg_purge_confirm_cursor, dir, 2, wrap);
        }
      #endif
      else if (menu_state == MENU_STATE_URNS_IDENTITIES) {
        buzzer_encoder_tick_melody();
        urns_identities_cursor = menu_clamp_cursor(urns_identities_cursor, dir, URNS_ID_ITEM_COUNT, wrap);
      } else if (menu_state == MENU_STATE_URNS_KEYS) {
        buzzer_encoder_tick_melody();
        urns_keys_cursor = menu_clamp_cursor(urns_keys_cursor, dir, URNS_KEYS_ITEM_COUNT, wrap);
      } else if (menu_state == MENU_STATE_URNS_FREE_DETAIL) {
        buzzer_encoder_tick_melody();
        urns_free_detail_cursor = menu_clamp_cursor(urns_free_detail_cursor, dir, URNS_FREE_DETAIL_ITEM_COUNT, wrap);
      } else if (menu_state == MENU_STATE_URNS_PATH_HASH_VIEW) {
        // Nothing to move a cursor across - any rotation just dismisses
        // it too, same as a confirm (see menu_confirm_select()).
        buzzer_encoder_tick_melody();
        menu_state = menu_hash_view_return_state;
      } else if (menu_state == MENU_STATE_URNS_IDENTITY_KEY_VIEW) {
        // Single fixed entry point (unlike MENU_STATE_URNS_PATH_HASH_VIEW,
        // which is shared by several callers), so it returns straight to
        // MENU_STATE_URNS_KEYS rather than through a shared return-state
        // variable. Any rotation dismisses it, same as a confirm.
        buzzer_encoder_tick_melody();
        menu_state = MENU_STATE_URNS_KEYS;
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
      } else if (menu_state == MENU_STATE_MSNGR_DISCARD_CONFIRM) {
        buzzer_encoder_tick_melody();
        msngr_discard_confirm_cursor = menu_clamp_cursor(msngr_discard_confirm_cursor, dir, 2, wrap);
      } else if (menu_state == MENU_STATE_MSNGR_PING_RESULT) {
        buzzer_encoder_tick_melody();
        msngr_ping_result_cursor = menu_clamp_cursor(msngr_ping_result_cursor, dir, 2, wrap);
      } else if (menu_state == MENU_STATE_MSNGR_SEND_RESULT) {
        buzzer_encoder_tick_melody();
        msngr_send_result_cursor = menu_clamp_cursor(msngr_send_result_cursor, dir, msngr_send_result_row_count(), wrap);
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
        else if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_PROP_ON_FAIL) staged_msngr_propagate_on_fail = !staged_msngr_propagate_on_fail;
        else if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_SYNC_INTERVAL) step_msngr_sync_interval(dir, wrap);
        else if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_SYNC_LIMIT) step_msngr_sync_limit(dir, wrap);
        else if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_STAMP_COST) step_msngr_stamp_cost(dir, wrap);
      } else if (menu_state == MENU_STATE_MSNGR_PRESETS) {
        buzzer_encoder_tick_melody();
        msngr_presets_cursor = menu_clamp_cursor(msngr_presets_cursor, dir, msngr_presets_row_count(), wrap);
      } else if (menu_state == MENU_STATE_MSNGR_PRESET_DETAIL) {
        buzzer_encoder_tick_melody();
        msngr_preset_detail_cursor = menu_clamp_cursor(msngr_preset_detail_cursor, dir, 3, wrap);
      }
      #if HAS_BLE_HID_HOST == true
        // Per user request: a real encoder (or the single-button's own
        // forward/backward split, menu_button_press()/menu_button_process(),
        // which already calls this with dir=+1/-1 exactly like any other
        // list-nav screen) should be able to select messages here too, not
        // just a BLE keyboard's Up/Down (blekbd_key_event(), below) - both
        // now call the exact same msngr_chat_nav_up()/_down() (Messenger.h/
        // this file, above), which already handles first-rotate-enters-
        // browsing the same way Up/Down always have. msngr_msg_view_active's
        // own check, above, takes priority while the Full Message view is
        // open on top of this screen.
        else if (menu_state == MENU_STATE_MSNGR_CHAT) {
          buzzer_encoder_tick_melody();
          if (dir > 0) msngr_chat_nav_down(); else msngr_chat_nav_up();
        }
      #endif
      #endif
    #endif
    #if HAS_BLE_HID_HOST == true
      else if (menu_state == MENU_STATE_BLEKBD_LIST) {
        buzzer_encoder_tick_melody();
        blekbd_menu_cursor = menu_clamp_cursor(blekbd_menu_cursor, dir, BLEKBD_ITEM_COUNT, wrap);
      } else if (menu_state == MENU_STATE_BLEKBD_EDIT) {
        buzzer_encoder_tick_melody();
        // Only row here is Enabled - same "single boolean toggle" shape as
        // GNSS_EDIT's staged_gnss_enabled.
        staged_blekbd_enabled = !staged_blekbd_enabled;
      } else if (menu_state == MENU_STATE_BLEKBD_SCAN) {
        buzzer_encoder_tick_melody();
        blekbd_scan_cursor = menu_clamp_cursor(blekbd_scan_cursor, dir, blekbd_scan_row_count(), wrap);
      } else if (menu_state == MENU_STATE_BLEKBD_PAIR_CONFIRM) {
        buzzer_encoder_tick_melody();
        blekbd_pair_confirm_cursor = menu_clamp_cursor(blekbd_pair_confirm_cursor, dir, 2, wrap);
      } else if (menu_state == MENU_STATE_BLEKBD_FORGET_CONFIRM) {
        buzzer_encoder_tick_melody();
        blekbd_forget_confirm_cursor = menu_clamp_cursor(blekbd_forget_confirm_cursor, dir, 2, wrap);
      } else if (menu_state == MENU_STATE_BLEKBD_PAIRING) {
        buzzer_encoder_tick_melody();
        blekbd_pairing_cursor = menu_clamp_cursor(blekbd_pairing_cursor, dir, 2, wrap);
      }
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
      } else if (menu_state == MENU_STATE_HW_REBOOT_CONFIRM) {
        buzzer_encoder_tick_melody();
        hw_reboot_confirm_cursor = menu_clamp_cursor(hw_reboot_confirm_cursor, dir, 2, wrap);
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
        // select navigation as everywhere else, same pattern as Messenger's
        // DISCARD/CANCEL (MENU_STATE_MSNGR_DISCARD_CONFIRM/msngr_discard_confirm_cursor).
        buzzer_encoder_tick_melody();
        fwupd_confirm_cursor = menu_clamp_cursor(fwupd_confirm_cursor, dir, 2, wrap);
      }
    #endif
    // Standalone (not part of the HAS_URNS-gated else-if chain above,
    // which ends the function right at that #endif) - MSNGR_TEXT_ENTRY's
    // on-screen keyboard needs to be reachable on HAS_WIFI boards that
    // don't have HAS_URNS/HAS_LXMF too (WiFi SSID/PSK entry reuses it -
    // see MSNGR_TEXT_ENTRY_PURPOSE_WIFI_SSID/_WIFI_PSK). Steps one key at
    // a time through the flattened active-layout grid, row-major, wrapping
    // at both ends - see msngr_kb_cursor's own declaration for why this is
    // a single linear cursor rather than real 2D nav. Bounded by msngr_kb_
    // active_key_count(), not the flat MSNGR_KB_KEY_COUNT - MSNGR_TEXT_
    // ENTRY_PURPOSE_BOOKMARK_HASH's hex grid has far fewer real cells than
    // the normal 4-row keyboard (msngr_kb_cursor_rc()'s own comment), and
    // would otherwise wrap through nonexistent cells past the end of it.
    #if HAS_LXMF == true || HAS_WIFI == true
      if (menu_state == MENU_STATE_MSNGR_TEXT_ENTRY) {
        buzzer_encoder_tick_melody();
        msngr_kb_cursor = menu_clamp_cursor(msngr_kb_cursor, dir, msngr_kb_active_key_count(), wrap);
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
    #if HAS_LXMF == true || HAS_WIFI == true
      if (menu_state == MENU_STATE_MSNGR_TEXT_ENTRY) {
        msngr_kb_chord_used = true;
        uint8_t kb_row, kb_col;
        msngr_kb_cursor_rc(msngr_kb_cursor, kb_row, kb_col);
        char key_ch = msngr_kb_active_layout()[kb_row][kb_col];
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
            msngr_kb_last_insert_ms = millis();
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

    #if HAS_LXMF == true || HAS_WIFI == true
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
    #if HAS_LXMF == true || HAS_WIFI == true
      // Chording needs the button held down while rotating, which can
      // easily run past the normal 700ms threshold on a slow or deliberate
      // turn - a much longer threshold here means an ordinary chord
      // attempt doesn't also risk throwing away a half-typed message (or
      // half-typed WiFi SSID/PSK) via the long-press-leave path below.
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
      #if HAS_LXMF == true || HAS_WIFI == true
        if (menu_state == MENU_STATE_MSNGR_TEXT_ENTRY) {
          buzzer_encoder_click_melody();
          menu_msngr_text_entry_leave();
          return;
        }
      #endif
      #if HAS_LXMF == true
        // Full Message view is open (checked BEFORE the MENU_STATE_MSNGR_
        // CHAT check below - menu_state stays MENU_STATE_MSNGR_CHAT the
        // whole time the view is open on top of it, so checking CHAT
        // first would exit Dialog Mode entirely on a long-press instead
        // of just closing the view) - a real encoder's long-press
        // otherwise means "commit & exit the WHOLE menu" (below), which
        // would be a surprising overreach while just viewing a message;
        // treat it the same as the single-button/short-click "leave the
        // view" gesture instead (menu_confirm_select()'s own matching
        // check) - msngr_msg_view_active's own declaration has the full
        // reasoning.
        if (msngr_msg_view_active) {
          buzzer_encoder_click_melody();
          msngr_msg_view_active = false;
          menu_state = msngr_msg_view_return_state;
          return;
        }
        #if HAS_BLE_HID_HOST == true
          // Per user request: leaves Dialog Mode, same as the single-
          // button's own long-press (menu_confirm_select()'s matching
          // check) - otherwise a real encoder's long-press would fall
          // through to "commit & exit the WHOLE menu" below, a bigger
          // overreach than just leaving this one screen.
          if (menu_state == MENU_STATE_MSNGR_CHAT) {
            buzzer_encoder_click_melody();
            menu_msngr_text_entry_leave();
            return;
          }
        #endif
      #endif
      // Long-press: identical from anywhere inside the menu - commit & exit.
      // Text entry (MENU_STATE_MSNGR_TEXT_ENTRY, handled above) already
      // returns before reaching here, so this always behaves like every
      // other state.
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
    #if HAS_LXMF == true || HAS_WIFI == true
      bool msngr_kb_alt_already_beeped = msngr_kb_lang_hold_fired_enc || msngr_kb_alt_hold_fired_enc || msngr_kb_del_hold_fired_enc;
    #else
      bool msngr_kb_alt_already_beeped = false;
    #endif
    #if HAS_BLE_HID_HOST == true
      // Per user request: the encoder's own press/click (as opposed to
      // rotating it, which only navigates - menu_encoder_rotate()'s own
      // MENU_STATE_MSNGR_CHAT check) opens the currently-selected
      // message's Full Message view, mirroring a BLE keyboard's Enter-
      // while-browsing exactly (blekbd_key_event(), below) - same cache
      // fetch, same msngr_msg_view_active/_scroll_line/_return_state
      // setup. While composing (nothing selected) it instead sends, again
      // mirroring Enter there. Checked before falling through to the
      // shared menu_confirm_select() (which would otherwise always leave
      // Dialog Mode here, the same unconditional action the single-
      // button's own long-press uses, its own comment) - but only when
      // NOT already viewing a message, so a click there still reaches
      // menu_confirm_select()'s own msngr_msg_view_active check first and
      // closes the view instead.
      if (menu_state == MENU_STATE_MSNGR_CHAT && !msngr_msg_view_active) {
        buzzer_encoder_click_melody();
        if (msngr_chat_sel != 0xFF) {
          RNS::Bytes msg_hash(msngr_chat_cache[msngr_chat_sel].hash, LXMF::MESSAGE_HASH_SIZE);
          messenger_refresh_msg_detail_cache(msg_hash);
          msngr_msg_view_active = true;
          msngr_msg_view_scroll_line = 0;
          msngr_msg_view_return_state = MENU_STATE_MSNGR_CHAT;
        } else {
          msngr_chat_do_send();
        }
        return;
      }
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
    #if HAS_LXMF == true
      // Full Message view (msngr_msg_view_active) is open, on top of
      // whatever menu_state it was opened from - a click/confirm gesture
      // here always means "leave the view", same as the physical single-
      // button's long-press (menu_button_press()) and the real encoder's
      // long-press (menu_encoder_button(), its own matching check) both
      // already route through this same function. See msngr_msg_view_
      // active's own declaration for the full reasoning.
      if (msngr_msg_view_active) {
        msngr_msg_view_active = false;
        menu_state = msngr_msg_view_return_state;
        return;
      }
      #if HAS_BLE_HID_HOST == true
        // Per user request: the single-button's long-press (menu_button_
        // press()) and the real encoder's own short-click both already
        // route here - in Dialog Mode this should leave the whole screen,
        // not just deselect a browsed message (unconditional, regardless
        // of the composing/browsing sub-state). Reuses menu_msngr_text_
        // entry_leave() - the exact same "discard unsent text?" check a
        // keyboard's own Esc-while-composing already goes through
        // (blekbd_key_event(), below), since msngr_chat_insert_char()/
        // _backspace() write into the same msngr_text_entry_buf that
        // function already checks.
        if (menu_state == MENU_STATE_MSNGR_CHAT) {
          menu_msngr_text_entry_leave();
          return;
        }
      #endif
    #endif
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
          // Opens the same on-screen keyboard Messenger uses for composing/
          // editing text (MENU_STATE_MSNGR_TEXT_ENTRY) - see
          // MSNGR_TEXT_ENTRY_PURPOSE_WIFI_SSID/_WIFI_PSK. Fresh session,
          // preloaded from the current staged value, same reset (cursor/
          // shift/lang) every other purpose's own invocation site does.
          msngr_text_entry_purpose = (wifi_menu_cursor == WIFI_ITEM_SSID)
            ? MSNGR_TEXT_ENTRY_PURPOSE_WIFI_SSID : MSNGR_TEXT_ENTRY_PURPOSE_WIFI_PSK;
          const char *src = (wifi_menu_cursor == WIFI_ITEM_SSID) ? staged_wifi_ssid : staged_wifi_psk;
          strncpy(msngr_text_entry_buf, src, 32); msngr_text_entry_buf[32] = 0;
          msngr_kb_cursor = 0;
          msngr_kb_shift_on = false;
          msngr_kb_lang_ru = false;
          // 0 (never "recently typed") - a preloaded saved PSK must show
          // fully masked from the very first frame, not briefly reveal
          // its last character the way an actually-just-typed one does.
          msngr_kb_last_insert_ms = 0;
          menu_state = MENU_STATE_MSNGR_TEXT_ENTRY;
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
        #if HAS_BLE_HID_HOST == true
          else if (bt_menu_cursor == BT_ITEM_KEYBOARD) {
            blekbd_menu_cursor = 0;
            menu_state = MENU_STATE_BLEKBD_LIST;
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
    #if HAS_BLE_HID_HOST == true
      else if (menu_state == MENU_STATE_BLEKBD_LIST) {
        if (blekbd_menu_cursor == BLEKBD_ITEM_ENABLED) {
          menu_state = MENU_STATE_BLEKBD_EDIT;
        } else if (blekbd_menu_cursor == BLEKBD_ITEM_SCAN) {
          if (!staged_blekbd_enabled) {
            menu_open_popup("ENABLE FIRST", MENU_STATE_BLEKBD_LIST);
            menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
          } else {
            blekbd_discovery_start();
            blekbd_scan_cursor = 0;
            menu_state = MENU_STATE_BLEKBD_SCAN;
          }
        } else if (blekbd_menu_cursor == BLEKBD_ITEM_FORGET) {
          if (!blekbd_peer_stored) {
            menu_open_popup("NOT PAIRED", MENU_STATE_BLEKBD_LIST);
            menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
          } else {
            blekbd_forget_confirm_cursor = 1; // default CANCEL
            menu_state = MENU_STATE_BLEKBD_FORGET_CONFIRM;
          }
        } else if (blekbd_menu_cursor == BLEKBD_ITEM_BACK) {
          menu_state = MENU_STATE_BT_LIST;
        }
        // STATUS is read-only, same shape as BT_LIST's MAC/Bonds rows.
      }
      else if (menu_state == MENU_STATE_BLEKBD_EDIT) {
        menu_state = MENU_STATE_BLEKBD_LIST; // confirms staged value, no write yet
      }
      else if (menu_state == MENU_STATE_BLEKBD_SCAN) {
        uint8_t row_count = blekbd_scan_row_count();
        bool have_rows = false;
        for (uint8_t i = 0; i < BLEKBD_MAX_DISCOVERED; i++) if (blekbd_discovered[i].in_use) { have_rows = true; break; }
        if (blekbd_scan_cursor == row_count - 1) {
          blekbd_scan_stop();
          menu_state = MENU_STATE_BLEKBD_LIST;
        } else if (have_rows) {
          uint8_t vis = 0;
          for (uint8_t i = 0; i < BLEKBD_MAX_DISCOVERED; i++) {
            if (!blekbd_discovered[i].in_use) continue;
            if (vis == blekbd_scan_cursor) {
              memcpy(blekbd_pending_addr, blekbd_discovered[i].addr, 6);
              blekbd_pending_addr_type = blekbd_discovered[i].addr_type;
              strncpy(blekbd_pending_name, blekbd_discovered[i].name, BLEKBD_NAME_MAX_LEN);
              blekbd_pending_name[BLEKBD_NAME_MAX_LEN] = 0;
              blekbd_pair_confirm_cursor = 1; // default CANCEL
              menu_state = MENU_STATE_BLEKBD_PAIR_CONFIRM;
              break;
            }
            vis++;
          }
        }
        // Row 0 with no discovered devices yet ("Scanning...") is read-only.
      }
      else if (menu_state == MENU_STATE_BLEKBD_PAIR_CONFIRM) {
        if (blekbd_pair_confirm_cursor == 0) { // PAIR
          blekbd_scan_stop();
          blekbd_connect_start(blekbd_pending_addr, blekbd_pending_addr_type);
          blekbd_pairing_cursor = 1;
          menu_state = MENU_STATE_BLEKBD_PAIRING;
        } else { // CANCEL - resume scanning
          menu_state = MENU_STATE_BLEKBD_SCAN;
        }
      }
      else if (menu_state == MENU_STATE_BLEKBD_PAIRING) {
        // Row 0 (status) is read-only - only BACK does anything, same shape
        // as MENU_STATE_MSNGR_SEND_RESULT.
        if (blekbd_pairing_cursor == 1) {
          blekbd_pair_result = BLEKBD_PAIR_NONE;
          menu_state = MENU_STATE_BLEKBD_LIST;
        }
      }
      else if (menu_state == MENU_STATE_BLEKBD_FORGET_CONFIRM) {
        if (blekbd_forget_confirm_cursor == 0) { // FORGET
          if (blekbd_open_dev) esp_hidh_dev_close(blekbd_open_dev);
          ble_addr_t peer_addr;
          memcpy(peer_addr.val, blekbd_peer_addr, 6);
          peer_addr.type = blekbd_peer_addr_type;
          ble_store_util_delete_peer(&peer_addr);
          blekbd_peer_conf_forget();
        }
        // CANCEL: leave the stored peer/bond untouched.
        menu_state = MENU_STATE_BLEKBD_LIST;
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
        } else if (urns_menu_cursor == URNS_ITEM_VAULT) {
          // Immediate action, not a staged toggle - see URNS_ITEM_VAULT's
          // own comment for why. Blocking (VaultUnlock.h takes over
          // encoder/button dispatch and the whole screen for the duration)
          // - menu_state stays MENU_STATE_URNS_LIST throughout, so the
          // list redraws normally once this returns.
          if (vault_enabled) {
            vault_disable_flow();
          } else {
            vault_enroll_flow();
          }
        } else if (urns_menu_cursor == URNS_ITEM_ENABLED || urns_menu_cursor == URNS_ITEM_TRANSPORT ||
                   #if HAS_ESPNOW == true
                   urns_menu_cursor == URNS_ITEM_INTERFACE ||
                   #endif
                   urns_menu_cursor == URNS_ITEM_LINK_MTU_DISCOVERY || urns_menu_cursor == URNS_ITEM_REMOTE_MGMT) {
          menu_state = MENU_STATE_URNS_EDIT;
        } else if (urns_menu_cursor == URNS_ITEM_PATHS) {
          menu_state = MENU_STATE_URNS_PATHS;
          urns_paths_menu_cursor = 0;
        } else if (urns_menu_cursor == URNS_ITEM_IDENTITIES) {
          urns_identities_cursor = 0;
          menu_state = MENU_STATE_URNS_IDENTITIES;
        } else if (urns_menu_cursor == URNS_ITEM_KEYS) {
          urns_keys_cursor = 0;
          menu_state = MENU_STATE_URNS_KEYS;
        } else if (urns_menu_cursor == URNS_ITEM_FREE) {
          urns_free_detail_refresh();
          urns_free_detail_cursor = URNS_FREE_DETAIL_ITEM_BACK;
          menu_state = MENU_STATE_URNS_FREE_DETAIL;
        }
      } else if (menu_state == MENU_STATE_URNS_FREE_DETAIL) {
        // Identity/Announce/Other stay read-only info - Paths and Messages
        // (on HAS_LXMF boards) are now also clickable, each opening its own
        // PURGE/CANCEL confirm dialog before wiping that whole bucket at
        // once, same "confirm before a destructive bulk action" shape as
        // MENU_STATE_URNS_PATH_DELETE_CONFIRM (one entry) scaled up to
        // "every entry".
        if (urns_free_detail_cursor == URNS_FREE_DETAIL_ITEM_BACK) {
          menu_state = MENU_STATE_URNS_LIST;
        } else if (urns_free_detail_cursor == URNS_FREE_DETAIL_ITEM_PATHS) {
          urns_path_purge_confirm_cursor = 1; // default CANCEL
          menu_state = MENU_STATE_URNS_PATH_PURGE_CONFIRM;
        #if HAS_LXMF == true
        } else if (urns_free_detail_cursor == URNS_FREE_DETAIL_ITEM_MESSAGES) {
          urns_msg_purge_confirm_cursor = 1; // default CANCEL
          menu_state = MENU_STATE_URNS_MSG_PURGE_CONFIRM;
        #endif
        } else if (urns_free_detail_cursor == URNS_FREE_DETAIL_ITEM_REFRESH) {
          // Screen stays open (menu_state untouched) - see this item's own
          // #define comment for why a purge doesn't already make this
          // redundant.
          urns_free_detail_refresh();
        }
      } else if (menu_state == MENU_STATE_URNS_PATH_PURGE_CONFIRM) {
        if (urns_path_purge_confirm_cursor == 0) { // PURGE
          urns_purge_path_table();
          urns_free_detail_refresh();
          urns_paths_menu_cursor = 0;
          menu_open_popup("PURGED", MENU_STATE_URNS_FREE_DETAIL);
          menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
        } else { // CANCEL
          menu_state = MENU_STATE_URNS_FREE_DETAIL;
        }
      #if HAS_LXMF == true
      } else if (menu_state == MENU_STATE_URNS_MSG_PURGE_CONFIRM) {
        if (urns_msg_purge_confirm_cursor == 0) { // PURGE
          // Same DIO0-masking reasoning as MENU_STATE_MSNGR_CLEAR_CONFIRM's
          // delete_conversation() call - clear_all() is the same MessageStore
          // flash I/O, just for every conversation at once.
          LoRa->maskDio0();
          if (urns_message_store) urns_message_store->clear_all();
          LoRa->unmaskDio0();
          urns_free_detail_refresh();
          menu_open_popup("PURGED", MENU_STATE_URNS_FREE_DETAIL);
          menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
        } else { // CANCEL
          menu_state = MENU_STATE_URNS_FREE_DETAIL;
        }
      #endif
      } else if (menu_state == MENU_STATE_URNS_EDIT) {
        // Same deferred-commit reasoning as ESP-NOW's own Enabled field -
        // urns_init()/urns_radio_bringup() are boot-only, so nothing is
        // written here, only staged.
        menu_state = MENU_STATE_URNS_LIST;
      } else if (menu_state == MENU_STATE_URNS_PATHS) {
        urns_path_cache_refresh_if_stale();
        uint8_t row_count = urns_path_display_row_count();
        if (urns_paths_menu_cursor == row_count - 1) {
          menu_state = MENU_STATE_URNS_LIST;
        } else if (urns_paths_menu_cursor < urns_path_cache_count) {
          // A real path row, not the inert "No Paths" placeholder (which
          // only ever sits at index 0 when the cache is empty - cursor==0
          // falls through to here doing nothing in that case). Full hash
          // comes straight out of urns_path_cache - no need to re-walk the
          // store a second time to find it by index, which is what this
          // used to do (see that cache's own comment for why that was
          // worth removing too, not just the draw path's own walk).
          urns_path_detail_hash = urns_path_cache[urns_paths_menu_cursor].hash;
          urns_path_detail_cursor = 0;
          menu_state = MENU_STATE_URNS_PATH_DETAIL;
        }
      } else if (menu_state == MENU_STATE_URNS_PATH_DETAIL) {
        // Expiry is read-only info - Hash (opens the full-hash view),
        // Delete Path (opens the DELETE/CANCEL confirm dialog), and BACK
        // are the only rows that do anything.
        if (urns_path_detail_cursor == URNS_PATH_DETAIL_ITEM_BACK) {
          menu_state = MENU_STATE_URNS_PATHS;
        } else if (urns_path_detail_cursor == URNS_PATH_DETAIL_ITEM_HASH) {
          menu_hash_view_return_state = MENU_STATE_URNS_PATH_DETAIL;
          menu_state = MENU_STATE_URNS_PATH_HASH_VIEW;
        } else if (urns_path_detail_cursor == URNS_PATH_DETAIL_ITEM_DELETE) {
          urns_path_delete_confirm_cursor = 1; // default CANCEL
          menu_state = MENU_STATE_URNS_PATH_DELETE_CONFIRM;
        }
      } else if (menu_state == MENU_STATE_URNS_PATH_DELETE_CONFIRM) {
        if (urns_path_delete_confirm_cursor == 0) { // DELETE
          // Real flash I/O (TypedStore::remove(), microStore) on the same
          // "urns" LittleFS partition the DIO0 RX ISR's own handleDio0Rise()
          // can be interrupted into mid-SPI-transfer - same hazard as
          // MessageStore's own delete_message()/delete_conversation() (see
          // MENU_STATE_MSNGR_DELETE_CONFIRM's own comment), so mask around
          // it the same way.
          LoRa->maskDio0();
          RNS::Transport::remove_path(urns_path_detail_hash);
          LoRa->unmaskDio0();
          // The path list this was opened from just shrank by one - land
          // back at the top of it rather than risking a stale index into a
          // now-shorter list, same reasoning as the Messenger delete flows.
          urns_paths_menu_cursor = 0;
          menu_open_popup("DELETED", MENU_STATE_URNS_PATHS);
          menu_popup_auto_dismiss_at = millis() + ACTION_POPUP_MS;
        } else { // CANCEL
          menu_state = MENU_STATE_URNS_PATH_DETAIL;
        }
      } else if (menu_state == MENU_STATE_URNS_IDENTITIES) {
        if (urns_identities_cursor == URNS_ID_ITEM_BACK) {
          menu_state = MENU_STATE_URNS_LIST;
        } else if (urns_identities_cursor == URNS_ID_ITEM_PROBE_DEST && !RNS::Transport::probe_destination()) {
          // Inactive row (see its own N/A comment) - no hash to show, so
          // selecting it is a no-op rather than opening the hash view on
          // whatever unrelated hash happened to be there from a previous
          // visit.
        } else {
          if (urns_identities_cursor == URNS_ID_ITEM_NODE_IDENTITY) {
            urns_path_detail_hash = urns_identity.hash();
          }
          #if HAS_LXMF == true
          else if (urns_identities_cursor == URNS_ID_ITEM_LXMF_DEST && urns_lxmf_router) {
            urns_path_detail_hash = urns_lxmf_router->delivery_destination().hash();
          }
          #endif
          else if (urns_identities_cursor == URNS_ID_ITEM_TRANSPORT_IDENTITY) {
            urns_path_detail_hash = RNS::Transport::identity().hash();
          } else if (urns_identities_cursor == URNS_ID_ITEM_PROBE_DEST) {
            urns_path_detail_hash = RNS::Transport::probe_destination().hash();
          }
          menu_hash_view_return_state = MENU_STATE_URNS_IDENTITIES;
          menu_state = MENU_STATE_URNS_PATH_HASH_VIEW;
        }
      } else if (menu_state == MENU_STATE_URNS_KEYS) {
        if (urns_keys_cursor == URNS_KEYS_ITEM_BACK) {
          menu_state = MENU_STATE_URNS_LIST;
        } else if (urns_keys_cursor == URNS_KEYS_ITEM_DISPLAY) {
          menu_state = MENU_STATE_URNS_IDENTITY_KEY_VIEW;
        #if HAS_LXMF == true
        } else if (urns_keys_cursor == URNS_KEYS_ITEM_RESTORE) {
          msngr_text_entry_purpose = MSNGR_TEXT_ENTRY_PURPOSE_IDENTITY_RESTORE;
          msngr_text_entry_buf[0] = 0;
          msngr_kb_cursor = 0;
          menu_state = MENU_STATE_MSNGR_TEXT_ENTRY;
        #endif
        }
      } else if (menu_state == MENU_STATE_URNS_PATH_HASH_VIEW) {
        // A single fixed view, nothing to select - any confirm just
        // dismisses it, same as MENU_STATE_STATUS_POPUP.
        menu_state = menu_hash_view_return_state;
      } else if (menu_state == MENU_STATE_URNS_IDENTITY_KEY_VIEW) {
        // Single fixed entry point - see menu_encoder_rotate()'s own
        // handling of this state for why it returns straight to
        // MENU_STATE_URNS_KEYS rather than menu_hash_view_return_state.
        menu_state = MENU_STATE_URNS_KEYS;
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
          // stamp cost via LXMRouter::announce()'s own app_data build) -
          // see urns_announce_lxmf()'s own comment (URNS.h).
          if (urns_ready && urns_lxmf_router) {
            urns_lxmf_router->announce();
            menu_open_popup("ANNOUNCED", MENU_STATE_MSNGR_LIST);
            menu_popup_auto_dismiss_at = millis() + MSNGR_ANNOUNCE_POPUP_MS;
          } else {
            menu_open_popup("NOT READY", MENU_STATE_MSNGR_LIST);
          }
        } else if (msngr_menu_cursor == MSNGR_TOP_ITEM_SYNC_PROP) {
          if (msngr_active_prop_node_hash.size() != LXMF::PEER_HASH_SIZE) {
            menu_open_popup("Prop Not Set", MENU_STATE_MSNGR_LIST);
          } else {
            msngr_prop_sync_start(msngr_active_prop_node_hash, MENU_STATE_MSNGR_LIST);
          }
        } else if (msngr_menu_cursor == MSNGR_TOP_ITEM_SETTINGS) {
          menu_state = MENU_STATE_MSNGR_SETTINGS;
          msngr_settings_cursor = 0;
        }
      } else if (menu_state == MENU_STATE_MSNGR_SETTINGS) {
        if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_RETRIES ||
            msngr_settings_cursor == MSNGR_SETTINGS_ITEM_RETRY_DELAY ||
            msngr_settings_cursor == MSNGR_SETTINGS_ITEM_ANNOUNCE_START ||
            msngr_settings_cursor == MSNGR_SETTINGS_ITEM_ANNOUNCE_INTERVAL ||
            msngr_settings_cursor == MSNGR_SETTINGS_ITEM_PROP_ON_FAIL ||
            msngr_settings_cursor == MSNGR_SETTINGS_ITEM_SYNC_INTERVAL ||
            msngr_settings_cursor == MSNGR_SETTINGS_ITEM_SYNC_LIMIT ||
            msngr_settings_cursor == MSNGR_SETTINGS_ITEM_STAMP_COST) {
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
          if (staged_msngr_propagate_on_fail != msngr_propagate_on_fail) {
            msngr_propagate_on_fail_conf_save(staged_msngr_propagate_on_fail);
          }
          if (staged_msngr_sync_interval_idx != msngr_sync_interval_idx) {
            msngr_sync_interval_conf_save(staged_msngr_sync_interval_idx);
          }
          if (staged_msngr_sync_limit != msngr_sync_limit) {
            msngr_sync_limit_conf_save(staged_msngr_sync_limit);
          }
          if (staged_msngr_stamp_cost != msngr_stamp_cost) {
            msngr_stamp_cost_conf_save(staged_msngr_stamp_cost);
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
        bool has_add_row = msngr_bookmark_count < MSNGR_MAX_BOOKMARKS;
        if (msngr_bookmarks_cursor == row_count - 1) {
          menu_state = MENU_STATE_MSNGR_LIST;
        } else if (has_add_row && msngr_bookmarks_cursor == msngr_bookmark_count) {
          // "Add by Hash" row - same reset-and-open sequence Presets' own
          // "Add Preset" row uses (MENU_STATE_MSNGR_PRESETS above).
          msngr_text_entry_purpose = MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH;
          msngr_text_entry_buf[0] = 0;
          msngr_kb_cursor = 0;
          msngr_kb_shift_on = false;
          msngr_kb_lang_ru = false;
          msngr_kb_bookmark_type = MSNGR_BOOKMARK_TYPE_LXMF;
          menu_state = MENU_STATE_MSNGR_TEXT_ENTRY;
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
      } else if (menu_state == MENU_STATE_MSNGR_PEER && messenger_bookmark_is_prop_node(msngr_active_peer_hash)) {
        if (msngr_peer_cursor == MSNGR_PEER_PROP_ACTION_SYNC) {
          msngr_prop_sync_start(msngr_active_peer_hash, MENU_STATE_MSNGR_PEER);
        } else if (msngr_peer_cursor == MSNGR_PEER_PROP_ACTION_PING) {
          messenger_ping_start(msngr_active_peer_hash, true);
          msngr_ping_result_cursor = 1; // default BACK - see its own declaration
          menu_state = MENU_STATE_MSNGR_PING_RESULT;
        } else if (msngr_peer_cursor == MSNGR_PEER_PROP_ACTION_SHOW_HASH) {
          urns_path_detail_hash = msngr_active_peer_hash;
          menu_hash_view_return_state = MENU_STATE_MSNGR_PEER;
          menu_state = MENU_STATE_URNS_PATH_HASH_VIEW;
        } else if (msngr_peer_cursor == MSNGR_PEER_PROP_ACTION_SET_ACTIVE) {
          if (messenger_prop_node_is_active(msngr_active_peer_hash)) {
            messenger_prop_node_clear_active();
          } else {
            messenger_prop_node_set_active(msngr_active_peer_hash);
          }
        } else if (msngr_peer_cursor == MSNGR_PEER_PROP_ACTION_RENAME) {
          // Pre-populated with whatever name is already set (blank for a
          // never-renamed node, same "real starting value, never blank
          // when there is one" intent as Display Name's own prefill
          // above) so re-opening Rename to tweak a name doesn't require
          // retyping it from scratch.
          msngr_text_entry_purpose = MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_RENAME;
          msngr_text_entry_buf[0] = 0;
          int8_t bm = messenger_bookmark_find(msngr_active_peer_hash);
          if (bm >= 0) snprintf(msngr_text_entry_buf, MSNGR_TEXT_ENTRY_MAX_LEN + 1, "%s", msngr_bookmarks[bm].name);
          msngr_kb_cursor = 0;
          msngr_kb_shift_on = false;
          msngr_kb_lang_ru = false;
          menu_state = MENU_STATE_MSNGR_TEXT_ENTRY;
        } else if (msngr_peer_cursor == MSNGR_PEER_PROP_ACTION_REMOVE) {
          messenger_bookmark_remove(msngr_active_peer_hash);
          menu_state = msngr_peer_return_state;
        } else { // MSNGR_PEER_PROP_ACTION_BACK
          menu_state = msngr_peer_return_state;
        }
      } else if (menu_state == MENU_STATE_MSNGR_PEER) {
        uint8_t msg_rows = msngr_peer_msg_row_count();
        if (msngr_peer_cursor < msg_rows) {
          // Reads msngr_peer_cache (Messenger.h), not MessageStore
          // directly - see that cache's own comment for why.
          RNS::Bytes msg_hash(msngr_peer_cache[msngr_peer_cursor].hash, LXMF::MESSAGE_HASH_SIZE);
          messenger_refresh_msg_detail_cache(msg_hash);
          msngr_msg_detail_refresh_wrap_cache();
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
              msngr_send_result_cursor = 1; // default BACK (2-row, fresh send never starts failed) - see its own declaration
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
            } else if (fixed_action == MSNGR_PEER_FIXED_ACTION_SHOW_HASH) {
              urns_path_detail_hash = msngr_active_peer_hash;
              menu_hash_view_return_state = MENU_STATE_MSNGR_PEER;
              menu_state = MENU_STATE_URNS_PATH_HASH_VIEW;
            } else if (fixed_action == MSNGR_PEER_FIXED_ACTION_CLEAR) {
              msngr_clear_confirm_cursor = 1; // default CANCEL - see its own declaration
              menu_state = MENU_STATE_MSNGR_CLEAR_CONFIRM;
            } else if (fixed_action == MSNGR_PEER_FIXED_ACTION_PING) {
              messenger_ping_start(msngr_active_peer_hash);
              msngr_ping_result_cursor = 1; // default BACK - see its own declaration
              menu_state = MENU_STATE_MSNGR_PING_RESULT;
            } else if (fixed_action == MSNGR_PEER_FIXED_ACTION_DELIVERY_MODE) {
              messenger_toggle_delivery_mode(msngr_active_peer_hash);
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
            #if HAS_BLE_HID_HOST == true
              else if (fixed_action == MSNGR_PEER_FIXED_ACTION_CHAT) {
                // Same purpose/buffer reset as Compose message above. No
                // on-screen-keyboard state to reset (msngr_kb_cursor/
                // shift_on/lang_ru) - this screen never reads them.
                // msngr_chat_cache (Messenger.h) is Chat's own, separate
                // from msngr_peer_cache - freshly windowed to the most
                // recent page here (jump-to-end, same as Enter/End would),
                // not just left however it was after a previous Chat
                // session.
                msngr_text_entry_purpose = MSNGR_TEXT_ENTRY_PURPOSE_MESSAGE;
                msngr_text_entry_buf[0] = 0;
                msngr_chat_cursor = 0;
                msngr_chat_compose_win_start = 0;
                msngr_chat_sel = 0xFF;
                msngr_chat_delete_confirm_pending = false;
                msngr_msg_view_active = false;
                messenger_refresh_chat_window(msngr_active_peer_hash, (size_t)-1);
                menu_state = MENU_STATE_MSNGR_CHAT;
              }
            #endif
          }
        }
      } else if (menu_state == MENU_STATE_MSNGR_MSG_DETAIL) {
        // Content lines are read-only - only the trailing REPLY/DELETE/
        // FULL MESSAGE/BACK rows do anything.
        uint8_t row_count = msngr_msg_detail_row_count();
        if (msngr_msg_detail_cursor == row_count - 1) {
          menu_state = MENU_STATE_MSNGR_PEER;
        } else if (msngr_msg_detail_cursor == row_count - 2) {
          // Full Message - per user request, right under Delete, above
          // BACK. menu_state deliberately stays MENU_STATE_MSNGR_MSG_
          // DETAIL (mirrors MENU_STATE_MSNGR_CHAT's own viewing-message
          // sub-mode, which never leaves MENU_STATE_MSNGR_CHAT either) -
          // the draw block (below) and the msngr_msg_view_active checks in
          // menu_encoder_rotate()/menu_confirm_select()/menu_encoder_
          // button() (their own comments) take over from here until the
          // view closes back to msngr_msg_view_return_state.
          msngr_msg_view_active = true;
          msngr_msg_view_scroll_line = 0;
          msngr_msg_view_return_state = MENU_STATE_MSNGR_MSG_DETAIL;
        } else if (msngr_msg_detail_cursor == row_count - 3) {
          msngr_delete_confirm_cursor = 1; // default CANCEL - see its own declaration
          menu_state = MENU_STATE_MSNGR_DELETE_CONFIRM;
        #if HAS_AUDIO == true
        } else if (msngr_msg_detail_has_audio && msngr_msg_detail_cursor == row_count - 5) {
          // Play / Stop (voice messages only, sits just above Reply).
          if (audio_state() == AUDIO_IDLE) messenger_voice_play(msngr_active_message_hash);
          else audio_play_stop();
        #endif
        } else if (msngr_msg_detail_cursor == row_count - 4) {
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
      } else if (menu_state == MENU_STATE_MSNGR_PING_RESULT) {
        // Row 0 (status) is read-only - only BACK does anything, and it
        // doubles as Cancel while a ping's still in flight.
        if (msngr_ping_result_cursor == 1) {
          messenger_ping_cancel();
          menu_state = MENU_STATE_MSNGR_PEER;
        }
      } else if (menu_state == MENU_STATE_MSNGR_SEND_RESULT) {
        // Row 0 (status) is read-only. Row 1 is BACK normally (2-row
        // layout) but becomes Retry, with Retry via Prop right below it
        // and BACK pushed to row 3 (4-row layout, draw code above), once
        // the send has reached a terminal failure state AND the router's
        // own outbound queue is actually free (msngr_send_result_row_
        // count() is the single source of truth both places agree on) -
        // while the original send is still retrying in the background
        // (msngr_send_result_queue_busy()), row_count()==2 same as an
        // in-flight/successful send, so cursor 1 is still BACK, not Retry.
        bool send_result_failed = msngr_send_result_failed();
        bool send_result_actionable = send_result_failed && !msngr_send_result_queue_busy();
        uint8_t back_row = msngr_send_result_row_count() - 1;
        if (send_result_actionable && msngr_send_result_cursor == 1) {
          // Retry - re-fires the exact same destination/content, using
          // whatever delivery method the peer's own setting calls for
          // (msngr_send_pending_dest_hash/_content, Messenger.h, populated
          // unconditionally by every messenger_send_lxmf() call) rather
          // than making the user back out and retype the message.
          messenger_send_lxmf(msngr_send_pending_dest_hash, msngr_send_pending_content);
        } else if (send_result_actionable && msngr_send_result_cursor == 2) {
          // Retry via Prop - same re-fire, but forces PROPAGATED for this
          // one send regardless of the peer's Send Direct/Send Propagated
          // setting (messenger_send_lxmf()'s forced_method, Messenger.h) -
          // useful when a direct/opportunistic attempt just timed out and
          // a propagation node is known to be reachable instead.
          messenger_send_lxmf(msngr_send_pending_dest_hash, msngr_send_pending_content, LXMF::Type::Message::PROPAGATED);
        } else if (msngr_send_result_cursor == back_row) {
          // BACK. If the packet's already gone out (PENDING/DELIVERED/
          // TIMEOUT) there's nothing to tear down, same as Ping - this just
          // stops watching for this send's proof so a late-arriving one
          // doesn't affect whatever the screen shows next time it's opened
          // for a different send. If still RESOLVING (waiting on identity/
          // path - messenger_send_lxmf()/_process(), Messenger.h), setting
          // state back to IDLE here doubles as a real cancel: nothing's
          // been sent yet in that case, and messenger_send_process() only
          // acts on MSNGR_SEND_RESOLVING, so the parked message is simply
          // abandoned rather than firing off later without the user
          // watching.
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
        else if (hw_menu_cursor == HW_ITEM_REBOOT) {
          hw_reboot_confirm_cursor = 1; // default CANCEL
          menu_state = MENU_STATE_HW_REBOOT_CONFIRM;
        }
      } else if (menu_state == MENU_STATE_HW_REBOOT_CONFIRM) {
        if (hw_reboot_confirm_cursor == 0) { // REBOOT
          hard_reset(); // never returns
        } else { // CANCEL
          menu_state = MENU_STATE_HW_LIST;
        }
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
    // Standalone (not part of the HAS_URNS-gated else-if chain above) -
    // MSNGR_TEXT_ENTRY/_DISCARD_CONFIRM need to be reachable on HAS_WIFI
    // boards that don't have HAS_URNS/HAS_LXMF too (WiFi SSID/PSK entry
    // reuses this same on-screen keyboard - see MSNGR_TEXT_ENTRY_PURPOSE_
    // WIFI_SSID/_WIFI_PSK).
    #if HAS_LXMF == true || HAS_WIFI == true
      if (menu_state == MENU_STATE_MSNGR_TEXT_ENTRY) {
        // "Presses" whichever key msngr_kb_cursor is currently highlighting -
        // insert/space/backspace mutate msngr_text_entry_buf in place (always
        // appending/trimming at the end, no mid-string edit point, same
        // simplification meshtastic's own VirtualKeyboard makes). Shift is a
        // persistent toggle here rather than meshtastic's one-shot long-press,
        // since confirm_select() is already spoken for as "press this key".
        uint8_t kb_row, kb_col;
        msngr_kb_cursor_rc(msngr_kb_cursor, kb_row, kb_col);
        char key_ch = msngr_kb_active_layout()[kb_row][kb_col];
        uint8_t key_type = msngr_kb_key_type(key_ch);

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
            char c = (key_type == MSNGR_KB_SPACE) ? ' ' : msngr_kb_apply_shift(key_ch, msngr_kb_shift_on);
            msngr_kb_insert_char(c);
          }
        } else if (key_type == MSNGR_KB_BACKSPACE) {
          if (msngr_kb_del_hold_fired_btn || msngr_kb_del_hold_fired_enc) {
            // Already deleted (at least once, maybe several times) live,
            // mid-hold (msngr_kb_del_hold_try()) - this release is just
            // the tail end of that gesture, not a fresh press, so it
            // shouldn't also delete one more character on top of it.
            msngr_kb_del_hold_fired_btn = false;
            msngr_kb_del_hold_fired_enc = false;
          } else {
            msngr_kb_do_backspace();
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
        } else if (key_type == MSNGR_KB_TYPE_TOGGLE) {
          #if HAS_LXMF == true
            msngr_kb_toggle_bookmark_type();
          #endif
        } else if (key_type == MSNGR_KB_BACK) {
          menu_msngr_text_entry_leave();
        } else if (key_type == MSNGR_KB_SEND) {
          msngr_kb_do_send();
        }
      } else if (menu_state == MENU_STATE_MSNGR_DISCARD_CONFIRM) {
        if (msngr_discard_confirm_cursor == 0) { // DISCARD
          msngr_text_entry_buf[0] = 0;
          menu_state = msngr_text_entry_return_state();
        } else { // CANCEL - resume typing, buffer/cursor/shift untouched
          menu_state = msngr_discard_confirm_return_state;
        }
      }
    #endif
    // menu_state == MENU_STATE_CLOSED + short click: no-op (reserved).
  }

  #if HAS_BLE_HID_HOST == true
    // Backspace, outside MENU_STATE_MSNGR_TEXT_ENTRY (blekbd_key_event()
    // handles that screen separately - Backspace stays character-delete
    // there), scoped to the Bluetooth/BLE Keyboard/Messenger screens - see
    // this file's own plan (purrfect-baking-candle.md) for why: this
    // firmware's menu system has no single generic "go back one level"
    // primitive, so covering every one of the ~65 MENU_STATE_* screens
    // would mean auditing all of them; this bounded, reviewable set covers
    // what a paired BLE keyboard user actually navigates through.
    //
    // Every list/status/confirm-dialog screen in this codebase puts its
    // BACK (or CANCEL) row at the *last* valid cursor index - a completely
    // consistent convention (verified directly against menu_encoder_
    // rotate()'s own per-state clamp calls, and BACK-row dispatch sites
    // like MSNGR_MSG_DETAIL's own "cursor == row_count - 1" check and
    // MSNGR_PEER_FIXED_ACTION_BACK being the last of MSNGR_PEER_FIXED_
    // ACTION_COUNT). So this never needs to know *where* Back leads, only
    // each screen's own cursor variable and row count - jump straight to
    // that last index and press it through the exact same menu_confirm_
    // select() path a real click would take, rather than duplicating each
    // screen's own destination logic (which would drift out of sync with
    // it over time).
    void blekbd_backspace_as_back() {
      // EDIT-shaped screens: any confirm_select() press already returns to
      // the parent unconditionally (e.g. MENU_STATE_BT_SETTINGS_EDIT ->
      // BT_SETTINGS regardless of the staged value) - jumping the cursor
      // first would only risk corrupting whatever field is being edited
      // (rotating/stepping an EDIT state's own value, not moving a list
      // cursor - there's no "last row" concept here at all).
      bool is_edit = (menu_state == MENU_STATE_BT_SETTINGS_EDIT ||
                      menu_state == MENU_STATE_BLEKBD_EDIT ||
                      menu_state == MENU_STATE_MSNGR_SETTINGS_EDIT);
      if (!is_edit) {
        // Jump straight to the last row rather than stepping there one
        // menu_encoder_rotate() call at a time, which would also spam a
        // tick sound per step for what should read as a single Back press.
             if (menu_state == MENU_STATE_BT_LIST)                bt_menu_cursor = BT_ITEM_COUNT - 1;
        else if (menu_state == MENU_STATE_BT_SETTINGS)            bt_settings_cursor = BT_SETTINGS_ITEM_COUNT - 1;
        else if (menu_state == MENU_STATE_BT_UNPAIR_CONFIRM)      bt_unpair_confirm_cursor = 1;
        else if (menu_state == MENU_STATE_BLEKBD_LIST)            blekbd_menu_cursor = BLEKBD_ITEM_COUNT - 1;
        else if (menu_state == MENU_STATE_BLEKBD_SCAN)            blekbd_scan_cursor = blekbd_scan_row_count() - 1;
        else if (menu_state == MENU_STATE_BLEKBD_PAIR_CONFIRM)    blekbd_pair_confirm_cursor = 1;
        else if (menu_state == MENU_STATE_BLEKBD_PAIRING)         blekbd_pairing_cursor = 1;
        else if (menu_state == MENU_STATE_BLEKBD_FORGET_CONFIRM)  blekbd_forget_confirm_cursor = 1;
        else if (menu_state == MENU_STATE_MSNGR_LIST)             msngr_menu_cursor = MSNGR_TOP_ITEM_COUNT - 1;
        else if (menu_state == MENU_STATE_MSNGR_INBOX)            msngr_inbox_cursor = msngr_inbox_row_count() - 1;
        else if (menu_state == MENU_STATE_MSNGR_BOOKMARKS)        msngr_bookmarks_cursor = msngr_bookmarks_row_count() - 1;
        else if (menu_state == MENU_STATE_MSNGR_ANNOUNCES)        msngr_announces_cursor = msngr_announces_row_count() - 1;
        else if (menu_state == MENU_STATE_MSNGR_PEER)             msngr_peer_cursor = msngr_peer_row_count() - 1;
        else if (menu_state == MENU_STATE_MSNGR_MSG_DETAIL)       msngr_msg_detail_cursor = msngr_msg_detail_row_count() - 1;
        else if (menu_state == MENU_STATE_MSNGR_DELETE_CONFIRM)   msngr_delete_confirm_cursor = 1;
        else if (menu_state == MENU_STATE_MSNGR_CLEAR_CONFIRM)    msngr_clear_confirm_cursor = 1;
        else if (menu_state == MENU_STATE_MSNGR_DISCARD_CONFIRM)  msngr_discard_confirm_cursor = 1;
        else if (menu_state == MENU_STATE_MSNGR_PING_RESULT)      msngr_ping_result_cursor = 1;
        else if (menu_state == MENU_STATE_MSNGR_SEND_RESULT)      msngr_send_result_cursor = msngr_send_result_row_count() - 1;
        else if (menu_state == MENU_STATE_MSNGR_SETTINGS)         msngr_settings_cursor = MSNGR_SETTINGS_ITEM_COUNT - 1;
        else if (menu_state == MENU_STATE_MSNGR_PRESETS)          msngr_presets_cursor = msngr_presets_row_count() - 1;
        else if (menu_state == MENU_STATE_MSNGR_PRESET_DETAIL)    msngr_preset_detail_cursor = 2;
        else return; // out of scope for Backspace-as-Back - dropped, same as every other key outside the covered screens
      }
      buzzer_encoder_click_melody();
      menu_confirm_select(0);
    }

    // Definition for the declaration in BLEKeyboardHost.h - same include-
    // order reason as blekbd_pair_result_process() (this file, above):
    // this needs menu_state/MENU_STATE_MSNGR_TEXT_ENTRY, the msngr_kb_*
    // helpers, and (for general navigation outside text entry) menu_
    // encoder_rotate()/menu_confirm_select()/menu_commit_and_exit()/menu_
    // open_from_closed()/messenger_open_from_closed() themselves - none of
    // which exist yet either at BLEKeyboardHost.h's point in the include
    // order, or (for these) this early in Menu.h itself, hence this living
    // all the way down here rather than next to blekbd_pair_result_
    // process(). Called from blekbd_loop() for every queued key, already
    // translated to a plain char, one of '\n'/'\b'/'\x1b' for Enter/
    // Backspace/Escape, BLEKBD_CH_UP/_DOWN for the arrow keys, or BLEKBD_
    // CH_OPEN_SETTINGS/_MESSENGER for WinKey/Alt+Tab.
    // Quick-open Dialog Mode shortcut (Ctrl+A or F2, blekbd_key_event(),
    // below) - per user request, jumps straight into MENU_STATE_MSNGR_
    // CHAT for whichever contact is currently relevant, from three
    // different starting screens:
    //   - MENU_STATE_MSNGR_INBOX, with a conversation row selected
    //     (msngr_inbox_cursor) - resolves the peer hash exactly the way
    //     that screen's own Enter/confirm handler does (menu_confirm_
    //     select(), above), just skipping the intermediate MENU_STATE_
    //     MSNGR_PEER screen entirely instead of landing there.
    //   - MENU_STATE_MSNGR_BOOKMARKS, with a contact row selected
    //     (msngr_bookmarks_cursor) - same idea, mirrors that screen's
    //     own confirm handler.
    //   - MENU_STATE_MSNGR_PEER itself ("within the Inbox message
    //     list") - msngr_active_peer_hash is already correct, nothing
    //     to resolve.
    // Silently does nothing from any other screen, or from INBOX/
    // BOOKMARKS with the cursor sitting on a non-contact row (BACK, Add
    // by Hash, ...) - same "not applicable here" restraint the other
    // keyboard shortcuts in this file already use rather than acting on
    // a row that isn't actually a contact.
    void blekbd_quick_open_dialog_mode() {
      RNS::Bytes peer_hash;
      bool have_peer = false;

      if (menu_state == MENU_STATE_MSNGR_PEER) {
        peer_hash = msngr_active_peer_hash;
        have_peer = true;
      } else if (menu_state == MENU_STATE_MSNGR_INBOX) {
        uint8_t row_count = msngr_inbox_row_count();
        if (msngr_inbox_cursor < row_count - 1 && urns_message_store && urns_message_store->get_conversation_count() > 0) {
          std::vector<RNS::Bytes> convs = urns_message_store->get_conversations();
          if (msngr_inbox_cursor < convs.size()) {
            peer_hash = convs[msngr_inbox_cursor];
            urns_message_store->mark_conversation_read(peer_hash);
            msngr_peer_return_state = MENU_STATE_MSNGR_INBOX;
            have_peer = true;
          }
        }
      } else if (menu_state == MENU_STATE_MSNGR_BOOKMARKS) {
        uint8_t row_count = msngr_bookmarks_row_count();
        bool has_add_row = msngr_bookmark_count < MSNGR_MAX_BOOKMARKS;
        bool on_contact_row = msngr_bookmarks_cursor < row_count - 1 &&
          !(has_add_row && msngr_bookmarks_cursor == msngr_bookmark_count) &&
          msngr_bookmark_count > 0;
        if (on_contact_row) {
          uint8_t vis = 0;
          for (uint8_t i = 0; i < MSNGR_MAX_BOOKMARKS; i++) {
            if (!msngr_bookmarks[i].in_use) continue;
            if (vis == msngr_bookmarks_cursor) {
              peer_hash = RNS::Bytes(msngr_bookmarks[i].hash, LXMF::PEER_HASH_SIZE);
              msngr_peer_return_state = MENU_STATE_MSNGR_BOOKMARKS;
              have_peer = true;
              break;
            }
            vis++;
          }
        }
      }

      if (!have_peer) return;

      msngr_active_peer_hash = peer_hash;
      msngr_peer_cursor = 0;
      msngr_last_send_result = 0xFF;
      msngr_text_entry_purpose = MSNGR_TEXT_ENTRY_PURPOSE_MESSAGE;
      msngr_text_entry_buf[0] = 0;
      msngr_chat_cursor = 0;
      msngr_chat_compose_win_start = 0;
      msngr_chat_sel = 0xFF;
      msngr_chat_delete_confirm_pending = false;
      msngr_msg_view_active = false;
      messenger_refresh_chat_window(msngr_active_peer_hash, (size_t)-1);
      menu_state = MENU_STATE_MSNGR_CHAT;
      buzzer_encoder_click_melody(); // same confirm sound a real Enter-to-select would play
    }

    // True for "value select" submenus - screens where menu_encoder_
    // rotate()'s dir argument steps a single field's value (its *_EDIT
    // branches, above) - as opposed to a plain list/multi-line screen
    // where dir moves a cursor between several rows. Per user request:
    // the BLE keyboard's Left/Right cursor keys should only act here,
    // and Up/Down only in the list-shaped states - a small keyboard's
    // four arrow keys would otherwise do the exact same thing twice,
    // which is confusing rather than useful.
    //
    // Matches every MENU_STATE_* whose own name contains "EDIT" - the
    // dispatch in menu_encoder_rotate() bears out that this naming is
    // completely consistent: every state listed there under an "_EDIT"
    // name steps ONE field's value with dir, no exceptions. (MENU_STATE_
    // MSNGR_TEXT_ENTRY is deliberately not listed here despite being a
    // single on-screen-keyboard cursor - it never reaches this check,
    // blekbd_key_event() below has its own dedicated block for it that
    // always returns first.)
    bool blekbd_menu_state_is_value_edit() {
      return menu_state == MENU_STATE_EDIT ||
             menu_state == MENU_STATE_WIFI_EDIT ||
             menu_state == MENU_STATE_HW_EDIT ||
             menu_state == MENU_STATE_GPIO_PIN_EDIT ||
             menu_state == MENU_STATE_ETH_EDIT ||
             menu_state == MENU_STATE_ETH_ADDR_EDIT ||
             menu_state == MENU_STATE_WIFI_ADDR_EDIT ||
             menu_state == MENU_STATE_RTC_EDIT ||
             menu_state == MENU_STATE_RTC_TZ_EDIT ||
             menu_state == MENU_STATE_GNSS_EDIT ||
             menu_state == MENU_STATE_ESPNOW_EDIT ||
             menu_state == MENU_STATE_URNS_EDIT ||
             menu_state == MENU_STATE_URNS_RADIO_EDIT ||
             menu_state == MENU_STATE_MSNGR_SETTINGS_EDIT ||
             menu_state == MENU_STATE_BT_SETTINGS_EDIT
             #if HAS_BLE_HID_HOST == true
               || menu_state == MENU_STATE_BLEKBD_EDIT
             #endif
             ;
    }

    void blekbd_key_event(char ch) {
      // Same activity-refresh/display-wake every other real input path
      // already does at its own top (menu_encoder_rotate(), menu_confirm_
      // select() via menu_encoder_button()/menu_button_press()) - BLE
      // keystrokes bypass all of those entirely (this function is the
      // physical keyboard's own dispatch, called from blekbd_loop()), so
      // without this, typing never reset the idle-close clock or woke a
      // blanked/dimmed screen - confirmed on hardware as a real gap, not
      // just a Chat-specific one.
      menu_last_activity_ms = millis();
      display_unblank();

      // Global shortcuts - reachable from any state, including
      // MENU_STATE_CLOSED (that's the whole point - WinKey/Alt+Tab open
      // something from nothing else being open), EXCEPT while actively
      // composing/editing text (MENU_STATE_MSNGR_TEXT_ENTRY, or MENU_STATE_
      // MSNGR_CHAT once HAS_BLE_HID_HOST exists): menu_open_from_closed()/
      // messenger_open_from_closed() are unconditional state jumps (same as
      // the physical long-press-from-closed/BUTTON_HOLD_TIER_MESSENGER hold
      // gestures they mirror) with no discard-confirm involved, so an
      // accidental hit while typing would silently lose unsent text.
      bool blekbd_composing =
        #if HAS_BLE_HID_HOST == true
          menu_state == MENU_STATE_MSNGR_TEXT_ENTRY || menu_state == MENU_STATE_MSNGR_CHAT;
        #else
          menu_state == MENU_STATE_MSNGR_TEXT_ENTRY;
        #endif
      if (!blekbd_composing) {
        // Toggle, not just open: pressed again while the menu is already
        // open (Settings, Messenger, or anywhere else in the tree - both
        // roots share the same menu_state), it closes back to the main
        // screen the same way Escape already does (menu_commit_and_exit(),
        // see its own use for '\x1b' further down) rather than jumping back
        // to the Settings top list a second time.
        if (ch == BLEKBD_CH_OPEN_SETTINGS) {
          if (menu_state == MENU_STATE_CLOSED) menu_open_from_closed();
          else {
            // Per user request: the lower-pitched rotate/tick sound, not
            // the confirm-click one - closing via a keyboard shortcut
            // shouldn't sound like a confirm/select action, just an
            // audible cue that something happened.
            buzzer_encoder_tick_melody();
            menu_commit_and_exit();
          }
          return;
        }
        #if HAS_LXMF == true
          if (ch == BLEKBD_CH_OPEN_MESSENGER) {
            if (menu_state == MENU_STATE_CLOSED) messenger_open_from_closed();
            else {
              buzzer_encoder_tick_melody();
              menu_commit_and_exit();
            }
            return;
          }
        #endif
      }

      // Brightness keys (Consumer Control usage page, BLEKeyboardHost.h) -
      // a real hardware-button-style shortcut, so always active regardless
      // of composing/menu state (including MENU_STATE_CLOSED, unlike
      // everything below this point) - no text-loss risk the way WinKey/
      // Alt+Tab's own composing exclusion above guards against.
      if (ch == BLEKBD_CH_BRIGHTNESS_UP) { blekbd_step_display_brightness(1); return; }
      if (ch == BLEKBD_CH_BRIGHTNESS_DOWN) { blekbd_step_display_brightness(-1); return; }
      #if HAS_BUZZER == true
        // Volume keys (Consumer Control usage page) - repurposed to
        // enable/disable the buzzer, same always-active shortcut shape as
        // Brightness above.
        if (ch == BLEKBD_CH_SOUND_ENABLE) { blekbd_set_sound_enabled(true); return; }
        if (ch == BLEKBD_CH_SOUND_DISABLE) { blekbd_set_sound_enabled(false); return; }
      #endif
      // Play/Pause key - toggles marquee scrolling, same always-active
      // shortcut shape as Brightness/Volume above.
      if (ch == BLEKBD_CH_TOGGLE_MARQUEE) { blekbd_toggle_marquee(); return; }
      // Quick-open Dialog Mode (Ctrl+A/F2) - only actually does anything
      // from MSNGR_INBOX/_PEER/_BOOKMARKS with a real contact resolvable
      // (see blekbd_quick_open_dialog_mode()'s own comment); silently
      // no-ops everywhere else, same restraint as every other screen-
      // specific shortcut in this file.
      if (ch == BLEKBD_CH_OPEN_DIALOG_MODE) { blekbd_quick_open_dialog_mode(); return; }

      if (menu_state == MENU_STATE_CLOSED) return; // nothing else to navigate/close

      if (menu_state == MENU_STATE_MSNGR_TEXT_ENTRY) {
        // Enter still Sends, Backspace still deletes a character (not
        // Back - Escape is the only physical-keyboard way to leave this
        // screen, confirmed). Alt+Shift/Ctrl+Shift toggle EN/RU - only
        // meaningful here (msngr_kb_lang_ru only affects this screen's own
        // character mapping/rendering) - mirrors exactly what a deliberate
        // hold on the on-screen keyboard's own Shift key already does
        // (msngr_kb_lang_hold_try(), above). Also mirrors the new value
        // into blekbd_lang_ru (BLEKeyboardHost.h) - that's the flag the
        // physical keyboard's own keycode->character resolution actually
        // reads (blekbd_translate_report()), since msngr_kb_lang_ru itself
        // isn't visible yet at that file's point in the include order.
        //
        // Up/Down/Left/Right: per user request, real 2D grid navigation
        // (msngr_kb_nav_row()/_col(), above) rather than aliasing the
        // single linear-cursor step menu_encoder_rotate() uses for a real
        // encoder/the single button - see those functions' own comments
        // for why the linear step alone isn't "proper" navigation,
        // especially for MSNGR_KB_LAYOUT_HEX's tab order.
        bool hex_mode = (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH);
        // Identity restore has its own restricted alphabet (Base32: A-Z/
        // 2-7/=), not hex's 0-9/A-F - a separate flag rather than folding
        // it into hex_mode, since the actual accepted character set (and
        // the Type-cell Tab/Space handling just below, which only ever
        // meant anything for BOOKMARK_HASH's bookmark-type toggle) differ.
        bool base32_mode = (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_IDENTITY_RESTORE);
        if (ch == '\n') msngr_kb_do_send();
        else if (ch == '\b') msngr_kb_do_backspace();
        else if (ch == '\x1b') menu_msngr_text_entry_leave();
        else if (ch == BLEKBD_CH_UP)    { buzzer_encoder_tick_melody(); msngr_kb_nav_row(-1); }
        else if (ch == BLEKBD_CH_DOWN)  { buzzer_encoder_tick_melody(); msngr_kb_nav_row(1); }
        else if (ch == BLEKBD_CH_LEFT)  { buzzer_encoder_tick_melody(); msngr_kb_nav_col(-1); }
        else if (ch == BLEKBD_CH_RIGHT) { buzzer_encoder_tick_melody(); msngr_kb_nav_col(1); }
        else if (ch == BLEKBD_CH_TOGGLE_LANG) {
          msngr_kb_lang_ru = !msngr_kb_lang_ru;
          blekbd_lang_ru = msngr_kb_lang_ru;
          msngr_kb_shift_on = false;
          buzzer_encoder_tick_melody();
        }
        else if (hex_mode && ch == '\t') {
          // Tab - per user request, toggles LXMF/Propagation from
          // anywhere on this screen, regardless of cursor position
          // (BLEKeyboardHost.h pushes plain Tab as literal '\t' - see its
          // own comment on why no new BLEKBD_CH_* sentinel was needed).
          buzzer_encoder_tick_melody();
          msngr_kb_toggle_bookmark_type();
        }
        else if (hex_mode && ch == ' ') {
          // Spacebar - per user request, only meaningful while the
          // highlight is actually on the Type cell (mirrors "pressing"
          // it, same as Enter/confirm would via menu_confirm_select());
          // otherwise silently ignored rather than inserted, same as any
          // other non-hex-digit character below - space was never a
          // valid hash character anyway, this is just an explicit,
          // documented case of that same rule.
          uint8_t row, col;
          msngr_kb_cursor_rc(msngr_kb_cursor, row, col);
          if (msngr_kb_key_type(msngr_kb_active_layout()[row][col]) == MSNGR_KB_TYPE_TOGGLE) {
            buzzer_encoder_tick_melody();
            msngr_kb_toggle_bookmark_type();
          }
        }
        else if (hex_mode && ch != 0) {
          // Per user request: only 0-9/A-F are valid hash characters - no
          // letters past F, no symbols, no space (handled separately,
          // above). Lowercase a-f normalized to uppercase, matching
          // MSNGR_KB_LAYOUT_HEX's own row 1 ('A'-'F', never lowercase) -
          // anything else is silently dropped rather than inserted.
          char up = (ch >= 'a' && ch <= 'f') ? (char)(ch - 'a' + 'A') : ch;
          if ((up >= '0' && up <= '9') || (up >= 'A' && up <= 'F')) msngr_kb_insert_char(up);
        }
        else if (base32_mode && ch != 0) {
          // Base32 (RFC 4648): only A-Z/2-7/= are valid - no 0/1/8/9 (not
          // in the alphabet, see identity_key_from_base32()'s own
          // comment), no other symbols. Lowercase a-z normalized to
          // uppercase, matching MSNGR_KB_LAYOUT_BASE32's own cells (never
          // lowercase) - anything else silently dropped rather than
          // inserted, same rule hex_mode's own filter documents.
          char up = (ch >= 'a' && ch <= 'z') ? (char)(ch - 'a' + 'A') : ch;
          if ((up >= 'A' && up <= 'Z') || (up >= '2' && up <= '7') || up == '=') msngr_kb_insert_char(up);
        }
        else if (!hex_mode && !base32_mode && ch != 0) msngr_kb_insert_char(ch);
        return;
      }

      #if HAS_BLE_HID_HOST == true
        if (menu_state == MENU_STATE_MSNGR_CHAT) {
          // Two focuses share this one screen/menu_state (no separate
          // sub-state - see msngr_chat_sel's own declaration, Messenger.h):
          // composing (the default - Left/Right move the text cursor,
          // Backspace/typing edit msngr_text_entry_buf in place) and
          // browsing the message history (Up/Down/PgUp/PgDn/Home/End move
          // a selected row, msngr_chat_sel != 0xFF - Backspace there opens
          // the DELETE MESSAGE? mini-dialog below instead). Typing/Left/
          // Right always return focus to composing first - the most
          // natural, least-surprising way back given none of Up/Down/PgUp/
          // PgDn/Home/End were ever meaningful in Chat before browsing
          // existed.
          //
          // The delete-confirm mini-dialog takes over Enter/Esc/Backspace
          // exclusively while pending - checked first, before either focus
          // gets a turn.
          if (msngr_chat_delete_confirm_pending) {
            // Counts as browsing activity too - resets the same idle
            // timeout the selection itself uses (MSNGR_CHAT_SEL_TIMEOUT_MS,
            // above), so a slow decision here doesn't get preempted out
            // from under the user mid-dialog.
            msngr_chat_sel_last_activity_ms = millis();
            // Enter or Y confirms, Backspace/Esc/N cancels - per user
            // request, Y/N work as plain shortcuts alongside the original
            // Enter/Backspace-or-Esc bindings. Case-insensitive since
            // Shift/Caps Lock both flip which case actually arrives here
            // (blekbd_translate_report(), BLEKeyboardHost.h).
            if (ch == '\n' || ch == 'y' || ch == 'Y') {
              msngr_chat_delete_confirm_pending = false;
              if (msngr_chat_sel != 0xFF && msngr_chat_sel < msngr_chat_cache_count) {
                RNS::Bytes msg_hash(msngr_chat_cache[msngr_chat_sel].hash, LXMF::MESSAGE_HASH_SIZE);
                // Masked for the whole delete+refresh sequence - both do
                // LittleFS I/O, same real, confirmed DIO0-ISR-vs-flash-I/O
                // crash hazard as MENU_STATE_MSNGR_DELETE_CONFIRM's own
                // DELETE branch (above) - see that branch's own comment.
                LoRa->maskDio0();
                if (urns_message_store) urns_message_store->delete_message(msg_hash);
                messenger_refresh_chat_window(msngr_active_peer_hash, msngr_chat_window_start);
                LoRa->unmaskDio0();
                msngr_chat_sel = (msngr_chat_cache_count > 0)
                  ? (msngr_chat_sel < msngr_chat_cache_count ? msngr_chat_sel : (uint8_t)(msngr_chat_cache_count - 1))
                  : (uint8_t)0xFF;
                // The message just deleted is exactly the one the full-
                // screen view (if open) was showing - nothing left to view.
                msngr_msg_view_active = false;
              }
            } else if (ch == '\b' || ch == '\x1b' || ch == 'n' || ch == 'N') {
              msngr_chat_delete_confirm_pending = false; // CANCEL
            }
            return;
          }

          if (msngr_msg_view_active) {
            // Full-screen, ornament-free single-message view (opened by
            // Enter while browsing, below) - per user request, no text-
            // entry box exists in this sub-mode at all, so Up/Down are
            // repurposed as pure line-scroll here instead of message-list
            // navigation, and everything else (Left/Right/typing/Home/End/
            // PgUp/PgDn) is silently ignored rather than falling through
            // to compose-box or message-list handling that doesn't apply.
            // Same shared msngr_msg_view_active this view also uses when
            // opened from MENU_STATE_MSNGR_MSG_DETAIL (menu_confirm_
            // select(), its own comment) - msngr_msg_view_return_state is
            // already MENU_STATE_MSNGR_CHAT here (set below, where this
            // view is opened), so exiting just lands back in Chat as
            // before.
            if (ch == BLEKBD_CH_UP) {
              if (msngr_msg_view_scroll_line > 0) msngr_msg_view_scroll_line--;
            } else if (ch == BLEKBD_CH_DOWN) {
              msngr_msg_view_scroll_line++; // clamped against real line count in the draw block, below
            } else if (ch == '\x1b') {
              // Esc or AC Home (BLEKeyboardHost.h pushes AC Home as a
              // literal '\x1b' too, so this already covers both per user
              // request with no extra dispatch needed).
              msngr_msg_view_active = false;
            } else if (ch == '\b') {
              msngr_chat_delete_confirm_pending = true; // same DELETE MESSAGE? dialog as browsing uses
            }
            return;
          }

          if (ch == BLEKBD_CH_UP) { msngr_chat_nav_up(); return; }
          if (ch == BLEKBD_CH_DOWN) { msngr_chat_nav_down(); return; }
          if (ch == BLEKBD_CH_PAGE_UP) { msngr_chat_nav_page_up(); return; }
          if (ch == BLEKBD_CH_PAGE_DOWN) { msngr_chat_nav_page_down(); return; }
          if (ch == BLEKBD_CH_HOME) { msngr_chat_nav_home(); return; }
          if (ch == BLEKBD_CH_END) { msngr_chat_nav_end(); return; }

          if (msngr_chat_sel != 0xFF) {
            // Browsing a selected message - Enter opens the full-screen
            // view above, Backspace opens the delete confirm, Esc
            // deselects (back to composing). Per user request, typing any
            // real character (space/letter/digit/symbol - anything that
            // would otherwise insert into the compose box) does the exact
            // same thing as Esc: just deselects, without also inserting -
            // same condition set the composing branch's own final insert-
            // eligibility check uses, minus the keys already handled here/
            // above (Left/Right/lang-toggle stay silently ignored while
            // browsing, unchanged).
            if (ch == '\b') msngr_chat_delete_confirm_pending = true;
            else if (ch == '\x1b') msngr_chat_exit_browsing();
            else if (ch == '\n') {
              // Reuses the exact same full-content fetch MENU_STATE_MSNGR_
              // MSG_DETAIL uses (messenger_refresh_msg_detail_cache(),
              // Messenger.h) rather than msngr_chat_cache[]'s own capped-
              // length snippet - the whole point is showing more than that
              // snippet ever could.
              RNS::Bytes msg_hash(msngr_chat_cache[msngr_chat_sel].hash, LXMF::MESSAGE_HASH_SIZE);
              messenger_refresh_msg_detail_cache(msg_hash);
              msngr_msg_view_active = true;
              msngr_msg_view_scroll_line = 0;
              msngr_msg_view_return_state = MENU_STATE_MSNGR_CHAT;
            }
            else if (ch != 0 && ch != BLEKBD_CH_LEFT && ch != BLEKBD_CH_RIGHT &&
                     ch != BLEKBD_CH_TOGGLE_LANG && ch != BLEKBD_CH_OPEN_MESSENGER && ch != BLEKBD_CH_OPEN_SETTINGS) {
              msngr_chat_exit_browsing();
            }
            return;
          }

          // Composing (the default/normal focus) - same shape as before
          // browsing existed. Backspace/insert use the cursor-aware msngr_
          // chat_* functions (above), not the on-screen keyboard's own
          // append/trim-the-tail-only msngr_kb_* ones - real in-place
          // editing (Left/Right move the cursor, typing/Backspace act
          // wherever it currently is), per user request.
          if (ch == '\n') msngr_chat_do_send();
          else if (ch == '\b') msngr_chat_backspace();
          else if (ch == '\x1b') menu_msngr_text_entry_leave();
          else if (ch == BLEKBD_CH_LEFT) msngr_chat_cursor_left();
          else if (ch == BLEKBD_CH_RIGHT) msngr_chat_cursor_right();
          else if (ch == BLEKBD_CH_TOGGLE_LANG) {
            msngr_kb_lang_ru = !msngr_kb_lang_ru;
            blekbd_lang_ru = msngr_kb_lang_ru;
            buzzer_encoder_tick_melody();
          }
          else if (ch != 0 && ch != BLEKBD_CH_OPEN_MESSENGER && ch != BLEKBD_CH_OPEN_SETTINGS) {
            msngr_chat_insert_char(ch);
          }
          return;
        }
      #endif

      // Every other open screen: Up/Down = encoder rotate, Enter = confirm/
      // select, Esc = close the whole menu - general, not scoped to
      // Bluetooth/Messenger, since this just reuses the exact same trusted
      // per-state dispatch the real encoder/a real long-press already goes
      // through (menu_encoder_rotate(), Encoder.h:153; menu_commit_and_
      // exit(), the same action a physical long-press takes from any state
      // - this codebase has no separate "discard and close" mechanic to
      // reach for instead) rather than any new per-state code. wrap=false
      // matches the real encoder's own default (stop at the ends) rather
      // than the button-only board's double-tap wrap=true convention.
      #if HAS_LXMF == true
        // Full Message view, opened from MENU_STATE_MSNGR_MSG_DETAIL (the
        // only way to reach this general fallback while msngr_msg_view_
        // active is true - MENU_STATE_MSNGR_CHAT's own dedicated block,
        // above, always returns before ever reaching here). Esc here
        // would otherwise fall into the branch just below and close the
        // WHOLE menu (menu_commit_and_exit()) - it should only back out
        // of the view instead, same as every other exit path
        // (menu_confirm_select()'s own matching check).
        if (msngr_msg_view_active && ch == '\x1b') {
          buzzer_encoder_tick_melody();
          msngr_msg_view_active = false;
          menu_state = msngr_msg_view_return_state;
          return;
        }
      #endif
      if (ch == '\x1b') {
        // Per user request: the lower-pitched rotate/tick sound, not the
        // confirm-click one - closing via a keyboard shortcut shouldn't
        // sound like a confirm/select action, just an audible cue that
        // something happened.
        buzzer_encoder_tick_melody();
        menu_commit_and_exit();
        return;
      }
      if (ch == BLEKBD_CH_UP || ch == BLEKBD_CH_DOWN || ch == BLEKBD_CH_LEFT || ch == BLEKBD_CH_RIGHT) {
        // Per user request, on a small keyboard Up/Down and Left/Right
        // shouldn't both do the same thing: Up/Down only navigate plain
        // list/multi-line screens, Left/Right only step a value in
        // "value select" submenus (blekbd_menu_state_is_value_edit(),
        // above) - a mismatched key (e.g. Left on a list screen) is a
        // silent no-op rather than falling back to acting like the other
        // pair. menu_encoder_rotate() itself still branches internally
        // on menu_state exactly like a real encoder turn would.
        bool is_lr = (ch == BLEKBD_CH_LEFT || ch == BLEKBD_CH_RIGHT);
        if (is_lr != blekbd_menu_state_is_value_edit()) return;
        int8_t dir = (ch == BLEKBD_CH_DOWN || ch == BLEKBD_CH_RIGHT) ? 1 : -1;
        menu_encoder_rotate(dir, false);
        return;
      }
      if (ch == '\n') {
        // Mirrors exactly what menu_encoder_button() does for a short
        // click - the buzzer click isn't inside menu_confirm_select()
        // itself, its callers each play it first.
        buzzer_encoder_click_melody();
        menu_confirm_select(0);
        return;
      }
      if (ch == '\b') blekbd_backspace_as_back();
    }
  #endif

  // The main button drives the menu everywhere. On boards without an
  // encoder (HAS_MENU without HAS_ENCODER - see Boards.h) this is the only
  // control; on encoder boards it's an alternate one. Short press cycles
  // forward
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
      #if HAS_LXMF == true || HAS_WIFI == true
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
    #if HAS_LXMF == true || HAS_WIFI == true
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
  // right_dx is icon_dx's mirror for the right-aligned value column - a
  // per-row nudge (px, positive = toward the screen edge) applied to both
  // right_icons[i] and its valbufs[i] text together, since they move as a
  // unit (val_right already accounts for the icon's reserved width).
  // Default nullptr/0, same opt-in pattern as icon_dx/text_dx.
  // separator_before: row index a dashed divider sits directly above (e.g.
  // MENU_STATE_MSNGR_PEER's boundary between message rows and the Compose
  // message/Ping/.../BACK action rows below them) - default -1 draws
  // nothing, same opt-in pattern as icons/right_icons. Only drawn when
  // both the row it sits above and the row before it are in the current
  // scroll window - if the boundary itself has scrolled out of view
  // there's nothing to visually divide.
  void draw_menu_list_disp(const char *title, const char **labels, char valbufs[][24], uint8_t count, uint8_t cursor, const uint8_t **icons = nullptr, const uint8_t *icon_widths = nullptr, const int8_t *icon_dx = nullptr, bool icon_col_shared = true, const int8_t *text_dx = nullptr, const uint8_t **right_icons = nullptr, const uint8_t *right_icon_widths = nullptr, const int8_t *right_dx = nullptr, int16_t separator_before = -1, const char *footer_override = nullptr) {
    MENU_GFX.setFont(MENU_FONT);
    MENU_GFX.setTextSize(1);
    MENU_GFX.setTextColor(SSD1306_WHITE);
    MENU_GFX.setCursor(6, MENU_HEADER_TEXT_Y);
    MENU_GFX.print(title);
    MENU_GFX.drawFastHLine(MENU_CONTENT_X, MENU_HEADER_HLINE_Y, MENU_CONTENT_W, SSD1306_WHITE);

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
        MENU_GFX.fillRect(MENU_CONTENT_X, row_top, MENU_CONTENT_W, row_h - 1, SSD1306_WHITE);
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
        uint8_t val_right = MENU_CONTENT_X + MENU_CONTENT_W - 2;
        // MENU_STATE_MSNGR_PEER's incoming messages (Menu.h) - reserve room
        // for a trailing direction icon by pulling the value's right edge
        // in first, same "shrink the text side, not the icon" idiom the
        // left-hand icon column above uses.
        if (right_icons && right_icons[i]) val_right -= (right_icon_widths[i] + MENU_ROW_ICON_GAP);
        if (right_dx) val_right += right_dx[i];
        MENU_GFX.setCursor(val_right - w, y);
        MENU_GFX.print(valbufs[i]);
        if (right_icons && right_icons[i]) {
          int16_t icon_y = row_top + (row_h - MENU_ICON_H) / 2 + MENU_ICON_Y_NUDGE;
          uint16_t fg = selected ? SSD1306_BLACK : SSD1306_WHITE;
          uint16_t bg = selected ? SSD1306_WHITE : SSD1306_BLACK;
          int16_t icon_x2 = MENU_CONTENT_X + MENU_CONTENT_W - 2 - right_icon_widths[i] + (right_dx ? right_dx[i] : 0);
          MENU_GFX.drawBitmap(icon_x2, icon_y, right_icons[i], right_icon_widths[i], MENU_ICON_H, fg, bg);
        }
      }
    }

    // Dashed divider - 1px blank, 1px dashed line, 1px blank (3px total),
    // sitting in the boundary between two adjacent rows rather than
    // stealing a row height of its own, per user request. Same dash/gap
    // idiom as any other 1bpp OLED UI's dashed rule, just hand-drawn since
    // Adafruit_GFX has no dashed-line primitive.
    if (separator_before >= 0 && separator_before > (int16_t)first && separator_before < (int16_t)(first + visible_rows)) {
      uint8_t sep_vi = (uint8_t)(separator_before - first);
      int16_t sep_y = MENU_LIST_TOP_Y + sep_vi * row_h - 2;
      for (int16_t dx = 0; dx < MENU_CONTENT_W; dx += 4) {
        int16_t seg_w = (MENU_CONTENT_W - dx) < 2 ? (MENU_CONTENT_W - dx) : 2;
        MENU_GFX.drawFastHLine(MENU_CONTENT_X + dx, sep_y, seg_w, SSD1306_WHITE);
      }
    }

    MENU_GFX.setTextColor(SSD1306_WHITE);
    MENU_GFX.drawFastHLine(MENU_CONTENT_X, MENU_LIST_FOOTER_HLINE_Y, MENU_CONTENT_W, SSD1306_WHITE);
    MENU_GFX.setCursor(6, MENU_LIST_FOOTER_TEXT_Y);
    // footer_override lets a specific caller replace the usual turn/tap
    // navigation hint outright with its own text - takes priority over
    // everything below.
    bool footer_drawn = false;
    if (footer_override) {
      MENU_GFX.print(footer_override);
      footer_drawn = true;
    }
    #if HAS_LXMF == true
      // Persistent, screen-agnostic reminder that LXMRouter's single-in-
      // flight outbound queue (process_outbound()'s own comment,
      // LXMRouter.cpp) is still occupied - a Messenger send silently
      // retrying in the background (LXMRouter.cpp's static_proof_timeout_
      // callback()) long after whatever screen started it either resolved
      // or was backed out of, with zero other on-screen indication
      // otherwise. Deliberately keyed off "is the queue busy at all", not
      // "is it MY message" - the previous per-screen version of this only
      // recognized its own tracked send, so starting a second send (or
      // just backing out and doing anything else) while the first was
      // still retrying made that background activity invisible again.
      // Applies to every screen through this shared renderer; the
      // handful of screens with their own separate footer-drawing code
      // (keyboard entry, in-place value editors, the memory screen) are
      // deliberately left alone - overwriting their own input hint mid-
      // edit would be actively confusing, not helpful.
      if (!footer_drawn && urns_lxmf_router && urns_lxmf_router->pending_outbound_count() > 0) {
        // Wider than the 24-byte convention used elsewhere on this screen -
        // "Awaiting Proof 48s (2/5)" alone is 24 visible chars, would
        // already be truncated by a 24-byte buffer (23 chars + null).
        char buf[32];
        int attempts = urns_lxmf_router->pending_outbound_front_attempts();
        double next_in = urns_lxmf_router->pending_outbound_front_next_action_in();
        bool has_retried = attempts > 1;
        // Round up, not down - next_in ticks continuously, and truncating
        // (the previous (unsigned)next_in) made the displayed countdown
        // run 9s->0s instead of 10s->1s, i.e. it visibly hit 0 for however
        // long remains between the deadline actually passing and this
        // screen's next redraw. Since any remaining time above 0 is still
        // genuinely "not yet", round up so the lowest number ever shown is
        // 1s - the countdown reaching 0 would wrongly read as "due now"
        // when it's actually already past due by definition of next_in
        // (pending_outbound_front_next_action_in()'s own >0.0 ? ... : 0.0).
        unsigned next_in_secs = (unsigned)next_in;
        if (next_in > next_in_secs) next_in_secs++;
        // Per user request: don't say "Retry" on a first attempt that
        // hasn't even failed yet (was reading "Retry 1/5" the instant a
        // message was first transmitted, before any real retry happened),
        // and label the two genuinely different waits distinctly instead
        // of one bare countdown that silently resets with no explanation -
        // SENT means still awaiting delivery proof (Reticulum's own
        // auto-computed first-hop timeout, Transport::first_hop_timeout());
        // anything else queued here (OUTBOUND/SENDING) polls again in
        // _outbound_retry_delay seconds (msngr_retry_delay_s, RNode
        // Settings > Messenger > Retry Delay), but for two conceptually
        // different reasons that shouldn't share a label:
        // - "Delay" (attempts > 0): a real attempt already genuinely
        //   failed and this is the backoff before retrying - true for
        //   OPPORTUNISTIC/DIRECT's own backoff, and for a PROPAGATED
        //   PropagatedOutcome::FAILED (LXMRouter.h's own comment).
        // - "Resolving" (attempts == 0): PROPAGATED still waiting on a
        //   precondition (propagation node/path/identity/link) - bound via
        //   resolution_attempts() instead, not delivery_attempts(), so
        //   nothing has actually failed yet even though the same ~10s poll
        //   cadence applies. Labeling this "Delay" too read as "something
        //   failed and is retrying" when nothing had - confirmed on
        //   hardware, this phase can legitimately poll a few times while
        //   the link comes up. Per user request, name the actual pending
        //   precondition instead of a single generic "Resolving" -
        //   mirrors send_propagated()'s own check order exactly
        //   (LXMRouter.cpp) so this never guesses: no propagation node
        //   configured at all, no known path to it yet, its identity not
        //   announced yet, or a link to it already created but not yet
        //   ACTIVE (is_outbound_propagation_link_establishing(), the same
        //   check the primary status line above already uses for its own
        //   "Establishing Link" text).
        if (urns_lxmf_router->pending_outbound_front_state() == LXMF::Type::Message::SENT) {
          if (next_in >= 0.0) {
            if (has_retried) snprintf(buf, sizeof(buf), "Awaiting Proof %us (%d/%u)", next_in_secs, attempts, (unsigned)msngr_max_retries);
            else              snprintf(buf, sizeof(buf), "Awaiting Proof %us", next_in_secs);
          } else {
            snprintf(buf, sizeof(buf), "Awaiting Proof");
          }
        } else if (next_in >= 0.0) {
          if (attempts > 0) {
            snprintf(buf, sizeof(buf), "Delay %us (%d/%u)", next_in_secs, attempts, (unsigned)msngr_max_retries);
          } else if (urns_lxmf_router->pending_outbound_front_stamp_running()) {
            // Checked ahead of the resolving-reason lookup below - stamp
            // generation only ever starts once the link to the
            // propagation node is already ACTIVE, so none of that lookup's
            // precondition checks would apply anyway, and it'd otherwise
            // fall through to the generic "Resolving" catch-all - see
            // pending_outbound_front_stamp_running()'s own comment,
            // LXMRouter.h, confirmed on hardware. No countdown here on
            // purpose - next_in is just this poll's ~10s check-in
            // interval, not an ETA for the grind itself (LXStamper's own
            // worst case is ~2 minutes, send_propagated()'s comment,
            // LXMRouter.cpp), so showing it would misleadingly imply the
            // stamp is about to land.
            snprintf(buf, sizeof(buf), "Generating Stamp...");
          } else if (urns_lxmf_router->pending_outbound_front_stamp_done()) {
            // The grind itself finished (is_propagation_stamp_running()
            // already false) but LXStamper's own state machine
            // (LXStamper.cpp) only returns to IDLE once send_propagated()
            // actually consumes the result on its own next ~10s poll - see
            // pending_outbound_front_stamp_done()'s own comment,
            // LXMRouter.h. Real, observable gap confirmed on hardware, not
            // a guess.
            snprintf(buf, sizeof(buf), "Stamp Ready...");
          } else {
            RNS::Bytes prop_node = urns_lxmf_router->get_outbound_propagation_node();
            if (prop_node.size() == 0) {
              snprintf(buf, sizeof(buf), "No Prop Node %us", next_in_secs);
            } else if (!RNS::Transport::has_path(prop_node)) {
              snprintf(buf, sizeof(buf), "No Path %us", next_in_secs);
            } else if (!RNS::Identity::recall(prop_node)) {
              snprintf(buf, sizeof(buf), "No Announce %us", next_in_secs);
            } else if (urns_lxmf_router->is_outbound_propagation_link_establishing()) {
              snprintf(buf, sizeof(buf), "Establishing Link %us", next_in_secs);
            } else if (urns_lxmf_router->is_outbound_propagation_link_stale()) {
              // A reused link from an earlier send that's gone idle - see
              // is_outbound_propagation_link_stale()'s own comment,
              // LXMRouter.h - confirmed on hardware, this is exactly what
              // fell through to the generic "Resolving" fallback below.
              snprintf(buf, sizeof(buf), "Link Stale %us", next_in_secs);
            } else if (urns_lxmf_router->outbound_propagation_stamp_cost() > 0) {
              // Link is already ACTIVE (every check above passed) and this
              // node requires a stamp, but neither stamp_running() nor
              // stamp_done() is true yet - the link only just became
              // ACTIVE and send_propagated() hasn't had its own next
              // ~10s poll to notice and kick the grind off yet. Same
              // "waiting on the router's own poll cadence to catch up"
              // gap as Stamp Ready above, just on the other side of the
              // grind - confirmed on hardware.
              snprintf(buf, sizeof(buf), "Preparing Stamp...");
            } else {
              // Every reachable RESOLVING/WAITING sub-state is named
              // explicitly above now (no path/announce/prop node, link
              // establishing/stale, stamp preparing/generating/ready) -
              // link status enum only has PENDING/HANDSHAKE/ACTIVE/STALE/
              // CLOSED (Type.h), CLOSED is handled by recreating the link
              // outright, and no stamp is required here, so the only thing
              // left this can be is "link just went ACTIVE, about to pack
              // and start the resource transfer on the router's next
              // ~10s poll" - not a guess/defensive catch-all anymore.
              snprintf(buf, sizeof(buf), "Preparing Send %us", next_in_secs);
            }
          }
        } else if (has_retried) {
          // No deadline known - e.g. a PROPAGATED retry, which never
          // registers a PendingProofSlot (send_propagated()'s own
          // resource-transfer path, LXMRouter.cpp).
          snprintf(buf, sizeof(buf), "Retrying (%d/%u)", attempts, (unsigned)msngr_max_retries);
        } else {
          snprintf(buf, sizeof(buf), "Sending...");
        }
        MENU_GFX.print(buf);
        footer_drawn = true;
      }
    #endif
    if (!footer_drawn) {
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
  }

  #if HAS_URNS == true
    // MENU_STATE_URNS_PATH_HASH_VIEW - the full 32-char path hash, two
    // plain centered lines, no field captions and no BACK row (any input
    // just dismisses it, see menu_confirm_select()/menu_encoder_rotate())
    // - the point of this screen is to be nothing but the hash, easy to
    // read at a glance instead of squeezed into a label+value list row.
    // Uppercase A-F (toHex(true)) - same convention as every other
    // destination/identity hash this firmware displays (Path Table,
    // Path Detail, Identities), per-user request to keep hex display
    // case consistent across the whole menu.
    void draw_menu_urns_path_hash_disp() {
      MENU_GFX.setFont(MENU_FONT);
      MENU_GFX.setTextSize(1);
      MENU_GFX.setTextColor(SSD1306_WHITE);

      std::string full_hex = urns_path_detail_hash.toHex(true);
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

    // MENU_STATE_URNS_IDENTITY_KEY_VIEW - the full Base32-encoded raw
    // private key of urns_identity, plain left-aligned lines, no field
    // captions, no header/footer chrome (any input dismisses it, see
    // menu_confirm_select()/menu_encoder_rotate()). Unlike every other
    // hash this firmware displays, this is a PRIVATE key, not a public
    // destination hash - reachable only from URNS_KEYS_ITEM_DISPLAY
    // (MENU_STATE_URNS_KEYS), which is itself only reachable once
    // vault_enabled is true (URNS_ITEM_KEYS). This is an
    // intentional, explicit, user-initiated action (the user has to be
    // physically at the device and navigate here on purpose), not an
    // oversight - same "deliberate, physically-present operation"
    // reasoning IdentityTransfer.h's own vault_identity_confirm() already
    // documents for identity replacement.
    //
    // Doesn't fit MENU_STATE_URNS_PATH_HASH_VIEW's 2-line layout, so this
    // gets its own function - wrapped to as many lines as it takes at
    // MENU_CONTENT_W, left-aligned rather than centered so every line
    // starts at the same x regardless of length. Real per-line wrap
    // (measures actual glyph widths via getTextBounds(), same technique
    // MENU_STATE_URNS_PATH_HASH_VIEW's own centered layout above already
    // uses) rather than a flat chars-per-line guess - confirmed on
    // hardware that a fixed budget either overflowed the right edge or
    // left it too conservative depending on the guess. Baseline starts at
    // MENU_HEADER_TEXT_Y, not y=2 - Org_01/SMALL_FONT's setCursor(x,y) is
    // a baseline, not a top-left corner (see feedback_org01_font_baseline
    // memory), so starting too close to y=0 clipped the first line's
    // ascenders against the top of the canvas; MENU_HEADER_TEXT_Y is the
    // same safe top-of-screen baseline every header title on this board
    // already uses. 9px line spacing (not the tighter 8 the first pass
    // used) to actually use the vertical room this chrome-less screen has
    // to spare, confirmed too cramped near the top on hardware otherwise.
    void draw_menu_urns_identity_key_disp() {
      MENU_GFX.setFont(MENU_FONT);
      MENU_GFX.setTextSize(1);
      MENU_GFX.setTextColor(SSD1306_WHITE);

      std::string full_key = urns_identity_key_encode();

      int16_t y = MENU_HEADER_TEXT_Y;
      size_t pos = 0;
      while (pos < full_key.size()) {
        size_t len = full_key.size() - pos;
        while (len > 1) {
          std::string candidate = full_key.substr(pos, len);
          int16_t bx, by; uint16_t bw, bh;
          MENU_GFX.getTextBounds(candidate.c_str(), 0, 0, &bx, &by, &bw, &bh);
          if ((int16_t)bw <= MENU_CONTENT_W) break;
          len--;
        }
        std::string line = full_key.substr(pos, len);
        MENU_GFX.setCursor(MENU_CONTENT_X, y);
        MENU_GFX.print(line.c_str());
        pos += len;
        y += 9;
      }
    }

  #if HAS_LXMF == true
    // MENU_STATE_MSNGR_TEXT_ENTRY's on-screen keyboard - a real grid (not
    // a draw_menu_list_disp() vertical list), so it gets its own draw
    // function, same as draw_menu_memory_disp() above. Header/footer reuse
    // draw_menu_list_disp()'s own fixed offsets so this screen still looks
    // like part of the same menu; only the middle content (input preview +
    // key grid) is bespoke. Column/row math is a first pass tuned by eye
    // for the generic 128x64/Org_01 combination (same font every HAS_URNS
    // board uses today, see msngr_wrap_text()'s own comment) - expect this
    // to need live on-hardware nudging like every other
    // pixel-level layout in this file.
    // Extracted from draw_menu_msngr_keyboard_disp()'s own non-hex-mode
    // preview box below - shared with MENU_STATE_MSNGR_CHAT's own draw
    // block, which wants the exact same box+border behavior, just
    // positioned at the bottom of the screen instead of right under the
    // title.
    //
    // cursor_pos < 0 (default): unchanged from this function's original
    // behavior - the on-screen keyboard only ever appends/trims-the-tail
    // (msngr_kb_insert_char()/_do_backspace(), no cursor concept at all),
    // so the caret is always right after the last character and the
    // visible window always anchors on the tail, trimming from the front
    // ("..." prefixed) when it doesn't fit.
    //
    // cursor_pos >= 0 (MENU_STATE_MSNGR_CHAT only, msngr_chat_cursor): the
    // caret can sit anywhere in the buffer, not just the end, so the
    // visible window has to track wherever the cursor currently is instead
    // of always the tail - see msngr_chat_compose_win_start's own comment.
    // Caret blink period - matches MSNGR_ENVELOPE_BLINK_MS's own cadence
    // (Display.h) for a consistent "blink speed" across the UI, per user
    // request that the compose caret blink like a real text cursor rather
    // than sit permanently solid.
    #if HAS_BLE_HID_HOST == true
      // MENU_STATE_MSNGR_CHAT's DELETE MESSAGE? confirm - shared by both
      // the normal message-list view and the full-screen single-message
      // view (draw_settings_menu_disp()'s own MSNGR_CHAT case, below),
      // since Backspace opens the same dialog from either. Deliberately
      // NOT draw_menu_status_rect() (that primitive's own single
      // footprint-tracking slot is already shared by the KBD CONNECTED/
      // DISCONNECTED notice and the button-hold overlay; reusing it here
      // for a two-line dialog would collide with those). Enter/Y=DELETE,
      // Esc/Backspace/N=CANCEL per user request, not a real cursor-based
      // CONFIRM/CANCEL list - see msngr_chat_delete_confirm_pending's own
      // comment.
      void draw_msngr_chat_delete_confirm_box() {
        const char *line1 = "DELETE MESSAGE?";
        const char *line2 = "Y/Enter:Yes N/Esc:No";
        int16_t x1, y1; uint16_t w1, h1, w2, h2;
        MENU_GFX.getTextBounds(line1, 0, 0, &x1, &y1, &w1, &h1);
        MENU_GFX.getTextBounds(line2, 0, 0, &x1, &y1, &w2, &h2);
        uint16_t inner_w = std::max(w1, w2);
        const int16_t pad_x = 4, pad_y = 3, line_gap = 3;
        int16_t box_w2 = (int16_t)inner_w + pad_x * 2;
        int16_t box_h2 = (int16_t)(h1 + h2) + pad_y * 2 + line_gap;
        int16_t bx = (128 - box_w2) / 2;
        int16_t by = (64 - box_h2) / 2;
        MENU_GFX.fillRect(bx, by, box_w2, box_h2, SSD1306_BLACK);
        MENU_GFX.drawRect(bx, by, box_w2, box_h2, SSD1306_WHITE);
        MENU_GFX.setTextColor(SSD1306_WHITE);
        MENU_GFX.setCursor(bx + (box_w2 - (int16_t)w1) / 2, by + pad_y + (int16_t)h1);
        MENU_GFX.print(line1);
        MENU_GFX.setCursor(bx + (box_w2 - (int16_t)w2) / 2, by + pad_y + (int16_t)h1 + line_gap + (int16_t)h2);
        MENU_GFX.print(line2);
      }
    #endif

    // Scroll position indicator for draw_msngr_full_message_view(), below -
    // per user-provided mockup (~/Downloads/scrollbar-mockup.c): a hollow
    // capsule track (rounded-corner outline) spanning almost the full
    // canvas height, plus a solid thumb sized to visible_lines/total_lines
    // and positioned to msngr_msg_view_scroll_line/max_scroll. Only ever
    // called when total_lines > visible_lines (caller's own check) - a
    // fully-visible message has nothing to scroll, so nothing is drawn.
    // Geometry is relative to MENU_CONTENT_X/_W (the board-group content-
    // area constants used throughout this file) rather than the mockup's
    // literal 123-126 pixel columns, even though this view exists on only
    // one board group today.
    void draw_msngr_msg_view_scrollbar(uint8_t total_lines, uint8_t visible_lines) {
      const int16_t track_x0 = (MENU_CONTENT_X + MENU_CONTENT_W) - 5; // 123 on this board group
      const int16_t track_x1 = track_x0 + 3;                          // 126
      const int16_t track_y0 = 1, track_y1 = 62;
      const int16_t track_h  = track_y1 - track_y0 + 1;               // 62
      MENU_GFX.drawFastVLine(track_x0, track_y0 + 1, track_h - 2, SSD1306_WHITE);
      MENU_GFX.drawFastVLine(track_x1, track_y0 + 1, track_h - 2, SSD1306_WHITE);
      MENU_GFX.drawFastHLine(track_x0 + 1, track_y0, 2, SSD1306_WHITE);
      MENU_GFX.drawFastHLine(track_x0 + 1, track_y1, 2, SSD1306_WHITE);

      // Thumb height scales with the visible fraction of the message,
      // clamped to a minimum so it never vanishes on a very long message -
      // the mockup's own 6px thumb was just one example ratio, not a fixed
      // size.
      const int16_t min_thumb_h = 3;
      int16_t thumb_h = (int16_t)((int32_t)track_h * visible_lines / total_lines);
      if (thumb_h < min_thumb_h) thumb_h = min_thumb_h;
      if (thumb_h > track_h) thumb_h = track_h;
      uint8_t max_scroll = total_lines - visible_lines; // caller's own check guarantees > 0
      int16_t thumb_y = track_y0 + (int16_t)((int32_t)(track_h - thumb_h) * msngr_msg_view_scroll_line / max_scroll);
      MENU_GFX.fillRect(track_x0, thumb_y, 4, thumb_h, SSD1306_WHITE);

      // Per user correction: the track's own rounded caps (the two
      // drawFastHLine() calls above, matching the mockup's own two-pixel-
      // narrower top/bottom rows) must stay rounded even when the thumb's
      // fillRect happens to cover that exact row (thumb at the very top or
      // very bottom of the track) - the fillRect above is a full 4px-wide
      // rect, which would otherwise square off that row's corners by
      // painting over the two side pixels the cap deliberately leaves
      // blank. Un-paint just those two pixels again, only when the thumb
      // actually reaches an end.
      if (thumb_y == track_y0) {
        MENU_GFX.drawPixel(track_x0, track_y0, SSD1306_BLACK);
        MENU_GFX.drawPixel(track_x1, track_y0, SSD1306_BLACK);
      }
      if (thumb_y + thumb_h - 1 == track_y1) {
        MENU_GFX.drawPixel(track_x0, track_y1, SSD1306_BLACK);
        MENU_GFX.drawPixel(track_x1, track_y1, SSD1306_BLACK);
      }
    }

    // Full-screen, ornament-free single-message view - the actual drawing
    // shared by both entry points (msngr_msg_view_active's own comment):
    // MENU_STATE_MSNGR_MSG_DETAIL's "Full Message" row and MENU_STATE_
    // MSNGR_CHAT's Enter-while-browsing. No message-list rows, no compose
    // box, no title/footer - just the wrapped content filling the whole
    // 64px-tall canvas. msngr_msg_detail_cache_content (Messenger.h) was
    // already populated right when this mode was entered - not re-fetched
    // here (same "only ever refresh from a discrete input event, never
    // the render path" discipline as every other flash-reading cache in
    // this file). Deliberately NOT gated behind HAS_BLE_HID_HOST - see
    // msngr_msg_view_active's own declaration.
    //
    // draw_delete_confirm: only ever true from the MENU_STATE_MSNGR_CHAT
    // call site - its own Backspace-opens-delete-confirm gesture
    // (HAS_BLE_HID_HOST only). MENU_STATE_MSNGR_MSG_DETAIL has no
    // equivalent in-place overlay here (its own Delete row is a full
    // MENU_STATE_MSNGR_DELETE_CONFIRM screen instead, reached after
    // leaving this view), so its own call site always passes false.
    void draw_msngr_full_message_view(bool draw_delete_confirm) {
      MENU_GFX.setFont(MENU_FONT);
      MENU_GFX.setTextSize(1);
      std::vector<std::string> lines = msngr_msg_view_wrap(msngr_msg_detail_cache_content);
      uint8_t total_lines = (uint8_t)lines.size();
      const uint8_t visible_lines = 7; // 7*9=63px, fits the 64px canvas
      uint8_t max_scroll = (total_lines > visible_lines) ? (uint8_t)(total_lines - visible_lines) : 0;
      if (msngr_msg_view_scroll_line > max_scroll) msngr_msg_view_scroll_line = max_scroll;
      MENU_GFX.setTextColor(SSD1306_WHITE);
      for (uint8_t vi = 0; vi < visible_lines; vi++) {
        uint8_t li = (uint8_t)(msngr_msg_view_scroll_line + vi);
        if (li >= total_lines) break;
        MENU_GFX.setCursor(MENU_CONTENT_X, 7 + (int16_t)vi * 9);
        MENU_GFX.print(lines[li].c_str());
      }
      if (total_lines > visible_lines) draw_msngr_msg_view_scrollbar(total_lines, visible_lines);
      #if HAS_BLE_HID_HOST == true
        if (draw_delete_confirm && msngr_chat_delete_confirm_pending) draw_msngr_chat_delete_confirm_box();
      #endif
    }

  #endif
  #endif

  // draw_msngr_compose_box()/draw_menu_msngr_keyboard_disp(), relocated
  // out from under the #if HAS_URNS==true wrapper (matching the same
  // relocation reasoning as the shared keyboard mechanics further up this
  // file) - HAS_WIFI boards need this on-screen-keyboard draw code too,
  // and HAS_URNS is not implied by HAS_WIFI.
  #if HAS_LXMF == true || HAS_WIFI == true
    #define MSNGR_COMPOSE_CARET_BLINK_MS 500
    void draw_msngr_compose_box(int16_t box_y, int32_t cursor_pos = -1) {
      const int16_t box_x = MENU_CONTENT_X, box_w = MENU_CONTENT_W;
      const uint8_t line_h = 9; // one text line's worth of box height
      const int16_t box_h = (int16_t)line_h;
      MENU_GFX.drawRect(box_x, box_y, box_w, box_h, SSD1306_WHITE);
      const int16_t max_w = box_w - 6;
      // Phase computed off millis() directly (no separate stored toggle/
      // timestamp needed) - both callers of this function already redraw
      // every real display cycle while their screen is open, so a plain
      // "which half of the current period are we in" check is enough to
      // blink smoothly, same idea as the envelope icon's own blink but
      // without needing persisted state.
      bool caret_visible = ((millis() / MSNGR_COMPOSE_CARET_BLINK_MS) % 2) == 0;

      if (cursor_pos < 0) {
        std::string shown;
        if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_WIFI_PSK) {
          // Mask every character with '*' - except the one just typed,
          // which stays in plain text for MSNGR_KB_PSK_REVEAL_MS (see that
          // constant's own comment) so a single mistyped character is
          // still catchable without leaving the whole password legible.
          size_t len = strlen(msngr_text_entry_buf);
          shown.assign(len, '*');
          if (len > 0 && millis() - msngr_kb_last_insert_ms < MSNGR_KB_PSK_REVEAL_MS) {
            shown[len - 1] = msngr_text_entry_buf[len - 1];
          }
        } else {
          shown = msngr_text_entry_buf;
        }
        size_t full_len = shown.size();
        int16_t x1, y1; uint16_t tw, th;
        MENU_GFX.getTextBounds(shown.c_str(), 0, 0, &x1, &y1, &tw, &th);
        while (tw > (uint16_t)max_w && !shown.empty()) {
          shown.erase(0, 1);
          std::string probe = "..." + shown;
          MENU_GFX.getTextBounds(probe.c_str(), 0, 0, &x1, &y1, &tw, &th);
        }
        if (shown.size() < full_len) shown = "..." + shown;
        MENU_GFX.setCursor(box_x + 2, box_y + 6); // nudged up 1px, per user request on real hardware
        MENU_GFX.print(shown.c_str());
        // getCursorX() after the real print(), not getTextBounds()'s tw -
        // getTextBounds() measures the ink-pixel bounding box, and a
        // trailing space draws no ink, so its width was being excluded from
        // tw entirely whenever the buffer ended in a space (visible on
        // hardware: caret sat ~1px after the last letter instead of after
        // the real space, until a further character made the space
        // internal rather than trailing and the measurement "corrected
        // itself"). print()'s own cursor always advances by each glyph's
        // true xAdvance regardless of ink, so reading it back after the
        // real print() gives the correct position unconditionally.
        int16_t caret_x = MENU_GFX.getCursorX() + 1;
        if (caret_visible && caret_x < box_x + box_w - 1) MENU_GFX.drawFastVLine(caret_x, box_y + 1, box_h - 2, SSD1306_WHITE);
        return;
      }

      #if HAS_BLE_HID_HOST == true
        // EN/RU layout indicator, in this box's own far-right corner -
        // per user request. Dialog Mode has no title/footer chrome at
        // all (MENU_STATE_MSNGR_CHAT's own draw block's comment) to put
        // one in otherwise, unlike MENU_STATE_MSNGR_TEXT_ENTRY's title
        // row (draw_menu_msngr_keyboard_disp(), above) - same dark-on-
        // bright box style as that one, just placed here instead. Drawn
        // first so the text-window math below (the shadowed max_w) knows
        // to leave room for it, rather than needing the indicator to
        // paint over already-scrolled text.
        const char *lang_label = msngr_kb_lang_ru ? "RU" : "EN";
        int16_t lx1, ly1; uint16_t lang_w, lang_h;
        MENU_GFX.getTextBounds(lang_label, 0, 0, &lx1, &ly1, &lang_w, &lang_h);
        const int16_t lang_pad_x = 2;
        // +1 - per user request on real hardware, widens the box 1px to
        // the left (the fillRect's own left edge, right edge unchanged)
        // so exactly 2 bright pixels are left on the left of the glyph
        // ink, matching the 2 the text's own +1 nudge below leaves on
        // the right (against the compose box's own border).
        const int16_t lang_box_w = (int16_t)lang_w + lang_pad_x * 2 + 1;
        const int16_t lang_box_h = box_h - 2; // fits inside the box's own 1px border, top and bottom
        const int16_t lang_box_x = box_x + box_w - 1 - lang_box_w;
        const int16_t lang_box_y = box_y + 1;
        MENU_GFX.fillRect(lang_box_x, lang_box_y, lang_box_w, lang_box_h, SSD1306_WHITE);
        MENU_GFX.setTextColor(SSD1306_BLACK);
        // +1 - per user request on real hardware, leaves exactly 2 bright
        // pixels between the glyph ink and the compose box's own right
        // border (1 from this nudge + the border pixel itself).
        MENU_GFX.setCursor(lang_box_x + lang_pad_x + 1, box_y + 6);
        MENU_GFX.print(lang_label);
        MENU_GFX.setTextColor(SSD1306_WHITE);

        // A separate, narrower budget than the function-level max_w
        // (used by the cursor_pos<0 branch above, unaffected - this
        // function has no nested scope here to actually shadow it in)
        // - text now wraps/scrolls around the indicator's own reserved
        // width plus a small gap, rather than running underneath it.
        const int16_t compose_max_w = box_w - 6 - lang_box_w - 2;

        std::string full(msngr_text_entry_buf);
        size_t cpos = (size_t)cursor_pos;
        if (cpos > full.size()) cpos = full.size();

        // Pull the window start forward to the cursor if it scrolled out
        // of view to the left (cursor moved/backspaced before win_start).
        if (msngr_chat_compose_win_start > cpos) msngr_chat_compose_win_start = cpos;

        auto slice_width = [&](size_t start, size_t end) -> uint16_t {
          std::string probe = full.substr(start, end - start);
          int16_t x1, y1; uint16_t w, h;
          MENU_GFX.getTextBounds(probe.c_str(), 0, 0, &x1, &y1, &w, &h);
          return w;
        };
        // Push the window start rightward until the cursor fits within
        // compose_max_w again (cursor moved/typed past the right edge).
        while (msngr_chat_compose_win_start < cpos &&
               slice_width(msngr_chat_compose_win_start, cpos) > (uint16_t)compose_max_w) {
          msngr_chat_compose_win_start++;
        }
        // Extend the tail as far past the cursor as still fits, for
        // trailing context - doesn't affect win_start.
        size_t win_end = cpos;
        while (win_end < full.size() && slice_width(msngr_chat_compose_win_start, win_end + 1) <= (uint16_t)compose_max_w) {
          win_end++;
        }

        std::string before_cursor = full.substr(msngr_chat_compose_win_start, cpos - msngr_chat_compose_win_start);
        std::string after_cursor = full.substr(cpos, win_end - cpos);

        MENU_GFX.setCursor(box_x + 2, box_y + 6);
        MENU_GFX.print(before_cursor.c_str());
        // Same getCursorX()-after-print() reasoning as the cursor_pos<0
        // path above - correct even if before_cursor ends in a space.
        int16_t caret_x = MENU_GFX.getCursorX() + 1;
        MENU_GFX.print(after_cursor.c_str());
        // Bounded by the indicator's own left edge now, not just the
        // box's physical right edge, so the caret can't land underneath it.
        if (caret_visible && caret_x < lang_box_x - 1) MENU_GFX.drawFastVLine(caret_x, box_y + 1, box_h - 2, SSD1306_WHITE);
      #endif
    }

    void draw_menu_msngr_keyboard_disp() {
      const bool hex_mode = (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH);
      // Identity restore shares hex_mode's "SAVE not SEND, no EN/RU
      // toggle, show a N/max progress count" treatment, but NOT its
      // compact 2-row grid/2-line preview (MSNGR_KB_LAYOUT_BASE32 needs
      // the full standard row count - see that table's own comment) - so
      // it gets its own flag rather than folding into hex_mode itself.
      const bool base32_mode = (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_IDENTITY_RESTORE);
      const bool wifi_mode = (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_WIFI_SSID ||
                               msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_WIFI_PSK);
      // Deliberately NOT wifi_mode - unlike hex_mode/base32_mode, WiFi
      // purposes keep both a real Space key and a language-style indicator
      // box (Letters/Symbols instead of EN/RU - see msngr_kb_active_layout()),
      // so they must NOT participate in this flag's other two jobs
      // (suppressing the indicator box below, and picking the action
      // column's width basis, "SAVE"-narrow vs "SPACE"-wide) - only in
      // save_not_send, just below, which picks the Send/Save key's label.
      const bool save_label_mode = hex_mode || base32_mode;
      const bool save_not_send = save_label_mode || wifi_mode;
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
                     msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_PRESET ? "Preset" :
                     msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH ? "Add Hash" :
                     msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_RENAME ? "Rename" :
                     msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_IDENTITY_RESTORE ? "Restore Key" :
                     msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_WIFI_SSID ? "SSID" :
                     msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_WIFI_PSK ? "Password" : "Send Msg");

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
        // Hash entry/identity restore care about hitting an exact target
        // length, not the UTF-8 payload size every other purpose here
        // sends/saves as - an N/max progress count is the useful number
        // to show instead.
        #if HAS_LXMF == true
        if (hex_mode) {
          snprintf(count_buf, sizeof(count_buf), " (%u/%u)", (unsigned)strlen(msngr_text_entry_buf), (unsigned)(LXMF::PEER_HASH_SIZE * 2));
        } else if (base32_mode) {
          snprintf(count_buf, sizeof(count_buf), " (%u/%u)", (unsigned)strlen(msngr_text_entry_buf), (unsigned)VAULT_IDENTITY_KEY_BASE32_LEN);
        } else
        #endif
        {
          snprintf(count_buf, sizeof(count_buf), " (%u bytes)", (unsigned)msngr_kb_utf8_len(msngr_text_entry_buf));
        }

        int16_t cx1, cy1; uint16_t count_w, count_h;
        MENU_GFX.getTextBounds(count_buf, 0, 0, &cx1, &cy1, &count_w, &count_h);

        if (save_label_mode) {
          // No EN/RU indicator - neither hex_mode's grid (MSNGR_KB_LAYOUT_
          // HEX) nor base32_mode's (MSNGR_KB_LAYOUT_BASE32) has a language
          // toggle at all (msngr_kb_lang_ru is forced/left false whenever
          // this screen opens for either purpose), so the box would just
          // be permanently stuck showing "EN" with nothing to indicate.
          MENU_GFX.setCursor(MENU_CONTENT_X + MENU_CONTENT_W - (int16_t)count_w, header_y);
          MENU_GFX.print(count_buf);
        } else {
          const char *lang_label = wifi_mode ? (msngr_kb_lang_ru ? "123" : "ABC") : (msngr_kb_lang_ru ? "RU" : "EN");

          int16_t lx1, ly1; uint16_t lang_w, lang_h;
          MENU_GFX.getTextBounds(lang_label, 0, header_y, &lx1, &ly1, &lang_w, &lang_h);

          const int16_t pad_x = 2, pad_y = 1;
          const int16_t lang_box_w = (int16_t)lang_w + pad_x * 2;
          const int16_t lang_box_h = (int16_t)lang_h + pad_y * 2;
          const int16_t lang_box_x = MENU_CONTENT_X + MENU_CONTENT_W - lang_box_w - (int16_t)count_w;
          const int16_t lang_box_y = ly1 - pad_y;

          MENU_GFX.fillRect(lang_box_x, lang_box_y, lang_box_w, lang_box_h, SSD1306_WHITE);
          MENU_GFX.setTextColor(SSD1306_BLACK);
          MENU_GFX.setCursor(lang_box_x + pad_x, header_y);
          MENU_GFX.print(lang_label);

          MENU_GFX.setTextColor(SSD1306_WHITE);
          MENU_GFX.setCursor(lang_box_x + lang_box_w, header_y);
          MENU_GFX.print(count_buf);
        }
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
      // just single-line since there's no vertical room to spare here -
      // EXCEPT for MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH below, which
      // trades the two now-unused MSNGR_KB_LAYOUT_HEX keyboard rows for a
      // second preview line instead, so all 32 hex chars are visible at
      // once with no scrolling.
      const int16_t box_x = MENU_CONTENT_X, box_y = MENU_HEADER_HLINE_Y + 1, box_w = MENU_CONTENT_W;
      const uint8_t line_h = 9; // one text line's worth of box height - matches the single-line box's own original box_h
      const int16_t box_h = hex_mode ? (int16_t)(line_h * 2) : (int16_t)line_h;
      MENU_GFX.drawRect(box_x, box_y, box_w, box_h, SSD1306_WHITE);
      if (hex_mode) {
        // Two lines of 16 chars each (32 total) fit this box_w at Org_01's
        // width (msngr_wrap_text() elsewhere in this file comfortably fits
        // 20+ chars in the same MENU_CONTENT_W) - exactly BOOKMARK_HASH's
        // own max_len (32), so its full buffer always fits with no
        // scrolling needed - windowed to the last 32 chars typed purely
        // as a defensive measure (never actually triggers at this
        // purpose's own max_len), not because it's expected to overflow.
        std::string full(msngr_text_entry_buf);
        std::string windowed = full.size() > 32 ? full.substr(full.size() - 32) : full;
        std::string line1 = windowed.size() > 16 ? windowed.substr(0, 16) : windowed;
        std::string line2 = windowed.size() > 16 ? windowed.substr(16) : "";
        MENU_GFX.setCursor(box_x + 2, box_y + 6);
        MENU_GFX.print(line1.c_str());
        MENU_GFX.setCursor(box_x + 2, box_y + 6 + line_h);
        MENU_GFX.print(line2.c_str());

        bool caret_on_line2 = windowed.size() >= 16;
        const std::string &caret_line = caret_on_line2 ? line2 : line1;
        int16_t cx1, cy1; uint16_t cw, ch;
        MENU_GFX.getTextBounds(caret_line.c_str(), 0, 0, &cx1, &cy1, &cw, &ch);
        int16_t caret_x = box_x + 2 + (int16_t)cw + 1;
        int16_t caret_line_y = caret_on_line2 ? box_y + line_h : box_y;
        if (caret_x < box_x + box_w - 1) MENU_GFX.drawFastVLine(caret_x, caret_line_y + 1, line_h - 2, SSD1306_WHITE);
      } else {
        // Border already drawn above (shared with the hex_mode branch) -
        // draw_msngr_compose_box() draws its own too, a harmless redundant
        // redraw of the same rect rather than restructuring that shared
        // line just to avoid it.
        draw_msngr_compose_box(box_y);
      }

      // Keyboard grid - reserve extra width for the last (action) column,
      // sized to the widest action label this layout actually uses
      // ("SPACE" normally; hex entry has no SPACE/Shift cell, so "SAVE"/
      // "BACK" - both 4 chars - are the widest it needs), split the rest
      // evenly across the other 10 columns, and hand any leftover pixels
      // to the first few columns - same approach meshtastic's own
      // VirtualKeyboard::draw() uses, just against Adafruit_GFX instead of
      // OLEDDisplay.
      //
      // row_h is always the standard MSNGR_KB_ROWS(4)-row height, even in
      // hex_mode - NOT (grid_bottom-grid_top)/msngr_kb_active_rows(),
      // which would stretch hex entry's 2 rows to fill all the vertical
      // space its own taller 2-line preview box left behind, ending up
      // visibly taller than every other keyboard screen. Any space below
      // the 2 actual rows (grid_bottom - grid_top - kb_rows*row_h) is
      // just left blank instead, on request - same "don't stretch cells
      // to fill unused space" call already made for MSNGR_KB_LAYOUT_HEX's
      // column widths below.
      const uint8_t kb_rows = msngr_kb_active_rows();
      const int16_t grid_top = box_y + box_h + 1;
      const int16_t grid_bottom = MENU_LIST_FOOTER_HLINE_Y;
      const uint8_t row_h = (uint8_t)((grid_bottom - (box_y + (int16_t)line_h + 1)) / MSNGR_KB_ROWS);

      int16_t x1, y1; uint16_t last_col_w, th;
      MENU_GFX.getTextBounds(save_label_mode ? "SAVE" : "SPACE", 0, 0, &x1, &y1, &last_col_w, &th);
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

      uint8_t cur_row, cur_col;
      msngr_kb_cursor_rc(msngr_kb_cursor, cur_row, cur_col);

      for (uint8_t r = 0; r < kb_rows; r++) {
        for (uint8_t c = 0; c < MSNGR_KB_COLS; c++) {
          char ch = msngr_kb_active_layout()[r][c];
          uint8_t type = msngr_kb_key_type(ch);
          if (type == MSNGR_KB_NONE) continue; // MSNGR_KB_LAYOUT_HEX's unused cells - nothing drawn, not even a blank button
          // col_x/col_w (computed above, shared by every row) - not a
          // per-row layout, so MSNGR_KB_LAYOUT_HEX's '0'-'9' and 'A'-'F'
          // land in identical columns, and DEL/SAVE/BACK all land in the
          // exact same rightmost column, on request.
          int16_t kx = col_x[c];
          int16_t ky = grid_top + r * row_h;
          int16_t kw = col_w[c];
          if (type == MSNGR_KB_TYPE_TOGGLE) {
            // Spans every narrow column the digit/letter rows above it
            // use (col 0 through the one right before the action column)
            // instead of just its own col 0 cell - "LXMF"/"Propagation"
            // need far more room than a single digit-width cell, and
            // nothing else lives in row 2's other cells to compete for it.
            kw = col_x[left_cols] - col_x[0];
          }

          char label_buf[2];
          const char *label;
          switch (type) {
            case MSNGR_KB_BACKSPACE:   label = "DEL"; break;
            case MSNGR_KB_SEND:        label = save_not_send ? "SAVE" : "SEND"; break; // hex_mode/base32_mode/wifi_mode save a bookmark/identity/credential locally, nothing goes out over the air - "SEND" would be misleading
            case MSNGR_KB_SPACE:       label = "SPACE"; break;
            case MSNGR_KB_BACK:        label = "BACK"; break;
            case MSNGR_KB_SHIFT:       label = msngr_kb_shift_on ? "^^" : "^"; break;
            #if HAS_LXMF == true
            case MSNGR_KB_TYPE_TOGGLE: label = msngr_kb_bookmark_type == MSNGR_BOOKMARK_TYPE_PROPAGATION ? "Propagation" : "LXMF"; break;
            #endif
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
          int16_t label_y = ky + row_h - 3; // nudged up 1px, per user request on real hardware - fits the key boxes better

          #if HAS_LXMF == true
          if (type == MSNGR_KB_TYPE_TOGGLE) {
            // Per user request: prefixes the label with the same node/
            // propagation-node icons already used elsewhere (Graphics.h) -
            // icon+label centered together as one unit in the cell, same
            // reasoning MENU_ROW_TEXT_X_ICONS's own icon+text pairing
            // uses elsewhere, just centered here instead of left-anchored.
            bool is_prop = (msngr_kb_bookmark_type == MSNGR_BOOKMARK_TYPE_PROPAGATION);
            const uint8_t *type_icon = is_prop ? bm_menu_icon_msngr_prop_node : bm_menu_icon_msngr_node;
            uint8_t type_icon_w = is_prop ? MENU_ICON_W_MSNGR_PROP_NODE : MENU_ICON_W_MSNGR_NODE;
            const int16_t icon_gap = 3;
            int16_t combined_w = (int16_t)type_icon_w + icon_gap + (int16_t)lw;
            int16_t combined_x = kx + (kw - combined_w) / 2;
            if (combined_x < kx) combined_x = kx;
            int16_t icon_y = ky + (row_h - MENU_ICON_H) / 2;
            uint16_t fg = selected ? SSD1306_BLACK : SSD1306_WHITE;
            uint16_t bg = selected ? SSD1306_WHITE : SSD1306_BLACK;
            MENU_GFX.drawBitmap(combined_x, icon_y, type_icon, type_icon_w, MENU_ICON_H, fg, bg);
            MENU_GFX.setCursor(combined_x + type_icon_w + icon_gap, label_y);
            MENU_GFX.print(label);
          } else
          #endif
          {
            int16_t label_x = kx + (kw - (int16_t)lw + 1) / 2;
            if (label_x < kx) label_x = kx;
            MENU_GFX.setCursor(label_x, label_y);
            MENU_GFX.print(label);
          }
        }
      }

      MENU_GFX.setTextColor(SSD1306_WHITE);
      MENU_GFX.drawFastHLine(MENU_CONTENT_X, MENU_LIST_FOOTER_HLINE_Y, MENU_CONTENT_W, SSD1306_WHITE);
      MENU_GFX.setCursor(6, MENU_LIST_FOOTER_TEXT_Y);
      #if HAS_ENCODER == true
        if (encoder_enabled) MENU_GFX.print("turn:move press:open");
        else                 MENU_GFX.print("tap:next hold:open");
      #else
        MENU_GFX.print("tap:next hold:open");
      #endif
    }
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
      MENU_GFX.drawFastHLine(MENU_CONTENT_X, MENU_HEADER_HLINE_Y, MENU_CONTENT_W, SSD1306_WHITE);

      const uint8_t row_h = MENU_LIST_ROW_H;
      const int16_t bar_x = 46;
      const int16_t bar_h = (row_h > 6) ? (row_h - 5) : (row_h - 2);

      // Percentage field is reserved at "100%"'s width (not each row's
      // actual string width) so the bar's right edge doesn't shift around
      // as the digit count changes - only the text within the field is
      // right-aligned per row.
      int16_t x1, y1; uint16_t pct_field_w, th;
      MENU_GFX.getTextBounds("100%", 0, 0, &x1, &y1, &pct_field_w, &th);
      const int16_t row_right  = MENU_CONTENT_X + MENU_CONTENT_W - 2;
      const int16_t pct_left   = row_right - pct_field_w;
      const int16_t bar_w      = (pct_left - 4) - bar_x;

      for (uint8_t i = 0; i < MEM_ITEM_COUNT; i++) {
        uint8_t row_top = MENU_LIST_TOP_Y + i * row_h;
        uint8_t y = row_top + MENU_LIST_BASELINE_OFF;
        uint16_t fg = SSD1306_WHITE;
        if (i == mem_menu_cursor) {
          MENU_GFX.fillRect(MENU_CONTENT_X, row_top, MENU_CONTENT_W, row_h - 1, SSD1306_WHITE);
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
      MENU_GFX.drawFastHLine(MENU_CONTENT_X, MENU_LIST_FOOTER_HLINE_Y, MENU_CONTENT_W, SSD1306_WHITE);
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
    MENU_GFX.drawFastHLine(MENU_CONTENT_X, MENU_EDIT_HEADER_HLINE_Y, MENU_CONTENT_W, SSD1306_WHITE);

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

    MENU_GFX.drawFastHLine(MENU_CONTENT_X, MENU_EDIT_FOOTER_HLINE_Y, MENU_CONTENT_W, SSD1306_WHITE);
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
    // "255.255.255.255", 15 chars, comfortably fits at size 1, no
    // windowing needed) with the octet currently being adjusted
    // highlighted via a fillRect+invert-color trick, per octet instead of
    // per character. Already-confirmed octets sit to
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
      MENU_GFX.drawFastHLine(MENU_CONTENT_X, MENU_EDIT_HEADER_HLINE_Y, MENU_CONTENT_W, SSD1306_WHITE);

      // Tamsyn6x12 (TEXT_ENTRY_FONT) for the address itself - title/
      // divider/footer stay on SMALL_FONT/Org_01. Narrower
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
          // Tamsyn6x12 glyphs span roughly baseline-9 to baseline+4, a
          // taller box than Org_01 would need.
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
      MENU_GFX.drawFastHLine(MENU_CONTENT_X, MENU_EDIT_FOOTER_HLINE_Y, MENU_CONTENT_W, SSD1306_WHITE);
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
      MENU_GFX.drawFastHLine(MENU_CONTENT_X, MENU_EDIT_HEADER_HLINE_Y, MENU_CONTENT_W, SSD1306_WHITE);

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
      MENU_GFX.drawFastHLine(MENU_CONTENT_X, MENU_EDIT_FOOTER_HLINE_Y, MENU_CONTENT_W, SSD1306_WHITE);
      MENU_GFX.setCursor(6, MENU_EDIT_FOOTER_TEXT_Y);
      #if HAS_ENCODER == true
        if (encoder_enabled) MENU_GFX.print("turn:adjust press:ok");
        else                 MENU_GFX.print("tap:adjust hold:ok");
      #else
        MENU_GFX.print("tap:adjust hold:ok");
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

        #if HAS_BLE_HID_HOST == true
          labels[BT_ITEM_KEYBOARD] = "BLE Keyboard";
          sprintf(valbufs[BT_ITEM_KEYBOARD], ">"); // opens MENU_STATE_BLEKBD_LIST
          icons[BT_ITEM_KEYBOARD] = bm_menu_icon_blekbd;
          icon_widths[BT_ITEM_KEYBOARD] = MENU_ICON_W_BLEKBD;
        #endif

        labels[BT_ITEM_BACK] = "BACK";
        valbufs[BT_ITEM_BACK][0] = 0;
        // Explicit here (not auto-detected) since this list already builds
        // its own icons[] table for Settings above - same reasoning as
        // MSNGR_LIST's own BACK row.
        icons[BT_ITEM_BACK] = bm_menu_icon_back;
        icon_widths[BT_ITEM_BACK] = MENU_ICON_W_BACK;

        // icon_col_shared=false - Settings and BLE Keyboard (the two rows
        // with their own icon) each shift just their own label to make
        // room; MAC/Bonds/Forget Bonds stay at the plain x=8 the list used
        // before Settings existed. BACK still auto-gets its own icon+narrow
        // position via the explicit icons[BT_ITEM_BACK]
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
    #if HAS_BLE_HID_HOST == true
      else if (menu_state == MENU_STATE_BLEKBD_LIST) {
        const char *labels[BLEKBD_ITEM_COUNT];
        char valbufs[BLEKBD_ITEM_COUNT][24];

        labels[BLEKBD_ITEM_ENABLED] = "Enabled";
        sprintf(valbufs[BLEKBD_ITEM_ENABLED], staged_blekbd_enabled ? "ON" : "OFF");

        labels[BLEKBD_ITEM_STATUS] = "Status";
        if (!staged_blekbd_enabled) {
          sprintf(valbufs[BLEKBD_ITEM_STATUS], "Disabled");
        } else if (!blekbd_peer_stored) {
          sprintf(valbufs[BLEKBD_ITEM_STATUS], "Not Paired");
        } else if (blekbd_open_dev) {
          sprintf(valbufs[BLEKBD_ITEM_STATUS], "Connected");
        } else {
          sprintf(valbufs[BLEKBD_ITEM_STATUS], "Not Connected");
        }

        labels[BLEKBD_ITEM_SCAN] = "Scan for Keyboard";
        sprintf(valbufs[BLEKBD_ITEM_SCAN], ">");

        labels[BLEKBD_ITEM_FORGET] = "Forget Keyboard";
        sprintf(valbufs[BLEKBD_ITEM_FORGET], ">");

        labels[BLEKBD_ITEM_BACK] = "BACK";
        valbufs[BLEKBD_ITEM_BACK][0] = 0;

        draw_menu_list_disp("BLE KEYBOARD", labels, valbufs, BLEKBD_ITEM_COUNT, blekbd_menu_cursor);
      }
      else if (menu_state == MENU_STATE_BLEKBD_EDIT) {
        draw_menu_edit_disp("ENABLED", staged_blekbd_enabled ? "ON" : "OFF");
      }
      else if (menu_state == MENU_STATE_BLEKBD_SCAN) {
        uint8_t row_count = blekbd_scan_row_count();
        const char *labels[BLEKBD_MAX_DISCOVERED + 1];
        char valbufs[BLEKBD_MAX_DISCOVERED + 1][24];
        const uint8_t *icons[BLEKBD_MAX_DISCOVERED + 1] = { nullptr };
        uint8_t icon_widths[BLEKBD_MAX_DISCOVERED + 1] = { 0 };

        uint8_t any = 0;
        for (uint8_t i = 0; i < BLEKBD_MAX_DISCOVERED; i++) if (blekbd_discovered[i].in_use) any++;

        if (any == 0) {
          labels[0] = "Scanning...";
          valbufs[0][0] = 0;
        } else {
          uint8_t vis = 0;
          for (uint8_t i = 0; i < BLEKBD_MAX_DISCOVERED; i++) {
            if (!blekbd_discovered[i].in_use) continue;
            labels[vis] = blekbd_discovered[i].name[0] ? blekbd_discovered[i].name : "(unnamed)";
            sprintf(valbufs[vis], "%ddBm", (int)blekbd_discovered[i].rssi);
            icons[vis] = bm_menu_icon_blekbd;
            icon_widths[vis] = MENU_ICON_W_BLEKBD;
            vis++;
          }
        }
        labels[row_count - 1] = "BACK";
        valbufs[row_count - 1][0] = 0;

        // icon_col_shared=false - only the real device rows (each with its
        // own icons[vis] entry above) shift for the glyph; "Scanning..."
        // (no devices found yet) stays at the plain x=8 that placeholder
        // always used. BACK still auto-gets bm_menu_icon_back via the
        // no-explicit-icon+label=="BACK" fallback (icons[row_count-1] is
        // left nullptr above), same as every other plain submenu list.
        draw_menu_list_disp("SCAN FOR KEYBOARD", labels, valbufs, row_count, blekbd_scan_cursor, icons, icon_widths, nullptr, false);
      }
      else if (menu_state == MENU_STATE_BLEKBD_PAIR_CONFIRM) {
        const char *labels[2] = { "PAIR", "CANCEL" };
        char valbufs[2][24];
        valbufs[0][0] = 0;
        valbufs[1][0] = 0;
        char title[24];
        snprintf(title, sizeof(title), "PAIR: %s", blekbd_pending_name[0] ? blekbd_pending_name : "(unnamed)");
        draw_menu_list_disp(title, labels, valbufs, 2, blekbd_pair_confirm_cursor);
      }
      else if (menu_state == MENU_STATE_BLEKBD_PAIRING) {
        // Reads blekbd_pair_result fresh on every redraw - blekbd_hidh_cb()
        // (BLEKeyboardHost.h) is what actually advances it, same "read live
        // state, don't poll from here" split as MSNGR_SEND_RESULT. Auto-
        // returns to MENU_STATE_BLEKBD_LIST on success after
        // BLEKBD_PAIR_RESULT_POPUP_MS (blekbd_pair_result_process(),
        // polled from loop()) - FAILED waits for manual BACK.
        const char *labels[2];
        char valbufs[2][24];
        const char *status;
        switch (blekbd_pair_result) {
          case BLEKBD_PAIR_OK:     status = "Paired!"; break;
          case BLEKBD_PAIR_FAILED: status = "Failed";  break;
          default:                 status = "Pairing...";  break;
        }
        labels[0] = status;
        valbufs[0][0] = 0;
        labels[1] = "BACK";
        valbufs[1][0] = 0;
        draw_menu_list_disp("PAIRING", labels, valbufs, 2, blekbd_pairing_cursor);
      }
      else if (menu_state == MENU_STATE_BLEKBD_FORGET_CONFIRM) {
        const char *labels[2] = { "FORGET", "CANCEL" };
        char valbufs[2][24];
        valbufs[0][0] = 0;
        valbufs[1][0] = 0;
        draw_menu_list_disp("FORGET KEYBOARD?", labels, valbufs, 2, blekbd_forget_confirm_cursor);
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

        labels[URNS_ITEM_VAULT] = "PIN Protection";
        // Reflects the live vault_enabled flag, not a staged value - this
        // row has no staged/commit-on-exit state (see its own comment).
        sprintf(valbufs[URNS_ITEM_VAULT], vault_enabled ? "ON" : "OFF");

        labels[URNS_ITEM_PATHS] = "Path Table";
        sprintf(valbufs[URNS_ITEM_PATHS], "%u", (unsigned)RNS::Transport::new_path_table().size());

        labels[URNS_ITEM_IDENTITIES] = "Identities";
        sprintf(valbufs[URNS_ITEM_IDENTITIES], ">"); // opens a submenu, not an inline value

        labels[URNS_ITEM_KEYS] = "Keys";
        sprintf(valbufs[URNS_ITEM_KEYS], ">"); // opens a submenu, not an inline value

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

        // URNS_ITEM_KEYS (URNS_ITEM_VAULT_ONLY_FIRST..._LAST) is only
        // shown once vault_enabled is true - see that range's own
        // comment. draw_menu_list_disp() needs a
        // contiguous array with cursor as a literal index into it, so
        // compact into a second pair of arrays when hiding, translating
        // urns_menu_cursor (still in raw enum-ID space, see menu_confirm_
        // select()/menu_encoder_rotate()) to its position in the visible
        // list.
        if (vault_enabled) {
          draw_menu_list_disp("URNS", labels, valbufs, URNS_ITEM_COUNT, urns_menu_cursor);
        } else {
          const char *visible_labels[URNS_ITEM_COUNT];
          char visible_valbufs[URNS_ITEM_COUNT][24];
          uint8_t visible_count = 0, visible_cursor = 0;
          for (uint8_t i = 0; i < URNS_ITEM_COUNT; i++) {
            if (i >= URNS_ITEM_VAULT_ONLY_FIRST && i <= URNS_ITEM_VAULT_ONLY_LAST) continue;
            visible_labels[visible_count] = labels[i];
            memcpy(visible_valbufs[visible_count], valbufs[i], 24);
            if (i == urns_menu_cursor) visible_cursor = visible_count;
            visible_count++;
          }
          draw_menu_list_disp("URNS", visible_labels, visible_valbufs, visible_count, visible_cursor);
        }
      } else if (menu_state == MENU_STATE_URNS_FREE_DETAIL) {
        // Static snapshot from urns_free_detail_refresh() (called once on
        // entry, menu_confirm_select()) - NOT recomputed here, same
        // throttling reasoning as URNS_ITEM_FREE's own cache above, except
        // here there's no periodic refresh at all since walking every
        // bucket's directory is a heavier operation than a single
        // esp_littlefs_info() call.
        const char *labels[URNS_FREE_DETAIL_ITEM_COUNT];
        char valbufs[URNS_FREE_DETAIL_ITEM_COUNT][24];
        // Only the 5 real data buckets (Identity..Other) come from this
        // vals/names pair - Refresh and BACK are plain fixed rows assigned
        // separately below, same as every other explicit-icon-table screen
        // (see MENU_STATE_MSNGR_PEER's own comment on why passing any icons
        // table at all switches a list to this shape).
        size_t vals[URNS_FREE_DETAIL_ITEM_OTHER + 1] = {
          urns_free_detail_identity, urns_free_detail_announce,
          urns_free_detail_paths, urns_free_detail_messages, urns_free_detail_other
        };
        const char *names[URNS_FREE_DETAIL_ITEM_OTHER + 1] = {
          "Identity", "Announce", "Paths", "Messages", "Other"
        };
        const uint8_t *icons[URNS_FREE_DETAIL_ITEM_COUNT] = { nullptr };
        uint8_t icon_widths[URNS_FREE_DETAIL_ITEM_COUNT] = { 0 };
        for (uint8_t i = 0; i <= URNS_FREE_DETAIL_ITEM_OTHER; i++) {
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
        labels[URNS_FREE_DETAIL_ITEM_REFRESH] = "Refresh";
        valbufs[URNS_FREE_DETAIL_ITEM_REFRESH][0] = 0;
        // Reuses the Propagation-bookmark Sync glyph - same "recompute
        // live state" concept, just for this screen's own cached byte
        // counts instead of a prop node's path table.
        icons[URNS_FREE_DETAIL_ITEM_REFRESH] = bm_menu_icon_msngr_prop_sync;
        icon_widths[URNS_FREE_DETAIL_ITEM_REFRESH] = MENU_ICON_W_MSNGR_PROP_SYNC;
        labels[URNS_FREE_DETAIL_ITEM_BACK] = "BACK";
        valbufs[URNS_FREE_DETAIL_ITEM_BACK][0] = 0;
        icons[URNS_FREE_DETAIL_ITEM_BACK] = bm_menu_icon_back;
        icon_widths[URNS_FREE_DETAIL_ITEM_BACK] = MENU_ICON_W_BACK;

        draw_menu_list_disp("URNS FREE", labels, valbufs, URNS_FREE_DETAIL_ITEM_COUNT, urns_free_detail_cursor, icons, icon_widths);
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
        // Reads urns_path_cache (refreshed only when new_path_table()'s
        // cheap size() actually changes, urns_path_cache_refresh_if_
        // stale() above), NOT rebuilt from the flash-backed store on every
        // redraw - see that cache's own comment for why walking it here
        // used to make this whole screen sluggish.
        urns_path_cache_refresh_if_stale();
        uint8_t row_count = urns_path_display_row_count();

        const char *labels[MENU_URNS_PATH_MAX_ROWS + 1];
        char valbufs[MENU_URNS_PATH_MAX_ROWS + 1][24];

        if (urns_path_cache_count == 0) {
          labels[0] = "No Paths";
          valbufs[0][0] = 0;
        } else {
          for (uint8_t i = 0; i < urns_path_cache_count; i++) {
            labels[i] = urns_path_cache[i].label;
            uint8_t hops = urns_path_cache[i].hops;
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
        std::string full_hex = urns_path_detail_hash.toHex(true);
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

        labels[URNS_PATH_DETAIL_ITEM_DELETE] = "Delete Path";
        valbufs[URNS_PATH_DETAIL_ITEM_DELETE][0] = 0;

        labels[URNS_PATH_DETAIL_ITEM_BACK] = "BACK";
        valbufs[URNS_PATH_DETAIL_ITEM_BACK][0] = 0;

        draw_menu_list_disp("PATH DETAIL", labels, valbufs, URNS_PATH_DETAIL_ITEM_COUNT, urns_path_detail_cursor);
      } else if (menu_state == MENU_STATE_URNS_PATH_DELETE_CONFIRM) {
        // Plain 2-item list, same draw_menu_list_disp() as everywhere else -
        // same pattern as F/W Update's UPDATE/CANCEL (MENU_STATE_FWUPD_CONFIRM).
        const char *labels[2] = { "DELETE", "CANCEL" };
        char valbufs[2][24];
        valbufs[0][0] = 0;
        valbufs[1][0] = 0;
        draw_menu_list_disp("DELETE PATH?", labels, valbufs, 2, urns_path_delete_confirm_cursor);
      } else if (menu_state == MENU_STATE_URNS_PATH_PURGE_CONFIRM) {
        // Plain 2-item list, same shape as MENU_STATE_URNS_PATH_DELETE_
        // CONFIRM just above.
        const char *labels[2] = { "PURGE", "CANCEL" };
        char valbufs[2][24];
        valbufs[0][0] = 0;
        valbufs[1][0] = 0;
        draw_menu_list_disp("PURGE PATHS?", labels, valbufs, 2, urns_path_purge_confirm_cursor);
      #if HAS_LXMF == true
      } else if (menu_state == MENU_STATE_URNS_MSG_PURGE_CONFIRM) {
        const char *labels[2] = { "PURGE", "CANCEL" };
        char valbufs[2][24];
        valbufs[0][0] = 0;
        valbufs[1][0] = 0;
        draw_menu_list_disp("PURGE MESSAGES?", labels, valbufs, 2, urns_msg_purge_confirm_cursor);
      #endif
      } else if (menu_state == MENU_STATE_URNS_IDENTITIES) {
        // Fixed 3-4 row list (Node ID + Transport ID + Probe ID, plus
        // LXMF ID on HAS_LXMF boards), in that display order - see
        // URNS_ID_ITEM_* comment for why this isn't a generic RNS::
        // Transport::destinations() dump. Each row's value is the same
        // 8-hex-char preview convention the Path Table list uses for its
        // own rows (uppercase A-F, same as every other hash display in
        // this firmware - see draw_menu_urns_path_hash_disp()'s own
        // comment); selecting one opens the shared full-hash view
        // (MENU_STATE_URNS_PATH_HASH_VIEW).
        const char *labels[URNS_ID_ITEM_COUNT];
        char valbufs[URNS_ID_ITEM_COUNT][24];

        labels[URNS_ID_ITEM_NODE_IDENTITY] = "Node ID";
        snprintf(valbufs[URNS_ID_ITEM_NODE_IDENTITY], 24, "%s", urns_identity.hash().toHex(true).substr(0, 8).c_str());

        labels[URNS_ID_ITEM_TRANSPORT_IDENTITY] = "Transport ID";
        snprintf(valbufs[URNS_ID_ITEM_TRANSPORT_IDENTITY], 24, "%s", RNS::Transport::identity().hash().toHex(true).substr(0, 8).c_str());

        #if HAS_LXMF == true
          labels[URNS_ID_ITEM_LXMF_DEST] = "LXMF ID";
          if (urns_lxmf_router) {
            snprintf(valbufs[URNS_ID_ITEM_LXMF_DEST], 24, "%s", urns_lxmf_router->delivery_destination().hash().toHex(true).substr(0, 8).c_str());
          } else {
            sprintf(valbufs[URNS_ID_ITEM_LXMF_DEST], "N/A");
          }
        #endif

        labels[URNS_ID_ITEM_PROBE_DEST] = "Probe ID";
        // Only exists at the RNS::Transport level once Probe Destination is
        // actually active (see URNS_ID_ITEM_PROBE_DEST's own comment) -
        // same live-flag-not-staged-value convention as the PIN Protection
        // row above, since this reflects RNS::Transport's actual runtime
        // state, not a pending menu edit.
        if (RNS::Transport::probe_destination()) {
          snprintf(valbufs[URNS_ID_ITEM_PROBE_DEST], 24, "%s", RNS::Transport::probe_destination().hash().toHex(true).substr(0, 8).c_str());
        } else {
          sprintf(valbufs[URNS_ID_ITEM_PROBE_DEST], "N/A");
        }

        labels[URNS_ID_ITEM_BACK] = "BACK";
        valbufs[URNS_ID_ITEM_BACK][0] = 0;

        draw_menu_list_disp("IDENTITIES", labels, valbufs, URNS_ID_ITEM_COUNT, urns_identities_cursor);
      } else if (menu_state == MENU_STATE_URNS_KEYS) {
        // Fixed 2-3 row list (Display always, Restore on HAS_LXMF boards),
        // same "own submenu, only BACK does anything besides opening a
        // row" shape as MENU_STATE_URNS_IDENTITIES above - no runtime
        // hiding needed here (unlike URNS_ITEM_KEYS itself), see that
        // item's own comment.
        const char *labels[URNS_KEYS_ITEM_COUNT];
        char valbufs[URNS_KEYS_ITEM_COUNT][24];

        labels[URNS_KEYS_ITEM_DISPLAY] = "Display Identity Key";
        valbufs[URNS_KEYS_ITEM_DISPLAY][0] = 0;

        #if HAS_LXMF == true
          labels[URNS_KEYS_ITEM_RESTORE] = "Restore Identity";
          valbufs[URNS_KEYS_ITEM_RESTORE][0] = 0;
        #endif

        labels[URNS_KEYS_ITEM_BACK] = "BACK";
        valbufs[URNS_KEYS_ITEM_BACK][0] = 0;

        draw_menu_list_disp("KEYS", labels, valbufs, URNS_KEYS_ITEM_COUNT, urns_keys_cursor);
      } else if (menu_state == MENU_STATE_URNS_PATH_HASH_VIEW) {
        draw_menu_urns_path_hash_disp();
      } else if (menu_state == MENU_STATE_URNS_IDENTITY_KEY_VIEW) {
        draw_menu_urns_identity_key_disp();
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

        labels[MSNGR_TOP_ITEM_SYNC_PROP] = "Sync to Prop";
        valbufs[MSNGR_TOP_ITEM_SYNC_PROP][0] = 0;
        icons[MSNGR_TOP_ITEM_SYNC_PROP] = bm_menu_icon_msngr_prop_node;
        icon_widths[MSNGR_TOP_ITEM_SYNC_PROP] = MENU_ICON_W_MSNGR_PROP_NODE;

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
        const uint8_t *icons[MENU_MSNGR_LIST_MAX_ROWS + 1] = { nullptr };
        uint8_t icon_widths[MENU_MSNGR_LIST_MAX_ROWS + 1] = { 0 };
        // Per-row nudges (icon_col_shared=false path, same idiom
        // MENU_STATE_MSNGR_PEER's incoming-message rows use) - icon 1px
        // from the screen's left edge (icon_dx=-2, same delta that landed
        // the incoming-message icon at MENU_ROW_ICON_X-2) and the label
        // 2px clear of the icon's own right edge, not draw_menu_list_
        // disp()'s wider default shared-column gap.
        int8_t icon_dx[MENU_MSNGR_LIST_MAX_ROWS + 1] = { 0 };
        int8_t text_dx[MENU_MSNGR_LIST_MAX_ROWS + 1] = { 0 };

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
            icons[i] = bm_menu_icon_msngr_node;
            icon_widths[i] = MENU_ICON_W_MSNGR_NODE;
            icon_dx[i] = -2;
            text_dx[i] = MENU_ROW_ICON_X + icon_dx[i] + MENU_ICON_W_MSNGR_NODE + 2 - MENU_ROW_TEXT_X_ICONS;
          }
        }
        labels[row_count - 1] = "BACK";
        valbufs[row_count - 1][0] = 0;

        // icon_col_shared=false - only the conversation rows opted into
        // icons[], BACK keeps its own auto-icon path (draw_menu_list_
        // disp()'s own comment), same shape as MENU_STATE_MSNGR_PEER above.
        draw_menu_list_disp("INBOX", labels, valbufs, row_count, msngr_inbox_cursor, icons, icon_widths, icon_dx, false, text_dx);
      } else if (menu_state == MENU_STATE_MSNGR_BOOKMARKS) {
        // +2, not +1 - bookmark rows plus "Add by Hash" (msngr_bookmarks_
        // row_count()'s own comment) plus BACK.
        uint8_t row_count = msngr_bookmarks_row_count();
        const char *labels[MSNGR_MAX_BOOKMARKS + 2];
        char label_bufs[MSNGR_MAX_BOOKMARKS][MSNGR_NAME_MAX_LEN + 1];
        char valbufs[MSNGR_MAX_BOOKMARKS + 2][24];
        const uint8_t *icons[MSNGR_MAX_BOOKMARKS + 2] = { nullptr };
        uint8_t icon_widths[MSNGR_MAX_BOOKMARKS + 2] = { 0 };

        uint8_t vis = 0;
        for (uint8_t i = 0; i < MSNGR_MAX_BOOKMARKS; i++) {
          if (!msngr_bookmarks[i].in_use) continue;
          // Through messenger_peer_display_name(), not msngr_bookmarks[i].
          // name directly - an "Add by Hash" bookmark (Menu.h's own SEND-
          // key handler for MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH) is
          // deliberately saved with an empty name so it keeps falling
          // through to that function's live Identity::recall_app_data()
          // backfill check until a real name resolves - reading the raw
          // buffer here would print a blank row in the meantime instead of
          // the same hex/live-name fallback the peer screen already shows.
          RNS::Bytes bm_hash(msngr_bookmarks[i].hash, LXMF::PEER_HASH_SIZE);
          snprintf(label_bufs[vis], sizeof(label_bufs[vis]), "%s", messenger_peer_display_name(bm_hash).c_str());
          labels[vis] = label_bufs[vis];
          valbufs[vis][0] = 0;
          // Same node glyph MENU_STATE_MSNGR_INBOX's conversation rows use
          // (Graphics.h) - a bookmark is still just a saved peer/node -
          // except Propagation-type bookmarks, which get their own
          // broadcast-tower glyph so they read as distinct from LXMF peers
          // at a glance (they don't behave like one - see
          // messenger_bookmark_is_prop_node()'s own comment, Messenger.h).
          if (msngr_bookmarks[i].type == MSNGR_BOOKMARK_TYPE_PROPAGATION) {
            icons[vis] = bm_menu_icon_msngr_prop_node;
            icon_widths[vis] = MENU_ICON_W_MSNGR_PROP_NODE;
          } else {
            icons[vis] = bm_menu_icon_msngr_node;
            icon_widths[vis] = MENU_ICON_W_MSNGR_NODE;
          }
          vis++;
        }
        if (msngr_bookmark_count < MSNGR_MAX_BOOKMARKS) {
          labels[vis] = "Add by Hash";
          valbufs[vis][0] = 0;
          icons[vis] = bm_menu_icon_msngr_add_by_hash;
          icon_widths[vis] = MENU_ICON_W_MSNGR_ADD_BY_HASH;
          vis++;
        }
        labels[row_count - 1] = "BACK";
        valbufs[row_count - 1][0] = 0;

        // icon_col_shared=false - only the bookmark/Add-by-Hash rows opted
        // into icons[], BACK keeps its own auto-icon path (draw_menu_list_
        // disp()'s own comment), same shape as MENU_STATE_MSNGR_INBOX above.
        draw_menu_list_disp("BOOKMARKS", labels, valbufs, row_count, msngr_bookmarks_cursor, icons, icon_widths, nullptr, false);
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
      } else if (menu_state == MENU_STATE_MSNGR_PEER && messenger_bookmark_is_prop_node(msngr_active_peer_hash)) {
        // Same fixed action set as msngr_peer_row_count()'s own
        // is_prop_node branch - no message cache to draw at all (a
        // propagation node bookmark never has a conversation).
        const char *labels[MSNGR_PEER_PROP_ACTION_COUNT];
        char valbufs[MSNGR_PEER_PROP_ACTION_COUNT][24];
        // Every row now has an icon - Ping/Show Hash/Remove Bookmark reuse
        // MENU_STATE_MSNGR_PEER's own LXMF-peer action icons for the same
        // concept, Rename reuses Compose message's (per user request - both
        // are "type some text for this peer"), Back gets the same arrow
        // every other explicit icon table's BACK row does (icon_col_shared
        // == true here bypasses draw_menu_list_disp()'s own BACK auto-icon
        // detection - see that function's own comment - so it has to be set
        // explicitly like every other row). Sync/Set-Active get their own
        // dedicated glyphs below.
        const uint8_t *icons[MSNGR_PEER_PROP_ACTION_COUNT] = { nullptr };
        uint8_t icon_widths[MSNGR_PEER_PROP_ACTION_COUNT] = { 0 };

        labels[MSNGR_PEER_PROP_ACTION_SYNC] = "Sync";
        valbufs[MSNGR_PEER_PROP_ACTION_SYNC][0] = 0;
        icons[MSNGR_PEER_PROP_ACTION_SYNC] = bm_menu_icon_msngr_prop_sync;
        icon_widths[MSNGR_PEER_PROP_ACTION_SYNC] = MENU_ICON_W_MSNGR_PROP_SYNC;
        labels[MSNGR_PEER_PROP_ACTION_PING] = "Ping";
        valbufs[MSNGR_PEER_PROP_ACTION_PING][0] = 0;
        icons[MSNGR_PEER_PROP_ACTION_PING] = bm_menu_icon_msngr_ping;
        icon_widths[MSNGR_PEER_PROP_ACTION_PING] = MENU_ICON_W_MSNGR_PING;
        labels[MSNGR_PEER_PROP_ACTION_SHOW_HASH] = "Show Hash";
        valbufs[MSNGR_PEER_PROP_ACTION_SHOW_HASH][0] = 0;
        icons[MSNGR_PEER_PROP_ACTION_SHOW_HASH] = bm_menu_icon_msngr_add_by_hash;
        icon_widths[MSNGR_PEER_PROP_ACTION_SHOW_HASH] = MENU_ICON_W_MSNGR_ADD_BY_HASH;
        bool is_active = messenger_prop_node_is_active(msngr_active_peer_hash);
        labels[MSNGR_PEER_PROP_ACTION_SET_ACTIVE] = is_active ? "Unset Active" : "Set Active";
        valbufs[MSNGR_PEER_PROP_ACTION_SET_ACTIVE][0] = 0;
        icons[MSNGR_PEER_PROP_ACTION_SET_ACTIVE] = is_active ? bm_menu_icon_msngr_prop_active : bm_menu_icon_msngr_prop_inactive;
        icon_widths[MSNGR_PEER_PROP_ACTION_SET_ACTIVE] = is_active ? MENU_ICON_W_MSNGR_PROP_ACTIVE : MENU_ICON_W_MSNGR_PROP_INACTIVE;
        labels[MSNGR_PEER_PROP_ACTION_RENAME] = "Rename";
        valbufs[MSNGR_PEER_PROP_ACTION_RENAME][0] = 0;
        icons[MSNGR_PEER_PROP_ACTION_RENAME] = bm_menu_icon_msngr_compose;
        icon_widths[MSNGR_PEER_PROP_ACTION_RENAME] = MENU_ICON_W_MSNGR_COMPOSE;
        labels[MSNGR_PEER_PROP_ACTION_REMOVE] = "Remove Bookmark";
        valbufs[MSNGR_PEER_PROP_ACTION_REMOVE][0] = 0;
        icons[MSNGR_PEER_PROP_ACTION_REMOVE] = bm_menu_icon_msngr_remove_bookmark;
        icon_widths[MSNGR_PEER_PROP_ACTION_REMOVE] = MENU_ICON_W_MSNGR_REMOVE_BOOKMARK;
        labels[MSNGR_PEER_PROP_ACTION_BACK] = "BACK";
        valbufs[MSNGR_PEER_PROP_ACTION_BACK][0] = 0;
        icons[MSNGR_PEER_PROP_ACTION_BACK] = bm_menu_icon_back;
        icon_widths[MSNGR_PEER_PROP_ACTION_BACK] = MENU_ICON_W_BACK;

        char title[24];
        snprintf(title, sizeof(title), "%s", messenger_peer_display_name(msngr_active_peer_hash).c_str());
        draw_menu_list_disp(title, labels, valbufs, MSNGR_PEER_PROP_ACTION_COUNT, msngr_peer_cursor, icons, icon_widths);
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
        // Message-row-only nudges (space-saving: pull each direction's
        // icon+text pair closer to its own screen edge) - zeroed for the
        // preset/action rows below, which keep draw_menu_list_disp()'s
        // plain default positions.
        int8_t icon_dx[MSNGR_PEER_MAX_ROWS] = { 0 };
        int8_t text_dx[MSNGR_PEER_MAX_ROWS] = { 0 };
        int8_t right_dx[MSNGR_PEER_MAX_ROWS] = { 0 };

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
        // Marquee-scroll the currently selected message row's snippet -
        // msngr_peer_cache[].snippet (Messenger.h) can hold up to
        // MSNGR_PEER_SNIPPET_CAP characters, far more than the fixed
        // MSNGR_PEER_SCROLL_WINDOW that fits in one row - sliding that
        // window across it one character every MSNGR_PEER_SCROLL_STEP_MS
        // reveals the rest of a long message without needing to open
        // MSG_DETAIL. Only the row the cursor is actually on scrolls (every
        // other row keeps the plain window starting at 0, same as before
        // this feature existed). Per user request, a fully-scrolled marquee
        // now loops: once the tail is reached, it holds there for MSNGR_
        // PEER_SCROLL_LOOP_PAUSE_MS, then jumps back to the start and
        // scrolls again, for as long as the row stays selected. Reset (not
        // just clamped back into bounds) the instant the cursor lands on a
        // different row - re-selecting a row always restarts its scroll
        // from the beginning, never resumes mid-way or mid-pause.
        if (msngr_peer_scroll_row != msngr_peer_cursor) {
          msngr_peer_scroll_row = msngr_peer_cursor;
          msngr_peer_scroll_offset = 0;
          msngr_peer_scroll_last_step_ms = millis();
          msngr_peer_scroll_paused_since_ms = 0;
          msngr_peer_scroll_start_pause_until_ms = 0;
        }

        for (uint8_t i = 0; i < msg_rows; i++) {
          const char *full = msngr_peer_cache[i].snippet;
          size_t full_len = strlen(full);
          char windowed[24];
          // Per user request, the cursor-selected row's own marquee is
          // always on, exempt from blekbd_marquee_enabled entirely - the
          // toggle only matters where there's a "background/other rows"
          // scroll to turn off (MENU_STATE_MSNGR_CHAT, below), which this
          // screen doesn't have (only ever the one selected row).
          if (i == msngr_peer_cursor && full_len > MSNGR_PEER_SCROLL_WINDOW) {
            uint8_t max_offset = (uint8_t)(full_len - MSNGR_PEER_SCROLL_WINDOW);
            unsigned long now = millis();
            if (msngr_peer_scroll_offset >= max_offset) {
              if (msngr_peer_scroll_paused_since_ms == 0) {
                msngr_peer_scroll_paused_since_ms = now;
              } else if ((int32_t)(now - msngr_peer_scroll_paused_since_ms) >= (int32_t)MSNGR_PEER_SCROLL_LOOP_PAUSE_MS) {
                // Loop back to the start - per user request, pause here
                // too (MSNGR_PEER_SCROLL_START_PAUSE_MS) before actually
                // scrolling again, so the beginning is readable.
                msngr_peer_scroll_offset = 0;
                msngr_peer_scroll_paused_since_ms = 0;
                msngr_peer_scroll_start_pause_until_ms = now + MSNGR_PEER_SCROLL_START_PAUSE_MS;
                msngr_peer_scroll_last_step_ms = now;
              }
            } else if (msngr_peer_scroll_start_pause_until_ms != 0) {
              // Still in the post-loop start-pause window - hold at 0
              // (not a row's very first scroll, which never arms this).
              if ((int32_t)(now - msngr_peer_scroll_start_pause_until_ms) >= 0) {
                msngr_peer_scroll_start_pause_until_ms = 0;
                msngr_peer_scroll_last_step_ms = now;
              }
            } else if ((int32_t)(now - msngr_peer_scroll_last_step_ms) >= (int32_t)MSNGR_PEER_SCROLL_STEP_MS) {
              msngr_peer_scroll_last_step_ms = now;
              msngr_peer_scroll_offset++;
            }
            snprintf(windowed, sizeof(windowed), "%s", full + msngr_peer_scroll_offset);
          } else {
            snprintf(windowed, sizeof(windowed), "%s", full);
          }

          if (msngr_peer_cache[i].incoming) {
            snprintf(label_bufs[i], sizeof(label_bufs[i]), "%s", windowed);
            labels[i] = label_bufs[i];
            valbufs[i][0] = 0;
            icons[i] = bm_menu_icon_msngr_msg_incoming;
            icon_widths[i] = MENU_ICON_W_MSNGR_MSG_INCOMING;
            icon_dx[i] = -2;
            text_dx[i] = -4;
          } else {
            labels[i] = "";
            snprintf(valbufs[i], 24, "%s", windowed);
            right_icons[i] = bm_menu_icon_msngr_msg_outgoing;
            right_icon_widths[i] = MENU_ICON_W_MSNGR_MSG_OUTGOING;
            right_dx[i] = 2;
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

        bool is_propagated = messenger_current_delivery_mode(msngr_active_peer_hash) == MSNGR_DELIVERY_MODE_PROPAGATED;
        labels[fixed_base + MSNGR_PEER_FIXED_ACTION_DELIVERY_MODE] = is_propagated ? "Send Propagated" : "Send Direct";
        valbufs[fixed_base + MSNGR_PEER_FIXED_ACTION_DELIVERY_MODE][0] = 0;
        icons[fixed_base + MSNGR_PEER_FIXED_ACTION_DELIVERY_MODE] = is_propagated ? bm_menu_icon_msngr_prop_node : bm_menu_icon_msngr_direct;
        icon_widths[fixed_base + MSNGR_PEER_FIXED_ACTION_DELIVERY_MODE] = is_propagated ? MENU_ICON_W_MSNGR_PROP_NODE : MENU_ICON_W_MSNGR_DIRECT;

        bool is_bookmarked = messenger_bookmark_find(msngr_active_peer_hash) >= 0;
        labels[fixed_base + MSNGR_PEER_FIXED_ACTION_BOOKMARK] = is_bookmarked ? "Remove Bookmark" : "Add Bookmark";
        valbufs[fixed_base + MSNGR_PEER_FIXED_ACTION_BOOKMARK][0] = 0;
        icons[fixed_base + MSNGR_PEER_FIXED_ACTION_BOOKMARK] = is_bookmarked ? bm_menu_icon_msngr_remove_bookmark : bm_menu_icon_msngr_bookmarks;
        icon_widths[fixed_base + MSNGR_PEER_FIXED_ACTION_BOOKMARK] = is_bookmarked ? MENU_ICON_W_MSNGR_REMOVE_BOOKMARK : MENU_ICON_W_MSNGR_BOOKMARKS;

        labels[fixed_base + MSNGR_PEER_FIXED_ACTION_SHOW_HASH] = "Show Hash";
        valbufs[fixed_base + MSNGR_PEER_FIXED_ACTION_SHOW_HASH][0] = 0;
        icons[fixed_base + MSNGR_PEER_FIXED_ACTION_SHOW_HASH] = bm_menu_icon_msngr_add_by_hash;
        icon_widths[fixed_base + MSNGR_PEER_FIXED_ACTION_SHOW_HASH] = MENU_ICON_W_MSNGR_ADD_BY_HASH;

        labels[fixed_base + MSNGR_PEER_FIXED_ACTION_CLEAR] = "Clear Conversation";
        valbufs[fixed_base + MSNGR_PEER_FIXED_ACTION_CLEAR][0] = 0;
        icons[fixed_base + MSNGR_PEER_FIXED_ACTION_CLEAR] = bm_menu_icon_msngr_delete;
        icon_widths[fixed_base + MSNGR_PEER_FIXED_ACTION_CLEAR] = MENU_ICON_W_MSNGR_DELETE;

        #if HAS_BLE_HID_HOST == true
          // Renamed from "Chat" per user request - reuses the same
          // keyboard glyph BT_LIST's own "BLE Keyboard" row uses
          // (bm_menu_icon_blekbd), since this row's whole point is "compose
          // with the physical keyboard".
          labels[fixed_base + MSNGR_PEER_FIXED_ACTION_CHAT] = "Dialog Mode";
          valbufs[fixed_base + MSNGR_PEER_FIXED_ACTION_CHAT][0] = 0;
          icons[fixed_base + MSNGR_PEER_FIXED_ACTION_CHAT] = bm_menu_icon_blekbd;
          icon_widths[fixed_base + MSNGR_PEER_FIXED_ACTION_CHAT] = MENU_ICON_W_BLEKBD;
        #endif

        labels[fixed_base + MSNGR_PEER_FIXED_ACTION_BACK] = "BACK";
        valbufs[fixed_base + MSNGR_PEER_FIXED_ACTION_BACK][0] = 0;

        char title[24];
        snprintf(title, sizeof(title), "%s", messenger_peer_display_name(msngr_active_peer_hash).c_str());
        // icon_col_shared=false - only the message rows and the Compose
        // message/Ping/Send Direct-Propagated/Bookmark/Clear Conversation
        // action rows above opted into icons[]/right_icons[], the
        // remaining action rows stay at the plain x=8 they always used.
        draw_menu_list_disp(title, labels, valbufs, row_count, msngr_peer_cursor, icons, icon_widths, icon_dx, false, text_dx, right_icons, right_icon_widths, right_dx, msg_rows);
      } else if (menu_state == MENU_STATE_MSNGR_MSG_DETAIL && msngr_msg_view_active) {
        // draw_delete_confirm=false - this screen's own Delete row is a
        // full MENU_STATE_MSNGR_DELETE_CONFIRM screen, reached after
        // leaving this view, not an in-place overlay (draw_msngr_full_
        // message_view()'s own comment).
        draw_msngr_full_message_view(false);
      } else if (menu_state == MENU_STATE_MSNGR_MSG_DETAIL) {
        uint8_t row_count = msngr_msg_detail_row_count();
        // +4, not +1 - content lines plus the trailing Reply, Delete,
        // Full Message and BACK rows.
        const char *labels[MSNGR_MSG_DETAIL_MAX_LINES + 5];
        char valbufs[MSNGR_MSG_DETAIL_MAX_LINES + 5][24];
        const uint8_t *icons[MSNGR_MSG_DETAIL_MAX_LINES + 5] = { nullptr };
        uint8_t icon_widths[MSNGR_MSG_DETAIL_MAX_LINES + 5] = { 0 };
        // Per user request: the content-preview rows lose their previous
        // 8px left padding (draw_menu_list_disp()'s own "plain x=8"
        // default for icon-less rows) to better use the screen width -
        // Reply/Delete/Full Message (icons[i] set, below) and BACK (its
        // own auto-icon path) are unaffected, left at 0/default.
        int8_t text_dx[MSNGR_MSG_DETAIL_MAX_LINES + 5] = { 0 };

        // msngr_msg_detail_wrapped_lines (populated by msngr_msg_detail_
        // refresh_wrap_cache() when the message row was selected, above) -
        // points labels[] straight at the cached std::strings rather than
        // re-chopping msngr_msg_detail_cache_content here, same "don't
        // recompute from flash-backed content on every draw" reasoning as
        // MENU_STATE_MSNGR_PEER above.
        const uint8_t voice_rows = msngr_msg_detail_has_audio ? 1 : 0;
        uint8_t lines = row_count - 4 - voice_rows; // content lines - [Play], Reply, Delete, Full Message, BACK are appended after
        for (uint8_t i = 0; i < lines; i++) {
          labels[i] = msngr_msg_detail_wrapped_lines[i].c_str();
          valbufs[i][0] = 0;
          // 8 -> 1, not all the way to 0 (MENU_CONTENT_X) - per user
          // feedback, flush against the physical edge looked too tight;
          // a 1px margin reads better while still reclaiming almost all
          // of the previous 8px padding.
          text_dx[i] = -7;
        }

        #if HAS_AUDIO == true
        if (voice_rows) {
          AudioState as = audio_state();
          if (as == AUDIO_IDLE) {
            labels[lines] = "Play";
            valbufs[lines][0] = 0;
          } else if (as == AUDIO_DECODING) {
            labels[lines] = "Stop";
            snprintf(valbufs[lines], sizeof(valbufs[lines]), "decoding");
          } else {
            labels[lines] = "Stop";
            snprintf(valbufs[lines], sizeof(valbufs[lines]), "%u/%us",
                     (unsigned)(audio_play_elapsed_ms() / 1000), (unsigned)((audio_play_total_ms() + 500) / 1000));
          }
        }
        #endif
        const uint8_t t = lines + voice_rows;
        labels[t] = "Reply";
        valbufs[t][0] = 0;
        icons[t] = bm_menu_icon_msngr_reply;
        icon_widths[t] = MENU_ICON_W_MSNGR_REPLY;
        labels[t + 1] = "Delete";
        valbufs[t + 1][0] = 0;
        icons[t + 1] = bm_menu_icon_msngr_delete;
        icon_widths[t + 1] = MENU_ICON_W_MSNGR_DELETE;
        // Per user request: right under Delete, above BACK.
        labels[t + 2] = "Full Message";
        valbufs[t + 2][0] = 0;
        icons[t + 2] = bm_menu_icon_msngr_full_message;
        icon_widths[t + 2] = MENU_ICON_W_MSNGR_FULL_MESSAGE;
        labels[t + 3] = "BACK";
        valbufs[t + 3][0] = 0;

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

        // icon_col_shared=false - only the Reply/Delete/Full Message rows
        // above opted into icons[], BACK keeps its own auto-icon path;
        // text_dx pulls just the content lines left (their own comment,
        // above). separator_before=lines - same "divide content from
        // commands" dashed rule as MENU_STATE_MSNGR_PEER's own boundary
        // between messages and Compose message/Ping/.../BACK.
        draw_menu_list_disp(title, labels, valbufs, row_count, msngr_msg_detail_cursor, icons, icon_widths, nullptr, false, text_dx, nullptr, nullptr, nullptr, lines);
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
      }
      #if HAS_BLE_HID_HOST == true
        else if (menu_state == MENU_STATE_MSNGR_CHAT) {
          // Same live-refresh-on-redraw idea as MENU_STATE_MSNGR_PEER's own
          // draw block - picks up a message that arrived while this screen
          // is sitting open - but against msngr_chat_cache/window_start
          // (Messenger.h), Chat's own separate, chronologically-windowed
          // cache, not msngr_peer_cache.
          messenger_refresh_chat_window_if_stale(msngr_active_peer_hash);

          // Per user request, an idle selection auto-deselects back to the
          // compose box after MSNGR_CHAT_SEL_TIMEOUT_MS with no navigation
          // activity - same effect as pressing Esc. Not gated on the
          // delete-confirm dialog being closed - if it's open, msngr_chat_
          // sel_last_activity_ms hasn't moved either (no nav key reaches
          // this far while it's pending), so both time out together.
          if (msngr_chat_sel != 0xFF &&
              (int32_t)(millis() - msngr_chat_sel_last_activity_ms) >= (int32_t)MSNGR_CHAT_SEL_TIMEOUT_MS) {
            msngr_chat_sel = 0xFF;
            msngr_chat_delete_confirm_pending = false;
          }

          MENU_GFX.setFont(MENU_FONT);
          MENU_GFX.setTextSize(1);

          if (msngr_msg_view_active) {
            // draw_delete_confirm=true - Chat's own Backspace-opens-
            // delete-confirm overlay (draw_msngr_full_message_view()'s
            // own comment).
            draw_msngr_full_message_view(true);
            return;
          }

          // No title, no header hline, no footer hint - deliberately, this
          // is the entire point of this screen vs. MENU_STATE_MSNGR_PEER/
          // _TEXT_ENTRY. Compose box pinned to the bottom (1px margin from
          // the physical screen edge); the 5 message rows fill whatever's
          // left above it. Starting estimate, like every other pixel value
          // in this file - real tuning happens live on hardware.
          const uint8_t row_h = 9;
          // 1px up from this screen's original baseline (7) - per user
          // request on real hardware, text sat flush with the bottom of
          // the selection highlight otherwise.
          const int16_t baseline_off = 6;
          const uint8_t compose_line_h = 9;
          const int16_t box_y = 64 - (int16_t)compose_line_h - 1;

          for (uint8_t i = 0; i < msngr_chat_cache_count; i++) {
            // Per user request, anchored to the BOTTOM (right above the
            // compose box) and growing upward, same as any real chat app -
            // the most recent loaded row (msngr_chat_cache_count-1) always
            // sits immediately above the compose box, not stranded at the
            // very top of the screen with a big gap underneath whenever
            // fewer than 5 messages are loaded (the previous top-anchored
            // math's actual bug).
            int16_t row_top = box_y - (int16_t)(msngr_chat_cache_count - i) * row_h;
            int16_t y = row_top + baseline_off;
            bool selected = (i == msngr_chat_sel);
            if (selected) {
              MENU_GFX.fillRect(MENU_CONTENT_X, row_top, MENU_CONTENT_W, row_h - 1, SSD1306_WHITE);
              MENU_GFX.setTextColor(SSD1306_BLACK);
            } else {
              MENU_GFX.setTextColor(SSD1306_WHITE);
            }

            // Per user request: incoming stays left-aligned with its
            // right-pointing arrow on the left; outgoing is now right-
            // aligned instead, with its left-pointing arrow on the right -
            // same "yours on the right" chat convention MSNGR_PEER's own
            // rows already use, just with an icon instead of the old plain
            // left/right text alignment alone. Icon position is fixed
            // either way; text position (and, for outgoing, its width) is
            // resolved after the marquee window is computed below, since
            // right-alignment needs to know how wide the windowed text
            // actually is.
            bool incoming = msngr_chat_cache[i].incoming;
            const uint8_t *dir_icon = incoming ? bm_menu_icon_right_arrow : bm_menu_icon_left_arrow;
            int16_t icon_y = row_top + (row_h - MENU_ICON_W_LEFT_ARROW) / 2;
            int16_t icon_x = incoming ? (MENU_CONTENT_X + 1) : (MENU_CONTENT_X + MENU_CONTENT_W - 1 - MENU_ICON_W_LEFT_ARROW);
            MENU_GFX.drawBitmap(icon_x, icon_y, dir_icon, MENU_ICON_W_LEFT_ARROW, MENU_ICON_W_LEFT_ARROW,
              selected ? SSD1306_BLACK : SSD1306_WHITE, selected ? SSD1306_WHITE : SSD1306_BLACK);

            // msngr_chat_cache[i].snippet is already decoded into this
            // device's internal single-byte glyph codes at cache-
            // population time (messenger_refresh_chat_window(), Messenger.
            // h) - printable as-is. Calling msngr_kb_decode_utf8() on it
            // here too double-decoded it as if it were still raw UTF-8,
            // corrupting every Cyrillic character into '?' (confirmed on
            // hardware, MSNGR_PEER's own draw block hit the same bug).
            //
            // Marquee-scroll, same mechanism as MSNGR_PEER's own
            // (MSNGR_PEER_SCROLL_STEP_MS/_WINDOW/_LOOP_PAUSE_MS, above).
            // Per user request: the cursor-highlighted row (i ==
            // msngr_chat_sel) always scrolls, exempt from blekbd_marquee_
            // enabled entirely - same "selected row's own marquee is
            // always on" rule MSNGR_PEER's own code above now follows too.
            // Every OTHER overflowing row also scrolls, but only while
            // blekbd_marquee_enabled is on (Play/Pause key, blekbd_
            // toggle_marquee() above) - that's the "general marquee" the
            // toggle actually controls here. Doesn't touch selection/
            // highlighting either way. Still keyed per-row (msngr_chat_
            // scroll_offset[]/_last_step_ms[]/_paused_since_ms[],
            // Messenger.h, one slot per visible row). Loops - see MSNGR_
            // PEER's own marquee logic above for the identical pause-
            // then-restart shape.
            const char *full = msngr_chat_cache[i].snippet;
            size_t full_len = strlen(full);
            char windowed[24];
            if ((i == msngr_chat_sel || blekbd_marquee_enabled) && full_len > MSNGR_PEER_SCROLL_WINDOW) {
              uint8_t max_offset = (uint8_t)(full_len - MSNGR_PEER_SCROLL_WINDOW);
              unsigned long now = millis();
              if (msngr_chat_scroll_offset[i] >= max_offset) {
                if (msngr_chat_scroll_paused_since_ms[i] == 0) {
                  msngr_chat_scroll_paused_since_ms[i] = now;
                } else if ((int32_t)(now - msngr_chat_scroll_paused_since_ms[i]) >= (int32_t)MSNGR_PEER_SCROLL_LOOP_PAUSE_MS) {
                  // Loop back to the start - per user request, pause here
                  // too (MSNGR_PEER_SCROLL_START_PAUSE_MS) before actually
                  // scrolling again, so the beginning is readable.
                  msngr_chat_scroll_offset[i] = 0;
                  msngr_chat_scroll_paused_since_ms[i] = 0;
                  msngr_chat_scroll_start_pause_until_ms[i] = now + MSNGR_PEER_SCROLL_START_PAUSE_MS;
                  msngr_chat_scroll_last_step_ms[i] = now;
                }
              } else if (msngr_chat_scroll_start_pause_until_ms[i] != 0) {
                // Still in the post-loop start-pause window - hold at 0
                // (not a row's very first scroll, which never arms this).
                if ((int32_t)(now - msngr_chat_scroll_start_pause_until_ms[i]) >= 0) {
                  msngr_chat_scroll_start_pause_until_ms[i] = 0;
                  msngr_chat_scroll_last_step_ms[i] = now;
                }
              } else if ((int32_t)(now - msngr_chat_scroll_last_step_ms[i]) >= (int32_t)MSNGR_PEER_SCROLL_STEP_MS) {
                msngr_chat_scroll_last_step_ms[i] = now;
                msngr_chat_scroll_offset[i]++;
              }
              snprintf(windowed, sizeof(windowed), "%s", full + msngr_chat_scroll_offset[i]);
            } else {
              snprintf(windowed, sizeof(windowed), "%s", full);
            }

            int16_t text_x;
            if (incoming) {
              text_x = MENU_CONTENT_X + 1 + MENU_ICON_W_LEFT_ARROW + 2;
            } else {
              int16_t x1, y1; uint16_t tw, th;
              MENU_GFX.getTextBounds(windowed, 0, 0, &x1, &y1, &tw, &th);
              text_x = icon_x - 2 - (int16_t)tw;
            }
            MENU_GFX.setCursor(text_x, y);
            MENU_GFX.print(windowed);
          }
          MENU_GFX.setTextColor(SSD1306_WHITE);

          draw_msngr_compose_box(box_y, (int32_t)msngr_chat_cursor);

          if (msngr_chat_delete_confirm_pending) draw_msngr_chat_delete_confirm_box();
        }
      #endif
      else if (menu_state == MENU_STATE_MSNGR_PING_RESULT) {
        // Reads msngr_ping_state/msngr_ping_rtt fresh on every redraw -
        // messenger_ping_process() (RNode_Firmware.ino's loop()) is what
        // actually advances them, same "read live state, don't poll from
        // here" split as every other MSNGR screen's draw code.
        const char *labels[2];
        char valbufs[2][24];
        char status_buf[24];
        // Per user request: a checkmark/X icon prefixes the status row on
        // a terminal outcome - success (bm_menu_icon_msngr_ping_ok) or any
        // of the three failure states (bm_menu_icon_msngr_ping_fail).
        // RESOLVING/ESTABLISHING are still in-progress, no outcome yet, so
        // neither icon applies - left nullptr, same "no icon" default
        // every other icon-less row here already uses.
        const uint8_t *icons[2] = { nullptr, nullptr };
        uint8_t icon_widths[2] = { 0, 0 };
        switch (msngr_ping_state) {
          case MSNGR_PING_RESOLVING:    snprintf(status_buf, sizeof(status_buf), "Resolving..."); break;
          case MSNGR_PING_ESTABLISHING: snprintf(status_buf, sizeof(status_buf), "Pinging..."); break;
          case MSNGR_PING_SUCCESS:
            snprintf(status_buf, sizeof(status_buf), "RTT: %.0f ms", msngr_ping_rtt * 1000.0);
            icons[0] = bm_menu_icon_msngr_ping_ok;
            icon_widths[0] = MENU_ICON_W_MSNGR_PING_OK;
            break;
          case MSNGR_PING_TIMEOUT:      snprintf(status_buf, sizeof(status_buf), "Timed Out"); break;
          case MSNGR_PING_NO_IDENTITY:  snprintf(status_buf, sizeof(status_buf), "Not Ready"); break;
          case MSNGR_PING_FAILED:       snprintf(status_buf, sizeof(status_buf), "Link Failed"); break;
          default:                      snprintf(status_buf, sizeof(status_buf), "..."); break;
        }
        if (msngr_ping_state == MSNGR_PING_TIMEOUT || msngr_ping_state == MSNGR_PING_NO_IDENTITY || msngr_ping_state == MSNGR_PING_FAILED) {
          icons[0] = bm_menu_icon_msngr_ping_fail;
          icon_widths[0] = MENU_ICON_W_MSNGR_PING_FAIL;
        }
        labels[0] = status_buf;
        valbufs[0][0] = 0;
        labels[1] = "BACK";
        valbufs[1][0] = 0;
        // MENU_ROW_TEXT_X_ICONS's fixed column only clears the narrower
        // ping-ok icon (13px) with room to spare - the wider ping-fail
        // icon (18px) ran into the label text. Push the label right by
        // the excess over the ok icon's width so both icons keep the
        // same ~1px gap to the label.
        const int8_t text_dx[2] = { (int8_t)(icon_widths[0] > MENU_ICON_W_MSNGR_PING_OK ? icon_widths[0] - MENU_ICON_W_MSNGR_PING_OK : 0), 0 };

        char title[24];
        snprintf(title, sizeof(title), "PING: %s", messenger_peer_display_name(msngr_active_peer_hash).c_str());
        draw_menu_list_disp(title, labels, valbufs, 2, msngr_ping_result_cursor, icons, icon_widths, nullptr, false, text_dx);
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
        const char *labels[4];
        char valbufs[4][24];
        char status_buf[24];
        // Per user request: same OK/FAIL icons as MSNGR_PING_RESULT above -
        // OK on confirmed delivery, FAIL on any of the three terminal
        // failure/uncertain-failure states. RESOLVING/PENDING are still
        // in-progress, no outcome yet, left nullptr (no icon), same
        // reasoning as Ping's own RESOLVING/ESTABLISHING. BACK never
        // carries an icon; Retry/Retry via Prop do (below).
        const uint8_t *icons[4] = { nullptr, nullptr, nullptr, nullptr };
        uint8_t icon_widths[4] = { 0, 0, 0, 0 };
        switch (msngr_send_state) {
          case MSNGR_SEND_RESOLVING:  snprintf(status_buf, sizeof(status_buf), "Resolving..."); break;
          case MSNGR_SEND_PENDING: {
            // msngr_send_method - which method LXMessage::pack() actually
            // resolved this send to (see its own declaration, Messenger.h) -
            // OPPORTUNISTIC vs the silent DIRECT upgrade for anything over
            // LORA_ENCRYPTED_PACKET_MDU (Send Direct), or PROPAGATED
            // (Send Propagated, MSNGR_PEER_FIXED_ACTION_DELIVERY_MODE) -
            // not just always "Sending...".
            const char *method_name = (msngr_send_method == LXMF::Type::Message::DIRECT) ? "Direct" :
                                       (msngr_send_method == LXMF::Type::Message::PROPAGATED) ? "Propagated" : "Opportunistic";
            // FIXED (local patch, not upstream): msngr_send_router_state
            // (Messenger.h, polled from LXMRouter::pending_outbound_state_
            // for()) now lets this distinguish "packet handed to the radio,
            // waiting on delivery proof" (SENT) from "still trying to get
            // it onto the radio" (OUTBOUND/SENDING) - previously both
            // looked identical on this screen. Kept to one extra phrase
            // ("Awaiting proof") rather than combining phase+method+
            // attempt in one string - status_buf is only 24 bytes and
            // "Transmitting Opportunistic (2/5)" alone would overflow it.
            // Per user request: report the proof-of-work stamp grind
            // explicitly rather than leaving it indistinguishable from
            // "Sending Propagated" - LXStamper's own worst case is ~2
            // minutes (send_propagated()'s comment, LXMRouter.cpp), so
            // without this a stalled-looking send is actually still
            // working. Checked ahead of the SENT/attempt-count branches
            // below - a message can only be mid-stamp before its resource
            // transfer even starts, so this and those are never both true
            // at once, but the ordering documents that priority anyway.
            // Checked ahead of the stamp/SENT/attempt-count branches below -
            // send_propagated() (LXMRouter.cpp) only starts the stamp grind
            // or the actual resource transfer once its own Link to the PN
            // is ACTIVE, so this and those are never both true at once, but
            // the ordering documents that priority anyway. PROPAGATED-only:
            // DIRECT/OPPORTUNISTIC establish their link to the peer itself,
            // not _outbound_propagation_link, so this would always read
            // false for them regardless, but gate explicitly for clarity.
            if (msngr_send_method == LXMF::Type::Message::PROPAGATED &&
                urns_lxmf_router && urns_lxmf_router->is_outbound_propagation_link_establishing()) {
              icons[0] = bm_menu_icon_msngr_waiting;
              icon_widths[0] = MENU_ICON_W_MSNGR_WAITING;
              snprintf(status_buf, sizeof(status_buf), "Establishing Link");
            } else if (urns_lxmf_router && urns_lxmf_router->pending_outbound_stamp_running_for(msngr_send_message_hash)) {
              icons[0] = bm_menu_icon_msngr_waiting;
              icon_widths[0] = MENU_ICON_W_MSNGR_WAITING;
              snprintf(status_buf, sizeof(status_buf), "Generating Stamp (%u)", (unsigned)urns_lxmf_router->pending_outbound_stamp_cost_for(msngr_send_message_hash));
            } else if (msngr_send_router_state == LXMF::Type::Message::SENT) {
              // Same left-prefix icon convention as the OK/FAIL icons
              // below (bm_menu_icon_msngr_ping_ok/_fail) - narrower than
              // both (9px vs 13/18px) so the text_dx formula below (tuned
              // against MENU_ICON_W_MSNGR_PING_OK) needs no adjustment.
              icons[0] = bm_menu_icon_msngr_waiting;
              icon_widths[0] = MENU_ICON_W_MSNGR_WAITING;
              if (msngr_send_attempt > 1) {
                snprintf(status_buf, sizeof(status_buf), "Awaiting Proof (%u/%u)", (unsigned)msngr_send_attempt, (unsigned)msngr_max_retries);
              } else {
                snprintf(status_buf, sizeof(status_buf), "Awaiting Proof");
              }
            } else if (msngr_send_method != LXMF::Type::Message::PROPAGATED && msngr_send_attempt > 1) {
              // Only show the attempt count once a retry has actually
              // started (msngr_send_attempt > 1, set by messenger_send_
              // process()'s live poll of the router's own delivery_
              // attempts() - see msngr_send_attempt's own declaration,
              // Messenger.h) - keeps the common first-try case uncluttered.
              // Drops the "Sending" prefix in that case to leave room for
              // the attempt count within status_buf's 24-byte budget.
              // PROPAGATED excluded (falls through to the plain "Sending
              // Propagated" branch below) - unlike DIRECT/OPPORTUNISTIC,
              // its delivery_attempts() also counts every process_outbound()
              // cycle spent waiting on path/link establishment (see this
              // method's own increment_delivery_attempts() call site,
              // LXMRouter.cpp), not just genuine resource-retry attempts,
              // so the count doesn't mean what it looks like it means here -
              // confirmed on hardware: "3/5" shown with only 1 real
              // low-level retry observed in the propagation node's own log.
              snprintf(status_buf, sizeof(status_buf), "%s (%u/%u)", method_name, (unsigned)msngr_send_attempt, (unsigned)msngr_max_retries);
            } else {
              snprintf(status_buf, sizeof(status_buf), "Sending %s", method_name);
            }
            break;
          }
          case MSNGR_SEND_DELIVERED:
            snprintf(status_buf, sizeof(status_buf), "Delivered");
            icons[0] = bm_menu_icon_msngr_ping_ok;
            icon_widths[0] = MENU_ICON_W_MSNGR_PING_OK;
            break;
          // PROPAGATED-only success (messenger_on_sent(), Messenger.h) -
          // reached the active propagation node, not the final recipient
          // (MSNGR_SEND_SENT_TO_NODE's own comment) - worded distinctly
          // from "Delivered" so it isn't read as end-to-end confirmation.
          case MSNGR_SEND_SENT_TO_NODE:
            snprintf(status_buf, sizeof(status_buf), "Sent to Node");
            icons[0] = bm_menu_icon_msngr_ping_ok;
            icon_widths[0] = MENU_ICON_W_MSNGR_PING_OK;
            break;
          case MSNGR_SEND_TIMEOUT:    snprintf(status_buf, sizeof(status_buf), "No Confirmation"); break;
          case MSNGR_SEND_UNRESOLVED: snprintf(status_buf, sizeof(status_buf), "Unknown Destination"); break;
          // Router-confirmed failure (messenger_on_failed(), Messenger.h) -
          // exhausted delivery_attempts() and gave up for good. Distinct
          // from MSNGR_SEND_TIMEOUT (this screen's own blind guess-timeout).
          case MSNGR_SEND_FAILED:     snprintf(status_buf, sizeof(status_buf), "Delivery Failed"); break;
          default:                    snprintf(status_buf, sizeof(status_buf), "..."); break;
        }
        bool send_failed = msngr_send_result_failed();
        // Still occupying the front of the router's own outbound queue,
        // silently retrying in the background even though this screen has
        // already given up waiting - see this bool's own declaration. While
        // true, Retry/Retry via Prop would just queue uselessly behind the
        // still-active original send (LXMRouter.cpp's single-in-flight
        // process_outbound()), so they're withheld the same way they are
        // for an in-flight/successful send - row_count()'s own comment.
        bool queue_busy = send_failed && msngr_send_result_queue_busy();
        if (send_failed) {
          // Waiting icon (not the fail icon) while queue_busy - visually
          // distinguishes "still working in the background" from "done,
          // your move" even though status_buf itself keeps saying No
          // Confirmation/Delivery Failed either way.
          if (queue_busy) {
            icons[0] = bm_menu_icon_msngr_waiting;
            icon_widths[0] = MENU_ICON_W_MSNGR_WAITING;
          } else {
            icons[0] = bm_menu_icon_msngr_ping_fail;
            icon_widths[0] = MENU_ICON_W_MSNGR_PING_FAIL;
          }
        }
        labels[0] = status_buf;
        valbufs[0][0] = 0;
        uint8_t row_count = msngr_send_result_row_count();
        if (row_count == 4) {
          labels[1] = "Retry";
          valbufs[1][0] = 0;
          icons[1] = bm_menu_icon_msngr_retry;
          icon_widths[1] = MENU_ICON_W_MSNGR_RETRY;
          // Forces PROPAGATED for this one send regardless of the peer's
          // own Send Direct/Send Propagated setting - see messenger_send_
          // lxmf()'s forced_method parameter, Messenger.h. Reuses the same
          // icon MSNGR_PEER_FIXED_ACTION_DELIVERY_MODE's "Send Propagated"
          // label uses above.
          labels[2] = "Retry via Prop";
          valbufs[2][0] = 0;
          icons[2] = bm_menu_icon_msngr_prop_node;
          icon_widths[2] = MENU_ICON_W_MSNGR_PROP_NODE;
          labels[3] = "BACK";
          valbufs[3][0] = 0;
        } else {
          labels[1] = "BACK";
          valbufs[1][0] = 0;
        }
        // Same fixed-column overlap fix as MSNGR_PING_RESULT above - the
        // wider ping-fail icon needs the label pushed right a bit further
        // than the narrower ping-ok icon does. The waiting icon (queue_busy)
        // is narrower than MENU_ICON_W_MSNGR_PING_OK, so this formula
        // already yields 0 for it - no separate case needed.
        const int8_t text_dx[4] = { (int8_t)(icon_widths[0] > MENU_ICON_W_MSNGR_PING_OK ? icon_widths[0] - MENU_ICON_W_MSNGR_PING_OK : 0), 0, 0, 0 };

        // No footer_override needed here - draw_menu_list_disp() already
        // shows the router's own live retry progress in the footer
        // whenever the outbound queue is occupied at all (its own comment),
        // which covers this screen automatically along with every other
        // list screen.
        char title[24];
        snprintf(title, sizeof(title), "SEND: %s", messenger_peer_display_name(msngr_active_peer_hash).c_str());
        draw_menu_list_disp(title, labels, valbufs, row_count, msngr_send_result_cursor, icons, icon_widths, nullptr, false, text_dx);
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

        labels[MSNGR_SETTINGS_ITEM_PROP_ON_FAIL] = "Propagate on Fail";
        sprintf(valbufs[MSNGR_SETTINGS_ITEM_PROP_ON_FAIL], staged_msngr_propagate_on_fail ? "ON" : "OFF");

        labels[MSNGR_SETTINGS_ITEM_SYNC_INTERVAL] = "Periodic Sync";
        sprintf(valbufs[MSNGR_SETTINGS_ITEM_SYNC_INTERVAL], msngr_sync_interval_labels[staged_msngr_sync_interval_idx]);

        labels[MSNGR_SETTINGS_ITEM_SYNC_LIMIT] = "Sync Limit";
        if (staged_msngr_sync_limit == 0) sprintf(valbufs[MSNGR_SETTINGS_ITEM_SYNC_LIMIT], "Unlimited");
        else sprintf(valbufs[MSNGR_SETTINGS_ITEM_SYNC_LIMIT], "%u", (unsigned)staged_msngr_sync_limit);

        labels[MSNGR_SETTINGS_ITEM_STAMP_COST] = "Req. Stamp Cost";
        if (staged_msngr_stamp_cost == 0) sprintf(valbufs[MSNGR_SETTINGS_ITEM_STAMP_COST], "OFF");
        else sprintf(valbufs[MSNGR_SETTINGS_ITEM_STAMP_COST], "%u", (unsigned)staged_msngr_stamp_cost);

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
        } else if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_ANNOUNCE_INTERVAL) {
          sprintf(valbuf, msngr_announce_interval_labels[staged_msngr_announce_interval_idx]);
          draw_menu_edit_disp("AUTO ANNOUNCE", valbuf);
        } else if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_PROP_ON_FAIL) {
          sprintf(valbuf, staged_msngr_propagate_on_fail ? "ON" : "OFF");
          draw_menu_edit_disp("PROPAGATE ON FAIL", valbuf);
        } else if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_SYNC_INTERVAL) {
          sprintf(valbuf, msngr_sync_interval_labels[staged_msngr_sync_interval_idx]);
          draw_menu_edit_disp("PERIODIC SYNC", valbuf);
        } else if (msngr_settings_cursor == MSNGR_SETTINGS_ITEM_SYNC_LIMIT) {
          if (staged_msngr_sync_limit == 0) sprintf(valbuf, "Unlimited");
          else sprintf(valbuf, "%u", (unsigned)staged_msngr_sync_limit);
          draw_menu_edit_disp("SYNC LIMIT", valbuf);
        } else {
          if (staged_msngr_stamp_cost == 0) sprintf(valbuf, "OFF");
          else sprintf(valbuf, "%u", (unsigned)staged_msngr_stamp_cost);
          draw_menu_edit_disp("REQUIRED STAMP COST", valbuf);
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
    // Standalone (not part of the HAS_URNS-gated else-if chain above) -
    // MSNGR_TEXT_ENTRY/_DISCARD_CONFIRM need to be reachable on HAS_WIFI
    // boards that don't have HAS_URNS/HAS_LXMF too (WiFi SSID/PSK entry
    // reuses this same on-screen keyboard - see MSNGR_TEXT_ENTRY_PURPOSE_
    // WIFI_SSID/_WIFI_PSK).
    #if HAS_LXMF == true || HAS_WIFI == true
      if (menu_state == MENU_STATE_MSNGR_TEXT_ENTRY) {
        draw_menu_msngr_keyboard_disp();
      } else if (menu_state == MENU_STATE_MSNGR_DISCARD_CONFIRM) {
        const char *labels[2] = { "DISCARD", "CANCEL" };
        char valbufs[2][24];
        valbufs[0][0] = 0;
        valbufs[1][0] = 0;
        const char *discard_title = "DISCARD MSG?";
        #if HAS_LXMF == true
          if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH) discard_title = "DISCARD HASH?";
          else if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_RENAME) discard_title = "DISCARD NAME?";
          else if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_IDENTITY_RESTORE) discard_title = "DISCARD KEY?";
        #endif
        if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_WIFI_SSID) discard_title = "DISCARD SSID?";
        else if (msngr_text_entry_purpose == MSNGR_TEXT_ENTRY_PURPOSE_WIFI_PSK) discard_title = "DISCARD PSK?";
        draw_menu_list_disp(discard_title, labels, valbufs, 2, msngr_discard_confirm_cursor);
      }
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

        labels[HW_ITEM_REBOOT] = "Reboot";
        sprintf(valbufs[HW_ITEM_REBOOT], ">"); // opens a confirm dialog, not an inline value

        labels[HW_ITEM_BACK] = "BACK";
        valbufs[HW_ITEM_BACK][0] = 0;

        draw_menu_list_disp("HARDWARE", labels, valbufs, HW_ITEM_COUNT, hw_menu_cursor);
      } else if (menu_state == MENU_STATE_HW_REBOOT_CONFIRM) {
        // Plain 2-item list, same draw_menu_list_disp() as everywhere else -
        // same pattern as F/W Update's UPDATE/CANCEL (MENU_STATE_FWUPD_CONFIRM).
        const char *labels[2] = { "REBOOT", "CANCEL" };
        char valbufs[2][24];
        valbufs[0][0] = 0;
        valbufs[1][0] = 0;
        draw_menu_list_disp("REBOOT?", labels, valbufs, 2, hw_reboot_confirm_cursor);
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
        // same pattern as Messenger's DISCARD/CANCEL (MENU_STATE_MSNGR_DISCARD_CONFIRM).
        const char *labels[2] = { "UPDATE", "CANCEL" };
        char valbufs[2][24];
        valbufs[0][0] = 0;
        valbufs[1][0] = 0;
        draw_menu_list_disp("UPDATE?", labels, valbufs, 2, fwupd_confirm_cursor);
      }
    #endif
  }

#endif
