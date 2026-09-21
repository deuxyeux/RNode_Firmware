// BLE HID keyboard host (MeshAdventurer-S3 / MCU_ESP32 only, HAS_BLE_HID_HOST
// in Boards.h). Lets the RNode pair with and receive input from a real BLE
// HID keyboard, running concurrently with the existing BLESerial peripheral
// role. Opt-in: gated end to end on blekbd_enabled (RNode Settings >
// Bluetooth > BLE Keyboard > Enabled, off by default). Menu flow (Menu.h,
// MENU_STATE_BLEKBD_*) drives an active discovery scan to find and pair with
// one specific keyboard; once paired, its identity is persisted (EEPROM,
// ROM.h ADDR_CONF_BLEKBD_*) and a passive background scan reconnects to that
// same keyboard automatically whenever it's back in range/awake, with no
// re-pairing or manual BT toggle needed.
//
// Two hard-won constraints from getting this working at all, both still
// load-bearing and NOT spike-specific:
//
// 1. esp_hidh_dev_open() internally blocks the calling task on a semaphore
//    that's only ever given by a later BLE_GAP_EVENT_CONNECT callback - fine
//    from an application task, but calling it directly from *inside* a GAP
//    callback self-deadlocks the NimBLE host task: it blocks waiting for an
//    event that only itself, by returning to its own event loop, could ever
//    deliver. Confirmed via task-name tracing inside esp_hidh's
//    nimble_hidh.c (WAIT_CB stuck forever on task "nimble_host"). Every
//    connect attempt below only ever sets blekbd_connect_pending; the actual
//    esp_hidh_dev_open() call happens from blekbd_loop() (the Arduino main
//    task).
//
// 2. ble_gap_disc_cancel() causes a real, separate, still-unexplained
//    multi-second stall when called while actively scanning with an
//    indefinite duration - confirmed via FreeRTOS task-dump tracing
//    (nimble_host/btController both showing zero CPU time for 9+ seconds
//    straight right after the call). Vanilla ESP-IDF's own "blecent" example
//    calls it from its own GAP callback with no issue, so this isn't the
//    same class of bug as #1 above - it's specific to this stack/config
//    combination and not chased further. Scanning below is always bounded
//    (BLEKBD_SCAN_MS) and left to time out naturally; disc_cancel() is never
//    called anywhere in this file.
//
// 3. esp_hidh_dev_output_set() (used for the Caps Lock LED) hits the exact
//    same blocking pattern as #1: its BLE backend (esp_ble_hidh_dev_report_
//    write(), nimble_hidh.c) does LOCK_OPS()/WAIT_CB()/UNLOCK_OPS() on a
//    GATT write-completion callback, so it must only ever be called from
//    blekbd_loop() (main task), never from blekbd_hidh_cb()/
//    blekbd_translate_report() (esp_hidh event task). esp_hidh_dev_reports_
//    get() is safe from the event task though - it just reads the already-
//    cached report list in memory, no GATT I/O - so it's used to discover
//    the connected keyboard's real OUTPUT report map_index/report_id right
//    when ESP_HIDH_OPEN_EVENT fires, and the deferred write itself happens
//    from blekbd_loop() via a dirty flag (blekbd_caps_led_dirty).

#pragma once

#if HAS_BLE_HID_HOST == true

#include "esp_hidh.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_hs_adv.h"
#include "host/ble_uuid.h"
#include "host/ble_store.h"

#define BLEKBD_SCAN_MS 10000
#define BLEKBD_HID_SVC_UUID 0x1812
#define BLEKBD_NAME_MAX_LEN 31
#define BLEKBD_MAX_DISCOVERED 8

// ---- Config (EEPROM-backed, ROM.h ADDR_CONF_BLEKBD_*) -----------------

bool blekbd_enabled = false;
bool blekbd_peer_stored = false;
uint8_t blekbd_peer_addr[6];
uint8_t blekbd_peer_addr_type;

static bool blekbd_conf_loaded = false;

void blekbd_conf_load() {
  blekbd_enabled = (EEPROM.read(ADDR_CONF_BLEKBD_ENABLED) == BLEKBD_ENABLE_BYTE);
  blekbd_peer_addr_type = EEPROM.read(ADDR_CONF_BLEKBD_PEER_ADDR_TYPE);
  blekbd_peer_stored = (blekbd_peer_addr_type <= 3);
  if (blekbd_peer_stored) {
    for (uint8_t i = 0; i < 6; i++) blekbd_peer_addr[i] = EEPROM.read(ADDR_CONF_BLEKBD_PEER_ADDR + i);
  }
  blekbd_conf_loaded = true;
}

void blekbd_enabled_conf_save(bool enabled) {
  blekbd_enabled = enabled;
  eeprom_update(ADDR_CONF_BLEKBD_ENABLED, enabled ? BLEKBD_ENABLE_BYTE : BLEKBD_DISABLE_BYTE);
}

void blekbd_peer_conf_save(const uint8_t addr[6], uint8_t addr_type) {
  for (uint8_t i = 0; i < 6; i++) eeprom_update(ADDR_CONF_BLEKBD_PEER_ADDR + i, addr[i]);
  eeprom_update(ADDR_CONF_BLEKBD_PEER_ADDR_TYPE, addr_type); // written last - see ROM.h comment on ADDR_CONF_BLEKBD_PEER_ADDR_TYPE
  memcpy(blekbd_peer_addr, addr, 6);
  blekbd_peer_addr_type = addr_type;
  blekbd_peer_stored = true;
}

void blekbd_peer_conf_forget() {
  eeprom_update(ADDR_CONF_BLEKBD_PEER_ADDR_TYPE, 0xFF); // invalidating just the type sentinel is enough; stale address bytes are inert
  blekbd_peer_stored = false;
}

// ---- Discovered-device list (active discovery / menu Scan screen) -----

struct BlekbdDiscovered {
  bool in_use = false;
  uint8_t addr[6];
  uint8_t addr_type;
  char name[BLEKBD_NAME_MAX_LEN + 1];
  int8_t rssi;
  unsigned long last_seen_ms;
};
BlekbdDiscovered blekbd_discovered[BLEKBD_MAX_DISCOVERED];

static void blekbd_discovered_clear() {
  for (uint8_t i = 0; i < BLEKBD_MAX_DISCOVERED; i++) blekbd_discovered[i].in_use = false;
}

static void blekbd_discovered_upsert(const uint8_t addr[6], uint8_t addr_type, const char *name, int8_t rssi) {
  int free_slot = -1;
  int oldest_slot = 0;
  unsigned long oldest_ms = ULONG_MAX;
  for (uint8_t i = 0; i < BLEKBD_MAX_DISCOVERED; i++) {
    if (blekbd_discovered[i].in_use && memcmp(blekbd_discovered[i].addr, addr, 6) == 0 && blekbd_discovered[i].addr_type == addr_type) {
      blekbd_discovered[i].rssi = rssi;
      blekbd_discovered[i].last_seen_ms = millis();
      if (name && name[0] && !blekbd_discovered[i].name[0]) strncpy(blekbd_discovered[i].name, name, BLEKBD_NAME_MAX_LEN);
      return;
    }
    if (!blekbd_discovered[i].in_use && free_slot == -1) free_slot = i;
    if (blekbd_discovered[i].in_use && blekbd_discovered[i].last_seen_ms < oldest_ms) {
      oldest_ms = blekbd_discovered[i].last_seen_ms;
      oldest_slot = i;
    }
  }
  int slot = (free_slot != -1) ? free_slot : oldest_slot;
  blekbd_discovered[slot].in_use = true;
  memcpy(blekbd_discovered[slot].addr, addr, 6);
  blekbd_discovered[slot].addr_type = addr_type;
  strncpy(blekbd_discovered[slot].name, name ? name : "", BLEKBD_NAME_MAX_LEN);
  blekbd_discovered[slot].name[BLEKBD_NAME_MAX_LEN] = 0;
  blekbd_discovered[slot].rssi = rssi;
  blekbd_discovered[slot].last_seen_ms = millis();
}

// ---- Scan mode / pairing state -----------------------------------------

enum BlekbdScanMode { BLEKBD_SCAN_IDLE, BLEKBD_SCAN_PASSIVE, BLEKBD_SCAN_ACTIVE_DISCOVERY };
BlekbdScanMode blekbd_scan_mode = BLEKBD_SCAN_IDLE;

