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

  // CPU temperature (pmu_temperature, Power.h) is only measured on
  // IS_ESP32S3 boards or boards with a real PMU - mirrors the exact guard
  // RNode_Firmware.ino already uses for init_pmu()/update_pmu().
  #if HAS_PMU == true || IS_ESP32S3
    #define MENU_HAS_HW_PAGE true
  #else
    #define MENU_HAS_HW_PAGE false
  #endif

  // Optional top-level items (WiFi, Hardware) shift indices around, so
  // build them up incrementally rather than hardcoding numbers per case.
  #define MENU_ITEM_DISPLAY_TIMEOUT    0
  #define MENU_ITEM_DISPLAY_BRIGHTNESS 1
  #define MENU_ITEM_ORIENTATION        2
  #define MENU_ITEM_SOUND              3

  #if HAS_WIFI == true
    #define MENU_ITEM_WIFI  4
    #define MENU_NEXT_IDX_A 5
  #else
    #define MENU_NEXT_IDX_A 4
  #endif

  #if MENU_HAS_HW_PAGE == true
    #define MENU_ITEM_HARDWARE MENU_NEXT_IDX_A
    #define MENU_NEXT_IDX_B (MENU_NEXT_IDX_A + 1)
  #else
    #define MENU_NEXT_IDX_B MENU_NEXT_IDX_A
  #endif

  #define MENU_ITEM_SAVE_EXIT MENU_NEXT_IDX_B
  #define MENU_ITEM_COUNT     (MENU_ITEM_SAVE_EXIT + 1)

  #if HAS_WIFI == true
    #define WIFI_ITEM_MODE  0
    #define WIFI_ITEM_SSID  1
    #define WIFI_ITEM_PSK   2
    #define WIFI_ITEM_BACK  3
    #define WIFI_ITEM_COUNT 4
  #endif

  #if MENU_HAS_HW_PAGE == true
    #define HW_ITEM_TEMP 0
    #define HW_NEXT_0    1

    #if HAS_VSENSE == true
      #define HW_ITEM_VOLTAGE HW_NEXT_0
      #define HW_NEXT_A       (HW_NEXT_0 + 1)
    #else
      #define HW_NEXT_A HW_NEXT_0
    #endif

    #if HAS_WIFI == true
      #define HW_ITEM_WIFI_IP  HW_NEXT_A
      #define HW_ITEM_WIFI_NM  (HW_NEXT_A + 1)
      #define HW_ITEM_WIFI_MAC (HW_NEXT_A + 2)
      #define HW_NEXT_B        (HW_NEXT_A + 3)
    #else
      #define HW_NEXT_B HW_NEXT_A
    #endif

    #if HAS_BLUETOOTH == true || HAS_BLE == true
      #define HW_ITEM_BT_MAC HW_NEXT_B
      #define HW_NEXT_C      (HW_NEXT_B + 1)
    #else
      #define HW_NEXT_C HW_NEXT_B
    #endif

    #define HW_ITEM_BACK  HW_NEXT_C
    #define HW_ITEM_COUNT (HW_ITEM_BACK + 1)
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

  uint8_t staged_display_timeout    = 0;   // seconds, 0-255, 0 = OFF
  uint8_t staged_display_brightness = 0;   // 0-255, raw SSD1306 contrast
  uint8_t staged_display_rotation   = 0;   // 0-3 = 0/90/180/270 degrees
  // Brightness is applied live the moment it's confirmed (see
  // menu_encoder_button()), unlike every other field which only takes
  // effect on the final commit - this is the baseline to diff against at
  // commit time so a live-applied value still gets persisted to EEPROM
  // (display_intensity itself no longer reflects "unchanged" by then).
  uint8_t live_display_brightness   = 0;
  bool    staged_sound_enabled      = true;
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

    // Working state while actively in MENU_STATE_WIFI_TEXT_EDIT, reset fresh
    // every time SSID or PSK is opened from the WiFi list.
    uint8_t text_edit_field = 0;   // WIFI_ITEM_SSID or WIFI_ITEM_PSK
    char    text_edit_buf[33] = {0};
    uint8_t wheel_index = 0;
    uint8_t text_confirm_cursor = 0;   // 0 = SAVE, 1 = DISCARD
  #endif

  #if MENU_HAS_HW_PAGE == true
    uint8_t hw_menu_cursor = 0;
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

  void menu_stage_from_live() {
    staged_display_timeout    = display_blanking_enabled ? (uint8_t)(display_blanking_timeout / 1000) : 0;
    staged_display_brightness = display_intensity;
    live_display_brightness   = display_intensity;
    staged_sound_enabled      = sound_enabled;
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
    if (staged_sound_enabled != sound_enabled) {
      snd_conf_save(staged_sound_enabled);
    }
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
      if (wifi_changed) { wifi_remote_init(); }
    #endif
    // Must be last: drot_conf_save() may call hard_reset() if the rotation
    // actually changed, which would otherwise discard any of the above
    // writes that hadn't happened yet.
    drot_conf_save(staged_display_rotation);
    menu_state  = MENU_STATE_CLOSED;
    menu_cursor = 0;
    display_unblank();
  }

  void menu_encoder_rotate(int8_t dir, bool wrap) {
    display_unblank();
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
      } else if (menu_edit_field == MENU_ITEM_SOUND) {
        staged_sound_enabled = !staged_sound_enabled;
      }
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
      }
    #endif
    #if MENU_HAS_HW_PAGE == true
      else if (menu_state == MENU_STATE_HW_LIST) {
        buzzer_encoder_tick_melody();
        hw_menu_cursor = menu_clamp_cursor(hw_menu_cursor, dir, HW_ITEM_COUNT, wrap);
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
    }
  }

  void menu_encoder_button(unsigned long duration) {
    display_unblank();

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
        } else {
          // SSID or PSK: fresh text-edit session, preloaded from the
          // current staged value, wheel starts at 'a'.
          text_edit_field = wifi_menu_cursor;
          const char *src = (text_edit_field == WIFI_ITEM_SSID) ? staged_wifi_ssid : staged_wifi_psk;
          strncpy(text_edit_buf, src, 32); text_edit_buf[32] = 0;
          wheel_index = 1; // 'a'
          menu_state = MENU_STATE_WIFI_TEXT_EDIT;
        }
      } else if (menu_state == MENU_STATE_WIFI_EDIT) {
        menu_state = MENU_STATE_WIFI_LIST; // confirms staged value, no write yet
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
    #if MENU_HAS_HW_PAGE == true
      else if (menu_state == MENU_STATE_HW_LIST) {
        // Read-only info page - only BACK does anything.
        if (hw_menu_cursor == HW_ITEM_BACK) {
          menu_state = MENU_STATE_LIST;
        }
      }
    #endif
    // menu_state == MENU_STATE_CLOSED + short click: no-op (reserved).
  }

  // The main (non-encoder) button doubles as an alternate control while the
  // menu is open, everywhere except WIFI_TEXT_EDIT (which keeps it as a
  // dedicated backspace key - see menu_main_button_del(), called separately
  // by button_event() for that one state). Short press cycles forward
  // through the current level, same as one encoder detent; a quick second
  // short press (double-tap) cycles backward instead - see
  // menu_btn_pending/menu_button_process(). Long press confirms/selects,
  // same as an encoder short-click. 150-499ms is a dead zone (no-op) so an
  // imprecise press doesn't do either by accident.
  void menu_button_press(unsigned long duration) {
    display_unblank();
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
    display.print("turn:move press:open");
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
    display.setCursor(64 - w/2, 32);
    display.print(valbuf);
    display.setTextSize(1);
    display.setCursor(18, 34);
    display.print("<");
    display.setCursor(106, 34);
    display.print(">");

    display.drawFastHLine(4, 50, 120, SSD1306_WHITE);
    display.setCursor(6, 59);
    display.print("turn:adjust press:ok");
  }

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

      labels[MENU_ITEM_SOUND] = "Sound";
      sprintf(valbufs[MENU_ITEM_SOUND], staged_sound_enabled ? "ON" : "OFF");

      #if HAS_WIFI == true
        labels[MENU_ITEM_WIFI] = "WiFi";
        sprintf(valbufs[MENU_ITEM_WIFI], ">"); // opens a submenu, not an inline value
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
      const char *title = "SOUND";
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
      } else {
        sprintf(valbuf, staged_sound_enabled ? "ON" : "OFF");
      }
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

        labels[WIFI_ITEM_BACK] = "BACK";
        valbufs[WIFI_ITEM_BACK][0] = 0;
        draw_menu_list_disp("WIFI", labels, valbufs, WIFI_ITEM_COUNT, wifi_menu_cursor);
      } else if (menu_state == MENU_STATE_WIFI_EDIT) {
        char valbuf[8];
        format_wifi_mode(staged_wifi_mode, valbuf);
        draw_menu_edit_disp("MODE", valbuf);
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
    #if MENU_HAS_HW_PAGE == true
      else if (menu_state == MENU_STATE_HW_LIST) {
        const char *labels[HW_ITEM_COUNT];
        char valbufs[HW_ITEM_COUNT][24];

        labels[HW_ITEM_TEMP] = "CPU Temp";
        sprintf(valbufs[HW_ITEM_TEMP], "%.1fC", pmu_temperature);

        #if HAS_VSENSE == true
          labels[HW_ITEM_VOLTAGE] = "Input Voltage";
          sprintf(valbufs[HW_ITEM_VOLTAGE], "%.2fV", vsense_voltage);
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
          {
            uint8_t mac[6];
            esp_read_mac(mac, ESP_MAC_BT);
            sprintf(valbufs[HW_ITEM_BT_MAC], "%02X:%02X:%02X:%02X:%02X:%02X",
              mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
          }
        #endif

        labels[HW_ITEM_BACK] = "BACK";
        valbufs[HW_ITEM_BACK][0] = 0;

        draw_menu_list_disp("HARDWARE", labels, valbufs, HW_ITEM_COUNT, hw_menu_cursor);
      }
    #endif
  }

#endif
