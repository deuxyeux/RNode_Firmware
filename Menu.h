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

  // The Hardware page exists whenever there's anything board-level worth
  // showing (battery/voltage sensing via HAS_PMU, or an ESP32-S3's CPU
  // temp) - mirrors the exact guard RNode_Firmware.ino already uses for
  // init_pmu()/update_pmu().
  #if HAS_PMU == true || IS_ESP32S3
    #define MENU_HAS_HW_PAGE true
  #else
    #define MENU_HAS_HW_PAGE false
  #endif

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
    #define MENU_NEXT_IDX_0   (MENU_NEXT_IDX_S + 1)
  #else
    #define MENU_NEXT_IDX_0 MENU_NEXT_IDX_S
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

  #if MENU_HAS_HW_PAGE == true
    #define MENU_ITEM_HARDWARE MENU_NEXT_IDX_A3
    #define MENU_NEXT_IDX_B (MENU_NEXT_IDX_A3 + 1)
  #else
    #define MENU_NEXT_IDX_B MENU_NEXT_IDX_A3
  #endif

  #define MENU_ITEM_SAVE_EXIT MENU_NEXT_IDX_B
  #define MENU_ITEM_COUNT     (MENU_ITEM_SAVE_EXIT + 1)

  #if HAS_WIFI == true
    #define WIFI_ITEM_MODE     0
    #define WIFI_ITEM_SSID     1
    #define WIFI_ITEM_PSK      2
    // Static IP/netmask (ADDR_CONF_IP/NM, ROM.h) - same all-zero/all-0xFF-
    // means-unset convention as everywhere else (see addr4_read(),
    // Utilities.h). Unlike Mode/SSID/PSK's WIFI_TEXT_EDIT flow, these don't
    // get their own "Save?" confirm dialog - MENU_STATE_WIFI_ADDR_EDIT
    // finishing just updates staged_wifi_ip/nm in RAM, still deferred to
    // SAVE & EXIT like every other field in this list.
    #define WIFI_ITEM_IP       3
    #define WIFI_ITEM_NETMASK  4
    // Gateway/DNS (ADDR_CONF_GW/DNS, ROM.h) - only meaningful once IP/NM
    // are actually static (DHCP already provides both otherwise), but kept
    // as plain always-present fields rather than conditionally hidden -
    // same reasoning as IP/NM themselves. Needed for anything that must
    // leave the local subnet while on a static IP, e.g. rtc_sync_ntp()
    // (RTC.h).
    #define WIFI_ITEM_GATEWAY  5
    #define WIFI_ITEM_DNS      6
    // Single-confirm action (same as MENU_ITEM_SAVE_EXIT), not a field -
    // stages all four back to 0.0.0.0, still deferred to SAVE & EXIT.
    #define WIFI_ITEM_CLEAR    7
    #define WIFI_ITEM_BACK     8
    #define WIFI_ITEM_COUNT    9
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
    #define RTC_ITEM_BACK  RTC_NEXT_0
    #define RTC_ITEM_COUNT (RTC_ITEM_BACK + 1)
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

    #if HAS_WIFI == true
      #define HW_ITEM_WIFI_IP  HW_NEXT_A2
      #define HW_ITEM_WIFI_NM  (HW_NEXT_A2 + 1)
      #define HW_ITEM_WIFI_MAC (HW_NEXT_A2 + 2)
      #define HW_NEXT_B        (HW_NEXT_A2 + 3)
    #else
      #define HW_NEXT_B HW_NEXT_A2
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

    #define HW_ITEM_BACK  HW_NEXT_D
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
  // below (Sync NTP's progress/result, Clear Static's confirmation - both
  // WiFi/Ethernet-specific, see MENU_STATE_STATUS_POPUP further down) and
  // button_hold_process()'s main-button-hold feedback (menu-closed,
  // applies to every HAS_MENU board regardless of WiFi/Ethernet) - hence
  // living here, ungated, rather than under either feature's own #if.
  #if HAS_INPUT == true || HAS_WIFI == true || HAS_ETHERNET == true
    // Remembers the last-drawn box's footprint so a later, narrower/
    // shorter message can erase just that area instead of the whole
    // screen - see draw_menu_status_rect().
    uint16_t menu_popup_prev_x = 0, menu_popup_prev_y = 0;
    uint16_t menu_popup_prev_w = 0, menu_popup_prev_h = 0;
    bool menu_popup_prev_valid = false;

    // Superimposes the box on whatever's already in the display buffer -
    // no clearDisplay() here. Only erases its own previous footprint (if
    // any), not the whole screen, so a shrinking message (e.g.
    // "CONNECTING" -> "SYNCED!") doesn't leave stale pixels poking out
    // around a narrower new box. Box height (11) and baseline offset (+7)
    // reuse draw_menu_list_disp()'s own row_h/baseline convention - the
    // same font at the same tightness, already proven to fit cleanly.
    void draw_menu_status_rect(const char *text) {
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
      if (box_w > 120) box_w = 120;
      const uint16_t box_h = 11;
      uint16_t box_x = (128 - box_w) / 2;
      const uint16_t box_y = (64 - box_h) / 2;

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
        if (held_ms > 7000) return BUTTON_HOLD_TIER_SLEEP;
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
    void draw_button_hold_overlay() {
      if (menu_is_open() || !button_pressed()) return;
      uint8_t tier = button_hold_tier(millis() - button_down_last);
      if (tier != BUTTON_HOLD_TIER_NONE) {
        draw_menu_status_rect(button_hold_tier_text(tier));
      }
    }
  #endif

  #if HAS_WIFI == true || HAS_ETHERNET == true
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
  #if HAS_ENCODER == true
    bool staged_encoder_enabled = false;
  #endif
  #if HAS_WIFI == true
    uint8_t wifi_menu_cursor = 0;
    uint8_t staged_wifi_mode = WR_WIFI_OFF;
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

  #if MENU_HAS_HW_PAGE == true
    uint8_t hw_menu_cursor = 0;
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

    // Rudimentary on-screen keyboard: a single cyclic wheel of 96 positions
    // (rotary encoder has no separate cursor-move axis, so editing is
    // append + backspace only - see [[project_encoder_settings_menu]] design
    // notes). Common characters first, DEL right after (frequently needed,
    // shouldn't require scrolling past rare symbols to reach). Saving is
    // handled by a long-press "Save?" dialog (MENU_STATE_WIFI_TEXT_CONFIRM),
    // not a wheel position - dialing all the way around to a DONE entry
    // every time was tedious.
    #define WHEEL_DEL_IDX 64
    #define WHEEL_LEN     96
    const char WHEEL_CORE[] = " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; // 64 chars, idx 0-63
    const char WHEEL_SYMS[] = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"; // 31 chars, idx 65-95

    // Returns 0 for the DEL meta position - callers check the index against
    // WHEEL_DEL_IDX before treating this as a literal character.
    char wheel_char_at(uint8_t idx) {
      if (idx < 64) return WHEEL_CORE[idx];
      if (idx == WHEEL_DEL_IDX) return 0;
      return WHEEL_SYMS[idx - 65];
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
  #endif

  void menu_stage_from_live() {
    staged_display_timeout    = display_blanking_enabled ? (uint8_t)(display_blanking_timeout / 1000) : 0;
    staged_display_brightness = display_intensity;
    live_display_brightness   = display_intensity;
    #if HAS_BUZZER == true
      staged_sound_enabled = sound_enabled;
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
        step_wifi_mode(dir, wrap);
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
      // The one exception is text entry, where dialing all the way around
      // to a "confirm and exit the whole menu" action would be tedious and
      // easy to trigger by accident mid-word - long-press there instead
      // opens a small local "Save?" dialog scoped to just this field.
      #if HAS_WIFI == true
        if (menu_state == MENU_STATE_WIFI_TEXT_EDIT) {
          buzzer_encoder_click_melody();
          text_confirm_cursor = 0; // default to SAVE
          menu_state = MENU_STATE_WIFI_TEXT_CONFIRM;
          return;
        }
      #endif
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
      #if MENU_HAS_HW_PAGE == true
        else if (menu_cursor == MENU_ITEM_HARDWARE) {
          menu_state = MENU_STATE_HW_LIST;
          hw_menu_cursor = 0;
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
        } else if (wifi_menu_cursor == WIFI_ITEM_MODE) {
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
          wheel_index = 1; // 'a'
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
        if (wheel_index == WHEEL_DEL_IDX) {
          if (len > 0) text_edit_buf[len-1] = 0;
        } else if (len < 32) {
          text_edit_buf[len] = wheel_char_at(wheel_index);
          text_edit_buf[len+1] = 0;
          wheel_index = 1; // reset to 'a' for the next character
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
    display.setFont(SMALL_FONT);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(6, 8);
    display.print(title);
    display.drawFastHLine(4, 12, 120, SSD1306_WHITE);

    const uint8_t row_h = 11;
    const uint8_t visible_rows = 4;
    uint8_t first = 0;
    if (count > visible_rows) {
      if (cursor >= visible_rows) first = cursor - visible_rows + 1;
      if (first > count - visible_rows) first = count - visible_rows;
    }

    for (uint8_t vi = 0; vi < visible_rows && (first + vi) < count; vi++) {
      uint8_t i = first + vi;
      uint8_t row_top = 15 + vi * row_h;
      uint8_t y = row_top + 7; // text baseline
      if (i == cursor) {
        display.fillRect(4, row_top, 120, 10, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }
      display.setCursor(8, y);
      display.print(labels[i]);

      if (valbufs[i][0] != 0) {
        int16_t x1, y1; uint16_t w, h;
        display.getTextBounds(valbufs[i], 0, 0, &x1, &y1, &w, &h);
        display.setCursor(122 - w, y);
        display.print(valbufs[i]);
      }
    }

    display.setTextColor(SSD1306_WHITE);
    display.drawFastHLine(4, 59, 120, SSD1306_WHITE);
    display.setCursor(6, 63);
    // Whether an encoder is actually populated is a runtime choice
    // (encoder_enabled) on boards where it's optional, not the compile-time
    // HAS_ENCODER capability flag - see MENU_ITEM_ENCODER.
    #if HAS_ENCODER == true
      if (encoder_enabled) display.print("turn:move press:open");
      else                 display.print("tap:next hold:open");
    #else
      display.print("tap:next hold:open");
    #endif
  }

  void draw_menu_edit_disp(const char *title, const char *valbuf) {
    display.setFont(SMALL_FONT);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(6, 9);
    display.print(title);
    display.drawFastHLine(4, 15, 120, SSD1306_WHITE);

    display.setTextSize(2);
    int16_t x1, y1; uint16_t w, h;
    display.getTextBounds(valbuf, 0, 0, &x1, &y1, &w, &h);
    // y=36, not the arrows' own 34 - Org_01 glyphs render above the
    // setCursor() baseline, not straddling it (see [[feedback_org01_font_baseline]]),
    // and that headroom roughly doubles at size 2 vs the arrows' size 1, so
    // matching baselines would leave the value looking a few px too high.
    // This offset centers the two visually instead of literally.
    display.setCursor(64 - w/2, 36);
    display.print(valbuf);
    display.setTextSize(1);
    // Pinned near the box edges (4..124) rather than the old fixed 18/106 -
    // frees up the center for wider size-2 values like "100/HALF" (Speed,
    // ETH_ITEM_SPEED) without colliding with the arrows. ">" is measured
    // rather than hardcoded so it can't run past the 124 edge.
    display.setCursor(5, 34);
    display.print("<");
    int16_t ax1, ay1; uint16_t aw, ah;
    display.getTextBounds(">", 0, 0, &ax1, &ay1, &aw, &ah);
    display.setCursor(123 - aw, 34);
    display.print(">");

    display.drawFastHLine(4, 50, 120, SSD1306_WHITE);
    display.setCursor(6, 59);
    #if HAS_ENCODER == true
      if (encoder_enabled) display.print("turn:adjust press:ok");
      else                 display.print("tap:adjust hold:ok");
    #else
      display.print("tap:adjust hold:ok");
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
      display.setFont(SMALL_FONT);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(6, 9);
      display.print(title);
      display.drawFastHLine(4, 15, 120, SSD1306_WHITE);

      // Same font as the SSID/PSK screen's typed content (TEXT_ENTRY_FONT,
      // Tamsyn6x12 - see draw_menu_text_edit_disp()) - title/divider/footer
      // stay on SMALL_FONT/Org_01, same split that screen uses. Narrower
      // per-glyph than Org_01 at the size Org_01 would otherwise need to be
      // legible here, so the full "255.255.255.255" fits comfortably.
      display.setFont(TEXT_ENTRY_FONT);
      display.setTextSize(1);

      char full[16];
      format_addr_octets(octets, full);
      int16_t fx1, fy1; uint16_t fw, fh;
      display.getTextBounds(full, 0, 0, &fx1, &fy1, &fw, &fh);
      uint16_t x = 64 - fw/2;
      const uint16_t y = 32;

      char seg[4];
      int16_t sx1, sy1; uint16_t sw, sh;
      for (uint8_t i = 0; i < 4; i++) {
        sprintf(seg, "%u", octets[i]);
        display.getTextBounds(seg, 0, 0, &sx1, &sy1, &sw, &sh);
        if (i == active_idx) {
          // Same box geometry as draw_menu_text_edit_disp()'s wheel
          // candidate - Tamsyn6x12 glyphs span roughly baseline-9 to
          // baseline+4, a taller box than Org_01 would need.
          display.fillRect(x - 1, 21, sw + 2, 18, SSD1306_WHITE);
          display.setTextColor(SSD1306_BLACK);
        } else {
          display.setTextColor(SSD1306_WHITE);
        }
        display.setCursor(x, y);
        display.print(seg);
        x += sw;
        display.setTextColor(SSD1306_WHITE);
        if (i < 3) {
          display.setCursor(x, y);
          display.print(".");
          int16_t dx1, dy1; uint16_t dw, dh;
          display.getTextBounds(".", 0, 0, &dx1, &dy1, &dw, &dh);
          x += dw;
        }
      }

      display.setFont(SMALL_FONT);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.drawFastHLine(4, 50, 120, SSD1306_WHITE);
      display.setCursor(6, 59);
      #if HAS_ENCODER == true
        if (encoder_enabled) display.print("turn:adjust press:ok");
        else                 display.print("tap:adjust hold:ok");
      #else
        display.print("tap:adjust hold:ok");
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
      display.setFont(SMALL_FONT);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(6, 9);
      // "UTC" suffix - the RTC list's own Time/Date rows show local
      // (Timezone-shifted) time, but this editor always reads/writes the
      // RTC in UTC, same as CMD_TIME/rtc_sync_ntp() - worth being explicit
      // about here since it'd otherwise be the one screen on this page
      // that doesn't match what's shown everywhere else.
      display.print("SET TIME/DATE UTC");
      display.drawFastHLine(4, 15, 120, SSD1306_WHITE);

      display.setFont(TEXT_ENTRY_FONT);
      display.setTextSize(1);

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
          display.getTextBounds(segs[i], 0, 0, &bx1, &by1, &bw, &bh);
          total_w += bw;
          if (seps[i][0]) { display.getTextBounds(seps[i], 0, 0, &bx1, &by1, &bw, &bh); total_w += bw; }
        }

        uint16_t x = 64 - total_w / 2;
        for (uint8_t i = first; i < first + 3; i++) {
          display.getTextBounds(segs[i], 0, 0, &bx1, &by1, &bw, &bh);
          if (i == active_idx) {
            display.fillRect(x - 1, y - 9, bw + 2, 12, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
          } else {
            display.setTextColor(SSD1306_WHITE);
          }
          display.setCursor(x, y);
          display.print(segs[i]);
          x += bw;
          display.setTextColor(SSD1306_WHITE);
          if (seps[i][0]) {
            display.setCursor(x, y);
            display.print(seps[i]);
            display.getTextBounds(seps[i], 0, 0, &bx1, &by1, &bw, &bh);
            x += bw;
          }
        }
      }

      display.setFont(SMALL_FONT);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.drawFastHLine(4, 50, 120, SSD1306_WHITE);
      display.setCursor(6, 59);
      #if HAS_ENCODER == true
        if (encoder_enabled) display.print("turn:adjust press:ok");
        else                 display.print("tap:adjust hold:ok");
      #else
        display.print("tap:adjust hold:ok");
      #endif
    }
  #endif

  #if HAS_WIFI == true
    // Character-count windowed (not pixel-precise) so a 32-char SSID/PSK
    // doesn't need to fit on screen at once - shows only the trailing
    // portion of the string plus the pending wheel selection, which is
    // always what's being actively edited (append-only, see plan notes).
    void draw_menu_text_edit_disp(const char *title, const char *text_buf, uint8_t wheel_idx) {
      display.setFont(SMALL_FONT);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(6, 9);
      display.print(title);
      display.drawFastHLine(4, 15, 120, SSD1306_WHITE);

      char candidate[6];
      if (wheel_idx == WHEEL_DEL_IDX) sprintf(candidate, "DEL");
      else { candidate[0] = wheel_char_at(wheel_idx); candidate[1] = 0; }

      // FreeMono9pt7b only for the actual typed content, at its natural
      // size (it's already a proper 9pt font, unlike Org_01 which needs
      // setTextSize(2) to be legible) - title/divider/footer stay on
      // SMALL_FONT/Org_01, same as every other menu screen.
      display.setFont(TEXT_ENTRY_FONT);
      display.setTextSize(1);

      // FreeMono9pt7b is monospace, but the window-growing logic still
      // measures real pixel widths rather than assuming a fixed advance,
      // so it stays correct if the font is ever swapped again.
      const uint16_t max_width = 120;
      int16_t cx1, cy1; uint16_t cw, ch;
      display.getTextBounds(candidate, 0, 0, &cx1, &cy1, &cw, &ch);
      uint16_t remaining_width = (max_width > cw + 3) ? (max_width - cw - 3) : 0;

      uint8_t text_len = strlen(text_buf);
      uint8_t prefix_len = 0;
      for (uint8_t try_len = 1; try_len <= text_len; try_len++) {
        int16_t px1, py1; uint16_t pw, ph;
        display.getTextBounds(text_buf + (text_len - try_len), 0, 0, &px1, &py1, &pw, &ph);
        if (pw > remaining_width) break;
        prefix_len = try_len;
      }
      const char *prefix_start = text_buf + (text_len - prefix_len);

      int16_t x1, y1; uint16_t pw, ph;
      display.getTextBounds(prefix_start, 0, 0, &x1, &y1, &pw, &ph);
      uint16_t cand_x = 4 + pw;

      display.setTextColor(SSD1306_WHITE);
      display.setCursor(4, 32);
      display.print(prefix_start);

      // FreeMono9pt7b glyphs span roughly baseline-9 (ascenders) to
      // baseline+4 (descenders like g/p/y) at this size - a taller box
      // than Org_01 needed.
      display.fillRect(cand_x, 21, cw + 3, 18, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(cand_x + 1, 32);
      display.print(candidate);

      display.setFont(SMALL_FONT);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.drawFastHLine(4, 50, 120, SSD1306_WHITE);
      display.setCursor(6, 59);
      display.print("turn:char hold:save");
    }
  #endif

  void draw_settings_menu_disp() {
    display.setTextWrap(false);

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

      #if MENU_HAS_HW_PAGE == true
        labels[MENU_ITEM_HARDWARE] = "Hardware";
        sprintf(valbufs[MENU_ITEM_HARDWARE], ">"); // opens a submenu, not an inline value
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
        format_wifi_mode(staged_wifi_mode, valbuf);
        draw_menu_edit_disp("MODE", valbuf);
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
          labels[RTC_ITEM_SYNC_NTP] = "Sync NTP";
          valbufs[RTC_ITEM_SYNC_NTP][0] = 0;
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
    #endif
  }

#endif