enum BlekbdPairResult { BLEKBD_PAIR_NONE, BLEKBD_PAIR_PENDING, BLEKBD_PAIR_OK, BLEKBD_PAIR_FAILED };
BlekbdPairResult blekbd_pair_result = BLEKBD_PAIR_NONE;
unsigned long blekbd_pair_result_at_ms = 0;
#define BLEKBD_PAIR_RESULT_POPUP_MS 1500

esp_hidh_dev_t *blekbd_open_dev = NULL; // live connection handle, so Forget Keyboard can close it before erasing EEPROM/deleting the bond

static bool blekbd_hidh_inited = false;
static bool blekbd_connecting = false;
static bool blekbd_found_pending = false;
static uint8_t blekbd_found_addr[6];
static uint8_t blekbd_found_addr_type;
static bool blekbd_connect_pending = false;
static bool blekbd_rescan_pending = false;
// esp_hidh_dev_open() -> ble_gap_connect() reliably fails synchronously
// (confirmed on hardware: "esp_ble_gattc_open failed: 15" = BLE_HS_EBUSY)
// when the discovery scan that found this device is still physically
// running - blekbd_scan_stop() only marks blekbd_scan_mode IDLE, it never
// calls ble_gap_disc_cancel() (see this file's header), so a scan started
// moments before the user hit PAIR can still have up to BLEKBD_SCAN_MS left
// to run. On that failure esp_ble_hidh_dev_open() returns NULL without ever
// posting ESP_HIDH_OPEN_EVENT, so blekbd_hidh_cb() never fires - retry
// instead of giving up silently; the in-flight scan is guaranteed to
// complete naturally (bounded duration) well within the deadline below.
static unsigned long blekbd_connect_retry_at_ms = 0;
static unsigned long blekbd_connect_deadline_ms = 0;
#define BLEKBD_CONNECT_RETRY_MS 300
#define BLEKBD_CONNECT_MAX_WAIT_MS 12000
// Only the explicit menu pairing flow (blekbd_connect_start(), driven by the
// user selecting a row on MENU_STATE_BLEKBD_SCAN) reports into
// blekbd_pair_result/menu_state - the passive background reconnect path
// must never touch either, so it stays silent/invisible unless the user is
// actually looking at the Scan/Pairing screens.
static bool blekbd_connect_is_active_pairing = false;

// Set in ESP_HIDH_OPEN_EVENT for every open that ISN'T the active pairing
// flow (background auto-reconnect), and in ESP_HIDH_CLOSE_EVENT for every
// close (whether a deliberate Forget Keyboard or the keyboard just dropping
// out of range/sleeping) - both consumed by blekbd_notice_process()
// (Menu.h), which shows a brief "KBD CONNECTED"/"KBD DISCONNECTED" overlay
// regardless of whatever else is currently on screen (Settings, Messenger,
// Chat, ...) - see draw_blekbd_notice_overlay()'s own comment for why this
// doesn't go through the menu-open popup machinery.
bool blekbd_connected_popup_pending = false;
bool blekbd_disconnected_popup_pending = false;

// Defined in Menu.h (same reason blekbd_pair_result_process() is only
// declared here, see that function's own comment) - the actual character/
// Enter/Backspace/Escape dispatch into the LXMF Messenger's text-entry
// buffer needs menu_state/MENU_STATE_MSNGR_TEXT_ENTRY and the msngr_kb_*
// helpers, none of which exist yet at this point in the include order.
void blekbd_key_event(char ch);

// ---- HID report -> character translation -------------------------------
//
// Standard USB HID boot-compatible keyboard report layout (Usage Page 0x07,
// "Keyboard/Keypad"): byte0 = modifier bitmap, byte1 = reserved, bytes2-7 =
// up to 6 simultaneously-held keycodes (0x00 = empty slot, 0x01 = phantom/
// rollover-error). Virtually every consumer keyboard - including under the
// HID Report protocol, not just Boot protocol - keeps its Report Map's
// keyboard input report in this exact shape for boot-protocol interop, so
// this is assumed here rather than actually parsed from Enigma's Report
// Map. If real keystrokes come out wrong, capture a raw hex dump of
// param->input.data and revisit this assumption (see the file-level plan
// this was implemented from).
//
// Only the subset actually reachable from a physical keyboard into a
// single-line text buffer is mapped: a-z, 1-0, the standard US punctuation
// row, Enter, Backspace, Space, and Escape (handled separately by keycode
// below, not through this table). Unmapped entries (0) - Tab, F-keys,
// arrows, media keys, anything past 0x38 - are silently ignored by
// blekbd_translate_report() below.
#define BLEKBD_HID_KEYMAP_LEN 0x39
static const char BLEKBD_HID_KEYMAP[BLEKBD_HID_KEYMAP_LEN] = {
  /* 0x00-0x03 */ 0, 0, 0, 0,
  /* 0x04-0x1D a-z */ 'a','b','c','d','e','f','g','h','i','j','k','l','m',
                      'n','o','p','q','r','s','t','u','v','w','x','y','z',
  /* 0x1E-0x27 1-0 */ '1','2','3','4','5','6','7','8','9','0',
  /* 0x28 Enter */ '\n',
  /* 0x29 Esc   */ 0, // handled separately (usage code checked directly)
  /* 0x2A Backspace */ '\b',
  /* 0x2B Tab   */ 0,
  /* 0x2C Space */ ' ',
  /* 0x2D-0x38 punctuation */ '-','=','[',']','\\', 0 /* non-US # */, ';','\'','`',',','.','/',
};
static const char BLEKBD_HID_KEYMAP_SHIFT[BLEKBD_HID_KEYMAP_LEN] = {
  /* 0x00-0x03 */ 0, 0, 0, 0,
  /* 0x04-0x1D A-Z */ 'A','B','C','D','E','F','G','H','I','J','K','L','M',
                      'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
  /* 0x1E-0x27 !@#$%^&*() */ '!','@','#','$','%','^','&','*','(',')',
  /* 0x28 Enter */ '\n',
  /* 0x29 Esc   */ 0,
  /* 0x2A Backspace */ '\b',
  /* 0x2B Tab   */ 0,
  /* 0x2C Space */ ' ',
  /* 0x2D-0x38 punctuation shifted */ '_','+','{','}','|', 0, ':','"','~','<','>','?',
};

// True while the on-screen keyboard's own EN/RU toggle (msngr_kb_lang_ru,
// Menu.h) is set to RU - mirrored into this plain bool by Menu.h's
// BLEKBD_CH_TOGGLE_LANG handler (Alt+Shift/Ctrl+Shift) whenever it flips,
// since msngr_kb_lang_ru itself isn't visible yet at this point in the
// include order. Read here, not in Menu.h, because the actual keycode->
// character resolution below (which table to use) has to stay in this
// file alongside the EN tables and Shift handling it already does.
bool blekbd_lang_ru = false;

// Caps Lock state - unlike Shift (which the keymap tables above apply
// uniformly to every key), real Caps Lock only affects letters, so it's
// tracked separately and XORed with shift just for the letter keycode range
// (0x04-0x1D) in blekbd_translate_report() below. blekbd_caps_led_dirty
// requests a deferred LED write from blekbd_loop() (main task only - see
// constraint #3 in this file's header comment); blekbd_output_report_known/
// _map_index/_report_id are discovered once per connection, in
// ESP_HIDH_OPEN_EVENT, via esp_hidh_dev_reports_get() (safe from the event
// task - no GATT I/O, just reads the already-cached report list).
bool blekbd_caps_lock = false;
bool blekbd_caps_led_dirty = false;
bool blekbd_output_report_known = false;
uint8_t blekbd_output_map_index = 0;
uint8_t blekbd_output_report_id = 0;

// Cyrillic (ЙЦУКЕН) physical-key mapping - standard Russian keyboard
// layout convention (every OS's own "Russian" layout driver maps the same
// physical key positions this way), NOT a copy of the on-screen keyboard's
// own MSNGR_KB_LAYOUT_RU grid (Menu.h) - that grid only has 4x11=44 cells
// to work with and has to compromise (its own comment: Б/Ё/Ж/Х/Ъ/Э/Ю don't
// fit at their real positions there, so it relocates them to the digit
// row instead). A physical keyboard has real [ ] ; ' ` keys available, so
// this uses the authentic full mapping instead of that compromise. Same
// single-byte internal Cyrillic codes msngr_text_entry_buf already stores
// (Fonts/Org_01.h's byte-per-glyph remap, expanded to real UTF-8 by
// msngr_kb_expand_utf8() right before the buffer's contents go anywhere
// else, Menu.h) - values cross-checked directly against MSNGR_KB_LAYOUT_
// RU's own literal byte constants (Menu.h) for the 26 letters that do
// appear there, and against msngr_kb_apply_shift()'s own "-0x21 for
// uppercase" arithmetic for the shifted table.
#define BLEKBD_CH_YO 0x80 // Ё - stored uppercase-only, matching MSNGR_KB_LAYOUT_RU's own row0 (0xA1/ё never appears there either) - no lowercase form reachable from either keyboard today
static const char BLEKBD_HID_KEYMAP_RU[BLEKBD_HID_KEYMAP_LEN] = {
  /* 0x00-0x03 */ 0, 0, 0, 0,
  /* 0x04 A -> ф */ '\xB6', /* 0x05 B -> и */ '\xAA', /* 0x06 C -> с */ '\xB3',
  /* 0x07 D -> в */ '\xA4', /* 0x08 E -> у */ '\xB5', /* 0x09 F -> а */ '\xA2',
  /* 0x0A G -> п */ '\xB1', /* 0x0B H -> р */ '\xB2', /* 0x0C I -> ш */ '\xBA',
  /* 0x0D J -> о */ '\xB0', /* 0x0E K -> л */ '\xAD', /* 0x0F L -> д */ '\xA6',
  /* 0x10 M -> ь */ '\xBE', /* 0x11 N -> т */ '\xB4', /* 0x12 O -> щ */ '\xBB',
  /* 0x13 P -> з */ '\xA9', /* 0x14 Q -> й */ '\xAB', /* 0x15 R -> к */ '\xAC',
  /* 0x16 S -> ы */ '\xBD', /* 0x17 T -> е */ '\xA7', /* 0x18 U -> г */ '\xA5',
  /* 0x19 V -> м */ '\xAE', /* 0x1A W -> ц */ '\xB8', /* 0x1B X -> ч */ '\xB9',
  /* 0x1C Y -> н */ '\xAF', /* 0x1D Z -> я */ '\xC1',
  /* 0x1E-0x27 1-0 */ '1','2','3','4','5','6','7','8','9','0', // digits unchanged, same as MSNGR_KB_LAYOUT_RU leaving them to a separate hold/chord instead of remapping
  /* 0x28 Enter */ '\n',
  /* 0x29 Esc   */ 0,
  /* 0x2A Backspace */ '\b',
  /* 0x2B Tab   */ 0,
  /* 0x2C Space */ ' ',
  /* 0x2D - */ '-', /* 0x2E = */ '=', /* 0x2F [ -> х */ '\xB7', /* 0x30 ] -> ъ */ '\xBC',
  /* 0x31 \ */ '\\', /* 0x32 non-US # */ 0,
  /* 0x33 ; -> ж */ '\xA8', /* 0x34 ' -> э */ '\xBF', /* 0x35 ` -> ё */ (char)BLEKBD_CH_YO,
  /* 0x36 , -> б */ '\xA3', /* 0x37 . -> ю */ '\xC0', /* 0x38 / -> . */ '.', // standard Russian layout: this key types a period, not a slash
};
static const char BLEKBD_HID_KEYMAP_RU_SHIFT[BLEKBD_HID_KEYMAP_LEN] = {
  /* 0x00-0x03 */ 0, 0, 0, 0,
  /* 0x04 A -> Ф */ '\x95', /* 0x05 B -> И */ '\x89', /* 0x06 C -> С */ '\x92',
  /* 0x07 D -> В */ '\x83', /* 0x08 E -> У */ '\x94', /* 0x09 F -> А */ '\x81',
  /* 0x0A G -> П */ '\x90', /* 0x0B H -> Р */ '\x91', /* 0x0C I -> Ш */ '\x99',
  /* 0x0D J -> О */ '\x8F', /* 0x0E K -> Л */ '\x8C', /* 0x0F L -> Д */ '\x85',
  /* 0x10 M -> Ь */ '\x9D', /* 0x11 N -> Т */ '\x93', /* 0x12 O -> Щ */ '\x9A',
  /* 0x13 P -> З */ '\x88', /* 0x14 Q -> Й */ '\x8A', /* 0x15 R -> К */ '\x8B',
  /* 0x16 S -> Ы */ '\x9C', /* 0x17 T -> Е */ '\x86', /* 0x18 U -> Г */ '\x84',
  /* 0x19 V -> М */ '\x8D', /* 0x1A W -> Ц */ '\x97', /* 0x1B X -> Ч */ '\x98',
  /* 0x1C Y -> Н */ '\x8E', /* 0x1D Z -> Я */ '\xA0',
  // Standard Russian keyboard layout's own shift-row (matches every real
  // Russian keyboard/OS "Russian" layout driver): !"#;%:?*() - only 3rd
  // position differs from this file's own EN table below, which is where
  // the real layout has No (the "numero" sign, U+2116) instead of '#'.
  // Left as '#' here (the same char the EN table already gives this key)
  // rather than adding No for real: Org_01.h's own glyph table ends at
  // 0xC1 (Org_01.h:222, right after 'я') with zero room to add a new
  // glyph without hand-drawing one and extending the font file itself -
  // out of scope for a keymap change.
  /* 0x1E-0x27 !"#;%:?*() */ '!','"','#',';','%',':','?','*','(',')',
  /* 0x28 Enter */ '\n',
  /* 0x29 Esc   */ 0,
  /* 0x2A Backspace */ '\b',
  /* 0x2B Tab   */ 0,
  /* 0x2C Space */ ' ',
  /* 0x2D - */ '-', /* 0x2E = */ '=', /* 0x2F [ -> Х */ '\x96', /* 0x30 ] -> Ъ */ '\x9B',
  /* 0x31 \ */ '\\', /* 0x32 non-US # */ 0,
  /* 0x33 ; -> Ж */ '\x87', /* 0x34 ' -> Э */ '\x9E', /* 0x35 ` -> Ё */ (char)BLEKBD_CH_YO,
  /* 0x36 , -> Б */ '\x82', /* 0x37 . -> Ю */ '\x9F', /* 0x38 / -> , */ ',', // standard Russian layout: shifted, this key types a comma
};

#define BLEKBD_HID_USAGE_ESC       0x29
#define BLEKBD_HID_USAGE_BACKSPACE 0x2A
#define BLEKBD_HID_USAGE_UP        0x52
#define BLEKBD_HID_USAGE_DOWN      0x51
#define BLEKBD_HID_USAGE_LEFT      0x50
#define BLEKBD_HID_USAGE_RIGHT     0x4F
#define BLEKBD_HID_USAGE_HOME      0x4A
#define BLEKBD_HID_USAGE_PAGE_UP   0x4B
#define BLEKBD_HID_USAGE_END       0x4D
#define BLEKBD_HID_USAGE_PAGE_DOWN 0x4E
#define BLEKBD_HID_USAGE_TAB  0x2B
#define BLEKBD_HID_USAGE_CAPS_LOCK 0x39
#define BLEKBD_HID_USAGE_A    0x04 // Ctrl+A - quick-open Dialog Mode shortcut, see BLEKBD_CH_OPEN_DIALOG_MODE
#define BLEKBD_HID_USAGE_F2   0x3B // secondary shortcut for the same action, per user request

// Consumer Control page (Usage Page 0x0C) reports - a keyboard's own
// Fn-less top-row media/brightness/volume keys report on this SEPARATE HID
// usage (ESP_HID_USAGE_CCONTROL, esp_hid_common.h), not the Boot Keyboard
// one (ESP_HID_USAGE_KEYBOARD) blekbd_translate_report() below handles -
// they were previously silently dropped entirely (not even logged) since
// blekbd_hidh_cb()'s ESP_HIDH_INPUT_EVENT case only ever matched usage==
// KEYBOARD.
//
// CONFIRMED ON HARDWARE (live DEBUG_LOG capture over the debug UART, not
// guessed): this keyboard's own report descriptor packs Consumer Control
// as a plain 3-byte BITMAP, one fixed bit per key, not real 16-bit HID
// Consumer usage codes (0x006F Brightness Up/0x0070 Down/0x0223 AC Home)
// the way the USB HID Usage Tables spec defines them - two earlier
// attempts assuming a literal usage-code encoding (1-byte truncated or
// 2-byte little-endian) both silently failed to match for exactly this
// reason. This bit layout is specific to this keyboard's own descriptor,
// not a USB HID standard - a different keyboard could easily use a
// different bit (or a real usage-code encoding instead), hence still
// logging every CCONTROL report unconditionally below, unmatched or not.
#define BLEKBD_CC_LEN               3
#define BLEKBD_CC_BYTE_BRIGHTNESS   0
#define BLEKBD_CC_BIT_BRIGHTNESS_DOWN 0x01
#define BLEKBD_CC_BIT_BRIGHTNESS_UP   0x02
#define BLEKBD_CC_BYTE_AC_HOME      1
#define BLEKBD_CC_BIT_AC_HOME       0x80
#define BLEKBD_CC_BYTE_VOLUME       1
#define BLEKBD_CC_BIT_VOLUME_DOWN   0x04
#define BLEKBD_CC_BIT_VOLUME_UP     0x08
#define BLEKBD_CC_BYTE_PLAY_PAUSE   0
#define BLEKBD_CC_BIT_PLAY_PAUSE    0x80
#define BLEKBD_HID_MOD_SHIFT 0x22 // bit1 (left shift) | bit5 (right shift)
#define BLEKBD_HID_MOD_CTRL  0x11 // bit0 (left ctrl) | bit4 (right ctrl)
#define BLEKBD_HID_MOD_ALT   0x44 // bit2 (left alt) | bit6 (right alt)
#define BLEKBD_HID_MOD_GUI   0x88 // bit3 (left GUI/Win/Cmd) | bit7 (right GUI/Win/Cmd)

// Sentinel chars for Up/Down/Left/Right/Home/End/PgUp/PgDn/Alt+Tab/WinKey/
// lang-toggle, pushed through the same queue/blekbd_key_event() path as
// every other key - not used anywhere else in this codebase's char
// conventions (Cyrillic remap is 0x80-0xC1, the on-screen keyboard's own
// meta keys are \b/\n/\x02/\x1b/\x04). Left/Right/Home/End/PgUp/PgDn are
// only consumed by MENU_STATE_MSNGR_CHAT (Menu.h) - Left/Right for cursor
// movement within the compose text, the other four for scrolling/selecting
// within the message history - every other screen leaves them unhandled
// the same as before. Plain Tab (without Alt) is also left unmapped.
#define BLEKBD_CH_UP             0x12
#define BLEKBD_CH_DOWN           0x11
#define BLEKBD_CH_LEFT           0x16
#define BLEKBD_CH_RIGHT          0x17
#define BLEKBD_CH_HOME           0x18
#define BLEKBD_CH_END            0x19
#define BLEKBD_CH_PAGE_UP        0x1A
#define BLEKBD_CH_PAGE_DOWN      0x1C
#define BLEKBD_CH_BRIGHTNESS_UP   0x1D // Consumer Control page, not Keyboard - see BLEKBD_CC_LEN's own comment
#define BLEKBD_CH_BRIGHTNESS_DOWN 0x1E
#define BLEKBD_CH_SOUND_ENABLE    0x1F // Volume Up - Consumer Control page, same as brightness above
#define BLEKBD_CH_SOUND_DISABLE   0x0F // Volume Down
#define BLEKBD_CH_TOGGLE_MARQUEE  0x10 // Play/Pause - Consumer Control page, same as brightness/volume above
#define BLEKBD_CH_OPEN_DIALOG_MODE 0x01 // Ctrl+A or F2 - quick-open Dialog Mode from Inbox/Peer/Bookmarks
#define BLEKBD_CH_OPEN_MESSENGER 0x13 // Alt+Tab
#define BLEKBD_CH_OPEN_SETTINGS  0x14 // WinKey
#define BLEKBD_CH_TOGGLE_LANG    0x15 // Alt+Shift or Ctrl+Shift

// Last report's 6 keycode slots, for edge detection (a physical keyboard
// resends the same report while a key is held, same as any USB keyboard -
// only a keycode's first appearance in a fresh report should type a
// character). Set-membership comparison, not positional - NKRO slot
// ordering isn't guaranteed stable across reports from every keyboard.
static uint8_t blekbd_prev_keys[6] = {0};

// WinKey (GUI) has no keycode of its own - unlike every other key here,
// it only ever shows up as a modifier bit (BLEKBD_HID_MOD_GUI) in byte0,
// so it needs its own press-edge tracking separate from the keys[] array
// edge detection above.
static uint8_t blekbd_prev_modifier = 0;

// Edge tracking for the Consumer Control Brightness Up/Down usages - see
// blekbd_hidh_cb()'s own ESP_HIDH_INPUT_EVENT/CCONTROL branch for why this
// can't just reuse blekbd_prev_keys[] (an entirely separate report/usage,
// arriving on its own ESP_HIDH_INPUT_EVENT with usage==ESP_HID_USAGE_
// CCONTROL, not ESP_HID_USAGE_KEYBOARD).
static bool blekbd_cc_brightness_up_active = false;
static bool blekbd_cc_brightness_down_active = false;
static bool blekbd_cc_ac_home_active = false;
static bool blekbd_cc_volume_up_active = false;
static bool blekbd_cc_volume_down_active = false;
static bool blekbd_cc_play_pause_active = false;

// Hold-to-repeat for Backspace and every plain typed character (letters,
// digits, punctuation, space) - matching the on-screen keyboard's own DEL
// hold-to-repeat timing exactly (MSNGR_KB_DEL_REPEAT_START_MS/_INTERVAL_MS,
// Menu.h - 500ms/120ms). blekbd_translate_report() only runs when a report
// actually arrives (state-change-driven), not on a fixed clock, so it
// alone can't pace a smooth repeat while a key is simply held with no new
// report coming in - blekbd_key_repeat_process() (Menu.h, polled every
// loop() tick on the main task) does the actual timing/repeat-firing using
// this state. Deliberately NOT used for Enter/Escape/arrows/WinKey/Alt+Tab/
// lang-toggle - those are one-shot actions where repeating would be
// actively harmful (e.g. repeat-sending a message) or simply meaningless,
// not "characters" - only the Backspace and printable-table branches below
// ever set this. Single "currently held" slot, not per-keycode - normal
// typing only ever has one key meaningfully held for repeat purposes at a
// time; a fresh press of a different key just takes over.
#define BLEKBD_KEY_REPEAT_START_MS    500
#define BLEKBD_KEY_REPEAT_INTERVAL_MS 120
static bool blekbd_held_active = false;
static uint8_t blekbd_held_keycode = 0;
static char blekbd_held_char = 0;
static unsigned long blekbd_held_press_ms = 0;
static unsigned long blekbd_held_last_repeat_ms = 0;

// blekbd_hidh_cb() (and therefore blekbd_translate_report(), called from
// its ESP_HIDH_INPUT_EVENT case) runs on the esp_hidh event task - a
// dedicated, comparatively tiny stack (esp_hidh_config_t's own
// event_stack_size, 4096 bytes, see blekbd_ensure_hidh_inited() below), NOT
// the Arduino main loop() task. blekbd_key_event() (Menu.h) calls straight
// into Messenger/menu-state code, including messenger_send_lxmf()'s full
// RNS/LXMF packet-construction chain on Enter - calling that directly from
// here crashed the board (confirmed on hardware: typing a plain word with
// no Enter was enough). Exact same class of bug, and exact same fix, as
// esp_hidh_dev_open() needing to be deferred to blekbd_loop() (see this
// file's header) - queue translated characters here, actually dispatch
// them from blekbd_loop() (main task) instead. Single-producer (this
// event task) / single-consumer (blekbd_loop()) ring buffer - typing speed
// is far below loop()'s drain rate, so 16 slots is generous headroom, and
// dropping on a genuinely full queue (rather than blocking) is the right
// failure mode for an event-task callback.
#define BLEKBD_KEY_QUEUE_LEN 16
static char blekbd_key_queue[BLEKBD_KEY_QUEUE_LEN];
static volatile uint8_t blekbd_key_queue_head = 0; // next write slot
static volatile uint8_t blekbd_key_queue_tail = 0; // next read slot

static void blekbd_key_queue_push(char ch) {
  uint8_t next = (uint8_t)((blekbd_key_queue_head + 1) % BLEKBD_KEY_QUEUE_LEN);
  if (next == blekbd_key_queue_tail) return; // full - drop rather than block the event task
  blekbd_key_queue[blekbd_key_queue_head] = ch;
  blekbd_key_queue_head = next;
}

static void blekbd_translate_report(const uint8_t *data, uint16_t length) {
  if (length < 8) return;
  uint8_t modifier = data[0];
  bool shift = (modifier & BLEKBD_HID_MOD_SHIFT) != 0;
  const uint8_t *keys = data + 2;

  for (uint8_t i = 0; i < 6; i++) {
    uint8_t kc = keys[i];
    if (kc == 0x00 || kc == 0x01) continue; // empty slot / rollover-error
    bool was_pressed = false;
    for (uint8_t j = 0; j < 6; j++) {
      if (blekbd_prev_keys[j] == kc) { was_pressed = true; break; }
    }
    if (was_pressed) continue; // still held from the previous report, not a fresh press

    if (kc == BLEKBD_HID_USAGE_ESC) {
      blekbd_key_queue_push('\x1b');
    } else if (kc == BLEKBD_HID_USAGE_BACKSPACE) {
      blekbd_key_queue_push('\b');
      blekbd_held_active = true;
      blekbd_held_keycode = kc;
      blekbd_held_char = '\b';
      blekbd_held_press_ms = millis();
      blekbd_held_last_repeat_ms = 0;
    } else if (kc == BLEKBD_HID_USAGE_UP) {
      blekbd_key_queue_push(BLEKBD_CH_UP);
      blekbd_held_active = true;
      blekbd_held_keycode = kc;
      blekbd_held_char = BLEKBD_CH_UP;
      blekbd_held_press_ms = millis();
      blekbd_held_last_repeat_ms = 0;
    } else if (kc == BLEKBD_HID_USAGE_DOWN) {
      blekbd_key_queue_push(BLEKBD_CH_DOWN);
      blekbd_held_active = true;
      blekbd_held_keycode = kc;
      blekbd_held_char = BLEKBD_CH_DOWN;
      blekbd_held_press_ms = millis();
      blekbd_held_last_repeat_ms = 0;
    } else if (kc == BLEKBD_HID_USAGE_LEFT) {
      blekbd_key_queue_push(BLEKBD_CH_LEFT);
      blekbd_held_active = true;
      blekbd_held_keycode = kc;
      blekbd_held_char = BLEKBD_CH_LEFT;
      blekbd_held_press_ms = millis();
      blekbd_held_last_repeat_ms = 0;
    } else if (kc == BLEKBD_HID_USAGE_RIGHT) {
      blekbd_key_queue_push(BLEKBD_CH_RIGHT);
      blekbd_held_active = true;
      blekbd_held_keycode = kc;
      blekbd_held_char = BLEKBD_CH_RIGHT;
      blekbd_held_press_ms = millis();
      blekbd_held_last_repeat_ms = 0;
    } else if (kc == BLEKBD_HID_USAGE_HOME) {
      blekbd_key_queue_push(BLEKBD_CH_HOME);
    } else if (kc == BLEKBD_HID_USAGE_END) {
      blekbd_key_queue_push(BLEKBD_CH_END);
    } else if (kc == BLEKBD_HID_USAGE_PAGE_UP) {
      blekbd_key_queue_push(BLEKBD_CH_PAGE_UP);
    } else if (kc == BLEKBD_HID_USAGE_PAGE_DOWN) {
      blekbd_key_queue_push(BLEKBD_CH_PAGE_DOWN);
    } else if (kc == BLEKBD_HID_USAGE_TAB && (modifier & BLEKBD_HID_MOD_ALT)) {
      blekbd_key_queue_push(BLEKBD_CH_OPEN_MESSENGER);
    } else if (kc == BLEKBD_HID_USAGE_TAB) {
      // Plain Tab (no Alt) - pushed as the literal '\t' (0x09), an
      // otherwise-unused byte in this file's low-control-char sentinel
      // namespace (see BLEKBD_CH_* below) rather than a new BLEKBD_CH_*
      // define, since '\t' already reads unambiguously as "Tab" at every
      // call site. Currently only meaningful on MENU_STATE_MSNGR_TEXT_
      // ENTRY's hash-entry screen (Menu.h, blekbd_key_event()) - toggles
      // the LXMF/Propagation Type cell regardless of cursor position, per
      // user request; silently ignored everywhere else, same as any
      // other screen-specific shortcut in this file.
      blekbd_key_queue_push('\t');
    } else if (kc == BLEKBD_HID_USAGE_CAPS_LOCK) {
      blekbd_caps_lock = !blekbd_caps_lock;
      blekbd_caps_led_dirty = true;
    } else if ((kc == BLEKBD_HID_USAGE_A && (modifier & BLEKBD_HID_MOD_CTRL)) || kc == BLEKBD_HID_USAGE_F2) {
      // Quick-open Dialog Mode - Ctrl+A or F2 (confirmed on hardware via
      // live capture, user-selected combo), intercepted here before the
      // generic printable-character branch below so Ctrl+A doesn't also
      // type a plain 'a' (Ctrl is otherwise ignored for character
      // resolution - see that branch's own comment).
      blekbd_key_queue_push(BLEKBD_CH_OPEN_DIALOG_MODE);
    } else if (kc < BLEKBD_HID_KEYMAP_LEN && !(modifier & BLEKBD_HID_MOD_GUI)) {
      // GUI (WinKey) held + any other key is an OS-level shortcut combo
      // this specific keyboard hardware-synthesizes (confirmed on
      // hardware: its "Delete/lock" key sends GUI+L, the standard Win+L
      // lock shortcut, as mod=0x08 keys=0x0F) - captured/swallowed here
      // the same way a real OS would, not typed as a literal character.
      // blekbd_prev_modifier's own edge-detection below still separately
      // handles GUI held *alone* (no other key) as this device's own
      // WinKey-opens-Settings shortcut.
      //
      // Caps Lock only flips the case of letters (real HID semantics) -
      // digits/punctuation stay governed by Shift alone - so it's XORed in
      // only for the letter keycode range (0x04-0x1D), not applied to
      // "shift" itself, which the GUI-combo check above still uses as-is.
      bool is_letter = (kc >= 0x04 && kc <= 0x1D);
      bool effective_shift = is_letter ? (shift != blekbd_caps_lock) : shift;
      char ch;
      if (blekbd_lang_ru) ch = effective_shift ? BLEKBD_HID_KEYMAP_RU_SHIFT[kc] : BLEKBD_HID_KEYMAP_RU[kc];
      else                ch = effective_shift ? BLEKBD_HID_KEYMAP_SHIFT[kc]    : BLEKBD_HID_KEYMAP[kc];
      if (ch != 0) {
        blekbd_key_queue_push(ch);
        blekbd_held_active = true;
        blekbd_held_keycode = kc;
        blekbd_held_char = ch;
        blekbd_held_press_ms = millis();
        blekbd_held_last_repeat_ms = 0;
      }
    }
  }

  // Release detection for the hold-to-repeat state above - the loop only
  // ever looks at keycodes actually present in this report, so it never
  // notices the held key disappearing on its own.
  if (blekbd_held_active) {
    bool still_down = false;
    for (uint8_t i = 0; i < 6; i++) if (keys[i] == blekbd_held_keycode) { still_down = true; break; }
    if (!still_down) blekbd_held_active = false;
  }

  memcpy(blekbd_prev_keys, keys, 6);

  // WinKey and Alt+Shift/Ctrl+Shift are both modifier-only combos with no
  // keycode of their own - see blekbd_prev_modifier's own declaration for
  // why these need separate edge detection from the keys[] loop above.
  // Only fires for GUI held *alone* (keys[] empty) - GUI+<key> is a
  // combo (e.g. this hardware's own Win+L "lock" key, see the printable-
  // char branch's own comment above) that shouldn't also pop this
  // device's own Settings shortcut just because GUI happened to be part
  // of it.
  bool gui_pressed = (modifier & BLEKBD_HID_MOD_GUI) != 0;
  bool gui_was_pressed = (blekbd_prev_modifier & BLEKBD_HID_MOD_GUI) != 0;
  bool any_key_down = keys[0] || keys[1] || keys[2] || keys[3] || keys[4] || keys[5];
  if (gui_pressed && !gui_was_pressed && !any_key_down) blekbd_key_queue_push(BLEKBD_CH_OPEN_SETTINGS);

  bool lang_combo = ((modifier & BLEKBD_HID_MOD_ALT) && (modifier & BLEKBD_HID_MOD_SHIFT)) ||
                     ((modifier & BLEKBD_HID_MOD_CTRL) && (modifier & BLEKBD_HID_MOD_SHIFT));
  bool lang_combo_was = ((blekbd_prev_modifier & BLEKBD_HID_MOD_ALT) && (blekbd_prev_modifier & BLEKBD_HID_MOD_SHIFT)) ||
                         ((blekbd_prev_modifier & BLEKBD_HID_MOD_CTRL) && (blekbd_prev_modifier & BLEKBD_HID_MOD_SHIFT));
  if (lang_combo && !lang_combo_was) blekbd_key_queue_push(BLEKBD_CH_TOGGLE_LANG);

  blekbd_prev_modifier = modifier;
}

static void blekbd_hidh_cb(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
  esp_hidh_event_t event = (esp_hidh_event_t)id;
  esp_hidh_event_data_t *param = (esp_hidh_event_data_t *)event_data;

  switch (event) {
    case ESP_HIDH_OPEN_EVENT: {
      if (param->open.status == ESP_OK) {
        blekbd_open_dev = param->open.dev;
        const uint8_t *bda = esp_hidh_dev_bda_get(param->open.dev);
        const char *name = esp_hidh_dev_name_get(param->open.dev);
        DEBUG_LOG("[BLEKBD] OPEN %02x:%02x:%02x:%02x:%02x:%02x name=%s\n",
          bda[0], bda[1], bda[2], bda[3], bda[4], bda[5], name ? name : "?");
        if (blekbd_connect_is_active_pairing) {
          blekbd_peer_conf_save(blekbd_found_addr, blekbd_found_addr_type);
          blekbd_pair_result = BLEKBD_PAIR_OK;
          blekbd_pair_result_at_ms = millis();
        } else {
          // Ambient "reconnected" notice for every OTHER open - the initial
          // pairing flow already has its own dedicated "Paired!" status
          // (MENU_STATE_BLEKBD_PAIRING, above), so this only fires for the
          // background auto-reconnects (keyboard woke up/came back in
          // range) that would otherwise happen with zero on-screen
          // feedback. Consumed by blekbd_notice_process()
          // (Menu.h) - can't call menu_open_popup() directly here, same
          // include-order reason as blekbd_pair_result_process().
          blekbd_connected_popup_pending = true;
        }
        // Discover the OUTPUT report's real map_index/report_id (can't
        // assume it shares the INPUT report's own report_id) - safe here,
        // no GATT I/O, just reads the already-cached report list. Reset
        // caps state fresh per connection and request an LED sync (off)
        // from blekbd_loop() so a keyboard reconnecting mid-caps-on
        // doesn't show a stale LED.
        blekbd_output_report_known = false;
        blekbd_caps_lock = false;
        size_t num_reports = 0;
        esp_hid_report_item_t *reports = NULL;
        if (esp_hidh_dev_reports_get(param->open.dev, &num_reports, &reports) == ESP_OK && reports) {
          for (size_t i = 0; i < num_reports; i++) {
            if (reports[i].report_type == ESP_HID_REPORT_TYPE_OUTPUT) {
              blekbd_output_map_index = reports[i].map_index;
              blekbd_output_report_id = reports[i].report_id;
              blekbd_output_report_known = true;
              break;
            }
          }
          free(reports);
        }
        blekbd_caps_led_dirty = blekbd_output_report_known;
      } else {
        DEBUG_LOG("[BLEKBD] OPEN FAILED status=%d\n", (int)param->open.status);
        blekbd_connecting = false;
        blekbd_rescan_pending = true;
        if (blekbd_connect_is_active_pairing) blekbd_pair_result = BLEKBD_PAIR_FAILED;
      }
      blekbd_connect_is_active_pairing = false;
      break;
    }
    case ESP_HIDH_INPUT_EVENT: {
      if (param->input.usage == ESP_HID_USAGE_KEYBOARD) {
        if (param->input.length >= 8) {
          DEBUG_LOG("[BLEKBD] INPUT mod=%02x keys=%02x %02x %02x %02x %02x %02x\n",
            param->input.data[0], param->input.data[2], param->input.data[3],
            param->input.data[4], param->input.data[5], param->input.data[6], param->input.data[7]);
        } else {
          DEBUG_LOG("[BLEKBD] INPUT unexpected len=%u (expected >=8, assumed boot-compatible layout)\n", param->input.length);
        }
        blekbd_translate_report(param->input.data, param->input.length);
      } else if (param->input.usage == ESP_HID_USAGE_CCONTROL) {
        // Media/brightness/volume keys - a keyboard's own Fn-less top row,
        // previously silently dropped entirely (see BLEKBD_CC_LEN's own
        // comment for why - wrong usage page, then two failed guesses at
        // the wire format before capturing the real bytes on hardware).
        // Logged unconditionally, regardless of whether it matches
        // anything below - this keyboard's own confirmed layout won't
        // necessarily match a different one.
        char hexbuf[24] = {0};
        for (uint16_t i = 0; i < param->input.length && i < 8; i++) {
          snprintf(hexbuf + i * 3, sizeof(hexbuf) - i * 3, "%02x ", param->input.data[i]);
        }
        DEBUG_LOG("[BLEKBD] CCONTROL len=%u data=%s\n", param->input.length, hexbuf);

        // Confirmed-on-hardware 3-byte bitmap (BLEKBD_CC_LEN's own
        // comment) - byte[BLEKBD_CC_BYTE_BRIGHTNESS] bit 0x01/0x02 for
        // Down/Up, byte[BLEKBD_CC_BYTE_AC_HOME] bit 0x80 for AC Home,
        // byte[BLEKBD_CC_BYTE_VOLUME] bit 0x04/0x08 for Down/Up, byte
        // [BLEKBD_CC_BYTE_PLAY_PAUSE] bit 0x80 for Play/Pause.
        bool up_active = false, down_active = false, ac_home_active = false;
        bool vol_up_active = false, vol_down_active = false, play_pause_active = false;
        if (param->input.length == BLEKBD_CC_LEN) {
          up_active = (param->input.data[BLEKBD_CC_BYTE_BRIGHTNESS] & BLEKBD_CC_BIT_BRIGHTNESS_UP) != 0;
          down_active = (param->input.data[BLEKBD_CC_BYTE_BRIGHTNESS] & BLEKBD_CC_BIT_BRIGHTNESS_DOWN) != 0;
          ac_home_active = (param->input.data[BLEKBD_CC_BYTE_AC_HOME] & BLEKBD_CC_BIT_AC_HOME) != 0;
          vol_up_active = (param->input.data[BLEKBD_CC_BYTE_VOLUME] & BLEKBD_CC_BIT_VOLUME_UP) != 0;
          vol_down_active = (param->input.data[BLEKBD_CC_BYTE_VOLUME] & BLEKBD_CC_BIT_VOLUME_DOWN) != 0;
          play_pause_active = (param->input.data[BLEKBD_CC_BYTE_PLAY_PAUSE] & BLEKBD_CC_BIT_PLAY_PAUSE) != 0;
        }
        if (up_active && !blekbd_cc_brightness_up_active) blekbd_key_queue_push(BLEKBD_CH_BRIGHTNESS_UP);
        if (down_active && !blekbd_cc_brightness_down_active) blekbd_key_queue_push(BLEKBD_CH_BRIGHTNESS_DOWN);
        // Pushed as a literal Esc ('\x1b'), not a new sentinel - needs no
        // dispatch changes anywhere else at all, since it's indistinguishable
        // from a real Esc keypress everywhere downstream.
        if (ac_home_active && !blekbd_cc_ac_home_active) blekbd_key_queue_push('\x1b');
        // Volume Up/Down repurposed to enable/disable the buzzer, per user
        // request - this device has no analog volume control for these to
        // meaningfully adjust anyway.
        if (vol_up_active && !blekbd_cc_volume_up_active) blekbd_key_queue_push(BLEKBD_CH_SOUND_ENABLE);
        if (vol_down_active && !blekbd_cc_volume_down_active) blekbd_key_queue_push(BLEKBD_CH_SOUND_DISABLE);
        // Play/Pause repurposed to toggle marquee scrolling on/off, per
        // user request.
        if (play_pause_active && !blekbd_cc_play_pause_active) blekbd_key_queue_push(BLEKBD_CH_TOGGLE_MARQUEE);
        blekbd_cc_brightness_up_active = up_active;
        blekbd_cc_brightness_down_active = down_active;
        blekbd_cc_ac_home_active = ac_home_active;
        blekbd_cc_volume_up_active = vol_up_active;
        blekbd_cc_volume_down_active = vol_down_active;
        blekbd_cc_play_pause_active = play_pause_active;
      }
      break;
    }
    case ESP_HIDH_CLOSE_EVENT: {
      DEBUG_LOG("[BLEKBD] CLOSE reason=%d\n", param->close.reason);
      blekbd_open_dev = NULL;
      blekbd_connecting = false;
      blekbd_rescan_pending = true;
      blekbd_held_active = false; // don't keep repeating forever if the keyboard drops mid-hold
      blekbd_caps_lock = false;
      blekbd_caps_led_dirty = false;
      blekbd_output_report_known = false;
      // Fires for every close - a deliberate Forget Keyboard just as much
      // as the keyboard passively dropping out of range/going to sleep -
      // both are worth a visible confirmation, unlike ESP_HIDH_OPEN_EVENT's
      // own pairing-flow exclusion (which already has its own dedicated
      // "Paired!" status screen to not duplicate).
      blekbd_disconnected_popup_pending = true;
      break;
    }
    default:
      break;
  }
}

static int blekbd_gap_event(struct ble_gap_event *event, void *arg) {
  switch (event->type) {
    case BLE_GAP_EVENT_DISC: {
      struct ble_hs_adv_fields fields;
      int rc = ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);
      if (rc != 0) return 0;

      char namebuf[BLEKBD_NAME_MAX_LEN + 1] = {0};
      if (fields.name && fields.name_len) {
        int n = fields.name_len < BLEKBD_NAME_MAX_LEN ? fields.name_len : BLEKBD_NAME_MAX_LEN;
        memcpy(namebuf, fields.name, n);
        namebuf[n] = 0;
      }

      const uint8_t *addr = event->disc.addr.val;

      if (blekbd_scan_mode == BLEKBD_SCAN_PASSIVE) {
        // Background reconnect - match only the one specific keyboard the
        // user paired via the menu. No UUID check: a keyboard's ordinary
        // "wake on keypress, reconnect using the existing bond"
        // advertisement doesn't carry the HID service UUID at all (only its
        // deliberate pairing-mode ad does) - address is the only reliable
        // signal here.
        if (!blekbd_peer_stored) return 0;
        bool is_match = (event->disc.addr.type == blekbd_peer_addr_type && memcmp(addr, blekbd_peer_addr, 6) == 0);
        if (is_match && !blekbd_connecting && !blekbd_found_pending) {
          blekbd_found_pending = true;
          memcpy(blekbd_found_addr, addr, 6);
          blekbd_found_addr_type = event->disc.addr.type;
          DEBUG_LOG("[BLEKBD] paired keyboard in range, waiting for scan to time out naturally\n");
        }
        return 0;
      }

      if (blekbd_scan_mode == BLEKBD_SCAN_ACTIVE_DISCOVERY) {
        // Menu-driven discovery - broader match (advertised HID service
        // UUID), populates the selectable list. Never auto-connects; the
        // actual pair only happens from the user's explicit selection
        // (blekbd_connect_start(), called from Menu.h).
        bool is_hid = false;
        for (int i = 0; i < fields.num_uuids16; i++) {
          if (ble_uuid_u16(&fields.uuids16[i].u) == BLEKBD_HID_SVC_UUID) { is_hid = true; break; }
        }
        if (is_hid) {
          blekbd_discovered_upsert(addr, event->disc.addr.type, namebuf, event->disc.rssi);
        }
        return 0;
      }

      return 0;
    }
    case BLE_GAP_EVENT_DISC_COMPLETE:
      if (blekbd_found_pending && !blekbd_connecting) {
        blekbd_found_pending = false;
        blekbd_connecting = true;
        DEBUG_LOG("[BLEKBD] connect pending, deferring to main task\n");
        // Do NOT call esp_hidh_dev_open() here - see this file's header.
        blekbd_connect_pending = true;
      } else if (!blekbd_connecting && blekbd_scan_mode != BLEKBD_SCAN_IDLE) {
        // Normal path every BLEKBD_SCAN_MS while idle (no target found yet)
        // - immediately restart, so the RNode is effectively scanning
        // continuously in bounded bursts rather than sitting idle. Skipped
        // once the Scan screen is closed or the master switch is turned off
        // (either way, blekbd_scan_mode is set back to IDLE by then - see
        // blekbd_scan_stop()/blekbd_stop()). Deliberately NOT also gated on
        // the live blekbd_enabled here - that only reflects the outer
        // Settings tree's last Save & Exit, not a switch just flipped ON in
        // the current menu session, and gating on it silently killed
        // continuous scanning mid-session (see blekbd_loop()'s own comment).
        blekbd_rescan_pending = true;
      }
      return 0;
    case BLE_GAP_EVENT_CONNECT:
      DEBUG_LOG("[BLEKBD] gap connect status=%d\n", event->connect.status);
      return 0;
    case BLE_GAP_EVENT_DISCONNECT:
      DEBUG_LOG("[BLEKBD] gap disconnect reason=%d\n", event->disconnect.reason);
      blekbd_connecting = false;
      blekbd_open_dev = NULL;
      // Seamless reconnect: a previously-paired keyboard should just work
      // again once it's back in range/awake, without a manual BT toggle -
      // resume scanning automatically instead of going idle.
      blekbd_rescan_pending = true;
      return 0;
    case BLE_GAP_EVENT_PASSKEY_ACTION:
      DEBUG_LOG("[BLEKBD] passkey action=%d\n", (int)event->passkey.params.action);
      return 0;
    default:
      return 0;
  }
}

// Scans for BLEKBD_SCAN_MS, restarted immediately by the DISC_COMPLETE
// handler above if nothing matched and blekbd_scan_mode is still active -
// effectively continuous scanning without ever calling
// ble_gap_disc_cancel() (see this file's header). Single shared low-level
// primitive for both scan modes - callers set blekbd_scan_mode first.
static void blekbd_start_scan() {
  blekbd_connecting = false;
  blekbd_found_pending = false;

  // BLESerial's own peripheral advertising can preempt an in-progress scan
  // (confirmed live: BLE_HS_EPREEMPTED/reason=29 right after "GAP procedure
  // initiated: advertise" from somewhere else) - re-assert it's off on
  // every (re)scan start, not just the very first one.
  int adv_stop_rc = ble_gap_adv_stop();

  uint8_t own_addr_type;
  int rc = ble_hs_id_infer_auto(0, &own_addr_type);
  if (rc != 0) {
    DEBUG_LOG("[BLEKBD] addr infer failed rc=%d\n", rc);
    return;
  }

  struct ble_gap_disc_params disc_params = {0};
  disc_params.passive = 0;
  disc_params.itvl = 0x50;
  disc_params.window = 0x30;
  disc_params.filter_duplicates = 1;

  rc = ble_gap_disc(own_addr_type, BLEKBD_SCAN_MS, &disc_params, blekbd_gap_event, NULL);
  DEBUG_LOG("[BLEKBD] scan start adv_stop_rc=%d disc_rc=%d\n", adv_stop_rc, rc);
}

static void blekbd_ensure_hidh_inited() {
  if (blekbd_hidh_inited) return;
  esp_hidh_config_t config = {
    .callback = blekbd_hidh_cb,
    .event_stack_size = 4096,
    .callback_arg = NULL,
  };
  esp_err_t err = esp_hidh_init(&config);
  DEBUG_LOG("[BLEKBD] esp_hidh_init=%d\n", (int)err);
  blekbd_hidh_inited = true;
}

void blekbd_passive_scan_start() {
  blekbd_ensure_hidh_inited();
  blekbd_scan_mode = BLEKBD_SCAN_PASSIVE;
  blekbd_start_scan();
}

// Called from Menu.h when the user opens MENU_STATE_BLEKBD_SCAN.
void blekbd_discovery_start() {
  blekbd_ensure_hidh_inited();
  blekbd_discovered_clear();
  blekbd_scan_mode = BLEKBD_SCAN_ACTIVE_DISCOVERY;
  blekbd_start_scan();
}

// Called from Menu.h when the user leaves MENU_STATE_BLEKBD_SCAN (BACK) or
// confirms a pairing selection. Does NOT call ble_gap_disc_cancel() (see
// this file's header) - an in-flight bounded scan is simply left to time
// out naturally; the DISC_COMPLETE handler checks blekbd_scan_mode and
// won't restart it once this is called.
void blekbd_scan_stop() {
  blekbd_scan_mode = BLEKBD_SCAN_IDLE;
}

// Called from Menu.h when the user confirms PAIR on a MENU_STATE_BLEKBD_SCAN
// row. Bypasses the GAP-found-pending step entirely since the address is
// already known - still goes through the same deferred-to-main-task
// esp_hidh_dev_open() discipline as the passive path (blekbd_loop() below).
void blekbd_connect_start(const uint8_t addr[6], uint8_t addr_type) {
  memcpy(blekbd_found_addr, addr, 6);
  blekbd_found_addr_type = addr_type;
  blekbd_connecting = true;
  blekbd_connect_is_active_pairing = true;
  blekbd_connect_pending = true;
  blekbd_connect_retry_at_ms = 0;
  blekbd_connect_deadline_ms = millis() + BLEKBD_CONNECT_MAX_WAIT_MS;
}

// Called from Menu.h when the user turns the master switch off. Tears down
// any in-flight state without ever touching ble_gap_disc_cancel().
void blekbd_stop() {
  if (blekbd_open_dev) {
    esp_hidh_dev_close(blekbd_open_dev);
    blekbd_open_dev = NULL;
  }
  blekbd_connect_pending = false;
  blekbd_connect_retry_at_ms = 0;
  blekbd_rescan_pending = false;
  blekbd_scan_mode = BLEKBD_SCAN_IDLE;
}

static uint8_t blekbd_prev_bt_state = BT_STATE_OFF;

void blekbd_loop() {
  if (!blekbd_conf_loaded) blekbd_conf_load();

  // Only the auto-reconnect-on-BT-power-up edge trigger below obeys the
  // live, PERSISTED blekbd_enabled - that's what "opt-in master switch"
  // actually means: whether the RNode reconnects on its own with no menu
  // interaction at all. connect_pending/rescan_pending further down must
  // NOT be gated on it: they're only ever set by explicit user action in
  // MENU_STATE_BLEKBD_SCAN/_PAIR_CONFIRM (via blekbd_scan_mode/
  // blekbd_connect_start()), which have to keep working the moment the
  // switch is flipped ON in the menu - before the outer Settings tree's
  // Save & Exit has actually copied staged_blekbd_enabled into this live
  // variable. Gating the whole function on blekbd_enabled here used to
  // silently drop every connect/rescan attempt made before that Save &
  // Exit (the obvious, expected order of operations - toggle on, then
  // immediately scan/pair in the same menu session), reproducing as
  // "stuck Pairing.../Scanning..." with zero [BLEKBD] log output.
  if (blekbd_enabled) {
    // Edge-triggered on leaving BT_STATE_OFF, for EITHER a short-tap
    // bt_start() (BT_STATE_ON - the plain phone-serial-link toggle) or the
    // >5s hold's bt_enable_pairing() (BT_STATE_PAIRING). Re-arms whenever
    // bt_state returns to OFF (bt_stop()). Only starts the PASSIVE
    // background scan, and only if a keyboard has actually been paired via
    // the menu - there's nothing to reconnect to otherwise.
    bool was_off = (blekbd_prev_bt_state == BT_STATE_OFF || blekbd_prev_bt_state == BT_STATE_NA);
    bool now_on = (bt_state == BT_STATE_ON || bt_state == BT_STATE_PAIRING || bt_state == BT_STATE_CONNECTED);
    if (was_off && now_on && blekbd_peer_stored) {
      // BLEDevice::init() (BLESerial::begin(), called from bt_start()) must
      // have already returned by the time this fires - bt_state only
      // leaves OFF after that succeeds (Bluetooth.h). Small extra delay so
      // the NimBLE host task is fully synced (ble_hs_synced) before we
      // touch GAP.
      delay(300);
      blekbd_passive_scan_start();
    }
  }
  blekbd_prev_bt_state = bt_state;

  // Retry a connect attempt that previously failed synchronously (EBUSY
  // while the scan that found this device was still finishing) - see
  // blekbd_connect_retry_at_ms's own declaration.
  if (blekbd_connect_retry_at_ms != 0 && (int32_t)(millis() - blekbd_connect_retry_at_ms) >= 0) {
    blekbd_connect_retry_at_ms = 0;
    blekbd_connect_pending = true;
  }

  // Runs on the Arduino main task (safe context) - see this file's header.
  if (blekbd_connect_pending) {
    blekbd_connect_pending = false;
    DEBUG_LOG("[BLEKBD] connecting...\n");
    esp_hidh_dev_t *dev = esp_hidh_dev_open(blekbd_found_addr, ESP_HID_TRANSPORT_BLE, blekbd_found_addr_type);
    if (dev == NULL) {
      // Synchronous failure - esp_ble_hidh_dev_open() bails out before ever
      // posting ESP_HIDH_OPEN_EVENT on this path (nimble_hidh.c), so
      // blekbd_hidh_cb() will never fire for this attempt; without this
      // retry the PAIRING screen was stuck on "Pairing..." forever with no
      // error - see blekbd_connect_retry_at_ms's own declaration.
      if ((int32_t)(millis() - blekbd_connect_deadline_ms) < 0) {
        DEBUG_LOG("[BLEKBD] open failed synchronously, retrying\n");
        blekbd_connect_retry_at_ms = millis() + BLEKBD_CONNECT_RETRY_MS;
      } else {
        DEBUG_LOG("[BLEKBD] connect retry budget exhausted, giving up\n");
        blekbd_connecting = false;
        if (blekbd_connect_is_active_pairing) blekbd_pair_result = BLEKBD_PAIR_FAILED;
        blekbd_connect_is_active_pairing = false;
      }
    }
  }

  // Seamless reconnect - see blekbd_rescan_pending's declaration.
  if (blekbd_rescan_pending && bt_state != BT_STATE_OFF) {
    blekbd_rescan_pending = false;
    blekbd_start_scan();
  }

  // Hold-to-repeat firing itself lives in Menu.h (blekbd_key_repeat_
  // process(), polled from loop() right alongside this function) - it
  // needs menu_state to only repeat while actually composing/editing text
  // (MENU_STATE_MSNGR_TEXT_ENTRY), not cascade through multiple Back
  // presses if Backspace is physically held elsewhere, and menu_state
  // isn't visible yet at this point in the include order.

  // Drain keystrokes queued by blekbd_translate_report() (esp_hidh event
  // task) - see blekbd_key_queue_push()'s own comment for why this can't
  // just call blekbd_key_event() directly from there. Draining the whole
  // queue every tick (not just one character) is safe here: blekbd_key_
  // event() itself is cheap except on Enter (messenger_send_lxmf()), and a
  // human can't type fast enough to keep more than one or two characters
  // queued between loop() iterations anyway.
  while (blekbd_key_queue_tail != blekbd_key_queue_head) {
    char ch = blekbd_key_queue[blekbd_key_queue_tail];
    blekbd_key_queue_tail = (uint8_t)((blekbd_key_queue_tail + 1) % BLEKBD_KEY_QUEUE_LEN);
    blekbd_key_event(ch);
  }

  // Deferred Caps Lock LED write - must run here (main task), never from
  // blekbd_translate_report()/blekbd_hidh_cb() (esp_hidh event task) - see
  // constraint #3 in this file's header comment. Standard boot-keyboard
  // OUTPUT report is a single-byte LED bitmap; bit1 is Caps Lock.
  if (blekbd_caps_led_dirty && blekbd_output_report_known && blekbd_open_dev) {
    uint8_t led_byte = blekbd_caps_lock ? 0x02 : 0x00;
    esp_hidh_dev_output_set(blekbd_open_dev, blekbd_output_map_index, blekbd_output_report_id, &led_byte, 1);
    blekbd_caps_led_dirty = false;
  }
}

// Definition lives in Menu.h (polled from loop(), RNode_Firmware.ino, right
// after blekbd_loop()) - it touches menu_state/MENU_STATE_BLEKBD_*, which
// aren't declared yet at this point in the include order (Menu.h is
// #include'd after this file, same layering reason msngr_send_result_
// process() lives in Menu.h instead of Messenger.h).
void blekbd_pair_result_process();

// Definition lives in Menu.h - see blekbd_pair_result_process()'s own
// comment on why. Polled from loop() right alongside it - consumes
// blekbd_connected_popup_pending/blekbd_disconnected_popup_pending, above.
void blekbd_notice_process();

// Definition lives in Menu.h - see blekbd_loop()'s own comment on why
// (needs menu_state, not visible yet here). Polled from loop() right
// alongside blekbd_loop()/blekbd_pair_result_process().
void blekbd_key_repeat_process();

#else
inline void blekbd_loop() {}
inline void blekbd_pair_result_process() {}
inline void blekbd_notice_process() {}
inline void blekbd_key_repeat_process() {}
#endif
