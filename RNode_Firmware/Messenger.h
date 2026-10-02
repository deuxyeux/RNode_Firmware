// Emergency RNode-only LXMF messenger (Tier 1) - a separate on-device
// "application" built on top of URNS.h's onboard microReticulum/LXMF
// node. Opened either via a dedicated main-button hold tier
// (BUTTON_HOLD_TIER_MESSENGER, Menu.h) or the RNode Settings > Messenger
// entry - see MENU_STATE_MSNGR_* (Menu.h) for the on-screen flow.
//
// No propagation, no text entry yet (Tier 2 on-screen keyboard) - sending
// is limited to three fixed presets (Hi/Bye/SOS) aimed at bookmarked
// addresses, addresses picked from the 1-hop announce list below, or
// addresses that have already messaged us (Inbox).
//
// Depends on URNS.h having already run urns_init() successfully
// (urns_ready) - messenger_init() (called right after, RNode_Firmware.ino)
// assumes the "urns" LittleFS partition is mounted, the filesystem is
// already registered with RNS::Utilities::OS, and urns_lxmf_router exists.

#ifndef MESSENGER_H
  #define MESSENGER_H

  // Same MTU-macro clobbering hazard URNS.h's own two includes guard
  // against (Config.h's plain #define MTU 508 vs microReticulum's
  // namespaced RNS::Type::Reticulum::MTU) - LXMessage.h/Type.h are
  // already fully included (and header-guarded) by URNS.h's own
  // <LXMF/LXMRouter.h> include by the time this file is reached, so this
  // is likely a no-op re-inclusion in practice, but guarding it costs
  // nothing and keeps this file safe even if that stops being true.
  #pragma push_macro("MTU")
  #undef MTU
  #include <LXMF/MessageStore.h>
  #include <LXMF/AudioField.h>
  #include <LXMF/PropagationNodeManager.h>
  #pragma pop_macro("MTU")

  // Not reliably pulled in transitively just via Arduino.h in this build -
  // lib/microLXMF's own LXStamper.cpp needs these same explicit includes
  // for its own FreeRTOS task/mutex use.
  #ifdef ESP_PLATFORM
    #include <freertos/FreeRTOS.h>
    #include <freertos/semphr.h>
  #endif

  #include <ArduinoJson.h>

  #define MSNGR_BOOKMARKS_PATH "/urns/bookmarks.json"
  #define MSNGR_NAME_MAX_LEN   31 // +NUL = 32
  #define MSNGR_MAX_BOOKMARKS  12
  #define MSNGR_MAX_ANNOUNCES  8
  #define MSNGR_PEER_MAX_MSG_ROWS 5

  // Cap for MessengerPeerMsgCacheRow's own snippet below - wide enough for
  // the marquee-scroll feature (MENU_STATE_MSNGR_PEER, Menu.h) to have real
  // hidden tail text to reveal on a long message. Deliberately short (this
  // is x5, permanently resident for as long as MENU_STATE_MSNGR_PEER's
  // cache is populated) - unlike messenger_refresh_msg_detail_cache()'s own
  // MSG_DETAIL/MSG_VIEW cache below, which decodes the message's full
  // stored length on demand into a buffer sized off the content itself, so
  // a long message is never truncated there.
  #define MSNGR_PEER_SNIPPET_CAP 160

  // A bookmark is either an LXMF peer (the default - Ping/Send Hi-Bye-SOS/
  // Compose target, MENU_STATE_MSNGR_PEER) or a propagation node (added
  // via Bookmarks > Add by Hash with Type switched to Propagation,
  // Menu.h's MSNGR_KB_LAYOUT_HEX row 2) - the latter isn't an LXMF
  // delivery destination at all (it answers on the "lxmf"/"propagation"
  // aspect, not "lxmf"/"delivery"), so MENU_STATE_MSNGR_PEER swaps its
  // entire action set for one of these instead of showing Ping/Compose/
  // Clear Conversation - see msngr_bookmark_is_prop_node().
  #define MSNGR_BOOKMARK_TYPE_LXMF        0
  #define MSNGR_BOOKMARK_TYPE_PROPAGATION 1

  // Per-contact desired outbound send method for an LXMF peer - MENU_STATE_
  // MSNGR_PEER's own "Send Direct"/"Send Propagated" row (Menu.h's MSNGR_
  // PEER_FIXED_ACTION_DELIVERY_MODE), right below Ping. DIRECT is every
  // send's behavior before this setting existed - OPPORTUNISTIC, silently
  // upgraded to DIRECT (an established Link) by LXMRouter for anything over
  // LORA_ENCRYPTED_PACKET_MDU, same as msngr_send_method's own comment
  // describes. PROPAGATED skips that path entirely - messenger_send_lxmf_
  // resolved() (below) requests LXMF::Type::Message::PROPAGATED up front,
  // which LXMRouter::handle_outbound() routes straight to the active
  // propagation node (Bookmarks > <node> > Set Active) instead of ever
  // attempting a direct exchange with the recipient. Persisted per-bookmark
  // (MessengerBookmark::delivery_mode below) - see messenger_current_
  // delivery_mode()'s own comment for what happens when the peer isn't
  // bookmarked.
  #define MSNGR_DELIVERY_MODE_DIRECT     0
  #define MSNGR_DELIVERY_MODE_PROPAGATED 1

  struct MessengerBookmark {
    bool in_use = false;
    uint8_t hash[LXMF::PEER_HASH_SIZE];
    char name[MSNGR_NAME_MAX_LEN + 1];
    uint8_t type = MSNGR_BOOKMARK_TYPE_LXMF;
    uint8_t delivery_mode = MSNGR_DELIVERY_MODE_DIRECT;
  };
  MessengerBookmark msngr_bookmarks[MSNGR_MAX_BOOKMARKS];
  uint8_t msngr_bookmark_count = 0;

  // The propagation-node bookmark currently in effect for outbound
  // PROPAGATED delivery/sync (RNode Settings > Messenger > Bookmarks >
  // <a Propagation-type bookmark> > Set Active) - empty when none is
  // selected, which also means PROPAGATED delivery/periodic sync/
  // Propagate on Fail are all inert regardless of their own settings
  // (LXMRouter::set_outbound_propagation_node() with an empty hash is
  // exactly what disables them - see messenger_prop_node_set_active()/
  // _clear_active() below). Persisted alongside the bookmarks themselves
  // (messenger_bookmarks_save/_load below) rather than EEPROM - it's a
  // 16-byte hash, and the bookmarks file already exists on LittleFS for
  // this exact kind of data.
  RNS::Bytes msngr_active_prop_node_hash;

  // User-configurable quick-send buttons on MENU_STATE_MSNGR_PEER (Menu.h) -
  // used to be a fixed compile-time {"Hi","Bye","SOS"}. Unlike bookmarks/
  // announces (removed by matching a hash, so their backing array is
  // sparse/in_use-flagged and order doesn't matter), presets are removed
  // by *position* ("delete preset #2"), so this stays packed and
  // contiguous instead - indices [0, msngr_preset_count) are always the
  // live entries, no gaps. Capped at 5 mainly to keep MENU_STATE_MSNGR_
  // PEER's action-row list from growing unbounded, not a technical limit.
  // MSNGR_NAME_MAX_LEN (31), not MSNGR_TEXT_ENTRY_MAX_LEN (140) - a
  // preset is a button label as much as message content ("Send: <text>"
  // on the peer screen), so it gets the same short cap bookmark/display
  // names already use.
  #define MSNGR_MAX_PRESETS 5
  #define MSNGR_PRESETS_PATH "/urns/msngr_presets.json"
  char msngr_presets[MSNGR_MAX_PRESETS][MSNGR_NAME_MAX_LEN + 1];
  uint8_t msngr_preset_count = 0;

  // Outbound LXMF delivery retry count - forwarded to urns_lxmf_router's
  // LXMRouter::set_max_delivery_attempts() (from messenger_init() below,
  // and again here on every save) - RNode Settings > Messenger > Settings
  // > Retries in the menu (Menu.h). Default matches LXMRouter's own
  // compiled default (5, same as the Python reference implementation) -
  // see ADDR_CONF_MSNGR_RETRIES (ROM.h) for the persisted-value/range
  // details.
  #define MSNGR_MAX_RETRIES_DEFAULT 5
  uint8_t msngr_max_retries = MSNGR_MAX_RETRIES_DEFAULT;

  void msngr_retries_conf_save(uint8_t retries) {
    msngr_max_retries = retries;
    eeprom_update(ADDR_CONF_MSNGR_RETRIES, retries);
    if (urns_lxmf_router) urns_lxmf_router->set_max_delivery_attempts(retries);
  }

  // Delay (seconds) between outbound LXMF delivery retries - forwarded to
  // LXMRouter::set_outbound_retry_delay(). RNode Settings > Messenger >
  // Settings > Retry Delay in the menu (Menu.h), right after Retries.
  // Default matches LXMRouter's own compiled default (10s, same as the
  // Python reference implementation's DELIVERY_RETRY_WAIT) - see
  // ADDR_CONF_MSNGR_RETRY_DELAY (ROM.h) for the persisted-value/range
  // details.
  #define MSNGR_RETRY_DELAY_DEFAULT 10
  uint8_t msngr_retry_delay_s = MSNGR_RETRY_DELAY_DEFAULT;

  void msngr_retry_delay_conf_save(uint8_t seconds) {
    msngr_retry_delay_s = seconds;
    eeprom_update(ADDR_CONF_MSNGR_RETRY_DELAY, seconds);
    if (urns_lxmf_router) urns_lxmf_router->set_outbound_retry_delay((double)seconds);
  }

  #if HAS_AUDIO == true
    // Voice playback volume in percent (10-100, steps of 10) - see
    // ADDR_CONF_MSNGR_PLAYBACK_VOLUME (ROM.h). Used by messenger_voice_play()
    // and the voice screen's preview (Menu.h); Audio.cpp's es_set_volume()
    // maps it to the codec's DAC register.
    #define MSNGR_PLAYBACK_VOLUME_DEFAULT 100
    uint8_t msngr_playback_volume_pct = MSNGR_PLAYBACK_VOLUME_DEFAULT;

    void msngr_playback_volume_conf_save(uint8_t pct) {
      msngr_playback_volume_pct = pct;
      eeprom_update(ADDR_CONF_MSNGR_PLAYBACK_VOLUME, pct);
    }
  #endif

  // Whether RNode_Firmware.ino's existing one-shot post-boot LXMF announce
  // (urns_announce_lxmf(), URNS.h) actually runs - gates that call, does
  // NOT touch LXMRouter's own _announce_at_start (see ADDR_CONF_MSNGR_
  // ANNOUNCE_AT_START's comment, ROM.h, for why). Default true - the
  // one-shot fires unconditionally today, this must preserve that for a
  // fresh/erased EEPROM.
  bool msngr_announce_at_start = true;

  void msngr_announce_at_start_conf_save(bool enabled) {
    msngr_announce_at_start = enabled;
    eeprom_update(ADDR_CONF_MSNGR_ANNOUNCE_AT_START,
      enabled ? MSNGR_ANNOUNCE_AT_START_ENABLE_BYTE : MSNGR_ANNOUNCE_AT_START_DISABLE_BYTE);
  }

  // Periodic LXMF re-announce interval, as a preset index into this table
  // (seconds) - forwarded to LXMRouter::set_announce_interval(), which
  // (as of this feature) process_outbound() actually checks on every
  // loop() iteration. Index 0 (Off) matches the pre-existing behavior of
  // no periodic auto-announce at all.
  #define MSNGR_ANNOUNCE_INTERVAL_PRESET_COUNT 8
  const uint32_t msngr_announce_interval_presets_s[MSNGR_ANNOUNCE_INTERVAL_PRESET_COUNT] = {
    0,      // Off
    900,    // 15 min
    1800,   // 30 min
    3600,   // 1h
    7200,   // 2h
    10800,  // 3h
    21600,  // 6h
    43200   // 12h
  };
  // Display labels, same index as msngr_announce_interval_presets_s above -
  // Menu.h's MSNGR_SETTINGS/_EDIT draw code uses this directly rather than
  // formatting seconds itself.
  const char *const msngr_announce_interval_labels[MSNGR_ANNOUNCE_INTERVAL_PRESET_COUNT] = {
    "Off", "15m", "30m", "1h", "2h", "3h", "6h", "12h"
  };
  uint8_t msngr_announce_interval_idx = 0;

  void msngr_announce_interval_conf_save(uint8_t idx) {
    msngr_announce_interval_idx = idx;
    eeprom_update(ADDR_CONF_MSNGR_ANNOUNCE_INTERVAL, idx);
    if (urns_lxmf_router) urns_lxmf_router->set_announce_interval(msngr_announce_interval_presets_s[idx]);
  }

  // Whether a failed DIRECT/OPPORTUNISTIC LXMF delivery automatically
  // retries via the active propagation node - forwarded to LXMRouter::
  // set_fallback_to_propagation(). RNode Settings > Messenger > Settings >
  // Propagate on Fail. Only takes effect once a propagation node is
  // actually active (Bookmarks > <node> > Set Active) - LXMRouter's own
  // fallback path no-ops with no outbound propagation node configured.
  // See ADDR_CONF_MSNGR_PROP_ON_FAIL (ROM.h) for the persisted-value
  // details.
  bool msngr_propagate_on_fail = false;

  void msngr_propagate_on_fail_conf_save(bool enabled) {
    msngr_propagate_on_fail = enabled;
    eeprom_update(ADDR_CONF_MSNGR_PROP_ON_FAIL,
      enabled ? MSNGR_PROP_ON_FAIL_ENABLE_BYTE : MSNGR_PROP_ON_FAIL_DISABLE_BYTE);
    if (urns_lxmf_router) urns_lxmf_router->set_fallback_to_propagation(enabled);
  }

  // Periodic propagation-node sync interval, as a preset index into this
  // table (seconds) - same "preset index, not raw value" shape as
  // msngr_announce_interval_presets_s above, but its own table since sync
  // is heavier (a full Link round-trip fetching queued messages) than a
  // one-packet announce, so the sane interval range skews longer. Index 0
  // (Off) means no periodic sync at all - see messenger_sync_process()
  // for where this is actually checked.
  #define MSNGR_SYNC_INTERVAL_PRESET_COUNT 8
  const uint32_t msngr_sync_interval_presets_s[MSNGR_SYNC_INTERVAL_PRESET_COUNT] = {
    0,      // Off
    900,    // 15 min
    1800,   // 30 min
    3600,   // 1h
    7200,   // 2h
    21600,  // 6h
    43200,  // 12h
    86400   // 24h
  };
  const char *const msngr_sync_interval_labels[MSNGR_SYNC_INTERVAL_PRESET_COUNT] = {
    "Off", "15m", "30m", "1h", "2h", "6h", "12h", "24h"
  };
  uint8_t msngr_sync_interval_idx = 0;

  void msngr_sync_interval_conf_save(uint8_t idx) {
    msngr_sync_interval_idx = idx;
    eeprom_update(ADDR_CONF_MSNGR_SYNC_INTERVAL, idx);
  }

  // Max messages requested per propagation sync - forwarded to LXMRouter::
  // set_sync_message_limit(). RNode Settings > Messenger > Settings >
  // Sync Limit. 0 = unlimited. See ADDR_CONF_MSNGR_SYNC_LIMIT (ROM.h) for
  // why the menu's own valid range stops at 254, not 255.
  #define MSNGR_SYNC_LIMIT_DEFAULT 8
  #define MSNGR_SYNC_LIMIT_MAX     254
  uint8_t msngr_sync_limit = MSNGR_SYNC_LIMIT_DEFAULT;

  void msngr_sync_limit_conf_save(uint8_t limit) {
    msngr_sync_limit = limit;
    eeprom_update(ADDR_CONF_MSNGR_SYNC_LIMIT, limit);
    if (urns_lxmf_router) urns_lxmf_router->set_sync_message_limit(limit);
  }

  // Required LXMF stamp cost for inbound delivery - forwarded to
  // LXMRouter::set_stamp_cost()/enforce_stamps()/ignore_stamps(). RNode
  // Settings > Messenger > Settings > Required Stamp Cost. 0 (the
  // compiled default) disables enforcement entirely; see ADDR_CONF_MSNGR_
  // STAMP_COST (ROM.h) for why the menu's own valid range stops at 254,
  // not 255.
  #define MSNGR_STAMP_COST_DEFAULT 0
  #define MSNGR_STAMP_COST_MAX     254
  uint8_t msngr_stamp_cost = MSNGR_STAMP_COST_DEFAULT;

  void msngr_stamp_cost_conf_save(uint8_t cost) {
    msngr_stamp_cost = cost;
    eeprom_update(ADDR_CONF_MSNGR_STAMP_COST, cost);
    if (urns_lxmf_router) {
      urns_lxmf_router->set_stamp_cost(cost);
      if (cost > 0) urns_lxmf_router->enforce_stamps();
      else urns_lxmf_router->ignore_stamps();
    }
  }

  // LXMF display name - persisted to URNS_DISPLAY_NAME_PATH (URNS.h), same
  // file urns_lxmf_display_name() already reads/falls back from at boot
  // (that read/fallback logic already existed; this is the first writer -
  // see URNS_DISPLAY_NAME_PATH's own comment). RNode Settings > Messenger
  // > Settings > Display Name in the menu (MENU_STATE_MSNGR_TEXT_ENTRY,
  // reused from the message composer - Menu.h). No length/emptiness
  // validation here - the menu's Send-key handler already only calls this
  // when text_len > 0, and MSNGR_TEXT_ENTRY_MAX_LEN already caps input.
  void msngr_display_name_conf_save(const char* name) {
    RNS::Utilities::OS::write_file(URNS_DISPLAY_NAME_PATH, RNS::bytesFromString(name));
    if (urns_lxmf_router) urns_lxmf_router->set_display_name(std::string(name));
  }

  // Ephemeral, in-RAM only - 1-hop lxmf.delivery announces heard on-air
  // since boot. Not persisted; a fresh boot starts with an empty list
  // until peers announce again. Oldest-by-last-heard is evicted once full.
  struct MessengerAnnounce {
    bool in_use = false;
    uint8_t hash[LXMF::PEER_HASH_SIZE];
    char name[MSNGR_NAME_MAX_LEN + 1];
    unsigned long last_heard_ms;
  };
  MessengerAnnounce msngr_announces[MSNGR_MAX_ANNOUNCES];

  LXMF::MessageStore *urns_message_store = nullptr;

  // Cache for MENU_STATE_MSNGR_PEER's message-snippet rows and
  // MENU_STATE_MSNGR_MSG_DETAIL's full content - populated once on entry
  // (messenger_refresh_peer_cache()/messenger_refresh_msg_detail_cache(),
  // called from Menu.h's confirm-select handlers), NOT on every redraw.
  //
  // update_display() (and so draw_settings_menu_disp(), and so whichever
  // MSNGR screen is open) runs many times a second while the menu is
  // open. MessageStore::load_message_metadata() reads from LittleFS
  // (flash) - calling it from the render path meant every redraw briefly
  // disabled the flash cache/interrupts (spi_flash_enable_interrupts_
  // caches_and_other_cpu()). sx126x's DIO0 GPIO interrupt handler
  // (handleDio0Rise() -> executeOpcodeRead()) does SPI work directly
  // inside the ISR (a pre-existing pattern, shared by every board) - if a
  // real LoRa packet's DIO0 IRQ landed inside that window, its own SPI
  // bus-arbitration mutex take (SPIClass::beginTransaction() ->
  // xQueueSemaphoreTake()) hit FreeRTOS's own "can't block on a
  // semaphore from here" assert and crashed. Vanishingly rare before
  // Messenger existed (LittleFS reads were boot-time/occasional-menu
  // only); reading from flash on every single redraw of a live screen
  // made the window common enough to hit reliably. Caching once per
  // screen-entry (rather than per redraw) is the fix - not touching the
  // radio ISR itself, which is shared, battle-tested code well outside
  // this feature's scope.
  struct MessengerPeerMsgCacheRow {
    uint8_t hash[LXMF::MESSAGE_HASH_SIZE];
    char snippet[MSNGR_PEER_SNIPPET_CAP];
    bool incoming;
    bool voice; // FIELD_AUDIO present - Menu.h draws the speaker icon instead of the direction arrow
  };
  MessengerPeerMsgCacheRow msngr_peer_cache[MSNGR_PEER_MAX_MSG_ROWS];
  uint8_t msngr_peer_cache_count = 0;
  // Conversation's total message_count at the moment msngr_peer_cache was
  // last populated - lets messenger_refresh_peer_cache_if_stale() below
  // detect a new message arriving while MENU_STATE_MSNGR_PEER is sitting
  // open, without needing a flash read to do it.
  size_t msngr_peer_cache_message_count = 0;

  // Marquee-scroll state for MENU_STATE_MSNGR_PEER's message rows (Menu.h,
  // draw_settings_menu_disp()'s own MSNGR_PEER case) - only the currently
  // selected message row scrolls, revealing snippet[] past whatever a
  // static 23-char window would show. Declared here (not Menu.h) so
  // messenger_refresh_peer_cache() below can reset them at the same single
  // "screen just (re-)entered, or a new message arrived" moment it already
  // resets everything else the cache holds - keeps the scroll position from
  // ever surviving stale across a real cache refresh.
  uint8_t msngr_peer_scroll_row = 0xFF; // 0xFF = none active yet
  uint8_t msngr_peer_scroll_offset = 0;
  unsigned long msngr_peer_scroll_last_step_ms = 0;
  // 0 = still scrolling, not yet paused at the end - once the marquee
  // reaches its own tail, it holds there for MSNGR_PEER_SCROLL_LOOP_
  // PAUSE_MS (Menu.h) before looping back to the start, per user request.
  unsigned long msngr_peer_scroll_paused_since_ms = 0;
  // 0 = no start-pause armed - set to a future deadline the instant a loop
  // resets back to offset 0, so the beginning of the message stays
  // readable for MSNGR_PEER_SCROLL_START_PAUSE_MS (Menu.h) before
  // scrolling resumes, per user request. Not armed for a row's very first
  // scroll (only loop restarts) - see the draw block's own comment.
  unsigned long msngr_peer_scroll_start_pause_until_ms = 0;

  // Same marquee mechanism, but for MENU_STATE_MSNGR_CHAT (Menu.h) - every
  // visible row that doesn't fit scrolls independently and simultaneously
  // (no single "currently selected" row the way MSNGR_PEER has), hence a
  // per-row array rather than one scalar offset/timer. Indices line up 1:1
  // with msngr_chat_cache, below - Chat's own message-history cache, not
  // msngr_peer_cache (see that cache's own comment for why it's genuinely
  // separate). 0 in msngr_chat_scroll_paused_since_ms means "still
  // scrolling, not yet paused at the end" - once a row's marquee reaches
  // its own tail, it holds there for MSNGR_PEER_SCROLL_LOOP_PAUSE_MS
  // (Menu.h) before looping back to the start, per user request.
  uint8_t msngr_chat_scroll_offset[MSNGR_PEER_MAX_MSG_ROWS] = {0};
  unsigned long msngr_chat_scroll_last_step_ms[MSNGR_PEER_MAX_MSG_ROWS] = {0};
  unsigned long msngr_chat_scroll_paused_since_ms[MSNGR_PEER_MAX_MSG_ROWS] = {0};
  // Same per-row "pause after a loop restart" as msngr_peer_scroll_start_
  // pause_until_ms's own comment above, just one slot per visible row.
  unsigned long msngr_chat_scroll_start_pause_until_ms[MSNGR_PEER_MAX_MSG_ROWS] = {0};

  // MENU_STATE_MSNGR_CHAT's own message-history cache - genuinely separate
  // from msngr_peer_cache above, not reused: unlike MSNGR_PEER's tail-
  // anchored (5 most recent, newest-first) list, Chat supports scrolling
  // through the ENTIRE conversation (Up/Down/PgUp/PgDn/Home/End, Menu.h) in
  // normal chronological reading order (oldest at the top, newest at the
  // bottom, same convention as any chat app) - a genuinely different
  // windowing/ordering scheme that would risk real cross-screen staleness
  // bugs if it shared MSNGR_PEER's own array (e.g. a screen transition that
  // doesn't happen to re-trigger the right refresh first leaving one
  // screen's ordering/window behind for the other to render as if it were
  // its own).
  MessengerPeerMsgCacheRow msngr_chat_cache[MSNGR_PEER_MAX_MSG_ROWS];
  uint8_t msngr_chat_cache_count = 0;
  // Absolute index (0-based, oldest-first across the WHOLE conversation) of
  // msngr_chat_cache[0] - i.e. which message the currently-loaded window
  // starts at.
  size_t msngr_chat_window_start = 0;
  // Total message count for the conversation - defines the valid range for
  // window_start/navigation (Home/End, PgUp/PgDn clamping). Refreshed
  // alongside the window itself, below.
  size_t msngr_chat_total_count = 0;
  // Which row within the current window (0..msngr_chat_cache_count-1) is
  // selected for deletion - 0xFF means "not browsing" (focus is the
  // compose box, the default/normal state). See blekbd_key_event()'s own
  // MENU_STATE_MSNGR_CHAT branch (Menu.h) for the full focus-switching
  // model between composing and browsing.
  uint8_t msngr_chat_sel = 0xFF;
  // Last time browsing (msngr_chat_sel != 0xFF) actually moved - checked
  // against MSNGR_CHAT_SEL_TIMEOUT_MS (Menu.h) every redraw while Chat is
  // open, same "poll from the render path, no separate process() needed"
  // idiom the marquee timers already use - a selection left untouched
  // auto-deselects back to the compose box after that long, per user
  // request.
  unsigned long msngr_chat_sel_last_activity_ms = 0;

  std::string msngr_msg_detail_cache_content;
  bool msngr_msg_detail_cache_incoming = false;
  bool msngr_msg_detail_cache_valid = false;
  // Set only when this board can actually play it (HAS_AUDIO + a supported mode).
  bool msngr_msg_detail_has_audio = false;
  // Unix epoch seconds, straight from LXMessage's own _timestamp (set at
  // pack time if never explicitly assigned - see LXMessage.cpp) - always
  // populated for any real message, 0 only when meta.valid is false.
  double msngr_msg_detail_cache_timestamp = 0;

  // Howard Hinnant's civil-from-days algorithm (public domain) - same one
  // RTC.h's own rtc_civil_from_days() uses, duplicated under a distinct
  // name rather than called directly because RTC.h (and that function) only
  // exists behind #if HAS_RTC, while Messenger is gated on HAS_LXMF - a
  // message's timestamp comes from the message itself (the sender's clock
  // at send time), not this board's own RTC, so formatting it as a date
  // shouldn't depend on this board happening to have an RTC chip.
  void msngr_civil_from_days(int32_t z, int32_t &y, uint32_t &m, uint32_t &d) {
    z += 719468;
    int32_t era = (z >= 0 ? z : z - 146096) / 146097;
    uint32_t doe = (uint32_t)(z - era * 146097);
    uint32_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    y = (int32_t)yoe + era * 400;
    uint32_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    uint32_t mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp + (mp < 10 ? 3 : -9);
    y += (m <= 2);
  }

  // Inverse of Menu.h's msngr_kb_expand_utf8() - decodes real UTF-8 (a
  // message's content, or a peer's announced/delivered display name,
  // both stored on disk as genuine UTF-8 - see MessageStore.cpp's own
  // "Store content as UTF-8 for fast loading" comment) into this
  // device's internal single-byte glyph codes, so it can render through
  // Org_01's Cyrillic block (Fonts/Org_01.h). Defined here rather than
  // shared with Menu.h's encoder because Messenger.h is included before
  // Menu.h (Utilities.h) - same "duplicated under a distinct name"
  // reasoning as msngr_civil_from_days() above, just for a file-order
  // boundary instead of an #if one.
  //
  // Any codepoint this device has no glyph for (any script besides the
  // Cyrillic letters Org_01.h added, or genuinely malformed UTF-8)
  // becomes '?' - one substitution per source *character*, not per
  // byte, so downstream word-wrap/substr math (Menu.h, which assumes 1
  // byte == 1 glyph, same invariant msngr_text_entry_buf keeps) doesn't
  // see a string longer or shorter than what will actually render.
  // out_cap must leave room for the worst case (every input byte
  // becomes one output byte - decoding only ever shrinks or holds
  // length steady, never grows it) plus the NUL.
  void msngr_kb_decode_utf8(const char *in, char *out, size_t out_cap) {
    size_t oi = 0, i = 0;
    while (in[i] != 0 && oi + 1 < out_cap) {
      unsigned char b0 = (unsigned char)in[i];
      if (b0 < 0x80) { out[oi++] = (char)b0; i++; continue; }

      uint8_t seq_len;
      uint32_t cp;
      if      ((b0 & 0xE0) == 0xC0) { seq_len = 2; cp = b0 & 0x1F; }
      else if ((b0 & 0xF0) == 0xE0) { seq_len = 3; cp = b0 & 0x0F; }
      else if ((b0 & 0xF8) == 0xF0) { seq_len = 4; cp = b0 & 0x07; }
      else { out[oi++] = '?'; i++; continue; } // stray continuation byte / invalid lead - resync 1 byte at a time

      // Reads one continuation byte at a time and stops the instant one
      // is invalid *or* is the string's own NUL - critical for a
      // truncated/malformed sequence sitting right at the end of `in`:
      // stopping there (and only advancing i by 1 below, not seq_len)
      // means the next byte this function ever reads is that same NUL,
      // never anything past the end of the buffer.
      bool complete = true;
      for (uint8_t k = 1; k < seq_len; k++) {
        unsigned char bk = (unsigned char)in[i+k];
        if (bk == 0 || (bk & 0xC0) != 0x80) { complete = false; break; }
        cp = (cp << 6) | (bk & 0x3F);
      }
      if (!complete) { out[oi++] = '?'; i++; continue; }
      i += seq_len;

      if      (cp == 0x0401)                cp = 0x80;               // Ё
      else if (cp >= 0x0410 && cp <= 0x042F) cp = 0x81 + (cp-0x0410); // А-Я
      else if (cp == 0x0451)                cp = 0xA1;               // ё
      else if (cp >= 0x0430 && cp <= 0x044F) cp = 0xA2 + (cp-0x0430); // а-я
      else                                    cp = '?';
      out[oi++] = (char)cp;
    }
    out[oi] = 0;
  }

  // Called once when MENU_STATE_MSNGR_PEER is (re-)entered - on first
  // opening it from Inbox/Bookmarks/Announces, and again right after a
  // successful Send (a new message just got added). NOT called from the
  // render path - see the cache fields' own comment above.
  void messenger_refresh_peer_cache(const RNS::Bytes &peer_hash) {
    // Let the confirm-click that just opened this screen finish playing
    // before the flash read below can freeze it mid-note - see
    // buzzer_wait_for_melody()'s own comment (Utilities.h).
    #if HAS_BUZZER == true
      buzzer_wait_for_melody();
    #endif
    msngr_peer_cache_count = 0;
    // A stale scroll position/timer from before this refresh could
    // otherwise survive onto a completely different message at the same
    // row index (rows shift as new messages arrive, most-recent-first).
    msngr_peer_scroll_row = 0xFF;
    msngr_peer_scroll_offset = 0;
    msngr_peer_scroll_last_step_ms = millis();
    msngr_peer_scroll_paused_since_ms = 0;
    msngr_peer_scroll_start_pause_until_ms = 0;
    if (!urns_message_store) return;
    std::vector<RNS::Bytes> hashes = urns_message_store->get_messages_for_conversation(peer_hash);
    msngr_peer_cache_message_count = hashes.size();
    uint8_t n = (uint8_t)hashes.size();
    if (n > MSNGR_PEER_MAX_MSG_ROWS) n = MSNGR_PEER_MAX_MSG_ROWS;
    for (uint8_t i = 0; i < n; i++) {
      // Most-recent-first - hashes itself is oldest-first (MessageStore.h).
      const RNS::Bytes &msg_hash = hashes[hashes.size() - 1 - i];
      memcpy(msngr_peer_cache[i].hash, msg_hash.data(), std::min((size_t)LXMF::MESSAGE_HASH_SIZE, msg_hash.size()));
      LXMF::MessageStore::MessageMetadata meta = urns_message_store->load_message_metadata(msg_hash);
      // No "<"/">" direction prefix here - MENU_STATE_MSNGR_PEER's own
      // draw code (Menu.h) uses the `incoming` flag below to left/right
      // align the row instead, so the full snippet width goes to content.
      // Decoded through msngr_kb_decode_utf8() first - meta.content is
      // genuine UTF-8 off disk, and this snippet gets rendered through
      // Org_01's glyph table same as everything else on this screen.
      if (meta.valid) {
        char snippet_decoded[sizeof(msngr_peer_cache[i].snippet)];
        msngr_kb_decode_utf8(meta.content.c_str(), snippet_decoded, sizeof(snippet_decoded));
        // A voice message is marked by the speaker icon (Menu.h) - no
        // duration text here, just whatever text was sent with it.
        snprintf(msngr_peer_cache[i].snippet, sizeof(msngr_peer_cache[i].snippet), "%s", snippet_decoded);
      } else {
        snprintf(msngr_peer_cache[i].snippet, sizeof(msngr_peer_cache[i].snippet), "?");
      }
      msngr_peer_cache[i].incoming = meta.valid && meta.incoming;
      msngr_peer_cache[i].voice = meta.valid && meta.audio_mode != 0;
    }
    msngr_peer_cache_count = n;
  }

  // MENU_STATE_MSNGR_CHAT's own windowed history fetch (Menu.h's
  // navigation functions call this on every Up/Down/PgUp/PgDn/Home/End) -
  // loads up to MSNGR_PEER_MAX_MSG_ROWS messages starting at start_index,
  // chronological/oldest-first (unlike messenger_refresh_peer_cache()
  // above, always the most recent N, newest-first). Same "only ever called
  // from a discrete input event, never the render path" discipline as
  // every other flash-reading refresh in this file - see that function's
  // own comment on why (a real, confirmed crash otherwise).
  //
  // start_index gets clamped into range here (not just by the caller) -
  // in particular, pulled back so a short/empty tail never leaves the
  // window showing fewer than a full page while older messages still
  // exist to fill it, same "clamp to the end" idiom trim_conversation_to_
  // retention() uses elsewhere (MessageStore.cpp), just applied to a
  // read-side window here. Callers can therefore pass any index (SIZE_MAX
  // for "jump to the end", 0 for "jump to the start") without pre-clamping
  // it themselves.
  void messenger_refresh_chat_window(const RNS::Bytes &peer_hash, size_t start_index) {
    #if HAS_BUZZER == true
      buzzer_wait_for_melody();
    #endif
    msngr_chat_cache_count = 0;
    for (uint8_t i = 0; i < MSNGR_PEER_MAX_MSG_ROWS; i++) {
      msngr_chat_scroll_offset[i] = 0;
      msngr_chat_scroll_last_step_ms[i] = millis();
      msngr_chat_scroll_paused_since_ms[i] = 0;
      msngr_chat_scroll_start_pause_until_ms[i] = 0;
    }
    if (!urns_message_store) return;
    std::vector<RNS::Bytes> hashes = urns_message_store->get_messages_for_conversation(peer_hash);
    msngr_chat_total_count = hashes.size();
    if (start_index > msngr_chat_total_count) start_index = msngr_chat_total_count;
    // FIXED: pulling start_index back to leave a full page ending at the
    // true end used to be gated on msngr_chat_total_count > MSNGR_PEER_
    // MAX_MSG_ROWS, so a conversation with 5 or fewer messages total never
    // re-triggered it - "jump to the end" (start_index passed in as
    // (size_t)-1, then clamped to msngr_chat_total_count by the line
    // above) left start_index sitting one past the last real message,
    // which the loop below correctly reads as "0 messages to show" (n =
    // min(MAX_ROWS, total - start_index) = min(MAX_ROWS, 0) = 0) - an
    // empty window even though real messages exist. Confirmed on hardware:
    // sending the first message into an empty conversation showed nothing
    // until Home (start_index=0 explicitly, bypassing this clamp
    // entirely) was pressed. Unconditional now - whenever the requested
    // window would run short of a full page, pull it back just enough to
    // end at the true last message instead, regardless of how few
    // messages exist in total.
    if (start_index + MSNGR_PEER_MAX_MSG_ROWS > msngr_chat_total_count) {
      start_index = (msngr_chat_total_count > MSNGR_PEER_MAX_MSG_ROWS)
        ? (msngr_chat_total_count - MSNGR_PEER_MAX_MSG_ROWS) : 0;
    }
    msngr_chat_window_start = start_index;
    uint8_t n = (uint8_t)std::min((size_t)MSNGR_PEER_MAX_MSG_ROWS, msngr_chat_total_count - start_index);
    for (uint8_t i = 0; i < n; i++) {
      // Chronological (oldest-first) - hashes itself already is, so this
      // is a direct forward index, unlike messenger_refresh_peer_cache()'s
      // own reversed one.
      const RNS::Bytes &msg_hash = hashes[start_index + i];
      memcpy(msngr_chat_cache[i].hash, msg_hash.data(), std::min((size_t)LXMF::MESSAGE_HASH_SIZE, msg_hash.size()));
      LXMF::MessageStore::MessageMetadata meta = urns_message_store->load_message_metadata(msg_hash);
      if (meta.valid) {
        char snippet_decoded[sizeof(msngr_chat_cache[i].snippet)];
        msngr_kb_decode_utf8(meta.content.c_str(), snippet_decoded, sizeof(snippet_decoded));
        if (meta.audio_mode) {
          // The dialog view has no icon column - spell it out instead.
          snprintf(msngr_chat_cache[i].snippet, sizeof(msngr_chat_cache[i].snippet), "Voice%s%s",
                   snippet_decoded[0] ? " " : "", snippet_decoded);
        } else {
          snprintf(msngr_chat_cache[i].snippet, sizeof(msngr_chat_cache[i].snippet), "%s", snippet_decoded);
        }
      } else {
        snprintf(msngr_chat_cache[i].snippet, sizeof(msngr_chat_cache[i].snippet), "?");
      }
      msngr_chat_cache[i].incoming = meta.valid && meta.incoming;
    }
    msngr_chat_cache_count = n;
  }

  // Safe to call every redraw while MENU_STATE_MSNGR_CHAT is open (unlike
  // messenger_refresh_chat_window() itself) - same get_conversation_info()
  // in-RAM-only check messenger_refresh_peer_cache_if_stale() below uses.
  //
  // Per user request, ANY message event (sent or received - either way
  // the conversation's message_count changes) jumps the window straight to
  // the end, unconditionally - "should scroll to the bottom on every
  // message event to simplify communication". Also drops any active
  // selection (msngr_chat_sel, Menu.h's browsing mode) rather than leaving
  // it pointing at a row index that may no longer mean the same message
  // once the window's jumped - deliberate messages ARE getting deleted
  // too, so a stale index there is a real, not just theoretical, hazard.
  void messenger_refresh_chat_window_if_stale(const RNS::Bytes &peer_hash) {
    if (!urns_message_store) return;
    LXMF::MessageStore::ConversationInfo info = urns_message_store->get_conversation_info(peer_hash);
    if (info.message_count != msngr_chat_total_count) {
      messenger_refresh_chat_window(peer_hash, (size_t)-1);
      msngr_chat_sel = 0xFF;
    }
  }

  // Safe to call every redraw while MENU_STATE_MSNGR_PEER is open (unlike
  // messenger_refresh_peer_cache() itself) - get_conversation_info() only
  // scans MessageStore's in-RAM _conversations_pool, no LittleFS read, so
  // it doesn't carry the DIO0-ISR-vs-flash-I/O risk described above. A
  // message arriving for this peer updates that in-RAM message_count
  // immediately, so comparing against the cache's own snapshot detects a
  // live new message; the actual (flash-reading) cache refresh still only
  // runs once per change, not once per redraw.
  void messenger_refresh_peer_cache_if_stale(const RNS::Bytes &peer_hash) {
    if (!urns_message_store) return;
    LXMF::MessageStore::ConversationInfo info = urns_message_store->get_conversation_info(peer_hash);
    if (info.message_count != msngr_peer_cache_message_count) {
      messenger_refresh_peer_cache(peer_hash);
    }
  }

  // Called once when a message row is selected from MENU_STATE_MSNGR_PEER,
  // right before switching to MENU_STATE_MSNGR_MSG_DETAIL.
  void messenger_refresh_msg_detail_cache(const RNS::Bytes &message_hash) {
    msngr_msg_detail_cache_valid = false;
    if (!urns_message_store) return;
    LXMF::MessageStore::MessageMetadata meta = urns_message_store->load_message_metadata(message_hash);
    if (meta.valid) {
      // Decoding only ever shrinks length (msngr_kb_decode_utf8()'s own
      // comment), so content.size()+1 is always enough - unlike the old
      // fixed MSNGR_CONTENT_DECODE_BUF_LEN scratch buffer (256 bytes),
      // this can't silently drop the tail of a message longer than that
      // (reported: a 588-char Cyrillic message truncated to ~249 chars
      // on-screen even though the full text was stored on flash intact).
      std::vector<char> content_decoded(meta.content.size() + 1);
      msngr_kb_decode_utf8(meta.content.c_str(), content_decoded.data(), content_decoded.size());
      msngr_msg_detail_cache_content = content_decoded.data();
    } else {
      msngr_msg_detail_cache_content = "(unavailable)";
    }
    msngr_msg_detail_has_audio = false;
    if (meta.valid && meta.audio_mode) {
      // Voice messages usually carry no text - show a label instead of an
      // empty preview.
      if (msngr_msg_detail_cache_content.empty()) msngr_msg_detail_cache_content = "Voice message";
      #if HAS_AUDIO == true
        msngr_msg_detail_has_audio = audio_available() && audio_mode_info(meta.audio_mode) != nullptr;
      #endif
    }
    msngr_msg_detail_cache_incoming = meta.valid && meta.incoming;
    msngr_msg_detail_cache_timestamp = meta.valid ? meta.timestamp : 0;
    msngr_msg_detail_cache_valid = true;
  }

  #if HAS_AUDIO == true
    // Loads the stored message, pulls the Codec2 payload out of FIELD_AUDIO
    // into a PSRAM buffer and hands it to the audio task (which owns/frees
    // it). Loop-task only: the LittleFS read is masked against DIO0 like
    // every other flash I/O in the menu handlers, and audio_play_lxmf()
    // does the codec's I2C setup (shared Wire with the display).
    bool messenger_voice_play(const RNS::Bytes &message_hash) {
      if (!urns_message_store || audio_state() != AUDIO_IDLE) return false;
      #if HAS_BUZZER == true
        buzzer_wait_for_melody();
      #endif
      uint8_t *buf = nullptr;
      size_t len = 0;
      uint8_t mode = 0;
      LoRa->maskDio0();
      {
        LXMF::LXMessage msg = urns_message_store->load_message(message_hash);
        LXMF::AudioField af;
        if (LXMF::parse_audio_field(msg, af) && af.size) {
          buf = (uint8_t *)heap_caps_malloc(af.size, MALLOC_CAP_SPIRAM);
          if (buf) { memcpy(buf, af.data, af.size); len = af.size; mode = af.mode; }
        }
      }
      LoRa->unmaskDio0();
      DEBUG_LOG("[Audio] play request: loaded=%d mode=%u len=%u\r\n", buf != nullptr, mode, (unsigned)len);
      if (!buf) return false;
      bool started = audio_play_lxmf(mode, buf, len, msngr_playback_volume_pct);
      DEBUG_LOG("[Audio] audio_play_lxmf -> %d\r\n", started);
      return started;
    }
  #endif

  bool messenger_hash_matches(const uint8_t *stored, const RNS::Bytes &hash) {
    if (hash.size() != LXMF::PEER_HASH_SIZE) return false;
    return memcmp(stored, hash.data(), LXMF::PEER_HASH_SIZE) == 0;
  }

  void messenger_store_hash(uint8_t *dst, const RNS::Bytes &hash) {
    size_t len = std::min((size_t)LXMF::PEER_HASH_SIZE, hash.size());
    memcpy(dst, hash.data(), len);
    if (len < LXMF::PEER_HASH_SIZE) memset(dst + len, 0, LXMF::PEER_HASH_SIZE - len);
  }

  void messenger_store_name(char *dst, const std::string &name) {
    snprintf(dst, MSNGR_NAME_MAX_LEN + 1, "%s", name.c_str());
  }

  // Manual hex entry (Menu.h, MSNGR_TEXT_ENTRY_PURPOSE_BOOKMARK_HASH and
  // _IDENTITY_RESTORE both go through here) - decodes a typed hex string
  // into a raw out_len-byte buffer. Requires exactly out_len*2 hex chars
  // (2 per byte, case-insensitive) and rejects anything else outright
  // rather than accepting a short/padded value, since a wrong hash/key
  // here silently addresses a different (or nonexistent) destination, or
  // replaces the identity with the wrong key, forever.
  bool messenger_hash_from_hex(const char *hex, uint8_t *out, size_t out_len) {
    size_t len = strlen(hex);
    if (len != out_len * 2) return false;
    for (size_t i = 0; i < out_len; i++) {
      char hi = hex[i * 2], lo = hex[i * 2 + 1];
      if (!isxdigit((unsigned char)hi) || !isxdigit((unsigned char)lo)) return false;
      auto nibble = [](char c) -> uint8_t {
        if (c >= '0' && c <= '9') return (uint8_t)(c - '0');
        if (c >= 'a' && c <= 'f') return (uint8_t)(c - 'a' + 10);
        return (uint8_t)(c - 'A' + 10);
      };
      out[i] = (uint8_t)((nibble(hi) << 4) | nibble(lo));
    }
    return true;
  }

  int8_t messenger_bookmark_find(const RNS::Bytes &hash) {
    for (uint8_t i = 0; i < MSNGR_MAX_BOOKMARKS; i++) {
      if (msngr_bookmarks[i].in_use && messenger_hash_matches(msngr_bookmarks[i].hash, hash)) return i;
    }
    return -1;
  }

  int8_t messenger_announce_find(const RNS::Bytes &hash) {
    for (uint8_t i = 0; i < MSNGR_MAX_ANNOUNCES; i++) {
      if (msngr_announces[i].in_use && messenger_hash_matches(msngr_announces[i].hash, hash)) return i;
    }
    return -1;
  }

  void messenger_bookmarks_save() {
    JsonDocument doc;
    JsonArray arr = doc["bookmarks"].to<JsonArray>();
    for (uint8_t i = 0; i < MSNGR_MAX_BOOKMARKS; i++) {
      if (!msngr_bookmarks[i].in_use) continue;
      JsonObject o = arr.add<JsonObject>();
      o["hash"] = RNS::Bytes(msngr_bookmarks[i].hash, LXMF::PEER_HASH_SIZE).toHex();
      o["name"] = msngr_bookmarks[i].name;
      o["type"] = msngr_bookmarks[i].type;
      o["delivery_mode"] = msngr_bookmarks[i].delivery_mode;
    }
    // Active propagation node (messenger_prop_node_set_active/_clear_
    // active below) - lives in this same file/object rather than its own
    // separate LittleFS file, since it's small and only ever meaningful
    // alongside the Propagation-type bookmark it points at.
    if (msngr_active_prop_node_hash.size() == LXMF::PEER_HASH_SIZE) {
      doc["active_prop_node"] = msngr_active_prop_node_hash.toHex();
    }
    std::string out;
    serializeJson(doc, out);
    RNS::Utilities::OS::write_file(MSNGR_BOOKMARKS_PATH, RNS::bytesFromString(out.c_str()));
  }

  void messenger_bookmarks_load() {
    for (uint8_t i = 0; i < MSNGR_MAX_BOOKMARKS; i++) msngr_bookmarks[i].in_use = false;
    msngr_bookmark_count = 0;
    msngr_active_prop_node_hash = RNS::Bytes();

    RNS::Bytes raw;
    if (RNS::Utilities::OS::read_file(MSNGR_BOOKMARKS_PATH, raw) == 0) return;

    JsonDocument doc;
    // Pointer+size form, not the std::string overload - matches
    // MessageStore.cpp's own deserializeJson() calls throughout, which
    // never pass a std::string either.
    if (deserializeJson(doc, (const char*)raw.data(), raw.size()) != DeserializationError::Ok) return;

    JsonArray arr = doc["bookmarks"].as<JsonArray>();
    uint8_t i = 0;
    for (JsonObject o : arr) {
      if (i >= MSNGR_MAX_BOOKMARKS) break;
      const char *hex = o["hash"] | "";
      RNS::Bytes hash;
      hash.assignHex(hex);
      if (hash.size() != LXMF::PEER_HASH_SIZE) continue;
      messenger_store_hash(msngr_bookmarks[i].hash, hash);
      messenger_store_name(msngr_bookmarks[i].name, (const char*)(o["name"] | ""));
      // Absent "type" (bookmarks saved before this feature) defaults to
      // LXMF - every bookmark was an LXMF peer before Propagation-type
      // bookmarks existed.
      msngr_bookmarks[i].type = (uint8_t)(o["type"] | MSNGR_BOOKMARK_TYPE_LXMF);
      // Absent "delivery_mode" (bookmarks saved before this feature)
      // defaults to DIRECT - every existing bookmark already behaved this
      // way, this setting only narrows that for whoever opts into PROPAGATED.
      msngr_bookmarks[i].delivery_mode = (uint8_t)(o["delivery_mode"] | MSNGR_DELIVERY_MODE_DIRECT);
      msngr_bookmarks[i].in_use = true;
      i++;
    }
    msngr_bookmark_count = i;

    const char *active_hex = doc["active_prop_node"] | "";
    if (active_hex[0] != 0) {
      RNS::Bytes active_hash;
      active_hash.assignHex(active_hex);
      if (active_hash.size() == LXMF::PEER_HASH_SIZE) msngr_active_prop_node_hash = active_hash;
    }
  }

  bool messenger_bookmark_add(const RNS::Bytes &hash, const std::string &name, uint8_t type = MSNGR_BOOKMARK_TYPE_LXMF) {
    if (messenger_bookmark_find(hash) >= 0) return true; // already bookmarked
    for (uint8_t i = 0; i < MSNGR_MAX_BOOKMARKS; i++) {
      if (msngr_bookmarks[i].in_use) continue;
      messenger_store_hash(msngr_bookmarks[i].hash, hash);
      messenger_store_name(msngr_bookmarks[i].name, name);
      msngr_bookmarks[i].type = type;
      msngr_bookmarks[i].in_use = true;
      msngr_bookmark_count++;
      messenger_bookmarks_save();
      return true;
    }
    return false; // pool full
  }

  bool messenger_bookmark_is_prop_node(const RNS::Bytes &hash) {
    int8_t idx = messenger_bookmark_find(hash);
    return idx >= 0 && msngr_bookmarks[idx].type == MSNGR_BOOKMARK_TYPE_PROPAGATION;
  }

  // Persisted half of the per-contact delivery-mode setting - MSNGR_
  // DELIVERY_MODE_DIRECT for any hash that isn't bookmarked (nothing to
  // read), same "not bookmarked = default behavior" fallback every other
  // per-bookmark setting here already uses. Callers wanting the *current*
  // mode (bookmarked or not, see the session fallback below) should use
  // messenger_current_delivery_mode() instead - this is the raw persisted
  // read only.
  uint8_t messenger_bookmark_delivery_mode(const RNS::Bytes &hash) {
    int8_t idx = messenger_bookmark_find(hash);
    return (idx >= 0) ? msngr_bookmarks[idx].delivery_mode : MSNGR_DELIVERY_MODE_DIRECT;
  }

  // Returns false (no-op) for a hash that isn't bookmarked - there's no
  // MessengerBookmark slot to write the mode into. messenger_set_delivery_
  // mode() below is what non-bookmarked peers actually go through; this is
  // just the persistence half of it.
  bool messenger_bookmark_set_delivery_mode(const RNS::Bytes &hash, uint8_t mode) {
    int8_t idx = messenger_bookmark_find(hash);
    if (idx < 0) return false;
    msngr_bookmarks[idx].delivery_mode = mode;
    messenger_bookmarks_save();
    return true;
  }

  // Session-only fallback for a peer that isn't bookmarked - toggling Send
  // Direct/Send Propagated (MSNGR_PEER_FIXED_ACTION_DELIVERY_MODE, Menu.h)
  // on a peer with nowhere to persist the choice (an Inbox conversation or
  // Announces-list contact that's never been bookmarked) would otherwise
  // silently revert to DIRECT the instant messenger_current_delivery_mode()
  // re-reads it, which looks like the toggle did nothing. Remembering the
  // last hash/mode pair here instead makes it stick for the rest of this
  // boot - same "one thing tracked at a time" shape as msngr_send_state/
  // msngr_ping_state above, since the contact screen only ever has one
  // active peer. Per explicit user decision: no implicit auto-bookmarking,
  // and no persistence across a reboot for a peer that was never bookmarked.
  RNS::Bytes msngr_session_delivery_mode_hash;
  uint8_t msngr_session_delivery_mode = MSNGR_DELIVERY_MODE_DIRECT;
  bool msngr_session_delivery_mode_valid = false;

  // What messenger_send_lxmf_resolved() (below) actually sends with, and
  // what MENU_STATE_MSNGR_PEER's row label reads - bookmarked peers always
  // reflect their persisted value (messenger_set_delivery_mode() below
  // keeps the session mirror and the bookmark record in lock-step), so the
  // session fallback only ever matters for a hash that was toggled while
  // NOT bookmarked.
  uint8_t messenger_current_delivery_mode(const RNS::Bytes &hash) {
    if (msngr_session_delivery_mode_valid && messenger_hash_matches(msngr_session_delivery_mode_hash.data(), hash)) {
      return msngr_session_delivery_mode;
    }
    return messenger_bookmark_delivery_mode(hash);
  }

  // Always updates the session mirror (so the toggle sticks for the rest of
  // this boot regardless of bookmark status), and additionally persists to
  // the bookmark record when one exists - messenger_bookmark_set_delivery_
  // mode() is a silent no-op otherwise.
  void messenger_set_delivery_mode(const RNS::Bytes &hash, uint8_t mode) {
    msngr_session_delivery_mode_hash = hash;
    msngr_session_delivery_mode = mode;
    msngr_session_delivery_mode_valid = true;
    messenger_bookmark_set_delivery_mode(hash, mode);
  }

  uint8_t messenger_toggle_delivery_mode(const RNS::Bytes &hash) {
    uint8_t next = (messenger_current_delivery_mode(hash) == MSNGR_DELIVERY_MODE_PROPAGATED)
      ? MSNGR_DELIVERY_MODE_DIRECT : MSNGR_DELIVERY_MODE_PROPAGATED;
    messenger_set_delivery_mode(hash, next);
    return next;
  }

  bool messenger_prop_node_is_active(const RNS::Bytes &hash) {
    return msngr_active_prop_node_hash.size() == LXMF::PEER_HASH_SIZE &&
           messenger_hash_matches(msngr_active_prop_node_hash.data(), hash);
  }

  // RNode Settings > Messenger > Bookmarks > <a Propagation-type bookmark>
  // > Set Active (Menu.h) - makes this the propagation node PROPAGATED
  // sends/periodic sync/Propagate on Fail all target, via LXMRouter::set_
  // outbound_propagation_node(). Only one node can be active at a time -
  // selecting a new one silently replaces whichever was active before, no
  // separate unset step required (same "last write wins" shape as every
  // other single-value setting in this menu).
  void messenger_prop_node_set_active(const RNS::Bytes &hash) {
    msngr_active_prop_node_hash = hash;
    if (urns_lxmf_router) urns_lxmf_router->set_outbound_propagation_node(hash);
    messenger_bookmarks_save();
  }

  // RNode Settings > Messenger > Bookmarks > <the active Propagation-type
  // bookmark> > Unset Active - clears LXMRouter's outbound propagation
  // node, which is what actually disables PROPAGATED delivery/periodic
  // sync/Propagate on Fail (they all no-op with no node configured,
  // regardless of their own individual settings) until another node is
  // set active again.
  void messenger_prop_node_clear_active() {
    msngr_active_prop_node_hash = RNS::Bytes();
    if (urns_lxmf_router) urns_lxmf_router->set_outbound_propagation_node(RNS::Bytes());
    messenger_bookmarks_save();
  }

  void messenger_bookmark_remove(const RNS::Bytes &hash) {
    int8_t idx = messenger_bookmark_find(hash);
    if (idx < 0) return;
    if (messenger_prop_node_is_active(hash)) messenger_prop_node_clear_active();
    msngr_bookmarks[idx].in_use = false;
    msngr_bookmark_count--;
    messenger_bookmarks_save();
  }

  // RNode Settings > Messenger > Bookmarks > <a Propagation-type bookmark>
  // > Rename (Menu.h, MSNGR_PEER_PROP_ACTION_RENAME) - a propagation node
  // never gets an automatic display name the way an LXMF peer does
  // (messenger_peer_display_name()'s MessageStore-cache/announce-list
  // fallbacks are both LXMF-delivery concepts; a propagation destination
  // never appears in either), so without this it's permanently stuck
  // showing truncated hex on the Bookmarks list. Setting a non-empty name
  // here makes messenger_peer_display_name()'s existing bookmark-name
  // check (its very first, highest-priority one) pick it up everywhere
  // that function is already used - no separate propagation-node-name
  // display path needed.
  void messenger_bookmark_rename(const RNS::Bytes &hash, const std::string &name) {
    int8_t idx = messenger_bookmark_find(hash);
    if (idx < 0) return;
    messenger_store_name(msngr_bookmarks[idx].name, name);
    messenger_bookmarks_save();
  }

  // Same save/load shape as messenger_bookmarks_save/_load above, just a
  // plain array of strings instead of hash+name objects - order matters
  // here (it's the same order MENU_STATE_MSNGR_PEER shows the Send:
  // buttons in), so this is a JSON array, not an object keyed by index.
  void messenger_presets_save() {
    JsonDocument doc;
    JsonArray arr = doc["presets"].to<JsonArray>();
    for (uint8_t i = 0; i < msngr_preset_count; i++) arr.add(msngr_presets[i]);
    std::string out;
    serializeJson(doc, out);
    RNS::Utilities::OS::write_file(MSNGR_PRESETS_PATH, RNS::bytesFromString(out.c_str()));
  }

  // Falls back to the old hardcoded {"Hi","Bye","SOS"} set when the file
  // doesn't exist yet (fresh device, or one flashed before this feature
  // existed) - preserves today's peer-screen behavior for anyone who
  // never opens the new Preset Messages settings screen, rather than
  // silently going from 3 quick-send buttons to 0.
  void messenger_presets_load() {
    msngr_preset_count = 0;

    RNS::Bytes raw;
    if (RNS::Utilities::OS::read_file(MSNGR_PRESETS_PATH, raw) == 0) {
      messenger_store_name(msngr_presets[0], "Hi");
      messenger_store_name(msngr_presets[1], "Bye");
      messenger_store_name(msngr_presets[2], "SOS");
      msngr_preset_count = 3;
      return;
    }

    JsonDocument doc;
    if (deserializeJson(doc, (const char*)raw.data(), raw.size()) != DeserializationError::Ok) return;

    JsonArray arr = doc["presets"].as<JsonArray>();
    uint8_t i = 0;
    for (JsonVariant v : arr) {
      if (i >= MSNGR_MAX_PRESETS) break;
      messenger_store_name(msngr_presets[i], (const char*)(v | ""));
      i++;
    }
    msngr_preset_count = i;
  }

  bool messenger_preset_add(const std::string &text) {
    if (msngr_preset_count >= MSNGR_MAX_PRESETS) return false;
    messenger_store_name(msngr_presets[msngr_preset_count], text);
    msngr_preset_count++;
    messenger_presets_save();
    return true;
  }

  void messenger_preset_update(uint8_t index, const std::string &text) {
    if (index >= msngr_preset_count) return;
    messenger_store_name(msngr_presets[index], text);
    messenger_presets_save();
  }

  // Shifts every entry above index down by one to keep the array packed -
  // unlike bookmark/announce removal, which just clears an in_use flag
  // and leaves a hole, since those are found by hash lookup rather than
  // rendered as a positional list.
  void messenger_preset_delete(uint8_t index) {
    if (index >= msngr_preset_count) return;
    for (uint8_t i = index; i < msngr_preset_count - 1; i++) {
      messenger_store_name(msngr_presets[i], msngr_presets[i + 1]);
    }
    msngr_preset_count--;
    messenger_presets_save();
  }

  // Inserts/refreshes an announce entry, evicting the oldest-heard slot
  // when the pool is full and this hash isn't already tracked - same
  // "bounded ring, evict oldest" shape as MessageStore's own conversation
  // hash-list cap (MessageStore.h), just for an in-RAM, non-persisted list.
  void messenger_announce_upsert(const RNS::Bytes &hash, const std::string &name) {
    int8_t idx = messenger_announce_find(hash);
    if (idx < 0) {
      for (uint8_t i = 0; i < MSNGR_MAX_ANNOUNCES; i++) {
        if (!msngr_announces[i].in_use) { idx = i; break; }
      }
    }
    if (idx < 0) {
      uint8_t oldest = 0;
      for (uint8_t i = 1; i < MSNGR_MAX_ANNOUNCES; i++) {
        if (msngr_announces[i].last_heard_ms < msngr_announces[oldest].last_heard_ms) oldest = i;
      }
      idx = oldest;
    }
    messenger_store_hash(msngr_announces[idx].hash, hash);
    if (!name.empty()) messenger_store_name(msngr_announces[idx].name, name);
    else if (!msngr_announces[idx].in_use) msngr_announces[idx].name[0] = 0;
    msngr_announces[idx].in_use = true;
    msngr_announces[idx].last_heard_ms = millis();
  }

  // Manual MessagePack decode of LXMF 0.5+ delivery-announce app_data
  // ([display_name, stamp_cost]) - mirrors decode_announce_stamp_cost()'s
  // own hand-rolled parse (LXMRouter.cpp, anonymous-namespace/private to
  // that file, so not reusable from here) rather than pulling in a full
  // MsgPack::Unpacker for a two-element array. Only the display-name
  // element is needed here.
  bool messenger_display_name_from_app_data(const RNS::Bytes &app_data, std::string &out_name) {
    if (!app_data) return false;
    const uint8_t *data = app_data.data();
    const size_t size = app_data.size();
    size_t offset = 0;
    if (offset >= size) return false;

    uint32_t array_size = 0;
    const uint8_t array_tag = data[offset++];
    if ((array_tag & 0xf0) == 0x90) {
      array_size = array_tag & 0x0f;
    } else if (array_tag == 0xdc) {
      if (offset + 2 > size) return false;
      array_size = (static_cast<uint16_t>(data[offset]) << 8) | data[offset + 1];
      offset += 2;
    } else if (array_tag == 0xdd) {
      if (offset + 4 > size) return false;
      array_size = (static_cast<uint32_t>(data[offset]) << 24) | (static_cast<uint32_t>(data[offset+1]) << 16) |
                   (static_cast<uint32_t>(data[offset+2]) << 8) | static_cast<uint32_t>(data[offset+3]);
      offset += 4;
    } else {
      return false;
    }
    if (array_size < 1 || offset >= size) return false;

    const uint8_t name_tag = data[offset++];
    uint32_t name_length = 0;
    if (name_tag == 0xc0) {
      return false; // nil display name
    } else if ((name_tag & 0xe0) == 0xa0) {
      name_length = name_tag & 0x1f;
    } else if (name_tag == 0xc4 || name_tag == 0xd9) {
      if (offset >= size) return false;
      name_length = data[offset++];
    } else if (name_tag == 0xc5 || name_tag == 0xda) {
      if (offset + 2 > size) return false;
      name_length = (static_cast<uint16_t>(data[offset]) << 8) | data[offset + 1];
      offset += 2;
    } else if (name_tag == 0xc6 || name_tag == 0xdb) {
      if (offset + 4 > size) return false;
      name_length = (static_cast<uint32_t>(data[offset]) << 24) | (static_cast<uint32_t>(data[offset+1]) << 16) |
                    (static_cast<uint32_t>(data[offset+2]) << 8) | static_cast<uint32_t>(data[offset+3]);
      offset += 4;
    } else {
      return false;
    }
    if (name_length == 0 || name_length > size - offset) return false;

    out_name.assign(reinterpret_cast<const char*>(data + offset), name_length);
    return true;
  }

  // Resolution order: bookmark name (explicitly set by the user) > cached
  // MessageStore display name (learned from a delivery, or an announce
  // for a peer we already have a conversation with) > this session's
  // announce list (a peer we've heard but never exchanged a message
  // with - MessageStore::set_display_name() is a no-op with no existing
  // conversation to attach the name to, so this is the only place that
  // name is ever cached) > truncated hex, so a bookmarked peer's chosen
  // label always wins even if they've since announced under a different
  // display name.
  std::string messenger_peer_display_name(const RNS::Bytes &peer_hash) {
    // Bookmark names are never typed fresh - messenger_bookmark_add()'s
    // only call site (Menu.h) always passes this very function's own
    // return value, so whatever's in msngr_bookmarks[bm].name was
    // already decoded (below) the moment it was captured. Decoding it a
    // second time here would corrupt it - our internal Cyrillic byte
    // codes (0x80-0xC1, see msngr_kb_decode_utf8()) look like invalid
    // stray UTF-8 continuation bytes to a UTF-8 decoder.
    int8_t bm = messenger_bookmark_find(peer_hash);
    if (bm >= 0 && msngr_bookmarks[bm].name[0] != 0) return std::string(msngr_bookmarks[bm].name);
    // MessageStore's cached name and the announce list, in contrast,
    // always hold genuine UTF-8 straight off disk / off the wire (see
    // MessageStore.cpp's own "Store content as UTF-8" comment and
    // messenger_display_name_from_app_data()'s raw msgpack parse above) -
    // decode both before returning.
    if (urns_message_store) {
      std::string cached = urns_message_store->get_display_name(peer_hash);
      if (!cached.empty()) {
        char decoded[MSNGR_NAME_MAX_LEN + 1];
        msngr_kb_decode_utf8(cached.c_str(), decoded, sizeof(decoded));
        return std::string(decoded);
      }
    }
    // Nothing cached yet for this peer (most likely a manually-added-by-
    // hash bookmark, or an inbound sender this node has never directly
    // announce-heard) - Identity::_known_destinations may since have
    // picked up app_data anyway, e.g. from a path response answering an
    // earlier Ping/Send's request_path() (Identity.cpp's own "app_data
    // backfill" comment on validate_announce() explains why that wasn't
    // possible before). Checked live on every redraw rather than only at
    // message-delivery time, since a path response can arrive well after
    // the message that prompted it did. Persisted via set_display_name()
    // so future lookups hit the MessageStore cache above instead of
    // re-decoding app_data every redraw.
    if (urns_message_store) {
      RNS::Bytes live_app_data = RNS::Identity::recall_app_data(peer_hash);
      std::string live_name;
      if (messenger_display_name_from_app_data(live_app_data, live_name)) {
        urns_message_store->set_display_name(peer_hash, live_name);
        char decoded[MSNGR_NAME_MAX_LEN + 1];
        msngr_kb_decode_utf8(live_name.c_str(), decoded, sizeof(decoded));
        return std::string(decoded);
      }
    }
    int8_t an = messenger_announce_find(peer_hash);
    if (an >= 0 && msngr_announces[an].name[0] != 0) {
      char decoded[MSNGR_NAME_MAX_LEN + 1];
      msngr_kb_decode_utf8(msngr_announces[an].name, decoded, sizeof(decoded));
      return std::string(decoded);
    }
    return peer_hash.toHex(true).substr(0, 16);
  }

  // Only ever sees announces from the delivery destinations of real LXMF
  // peers (aspect_filter "lxmf.delivery").
  class MessengerAnnounceHandler : public RNS::AnnounceHandler {
  public:
    MessengerAnnounceHandler() : RNS::AnnounceHandler("lxmf.delivery") {}
    virtual ~MessengerAnnounceHandler() {}
    virtual void received_announce(const RNS::Bytes &destination_hash, const RNS::Identity &announced_identity, const RNS::Bytes &app_data) override {
      std::string name;
      bool have_name = messenger_display_name_from_app_data(app_data, name);
      DEBUG_LOG("[Messenger] RX 1-hop announce from %s%s%s\r\n", destination_hash.toHex().c_str(),
        have_name ? ", name=" : "", have_name ? name.c_str() : "");
      messenger_announce_upsert(destination_hash, have_name ? name : std::string());
      if (have_name && urns_message_store) urns_message_store->set_display_name(destination_hash, name);
    }
  };
  RNS::HAnnounceHandler msngr_announce_handler(new MessengerAnnounceHandler());

  // Learns propagation nodes' advertised metadata (stamp_cost/transfer_
  // limit/etc.) from their own "lxmf.propagation" announces - a complete,
  // ready-made class already in lib/microLXMF, just never previously
  // instantiated anywhere in this firmware. Plain global instance (not
  // heap-allocated via `new`, unlike MessengerAnnounceHandler above) so
  // messenger_refresh_prop_node_stamp_cost() below can call get_node()/
  // has_node() on it directly - registered via a shared_ptr with a no-op
  // deleter (Transport::register_announce_handler() needs an HAnnounceHandler,
  // a std::shared_ptr<AnnounceHandler>) rather than one that would try to
  // delete this static-storage object.
  //
  // Added specifically to fix PROPAGATED sends that silently vanish:
  // send_propagated() (LXMRouter.cpp) already fully implements attaching a
  // proof-of-work stamp when the router's _outbound_propagation_stamp_cost
  // is non-zero, but LXMRouter::set_outbound_propagation_stamp_cost() had
  // zero callers anywhere in this codebase - so a propagation node that
  // requires a stamp (many do, as an anti-spam measure) would accept the
  // resource transfer at the network layer (confirmed via _sent_callback,
  // "Sent to Node" in the UI) and then silently discard the message at the
  // LXMF layer for lacking one, never queuing it for the recipient -
  // exactly the "Sent to Node, but sync shows 0 messages" symptom this
  // closes.
  LXMF::PropagationNodeManager msngr_prop_node_manager;
  RNS::HAnnounceHandler msngr_prop_node_announce_handler(&msngr_prop_node_manager, [](RNS::AnnounceHandler*){});

  // Pushes whatever msngr_prop_node_manager currently knows about the
  // active propagation node's required stamp cost into the router -
  // called by messenger_send_lxmf_resolved() right before every PROPAGATED
  // send (defensively, every time, rather than once at Set Active - a
  // node's announce/stamp cost may only be learned later, or change; this
  // is cheap enough to just always re-check). Declared after messenger_
  // prop_node_set_active() (above) but called only from further down this
  // file - messenger_prop_node_set_active() itself doesn't call this, see
  // its own comment for why (forward-declaration ordering; the boot-time
  // request_path() nudge there is what actually matters for freshness).
  // FIXED (local patch, not upstream): the request_path() nudges in
  // messenger_prop_node_set_active()/messenger_init() turned out not to
  // help at all - Transport::inbound() (microReticulum's Transport.cpp)
  // explicitly skips notifying registered AnnounceHandlers for PATH_
  // RESPONSE packets ("if (packet.context() != Type::Packet::PATH_
  // RESPONSE) { ...dispatch to handlers... }"), on purpose (a path
  // response is meant to be the lightweight option). So msngr_prop_node_
  // manager could sit with has_node()==false indefinitely, until the node
  // happens to emit a genuine spontaneous announce on its own schedule -
  // commonly a long interval for a propagation node specifically, to
  // reduce mesh chatter. Confirmed live: a real sync round-trip (which
  // definitely resolves this node's identity/path) produced no "Discovered
  // propagation node" log line at all, and a subsequent PROPAGATED send
  // skipped the stamp step entirely.
  //
  // Fallback: RNS::Identity::recall_app_data() is a general-purpose cache
  // that Transport populates from ANY validated announce for a
  // destination, past or present, independent of whether an AnnounceHandler
  // was even registered at the time - the same mechanism messenger_on_
  // delivery()'s own comment above already relies on for peer display
  // names. Feed whatever's currently cached there through the exact same
  // received_announce() parsing path a live dispatch would have used,
  // every time (see the FIXED comment below for why unconditionally, not
  // just once).
  //
  // FIXED (local patch, not upstream): this used to only take the fallback
  // path `if (!msngr_prop_node_manager.has_node(hash))` - meaning it only
  // ever ran ONCE, the first time this node was seen (almost always via
  // the one genuine live-dispatched announce that happened to arrive
  // during initial discovery). Every subsequent call just re-read msngr_
  // prop_node_manager's OWN pool - a SEPARATE cache from Identity's,
  // populated once and never touched again by this function - so a
  // manually-added propagation node's stamp cost was permanently frozen
  // at whatever it was on first discovery for the rest of this boot, even
  // after Identity::recall_app_data() itself got fresher data (e.g. via
  // the Identity.cpp app_data-refresh fix this same session added) from a
  // later path-response. Confirmed live: lowering the node's configured
  // stamp cost server-side never took effect no matter how many sends or
  // reboots followed. Now always re-derives from recall_app_data() and
  // re-feeds it through received_announce() (a no-op-ish "Updated" log
  // line if the content hasn't actually changed - cheap, and only runs
  // once per deliberate PROPAGATED send, not continuously).
  void messenger_refresh_prop_node_stamp_cost() {
    if (!urns_lxmf_router) return;
    if (msngr_active_prop_node_hash.size() != LXMF::PEER_HASH_SIZE) return;
    RNS::Bytes cached_app_data = RNS::Identity::recall_app_data(msngr_active_prop_node_hash);
    if (cached_app_data) {
      RNS::Identity node_identity = RNS::Identity::recall(msngr_active_prop_node_hash);
      msngr_prop_node_manager.received_announce(msngr_active_prop_node_hash, node_identity, cached_app_data);
    }
    LXMF::PropagationNodeInfo info = msngr_prop_node_manager.get_node(msngr_active_prop_node_hash);
    if (info) {
      urns_lxmf_router->set_outbound_propagation_stamp_cost(info.stamp_cost);
    }
  }

  // Delivery callback hook (registered from URNS.h's urns_init(), which
  // forward-declares this) - persists the message, refreshes the sender's
  // cached display name (Identity::recall_app_data picks up whatever
  // app_data the router's own identity-recall path already cached during
  // signature validation, covering senders we haven't directly heard
  // announce ourselves), and sounds the emergency-alert chirp.
  void messenger_on_delivery(LXMF::LXMessage &msg) {
    if (!urns_message_store) return;
    // No mask/unmask here anymore - this whole call is now nested inside
    // urns_lxmf_loop()'s single outer LoRa->maskDio0()/unmaskDio0() guard
    // (URNS.h), which covers the entire handle_incoming()/process_inbound()
    // pipeline this callback fires from, not just this one save. See that
    // guard's own comment for why an inner unmask here would have been
    // actively wrong (maskDio0()/unmaskDio0() aren't nesting-safe - a bare
    // detachInterrupt/attachInterrupt pair, not a counter - so unmasking
    // here would re-arm the interrupt for whatever of the outer pipeline
    // still had to run afterward).
    bool saved = urns_message_store->save_message(msg);
    if (!saved) DEBUG_LOG("[Messenger] on_delivery: save_message failed for %s\r\n", msg.hash().toHex().c_str());

    std::string name;
    RNS::Bytes cached_app_data = RNS::Identity::recall_app_data(msg.source_hash());
    if (messenger_display_name_from_app_data(cached_app_data, name)) {
      urns_message_store->set_display_name(msg.source_hash(), name);
    } else {
      // Still no cached app_data for this sender (typical for a manually-
      // added-by-hash bookmark - their identity got resolved via a bare
      // path response while Pinging/Sending, Menu.h, which carries no
      // app_data of its own) - a real LXMF delivery just proved this peer
      // is reachable right now, so it's worth spending one more path
      // request to try to actually pick up their announce (and therefore
      // their display name) rather than silently staying nameless
      // forever. Harmless no-op if it never resolves - just falls back to
      // the hex label already shown, same as today. Whatever comes back
      // is picked up live by messenger_peer_display_name()'s own
      // Identity::recall_app_data() check (Identity.cpp's app_data
      // backfill comment explains why a later, better answer isn't
      // silently dropped anymore) rather than needing to be awaited here.
      RNS::Transport::request_path(msg.source_hash());
    }

    #if HAS_BUZZER == true
      buzzer_lxmf_rx_melody();
    #endif
  }

  // Proof-of-delivery tracking for the most recently sent message - same
  // "app-level state machine polled from loop(), screen just reads it live
  // on every redraw" shape as the Ping feature below (messenger_ping_*),
  // and for the same underlying reason: LXMRouter's static_proof_callback
  // (LXMRouter.cpp) fires asynchronously, sometime after handle_outbound()
  // returns, whenever - if ever - the recipient's proof packet makes it
  // back. Only one send is tracked at a time (the UI only ever has one
  // send in flight - Text Entry/preset actions are the only ways to
  // trigger a send, and both leave the peer screen while pending).
  #define MSNGR_SEND_IDLE       0
  #define MSNGR_SEND_PENDING    1 // enqueued, awaiting delivery proof
  #define MSNGR_SEND_DELIVERED  2
  #define MSNGR_SEND_TIMEOUT    3 // no proof within MSNGR_SEND_DELIVERY_TIMEOUT_MS
  // Identity::recall() came up empty (this node has never received/relayed
  // an announce from this peer, so it has neither their identity nor a
  // path - both only ever arrive via an announce). Rather than failing
  // outright, request_path() has been sent and the message content is
  // parked in msngr_send_pending_content until either an identity shows up
  // (some node forwards a cached announce back as a PATH_RESPONSE) or
  // MSNGR_SEND_RESOLVE_TIMEOUT_MS elapses. Same shape as messenger_ping_
  // start()/_process()'s own MSNGR_PING_RESOLVING, which already does this
  // for the path-only case - this covers path+identity together, since a
  // recall() failure means neither is known yet.
  #define MSNGR_SEND_RESOLVING  4
  #define MSNGR_SEND_UNRESOLVED 5 // no identity within MSNGR_SEND_RESOLVE_TIMEOUT_MS
  // Router-confirmed failure (messenger_on_failed(), registered via
  // LXMRouter::register_failed_callback()) - LXMRouter.cpp's
  // static_proof_timeout_callback() exhausted msngr_max_retries attempts
  // for this OPPORTUNISTIC send and gave up for good. Distinct from
  // MSNGR_SEND_TIMEOUT (this screen's own blind guess-timeout, which still
  // exists as a backstop for delivery methods this specific fix doesn't
  // cover) - this one is a real, router-confirmed "it's not going to be
  // delivered", not just "we haven't heard back yet".
  #define MSNGR_SEND_FAILED     6
  // PROPAGATED-only terminal success state - the message reached the
  // active propagation node (static_propagation_resource_concluded(),
  // LXMRouter.cpp, confirmed the resource transfer), not the final
  // recipient. Distinct from MSNGR_SEND_DELIVERED on purpose: python LXMF
  // itself draws this same distinction (a PN handoff is "sent", full
  // end-to-end delivery confirmation for a PROPAGATED message isn't
  // something this firmware tracks any further than that) - see
  // messenger_on_sent()'s own comment below.
  #define MSNGR_SEND_SENT_TO_NODE 7
  // Generous - OPPORTUNISTIC delivery's proof has to travel from the
  // recipient back to us, potentially multiple LoRa hops each way, with
  // no guaranteed path warm already. PacketReceipt's own auto-computed
  // timeout (Packet::receipt_send(), Reticulum::get_first_hop_timeout())
  // fires independently of this - this is purely the UI's own "how long
  // will the Messenger screen wait before giving up on this specific
  // send", same reasoning as MSNGR_PING_PATH_TIMEOUT_MS/LINK_TIMEOUT_MS.
  #define MSNGR_SEND_DELIVERY_TIMEOUT_MS 60000
  // Extra UI-timeout budget added on top of the above (and of the retry-
  // budget extension below it) specifically for a PROPAGATED send whose
  // active node requires a stamp - LXStamper's own documented worst case
  // is ~2 minutes (send_propagated()'s comment, LXMRouter.cpp); this gives
  // a comfortable margin above that rather than cutting it close.
  #define MSNGR_SEND_STAMP_GRIND_BUDGET_MS 150000
  // LXMRouter::PATH_REQUEST_WAIT (LXMRouter.h) is the library's own
  // considered value for "how long a path request needs on LoRa" (its
  // comment: "Python: 7s, but LoRa needs more RX window") - matched here
  // rather than messenger_ping's own shorter MSNGR_PING_PATH_TIMEOUT_MS
  // (10s, path-only) since this also has to wait for a full relayed
  // announce (path+identity), not just a path table entry.
  #define MSNGR_SEND_RESOLVE_TIMEOUT_MS 15000
  // How long the terminal Delivered/No Confirmation/Unknown Destination
  // result stays on screen before auto-returning to the peer screen with
  // no input needed - same auto-dismiss convention as the Announce/GPS-
  // Sync/NTP-Sync popups (Menu.h's menu_popup_process()), just implemented
  // locally here since this is a dedicated live-status screen
  // (MENU_STATE_MSNGR_SEND_RESULT), not the generic MENU_STATE_STATUS_POPUP.
  #define MSNGR_SEND_RESULT_POPUP_MS 20000
  // Matches Menu.h's MSNGR_TEXT_ENTRY_MAX_LEN - can't reference that
  // constant directly, Menu.h is #include'd after this file (see
  // messenger_send_process()'s own comment on the same layering split).
  #define MSNGR_SEND_CONTENT_MAX_LEN 140

  uint8_t msngr_send_state = MSNGR_SEND_IDLE;
  RNS::Bytes msngr_send_message_hash;
  unsigned long msngr_send_started_ms = 0;
  // Live attempt count for the MSNGR_SEND_PENDING screen (Menu.h) - kept
  // in sync from urns_lxmf_router->pending_outbound_front()->
  // delivery_attempts() by messenger_send_process()'s own poll below,
  // since OPPORTUNISTIC retries (LXMRouter.cpp's static_proof_timeout_
  // callback()) happen entirely inside the router, asynchronously, with
  // no other signal back to the UI in between the PENDING and final
  // DELIVERED/FAILED states. 1 means "first attempt, no retry yet".
  uint8_t msngr_send_attempt = 1;
  bool msngr_send_stamp_seen = false; // this send's stamp was observed being mined - see messenger_send_process()
  // Which method LXMessage::pack() actually resolved this send to
  // (LXMF::Type::Message::OPPORTUNISTIC, ::DIRECT, or ::PROPAGATED) - read
  // straight off the local msg object in messenger_send_lxmf_resolved()
  // right after handle_outbound() returns, since handle_outbound() calls
  // pack() synchronously (LXMRouter.cpp) before queueing, so msg.method()
  // is already resolved by then even though the caller only ever
  // *requested* OPPORTUNISTIC or PROPAGATED (messenger_current_delivery_
  // mode(), per the peer's Send Direct/Send Propagated setting) - for
  // Send Direct, LXMRouter silently upgrades OPPORTUNISTIC to DIRECT for
  // anything over LORA_ENCRYPTED_PACKET_MDU. Displayed on the MSNGR_SEND_
  // PENDING screen (Menu.h) so it's clear which path a given send actually
  // took.
  uint8_t msngr_send_method = LXMF::Type::Message::OPPORTUNISTIC;
  // Live LXMF::Type::Message::State for the in-flight send (OUTBOUND/
  // SENDING/SENT), polled from urns_lxmf_router->pending_outbound_state_for()
  // the same way msngr_send_attempt is polled from pending_outbound_
  // attempts_for() above - lets the MSNGR_SEND_PENDING screen (Menu.h)
  // distinguish "still trying to get the packet onto the radio" from
  // "handed off, waiting on delivery proof" instead of one static string.
  uint8_t msngr_send_router_state = LXMF::Type::Message::OUTBOUND;
  unsigned long msngr_send_result_at_ms = 0; // set when state becomes DELIVERED/TIMEOUT/UNRESOLVED
  // Valid only while msngr_send_state == MSNGR_SEND_RESOLVING - the send
  // that's parked waiting for messenger_send_process() to find out whether
  // Identity::recall() ever comes good.
  RNS::Bytes msngr_send_pending_dest_hash;
  char msngr_send_pending_content[MSNGR_SEND_CONTENT_MAX_LEN + 1];
  unsigned long msngr_send_resolve_started_ms = 0;
  // 0 = no override, use messenger_current_delivery_mode() as normal -
  // otherwise an explicit LXMF::Type::Message::Method (e.g. PROPAGATED for
  // MENU_STATE_MSNGR_SEND_RESULT's "Retry via Prop" row, Menu.h) that wins
  // regardless of the peer's own Send Direct/Send Propagated setting, for
  // this one send only. Remembered alongside the other msngr_send_pending_*
  // fields so it survives the RESOLVING wait too (messenger_send_process()
  // below passes it through once Identity::recall() comes good).
  uint8_t msngr_send_pending_forced_method = 0;
  // Raw msgpack FIELD_AUDIO value (LXMF::build_audio_field()) for a voice
  // send, empty for text sends. Lives alongside msngr_send_pending_content
  // for the same reason: Retry, the post-RESOLVING deferred build and
  // messenger_on_delivered()'s propagated-history reconstruction all rebuild
  // the message from these globals rather than from the original object.
  RNS::Bytes msngr_send_pending_audio;
  // The exact packed bytes (hash/signature/timestamp/fields) of the message
  // currently being tracked, kept from handle_outbound() until the next send.
  // DIRECT deliveries (any Resource: long text, voice) are confirmed with a
  // hash-only placeholder from LXMRouter::handle_direct_proof(), and so are
  // PROPAGATED handoffs - neither carries the real message, so history is
  // saved by unpacking these bytes instead (messenger_save_sent_from_packed()).
  RNS::Bytes msngr_send_pending_packed;
  uint8_t msngr_send_pending_audio_mode = 0;
  // Set by messenger_send_lxmf_resolved() whenever it actually saves a new
  // outgoing message - consumed by Menu.h's msngr_send_result_process()
  // (polled from loop() same as this file's own messenger_send_process())
  // to refresh the on-screen peer cache. A single mechanism for both the
  // immediate (identity already known) and deferred (post-RESOLVING) send
  // paths, since the latter completes from inside this file where Menu.h's
  // messenger_refresh_peer_cache() isn't visible yet (same layering split
  // as msngr_send_result_process() itself).
  bool msngr_send_needs_cache_refresh = false;

  // Saves the tracked send to history from its retained packed bytes - an
  // exact reconstruction (same hash, signature, timestamp, fields) of what
  // went on the air, unlike rebuilding from the content string. Returns false
  // if `hash` isn't the tracked message or nothing was retained.
  bool messenger_save_sent_from_packed(const RNS::Bytes &hash) {
    if (!urns_message_store || !msngr_send_pending_packed.size() || !(hash == msngr_send_message_hash)) return false;
    bool saved = false;
    LoRa->maskDio0();
    try {
      LXMF::LXMessage saved_msg = LXMF::LXMessage::unpack_from_bytes(msngr_send_pending_packed, LXMF::Type::Message::DIRECT, true);
      saved_msg.incoming(false);
      saved_msg.state(LXMF::Type::Message::DELIVERED);
      if (saved_msg.hash() == hash) saved = urns_message_store->save_message(saved_msg);
    } catch (const std::exception &e) {
      DEBUG_LOG("[Messenger] save_sent_from_packed: %s\r\n", e.what());
    }
    LoRa->unmaskDio0();
    if (!saved) DEBUG_LOG("[Messenger] save_sent_from_packed failed for %s\r\n", hash.toHex().c_str());
    return saved;
  }

  // Registered via LXMRouter::register_delivered_callback() (URNS.h) -
  // fires once for every outbound message this router gets delivery proof
  // for, not just the one the UI happens to be tracking, so this must
  // check the hash before touching UI state (a proof for some earlier,
  // already-timed-out send arriving late shouldn't resurrect/overwrite
  // whatever's currently being tracked).
  //
  // FIXED (local patch, not upstream): save_message() used to happen
  // unconditionally in messenger_send_lxmf_resolved(), at send time,
  // regardless of whether delivery was ever actually confirmed - so a
  // message that failed still showed up in the conversation history as if
  // it had gone through. Moved here, since this only ever fires once
  // delivery is genuinely confirmed (LXMRouter.cpp's static_proof_
  // callback() now pops the real, full-content message from its outbound
  // queue and hands it to this callback - see that function's own
  // comment - rather than the hash-only placeholder it used to pass).
  void messenger_on_delivered(LXMF::LXMessage &msg) {
    if (msg.destination_hash().size() == 0) {
      // Hash-only placeholder (DIRECT delivery proofs, incl. every Resource
      // send - long text and voice): saving it would index a bogus empty-peer
      // conversation, and it holds none of the real content anyway.
      // Reconstruct the real message from what was retained at send time.
      if (!messenger_save_sent_from_packed(msg.hash())) {
        DEBUG_LOG("[Messenger] delivered: placeholder for %s, nothing retained to save\r\n", msg.hash().toHex().c_str());
      }
    } else {
      // See messenger_on_delivery()'s own comment (this file) for why -
      // same flash-I/O-vs-DIO0-ISR hazard.
      LoRa->maskDio0();
      bool saved = urns_message_store->save_message(msg);
      LoRa->unmaskDio0();
      if (!saved) {
        DEBUG_LOG("[Messenger] delivered: save_message failed for %s\r\n", msg.hash().toHex().c_str());
      }
    }
    msngr_send_needs_cache_refresh = true;

    if (msngr_send_state == MSNGR_SEND_PENDING && msg.hash() == msngr_send_message_hash) {
      msngr_send_state = MSNGR_SEND_DELIVERED;
      msngr_send_result_at_ms = millis();
    }
  }

  // Registered via LXMRouter::register_failed_callback() (URNS.h) - fires
  // once LXMRouter has exhausted msngr_max_retries attempts for an
  // OPPORTUNISTIC send (see static_proof_timeout_callback()'s own comment,
  // LXMRouter.cpp) and given up for good. Deliberately does NOT call
  // save_message() - a message that never got delivered has no business
  // in the conversation history (see messenger_on_delivered()'s own
  // comment for the matching success-side half of this fix). Same hash-
  // check reasoning as messenger_on_delivered() - fires for every failed
  // outbound message on this router, not just the one the UI is tracking.
  void messenger_on_failed(LXMF::LXMessage &msg) {
    if (msngr_send_state == MSNGR_SEND_PENDING && msg.hash() == msngr_send_message_hash) {
      msngr_send_state = MSNGR_SEND_FAILED;
      msngr_send_result_at_ms = millis();
    }
  }

  // Registered via LXMRouter::register_sent_callback() (URNS.h) - the ONLY
  // reliable, router-confirmed success signal PROPAGATED sends ever get in
  // this library. PROPAGATED never routes through messenger_on_delivered()
  // above - LXMRouter.cpp's static_propagation_resource_concluded()
  // deliberately reports a confirmed PN handoff via _sent_callback, not
  // _delivered_callback (see MSNGR_SEND_SENT_TO_NODE's own comment for
  // why) - without this, a PROPAGATED send had no way to ever leave
  // MSNGR_SEND_PENDING except the UI's own blind MSNGR_SEND_DELIVERY_
  // TIMEOUT_MS backstop, which is exactly the "stuck on Sending Propagated,
  // then Failed/No Confirmation" bug this fixes.
  //
  // This callback is NOT propagation-specific, though - OPPORTUNISTIC and
  // DIRECT-via-link both also call it (LXMRouter.cpp's process_outbound()),
  // immediately at transmission time, with the real full-content message,
  // well before their own eventual messenger_on_delivered()/_on_failed()
  // terminal outcome - that's not a terminal result for those two methods
  // (delivery proof is still pending), so this must ignore those calls
  // entirely and let their existing PENDING->DELIVERED/FAILED flow run
  // undisturbed. msg.method() can't tell the two apart here - the hash-
  // only placeholder static_propagation_resource_concluded() constructs
  // never sets it, so it silently defaults to DIRECT (LXMessage's own
  // member default) - msngr_send_method (already resolved and stashed
  // right after handle_outbound() returns, see its own declaration) is
  // the reliable signal instead: the currently-tracked send's own real,
  // resolved method.
  void messenger_on_sent(LXMF::LXMessage &msg) {
    if (msngr_send_method != LXMF::Type::Message::PROPAGATED) return;
    if (msngr_send_state == MSNGR_SEND_PENDING && msg.hash() == msngr_send_message_hash) {
      // FIXED (local patch, not upstream): this used to call save_message()
      // directly on msg here, same as messenger_on_delivered() above - but
      // msg is the hash-only placeholder static_propagation_resource_
      // concluded() constructs (empty destination/source hash), and
      // MessageStore groups a saved OUTGOING message's conversation by
      // destination_hash() (MessageStore.cpp's save_message()) - so that
      // was writing a bogus conversation keyed by an EMPTY peer hash into
      // the store's index every time a propagated send confirmed. That
      // corrupted index then got rejected wholesale on reload
      // (MessageStore.cpp's load_index_file(), now itself hardened to skip
      // just the bad entry instead - see that fix's own comment), wiping
      // every real conversation from the Inbox.
      //
      // Rebuilt here instead using what this firmware layer already
      // retains locally - msngr_send_pending_dest_hash (the destination
      // this exact send used, unconditionally populated by every
      // messenger_send_lxmf() call, this file's own declaration) and
      // msngr_send_pending_content (same call, same guarantee) - same
      // construction shape messenger_send_lxmf_resolved() used to build
      // the original message, just reconstructed here since process_
      // outbound() (LXMRouter.cpp) already popped and discarded the real
      // one the moment the resource transfer began, well before this
      // confirmation ever arrives. Menu.h's msngr_active_peer_hash isn't
      // visible from this file (included first, same layering split as
      // everywhere else in this function) but would hold the same value.
      RNS::Identity dest_identity = RNS::Identity::recall(msngr_send_pending_dest_hash);
      if (messenger_save_sent_from_packed(msg.hash())) {
        // Saved exactly as sent (including any voice field) - nothing to rebuild.
      } else if (dest_identity) {
        RNS::Destination dest(dest_identity, RNS::Type::Destination::OUT, RNS::Type::Destination::SINGLE, "lxmf", "delivery");
        LXMF::LXMessage saved_msg(dest, urns_lxmf_router->delivery_destination(),
          RNS::bytesFromString(msngr_send_pending_content), RNS::Bytes(), LXMF::Type::Message::PROPAGATED);
        if (msngr_send_pending_audio.size()) {
          const uint8_t audio_key = LXMF::FIELD_AUDIO;
          saved_msg.fields_set(RNS::Bytes(&audio_key, 1), msngr_send_pending_audio);
        }
        saved_msg.hash(msngr_send_message_hash);
        // _timestamp defaults to 0.0 and is normally only ever assigned
        // inside pack() at actual send time (LXMessage.h's own comment) -
        // this reconstructed message never goes through pack()/
        // handle_outbound(), so it'd otherwise save with no timestamp at
        // all. The exact original compose time isn't available here (the
        // real message was already popped/discarded well before this
        // confirmation arrives, see this function's own comment above) -
        // "now" is the closest available approximation, and this whole
        // round trip normally completes within seconds of the original
        // send anyway.
        saved_msg.timestamp(RNS::Utilities::OS::time());
        // Same flash-I/O-vs-DIO0-ISR guard as messenger_on_delivered() above.
        LoRa->maskDio0();
        bool saved = urns_message_store->save_message(saved_msg);
        LoRa->unmaskDio0();
        if (!saved) {
          DEBUG_LOG("[Messenger] sent-to-node: save_message failed for %s\r\n", saved_msg.hash().toHex().c_str());
        }
      } else {
        DEBUG_LOG("[Messenger] sent-to-node: identity for %s not recallable, not saving to history\r\n", msngr_send_pending_dest_hash.toHex().c_str());
      }
      msngr_send_needs_cache_refresh = true;

      msngr_send_state = MSNGR_SEND_SENT_TO_NODE;
      msngr_send_result_at_ms = millis();
    }
  }

  // Polled every loop() iteration (RNode_Firmware.ino, alongside
  // messenger_ping_process()) - enforces MSNGR_SEND_DELIVERY_TIMEOUT_MS
  // (LXMRouter's own proof-timeout callback frees the library's internal
  // tracking slot independently, see static_proof_timeout_callback's own
  // comment, LXMRouter.h - this is purely the UI-facing timeout). Doesn't
  // touch menu_state/navigation at all - same layering as
  // messenger_ping_process() (Menu.h is #include'd after this file, so it
  // can't reference Menu.h's menu_state/MENU_STATE_* anyway). The
  // MENU_STATE_MSNGR_SEND_RESULT screen's own auto-dismiss (Menu.h) is
  // what actually navigates away once msngr_send_result_at_ms is old
  // enough - this function only owns the PENDING->DELIVERED/TIMEOUT edge.
  // The actual build-and-enqueue tail, factored out of messenger_send_lxmf()
  // so both the immediate path (identity already known) and the deferred
  // one (messenger_send_process() below, once Identity::recall() finally
  // comes good after a RESOLVING wait) can share it. dest_identity is
  // passed in rather than re-recalled since both callers already have it
  // in hand. Defined before messenger_send_process() since that function
  // calls it.
  void messenger_send_lxmf_resolved(const RNS::Bytes &dest_hash, RNS::Identity &dest_identity, const char *content, uint8_t forced_method = 0) {
    RNS::Destination dest(dest_identity, RNS::Type::Destination::OUT, RNS::Type::Destination::SINGLE, "lxmf", "delivery");
    // Send Direct (the default, requests OPPORTUNISTIC - LXMRouter silently
    // upgrades to DIRECT for anything over LORA_ENCRYPTED_PACKET_MDU, see
    // msngr_send_method's own comment) vs Send Propagated (requests
    // PROPAGATED up front, which handle_outbound() routes straight to the
    // active propagation node instead of ever attempting delivery to
    // dest_hash directly) - MENU_STATE_MSNGR_PEER's own row, right below
    // Ping. See messenger_current_delivery_mode()'s own comment for the
    // bookmarked-vs-session-only distinction. forced_method (non-zero)
    // overrides both - see msngr_send_pending_forced_method's own comment.
    LXMF::Type::Message::Method desired_method = forced_method != 0
      ? (LXMF::Type::Message::Method)forced_method
      : ((messenger_current_delivery_mode(dest_hash) == MSNGR_DELIVERY_MODE_PROPAGATED)
          ? LXMF::Type::Message::PROPAGATED : LXMF::Type::Message::OPPORTUNISTIC);
    // Defensive refresh, every PROPAGATED send - see this function's own
    // declaration for why. Must happen before handle_outbound() below:
    // send_propagated() (LXMRouter.cpp) reads _outbound_propagation_stamp_
    // cost the first time process_outbound() actually processes this
    // message, which can happen as early as the very next loop() tick.
    if (desired_method == LXMF::Type::Message::PROPAGATED) {
      messenger_refresh_prop_node_stamp_cost();
    }
    // No stored announce app_data for this peer means its stamp cost can't
    // be known (LXMRouter::resolve_outbound_stamp_cost() reads it from
    // there) - this send may go out unstamped and be silently dropped by a
    // stamp-enforcing receiver even though its packet proof still comes
    // back as "Delivered". Ask the mesh for a fresh announce so the next
    // send knows; not worth blocking this one on (a peer that truly has no
    // app_data would otherwise stall every send).
    if (desired_method != LXMF::Type::Message::PROPAGATED && !RNS::Identity::recall_app_data(dest_hash)) {
      DEBUG_LOG("[Messenger] send: no announce app_data for %s, stamp cost unknown - requesting path\r\n", dest_hash.toHex().c_str());
      RNS::Transport::request_path(dest_hash);
    }
    LXMF::LXMessage msg(dest, urns_lxmf_router->delivery_destination(), RNS::bytesFromString(content),
      RNS::Bytes(), desired_method);
    if (msngr_send_pending_audio.size()) {
      const uint8_t audio_key = LXMF::FIELD_AUDIO;
      msg.fields_set(RNS::Bytes(&audio_key, 1), msngr_send_pending_audio);
    }
    urns_lxmf_router->handle_outbound(msg);
    // handle_outbound() calls pack() synchronously before queueing
    // (LXMRouter.cpp), so msg.method() already reflects OPPORTUNISTIC vs
    // the silent DIRECT upgrade for oversized content - see
    // msngr_send_method's own declaration.
    msngr_send_method = msg.method();
    // FIXED (local patch, not upstream): save_message() moved to
    // messenger_on_delivered() - see that function's own comment for why
    // saving unconditionally here (regardless of whether delivery is ever
    // confirmed) was wrong.
    msngr_send_needs_cache_refresh = true;

    // Start delivery-proof tracking - see its own declaration above for
    // why the UI can't just show "Sent" here: handle_outbound() only
    // enqueued/transmitted the packet, static_proof_callback() (LXMRouter.
    // cpp) fires later, asynchronously, if and when the recipient's proof
    // makes it back.
    msngr_send_message_hash = msg.hash();
    msngr_send_pending_packed = msg.packed();
    msngr_send_state = MSNGR_SEND_PENDING;
    msngr_send_started_ms = millis();
    msngr_send_stamp_seen = false;
    msngr_send_attempt = 1;
    msngr_send_router_state = LXMF::Type::Message::OUTBOUND;

    DEBUG_LOG("[Messenger] send: queued message to %s\r\n", dest_hash.toHex().c_str());
  }

  void messenger_send_process() {
    if (msngr_send_state == MSNGR_SEND_PENDING) {
      // Live-refresh the attempt count for MENU_STATE_MSNGR_SEND_RESULT
      // (Menu.h) - see msngr_send_attempt's own declaration for why this
      // has to be polled rather than pushed. The message being tracked is
      // only ever at the front of the router's outbound queue (single-
      // in-flight-message design, see LXMRouter.cpp's process_outbound())
      // while a retry cycle is in progress - once delivered or failed for
      // good it's popped and messenger_on_delivered()/_on_failed() (above)
      // take over via their own hash check.
      int attempts = urns_lxmf_router->pending_outbound_attempts_for(msngr_send_message_hash);
      if (attempts > 0) { msngr_send_attempt = (uint8_t)attempts; }
      // Same "only trust a real match" guard as the attempts poll above -
      // a stale read exactly at the DELIVERED/FAILED pop transition
      // (message already gone from the front of the queue) would
      // otherwise clobber the last known phase with the GENERATING
      // sentinel right as the result screen is about to take over.
      LXMF::Type::Message::State router_state = urns_lxmf_router->pending_outbound_state_for(msngr_send_message_hash);
      if (router_state != LXMF::Type::Message::GENERATING) { msngr_send_router_state = router_state; }
      // FIXED (local patch, not upstream): this UI backstop used to be a
      // fixed MSNGR_SEND_DELIVERY_TIMEOUT_MS, sized for "no real retries
      // ever happen". Now that OPPORTUNISTIC/DIRECT genuinely retry up to
      // msngr_max_retries times with msngr_retry_delay_s backoff between
      // each (LXMRouter.cpp), the worst case can take far longer than
      // 60s - compute the effective timeout dynamically so this screen
      // doesn't auto-dismiss to "No Confirmation" before the router's own
      // retry cycle has actually finished (which would otherwise silently
      // drop the real, later messenger_on_delivered()/messenger_on_failed()
      // callback, since both guard on msngr_send_state == MSNGR_SEND_PENDING).
      unsigned long effective_timeout_ms = MSNGR_SEND_DELIVERY_TIMEOUT_MS;
      unsigned long retry_budget_ms = (unsigned long)msngr_max_retries * (unsigned long)msngr_retry_delay_s * 1000UL + 10000UL;
      if (retry_budget_ms > effective_timeout_ms) { effective_timeout_ms = retry_budget_ms; }
      // Same reasoning as the retry-budget extension above, for PROPAGATED's
      // own extra pre-transfer phase: LXStamper's proof-of-work grind
      // (send_propagated()'s own comment, LXMRouter.cpp) doesn't burn a
      // delivery attempt while running (LXMRouter.cpp's own FIXED patch),
      // so it isn't covered by retry_budget_ms at all - without this, this
      // screen would auto-dismiss to "No Confirmation" on a send that's
      // still genuinely, successfully grinding in the background whenever
      // the active propagation node requires a stamp.
      if (msngr_send_method == LXMF::Type::Message::PROPAGATED && urns_lxmf_router->outbound_propagation_stamp_cost() > 0) {
        effective_timeout_ms += MSNGR_SEND_STAMP_GRIND_BUDGET_MS;
      }
      // Direct/opportunistic sends to a stamp-requiring peer mine their
      // stamp on the LXStamper worker before any transmit, not covered by
      // retry_budget_ms either. Latched once seen (the router drops the
      // pending flag the moment the stamp lands) so the extra budget stays
      // for the rest of this send instead of vanishing mid-proof-wait.
      if (urns_lxmf_router->pending_outbound_front_stamp_pending_for(msngr_send_message_hash)) {
        msngr_send_stamp_seen = true;
      }
      if (msngr_send_stamp_seen) {
        effective_timeout_ms += MSNGR_SEND_STAMP_GRIND_BUDGET_MS;
      }
      if (millis() - msngr_send_started_ms > effective_timeout_ms) {
        msngr_send_state = MSNGR_SEND_TIMEOUT;
        msngr_send_result_at_ms = millis();
      }
    } else if (msngr_send_state == MSNGR_SEND_RESOLVING) {
      RNS::Identity dest_identity = RNS::Identity::recall(msngr_send_pending_dest_hash);
      if (dest_identity) {
        messenger_send_lxmf_resolved(msngr_send_pending_dest_hash, dest_identity, msngr_send_pending_content, msngr_send_pending_forced_method);
      } else if (millis() - msngr_send_resolve_started_ms > MSNGR_SEND_RESOLVE_TIMEOUT_MS) {
        msngr_send_state = MSNGR_SEND_UNRESOLVED;
        msngr_send_result_at_ms = millis();
      }
    }
  }

  // General-purpose send, shared by every Messenger screen (bookmark/
  // announce/inbox peer actions) - same OPPORTUNISTIC-delivery shape as
  // URNS.h's own urns_send_test_lxmf(), generalized to an arbitrary
  // destination hash instead of the hardcoded Phase 1 test one. Persists
  // the sent message to urns_message_store on success so it shows up in
  // that peer's thread alongside anything they send back.
  uint8_t messenger_send_lxmf(const RNS::Bytes &dest_hash, const char *content, uint8_t forced_method = 0, bool keep_audio = false) {
    // Let the confirm-click that triggered this Send finish playing
    // before the TX below can freeze it mid-note - see
    // buzzer_wait_for_melody()'s own comment (Utilities.h).
    #if HAS_BUZZER == true
      buzzer_wait_for_melody();
    #endif
    if (!urns_ready || !urns_message_store) return URNS_LXMF_SEND_NOT_READY;

    // Remembered unconditionally (not just on the RESOLVING path below) so
    // a later manual Retry from MENU_STATE_MSNGR_SEND_RESULT (Menu.h) can
    // always re-fire the exact same destination/content without the caller
    // having to keep its own copy around. forced_method rides along the
    // same way - see msngr_send_pending_forced_method's own comment.
    msngr_send_pending_dest_hash = dest_hash;
    // Text sends drop any leftover voice payload; messenger_send_voice() and
    // Menu.h's Retry rows (re-sending whatever is pending) pass keep_audio.
    if (!keep_audio) msngr_send_pending_audio = RNS::Bytes();
    strncpy(msngr_send_pending_content, content, MSNGR_SEND_CONTENT_MAX_LEN);
    msngr_send_pending_content[MSNGR_SEND_CONTENT_MAX_LEN] = 0;
    msngr_send_pending_forced_method = forced_method;

    RNS::Identity dest_identity = RNS::Identity::recall(dest_hash);
    if (!dest_identity) {
      // Neither identity nor path is known - this node has never received/
      // relayed an announce from this peer (both only ever arrive via one,
      // see project_path_table_persistence_fix memory's sibling
      // investigation for why an inbound LXMF message alone never teaches
      // either). request_path() asks the mesh instead of failing outright -
      // any node that still has this peer's announce cached answers with a
      // PATH_RESPONSE, which is processed exactly like a fresh announce
      // (populates both identity and path together). The actual message is
      // parked in msngr_send_pending_content and built once
      // messenger_send_process() sees recall() succeed, or abandoned after
      // MSNGR_SEND_RESOLVE_TIMEOUT_MS.
      DEBUG_LOG("[Messenger] send: no known identity for %s, requesting path\r\n", dest_hash.toHex().c_str());
      RNS::Transport::request_path(dest_hash);
      msngr_send_state = MSNGR_SEND_RESOLVING;
      msngr_send_resolve_started_ms = millis();
      return URNS_LXMF_SEND_RESOLVING;
    }

    messenger_send_lxmf_resolved(dest_hash, dest_identity, content, forced_method);
    return URNS_LXMF_SEND_OK;
  }

  #if HAS_AUDIO == true
    // Voice message: empty text plus FIELD_AUDIO [mode, codec2 frames]. Always
    // larger than one packet, so LXMRouter upgrades it to a DIRECT Resource
    // (or PROPAGATED, per the peer's delivery-mode setting) by itself.
    uint8_t messenger_send_voice(const RNS::Bytes &dest_hash, uint8_t mode, const uint8_t *data, size_t len, const char *text = "") {
      msngr_send_pending_audio = LXMF::build_audio_field(mode, data, len);
      msngr_send_pending_audio_mode = mode;
      return messenger_send_lxmf(dest_hash, text ? text : "", 0, true);
    }
  #endif

  // "Ping" - not an LXMF message at all, an RNS::Link established directly
  // to the peer's lxmf.delivery destination, torn down again once its RTT
  // is known. Works against any LXMF-reachable peer (this firmware,
  // Sideband, NomadNet, ...) with zero cooperation needed on their end -
  // LXMRouter already accepts incoming links on its own delivery
  // destination for the DIRECT delivery method (see
  // set_link_established_callback() in lib/microLXMF's LXMRouter.cpp), so
  // the link handshake itself (cryptographic open+proof, same as
  // rnprobe/Sideband's own "ping" use of RNS::Link) is all a remote needs
  // to answer. An idle link left open after that is harmless on their
  // side - it just sits registered for a delivery that never comes and
  // eventually goes stale on its own.
  //
  // Also usable against a Propagation-type bookmark (msngr_ping_target_
  // is_prop below) - a reference-implementation propagation node has to
  // accept link requests on its lxmf.propagation destination too, to serve
  // syncs and propagated deliveries at all (see send_propagated()/request_
  // messages_from_propagation_node(), lib/microLXMF's LXMRouter.cpp, which
  // both open exactly this kind of Link against that same destination) -
  // so the same bare handshake-as-pong trick applies, just against
  // "propagation" instead of "delivery".
  #define MSNGR_PING_IDLE         0
  #define MSNGR_PING_RESOLVING    1 // waiting on Transport::request_path()
  #define MSNGR_PING_ESTABLISHING 2 // Link constructed, LINKREQUEST sent, awaiting proof
  #define MSNGR_PING_SUCCESS      3
  #define MSNGR_PING_TIMEOUT      4
  // Despite the name, now only reachable via !urns_ready - a genuinely
  // missing identity/path goes through MSNGR_PING_RESOLVING instead (see
  // messenger_ping_start()'s own comment). Kept as a distinct state/name
  // rather than renamed, to avoid touching every existing reference for a
  // condition that still means exactly "can't even attempt this yet".
  #define MSNGR_PING_NO_IDENTITY  5
  #define MSNGR_PING_FAILED       6 // link closed for a reason other than our own timeout

  // Generous budgets - this can easily be a multi-hop LoRa round trip.
  // The vendored RNS::Link has no working establishment-timeout watchdog
  // of its own (Link::__watchdog_job(), Link.cpp, is an unported stub -
  // start_watchdog() is a no-op and the timeout logic in its comment block
  // never runs), so a PENDING/HANDSHAKE link that never gets a response
  // would otherwise sit forever - messenger_ping_process() below is what
  // actually enforces these.
  #define MSNGR_PING_PATH_TIMEOUT_MS 10000
  #define MSNGR_PING_LINK_TIMEOUT_MS 20000

  RNS::Link msngr_ping_link({RNS::Type::NONE});
  RNS::Bytes msngr_ping_target_hash;
  // Set by messenger_ping_start()'s is_prop argument - selects which
  // destination aspect messenger_ping_issue_link() builds against
  // ("propagation" vs the default "delivery").
  bool msngr_ping_target_is_prop = false;
  uint8_t msngr_ping_state = MSNGR_PING_IDLE;
  double msngr_ping_rtt = 0.0; // seconds, valid once msngr_ping_state == MSNGR_PING_SUCCESS
  unsigned long msngr_ping_phase_started_ms = 0;
  // Set alongside MSNGR_PING_SUCCESS below - consumed by Menu.h's
  // msngr_ping_result_process() to auto-dismiss MENU_STATE_MSNGR_PING_
  // RESULT after MSNGR_SEND_RESULT_POPUP_MS, same convention as (and
  // reusing the same constant as) MSNGR_SEND_RESULT screen's own
  // Delivered/Sent to Node auto-dismiss - only success does; TIMEOUT/
  // NO_IDENTITY/FAILED still require manual BACK, same reasoning as
  // MSNGR_SEND_RESULT's own error states.
  unsigned long msngr_ping_result_at_ms = 0;

  // Set by msngr_ping_link_established() and consumed by
  // messenger_ping_process() on the next loop() pass - teardown() mutates
  // Transport's active-link bookkeeping (link_closed(), Link.cpp), which
  // isn't safe to run synchronously from inside the established callback:
  // that callback fires from deep inside Transport's own inbound-packet
  // dispatch (Link::validate_proof(), reached while processing the LRPROOF
  // packet that just made the link ACTIVE), the same "don't do heavy/
  // reentrant work from inside a callback that's still on someone else's
  // call stack" reasoning as the DIO0 ISR deferring Messenger's own
  // handle_incoming() to loop() instead of running it inline.
  bool msngr_ping_teardown_pending = false;

  // Link::Callbacks::established/closed are plain function pointers (no
  // captures), so these have to be free functions rather than lambdas -
  // same constraint messenger_on_delivery() and MessengerAnnounceHandler
  // already work within.
  void msngr_ping_link_established(RNS::Link &link) {
    msngr_ping_rtt = link.rtt();
    msngr_ping_state = MSNGR_PING_SUCCESS;
    msngr_ping_result_at_ms = millis();
    msngr_ping_teardown_pending = true;
  }

  void msngr_ping_link_closed(RNS::Link &link) {
    // teardown() called from messenger_ping_process() after a SUCCESS (see
    // msngr_ping_teardown_pending above) also runs this callback - only
    // treat it as a real failure if we were still actively waiting on it.
    if (msngr_ping_state == MSNGR_PING_ESTABLISHING) {
      msngr_ping_state = MSNGR_PING_FAILED;
    }
  }

  void messenger_ping_issue_link() {
    RNS::Identity peer_identity = RNS::Identity::recall(msngr_ping_target_hash);
    if (!peer_identity) {
      msngr_ping_state = MSNGR_PING_NO_IDENTITY;
      return;
    }
    RNS::Destination dest(peer_identity, RNS::Type::Destination::OUT, RNS::Type::Destination::SINGLE, "lxmf", msngr_ping_target_is_prop ? "propagation" : "delivery");
    msngr_ping_link = RNS::Link(dest, msngr_ping_link_established, msngr_ping_link_closed);
    msngr_ping_state = MSNGR_PING_ESTABLISHING;
    msngr_ping_phase_started_ms = millis();
  }

  // Kicks off a ping to dest_hash - called from Menu.h when the Ping
  // action is confirmed on MENU_STATE_MSNGR_PEER (is_prop left at its
  // default false), or when MSNGR_PEER_PROP_ACTION_PING is confirmed
  // against a Propagation-type bookmark (is_prop true). Non-blocking: this only
  // starts path/identity resolution (or the link itself, if both are
  // already known) - messenger_ping_process() carries it the rest of the
  // way, and MENU_STATE_MSNGR_PING_RESULT's own draw code just reads
  // msngr_ping_state/msngr_ping_rtt fresh on every redraw.
  //
  // FIXED (local patch, not upstream): used to fail immediately with
  // MSNGR_PING_NO_IDENTITY whenever Identity::recall() came up empty,
  // before request_path() ever got a chance to run - same bug as
  // messenger_send_lxmf() had (see project_messenger_reply_unknown_
  // identity_fix memory for the full mechanism: identity and path are
  // both only ever learned together, via an announce/PATH_RESPONSE, never
  // from anything else). Now requests a path unconditionally whenever
  // either is missing - messenger_ping_process()'s own RESOLVING check
  // waits on Identity::recall() rather than has_path() specifically for
  // the same reason (whichever one request_path() actually resolves,
  // both arrive together).
  void messenger_ping_start(const RNS::Bytes &dest_hash, bool is_prop = false) {
    msngr_ping_target_hash = dest_hash;
    msngr_ping_target_is_prop = is_prop;
    msngr_ping_rtt = 0.0;
    msngr_ping_teardown_pending = false;
    msngr_ping_link = RNS::Link({RNS::Type::NONE});

    if (!urns_ready) {
      msngr_ping_state = MSNGR_PING_NO_IDENTITY;
      return;
    }

    if (RNS::Identity::recall(dest_hash) && RNS::Transport::has_path(dest_hash)) {
      messenger_ping_issue_link();
    } else {
      RNS::Transport::request_path(dest_hash);
      msngr_ping_state = MSNGR_PING_RESOLVING;
      msngr_ping_phase_started_ms = millis();
    }
  }

  // Cancels whatever's currently in flight - called from Menu.h when the
  // user backs out of MENU_STATE_MSNGR_PING_RESULT while a ping is still
  // pending. Safe to call teardown() here (unlike from inside the
  // callbacks above) since this only ever runs from the main button/
  // encoder path, never from inside Transport's own callback dispatch.
  void messenger_ping_cancel() {
    if (msngr_ping_state == MSNGR_PING_RESOLVING || msngr_ping_state == MSNGR_PING_ESTABLISHING) {
      if (msngr_ping_link) msngr_ping_link.teardown();
    }
    msngr_ping_teardown_pending = false;
    msngr_ping_state = MSNGR_PING_IDLE;
  }

  // Polled every loop() iteration (RNode_Firmware.ino, alongside
  // urns_lxmf_loop()) - see msngr_ping_teardown_pending's own comment for
  // the deferred-teardown half of this, and MSNGR_PING_PATH_TIMEOUT_MS/
  // MSNGR_PING_LINK_TIMEOUT_MS's own comment for why this also owns the
  // establishment timeout the vendored Link doesn't enforce itself.
  void messenger_ping_process() {
    if (msngr_ping_teardown_pending) {
      msngr_ping_teardown_pending = false;
      if (msngr_ping_link) msngr_ping_link.teardown();
    }

    if (msngr_ping_state == MSNGR_PING_RESOLVING) {
      // Identity::recall() rather than has_path() - see messenger_ping_
      // start()'s own comment for why: both arrive together via the same
      // announce/PATH_RESPONSE, and identity is the more fundamental of
      // the two blockers (messenger_ping_issue_link() needs it to even
      // construct the outbound Destination).
      if (RNS::Identity::recall(msngr_ping_target_hash)) {
        messenger_ping_issue_link();
      } else if (millis() - msngr_ping_phase_started_ms > MSNGR_PING_PATH_TIMEOUT_MS) {
        msngr_ping_state = MSNGR_PING_TIMEOUT;
      }
    } else if (msngr_ping_state == MSNGR_PING_ESTABLISHING) {
      if (millis() - msngr_ping_phase_started_ms > MSNGR_PING_LINK_TIMEOUT_MS) {
        msngr_ping_state = MSNGR_PING_TIMEOUT;
        if (msngr_ping_link) msngr_ping_link.teardown();
      }
    }
  }

  #if HAS_DEBUG_UART == true
    // Note: the RNS log callback itself (formerly defined here as
    // messenger_rns_log_to_debug_uart(), teeing to Serial0) now lives in
    // URNS.h as urns_rns_log_callback() - see that function's own comment
    // for why it moved (this file's messenger_init() is HAS_LXMF-only,
    // which left HAS_URNS-but-not-HAS_LXMF boards with no callback
    // registered at all).

    // User-requested: a periodic Serial0 line (independent of anything
    // else logging) so a physical TX LED on the debug UART adapter keeps
    // blinking on a known cadence while the device is alive - the last
    // blink before it stops is the freeze moment, without needing a
    // laptop actively capturing when it happens. Free heap included since
    // it's already right here and a slow leak trending toward the freeze
    // would show up in this same trail.
    #define MSNGR_HEARTBEAT_INTERVAL_MS 5000
    unsigned long msngr_heartbeat_last_ms = 0;

    // A periodic Serial0 line (independent of anything else logging) so a
    // physical TX LED on the debug UART adapter keeps blinking on a known
    // cadence while the device is alive.
    void messenger_heartbeat_process() {
      unsigned long now = millis();
      if (now - msngr_heartbeat_last_ms < MSNGR_HEARTBEAT_INTERVAL_MS) return;
      msngr_heartbeat_last_ms = now;
      // heap= (ESP.getFreeHeap()) turned out to already equal int_heap=
      // (MALLOC_CAP_INTERNAL) on every sample - PSRAM isn't merged into the
      // default allocator pool here, so every plain new/malloc/std::* in
      // the whole Link/Resource/Bytes object graph lands in the same small
      // internal-DRAM pool esp-aes's DMA buffers need, regardless of the
      // mbedtls_psram_calloc redirect (RNode_Firmware.ino) - that redirect
      // only catches mbedTLS's own calloc/free calls, not general C++
      // allocation. psram_heap= checks whether PSRAM is even being used
      // for anything right now.
      DEBUG_LOG(
        "[Heartbeat] alive, uptime=%lus heap=%u int_min=%u psram_heap=%u\r\n",
        now / 1000,
        (unsigned)ESP.getFreeHeap(),
        (unsigned)heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),
        (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
      );
      #if HAS_AUDIO == true
        {
          static uint32_t audio_diag_seen = 0;
          if (audio_diag_seen != audio_diag_seq) {
            audio_diag_seen = audio_diag_seq;
            DEBUG_LOG("[Audio] %s\r\n", audio_diag);
          }
        }
        if (audio_task_stack_hwm) {
          DEBUG_LOG("[Audio] play task done, min free stack=%u bytes\r\n", (unsigned)audio_task_stack_hwm);
          audio_task_stack_hwm = 0;
        }
        {
          // Repeats for the first few heartbeats - the very first one
          // (and anything else logged in the boot burst) gets dropped.
          static uint8_t audio_reports = 0;
          if (audio_reports < 4 && audio_probe_done) {
            audio_reports++;
            DEBUG_LOG("[Audio] ES8311 %s addr=0x%02X chipid=0x%02X/0x%02X\r\n",
                      audio_probe_found ? "found" : "NOT FOUND",
                      audio_probe_addr, audio_probe_id1, audio_probe_id2);
          }
        }
      #endif
      // One-shot path table size report, a few seconds into uptime - fires
      // through this function's own direct-Serial0 DEBUG_LOG call (not
      // RNS::logf()'s queued path, which drops lines under the boot-time
      // burst urns_init() itself produces).
      static bool path_table_boot_logged = false;
      if (!path_table_boot_logged) {
        path_table_boot_logged = true;
        DEBUG_LOG("[URNS] path table entries at boot: %u\r\n", (unsigned)RNS::Transport::new_path_table().size());
      }
    }
  #endif

  // Display.h's draw_lora_icon() (included before this file - see the
  // extern forward-declaration there) needs this to decide whether to
  // blink the new-message envelope over the LoRa status icon.
  bool messenger_has_unread() {
    if (!urns_message_store) return false;
    return urns_message_store->get_unread_count() > 0;
  }

  // Message count from the most recently *completed* sync (manual
  // MSNGR_TOP_ITEM_SYNC_PROP or periodic messenger_sync_process() below,
  // whichever finishes next) - set by the register_sync_complete_
  // callback() lambda in messenger_init() below, which LXMRouter always
  // calls with a real count right before flipping to PR_COMPLETE (never
  // on PR_FAILED - see on_message_get_response()/on_message_list_
  // response(), LXMRouter.cpp), so by the time anything polling get_sync_
  // state() sees PR_COMPLETE this is already correct for that same sync.
  // Menu.h's msngr_sync_popup_process() is the only reader today.
  // Declared ahead of messenger_init() (not next to messenger_sync_
  // process() further down, where it's otherwise topically closer) since
  // that function's own lambda needs to see it already declared.
  size_t msngr_sync_last_count = 0;

  void messenger_init() {
    // RNS::set_log_callback()/loglevel() now happen unconditionally in
    // urns_init() (URNS.h, HAS_URNS-gated) instead of here - this used to
    // be HAS_LXMF-only, which left HAS_URNS-but-not-HAS_LXMF boards
    // (MeshPoE-S3) with no log callback registered at all and RNS::doLog()
    // silently corrupting the main KISS Serial stream with raw log text
    // (see urns_rns_log_callback()'s own comment, URNS.h). urns_init()
    // always runs before messenger_init() (RNode_Firmware.ino setup()), so
    // this is already done by the time we get here.
    urns_message_store = new LXMF::MessageStore(URNS_BASE_PATH "/messages");
    // PIN/passphrase vault (phase (c), Vault.h) - only wired when the
    // vault is actually on, so a device that never opts in gets the
    // store's original zero-overhead plaintext behavior untouched. Safe
    // to check vault_enabled here (not vault_unlocked): setup() already
    // ran vault_unlock_boot_screen() to completion before urns_init() and
    // this messenger_init() call, so vault_enabled implies vault_unlocked
    // by this point (see RNode_Firmware.ino's setup() ordering).
    #if HAS_URNS == true
      if (vault_enabled) {
        urns_message_store->set_field_cipher(vault_encrypt_message_field, vault_decrypt_message_field);
      }
    #endif
    RNS::Transport::register_announce_handler(msngr_announce_handler);
    RNS::Transport::register_announce_handler(msngr_prop_node_announce_handler);
    messenger_bookmarks_load();
    messenger_presets_load();

    // Re-arm the active propagation node (if any) loaded from bookmarks.json -
    // LXMRouter itself starts with no outbound propagation node configured
    // every boot, this is the only thing that re-applies a prior Set Active.
    if (urns_lxmf_router && msngr_active_prop_node_hash.size() == LXMF::PEER_HASH_SIZE) {
      urns_lxmf_router->set_outbound_propagation_node(msngr_active_prop_node_hash);
    }

    // Feeds msngr_sync_last_count above - see its own comment for why
    // reading it once get_sync_state() reports PR_COMPLETE is safe.
    if (urns_lxmf_router) {
      urns_lxmf_router->register_sync_complete_callback([](size_t count) {
        msngr_sync_last_count = count;
      });
    }

    // ADDR_CONF_MSNGR_RETRIES (ROM.h) - raw physical byte, not through
    // eeprom_addr(), same "out-of-range/erased (0xFF) keeps the compiled
    // default" convention as ADDR_CONF_GNSS_INTERVAL's own boot-time load
    // (RNode_Firmware.ino). Self-contained here (rather than in that big
    // early-boot EEPROM block) since nothing needs msngr_max_retries
    // before this point - messenger_init() always runs after urns_init(),
    // so urns_lxmf_router already exists to push it into.
    uint8_t retries_raw = EEPROM.read(ADDR_CONF_MSNGR_RETRIES);
    if (retries_raw <= 5) msngr_max_retries = retries_raw;
    if (urns_lxmf_router) urns_lxmf_router->set_max_delivery_attempts(msngr_max_retries);

    // ADDR_CONF_MSNGR_RETRY_DELAY (ROM.h) - out-of-range/erased (0xFF, or
    // anything outside the menu's 1-60 range) keeps the compiled default.
    uint8_t retry_delay_raw = EEPROM.read(ADDR_CONF_MSNGR_RETRY_DELAY);
    if (retry_delay_raw >= 1 && retry_delay_raw <= 60) msngr_retry_delay_s = retry_delay_raw;
    if (urns_lxmf_router) urns_lxmf_router->set_outbound_retry_delay((double)msngr_retry_delay_s);

    #if HAS_AUDIO == true
      // ADDR_CONF_MSNGR_PLAYBACK_VOLUME (ROM.h) - erased/out-of-range keeps
      // the compiled default.
      uint8_t pbvol_raw = EEPROM.read(ADDR_CONF_MSNGR_PLAYBACK_VOLUME);
      if (pbvol_raw >= 10 && pbvol_raw <= 100 && pbvol_raw % 10 == 0) msngr_playback_volume_pct = pbvol_raw;
    #endif

    // ADDR_CONF_MSNGR_ANNOUNCE_AT_START (ROM.h) - only ENABLE_BYTE/
    // DISABLE_BYTE are valid, so anything else (including erased 0xFF)
    // keeps the compiled default (true) rather than the usual "0/absent
    // means off" convention - see that address's own comment for why.
    uint8_t announce_at_start_raw = EEPROM.read(ADDR_CONF_MSNGR_ANNOUNCE_AT_START);
    if (announce_at_start_raw == MSNGR_ANNOUNCE_AT_START_DISABLE_BYTE) msngr_announce_at_start = false;
    else if (announce_at_start_raw == MSNGR_ANNOUNCE_AT_START_ENABLE_BYTE) msngr_announce_at_start = true;

    // ADDR_CONF_MSNGR_ANNOUNCE_INTERVAL (ROM.h) - same "out-of-range/
    // erased keeps compiled default (0/Off)" shape as Retries above.
    uint8_t announce_interval_raw = EEPROM.read(ADDR_CONF_MSNGR_ANNOUNCE_INTERVAL);
    if (announce_interval_raw < MSNGR_ANNOUNCE_INTERVAL_PRESET_COUNT) msngr_announce_interval_idx = announce_interval_raw;
    if (urns_lxmf_router) urns_lxmf_router->set_announce_interval(msngr_announce_interval_presets_s[msngr_announce_interval_idx]);

    // ADDR_CONF_MSNGR_PROP_ON_FAIL (ROM.h) - only ENABLE_BYTE/DISABLE_BYTE
    // are valid, anything else (including erased 0xFF) keeps the compiled
    // default (false).
    uint8_t prop_on_fail_raw = EEPROM.read(ADDR_CONF_MSNGR_PROP_ON_FAIL);
    if (prop_on_fail_raw == MSNGR_PROP_ON_FAIL_ENABLE_BYTE) msngr_propagate_on_fail = true;
    else if (prop_on_fail_raw == MSNGR_PROP_ON_FAIL_DISABLE_BYTE) msngr_propagate_on_fail = false;
    if (urns_lxmf_router) urns_lxmf_router->set_fallback_to_propagation(msngr_propagate_on_fail);

    // ADDR_CONF_MSNGR_SYNC_INTERVAL (ROM.h) - same "out-of-range/erased
    // keeps compiled default (0/Off)" shape as Announce Interval above.
    uint8_t sync_interval_raw = EEPROM.read(ADDR_CONF_MSNGR_SYNC_INTERVAL);
    if (sync_interval_raw < MSNGR_SYNC_INTERVAL_PRESET_COUNT) msngr_sync_interval_idx = sync_interval_raw;

    // ADDR_CONF_MSNGR_SYNC_LIMIT (ROM.h) - out-of-range/erased (255) keeps
    // the compiled default (8); 0 (unlimited) is a valid, distinct value.
    uint8_t sync_limit_raw = EEPROM.read(ADDR_CONF_MSNGR_SYNC_LIMIT);
    if (sync_limit_raw <= MSNGR_SYNC_LIMIT_MAX) msngr_sync_limit = sync_limit_raw;
    if (urns_lxmf_router) urns_lxmf_router->set_sync_message_limit(msngr_sync_limit);

    // ADDR_CONF_MSNGR_STAMP_COST (ROM.h) - out-of-range/erased (255) keeps
    // the compiled default (0/disabled), same shape as Sync Limit above.
    uint8_t stamp_cost_raw = EEPROM.read(ADDR_CONF_MSNGR_STAMP_COST);
    if (stamp_cost_raw <= MSNGR_STAMP_COST_MAX) msngr_stamp_cost = stamp_cost_raw;
    if (urns_lxmf_router) {
      urns_lxmf_router->set_stamp_cost(msngr_stamp_cost);
      if (msngr_stamp_cost > 0) urns_lxmf_router->enforce_stamps();
      else urns_lxmf_router->ignore_stamps();
    }

    DEBUG_LOG("[Messenger] ready, %u bookmark(s) loaded, max_retries=%u, retry_delay_s=%u, announce_at_start=%u, announce_interval_idx=%u, propagate_on_fail=%u, sync_interval_idx=%u, sync_limit=%u, stamp_cost=%u\r\n",
      (unsigned)msngr_bookmark_count, (unsigned)msngr_max_retries, (unsigned)msngr_retry_delay_s, (unsigned)msngr_announce_at_start, (unsigned)msngr_announce_interval_idx,
      (unsigned)msngr_propagate_on_fail, (unsigned)msngr_sync_interval_idx, (unsigned)msngr_sync_limit, (unsigned)msngr_stamp_cost);
  }

  #if HAS_LXMF == true
    // Bulk re-save every stored message through save_message(), forward-
    // declared for VaultUnlock.h's vault_enroll_flow() (see that header's
    // own comment on why the forward declaration is needed) - called
    // right after vault_enable() commits, while this session's vault_key
    // is already resident, so the user isn't asked to re-enter the PIN a
    // third time just to encrypt history that already exists.
    //
    // Walking + re-saving through the normal save_message() path (rather
    // than a bespoke bulk-rewrite) reuses its already-tested two-phase
    // .tmp/.bak commit for each individual file, so a power loss mid-walk
    // leaves every file it touches in a consistent state (either still
    // the old plaintext generation, or the new encrypted one) - it does
    // NOT make the whole migration atomic across the store as a unit. A
    // power loss partway through leaves some messages encrypted and some
    // still plaintext; simply re-running this (e.g. by re-toggling PIN
    // Protection) finishes the job - re-encrypting an already-encrypted
    // message is a safe, idempotent-in-effect re-save (produces a new
    // envelope under the same subkey), not a hazard.
    void vault_migrate_messages_encrypt() {
      if (!urns_message_store) return;
      urns_message_store->set_field_cipher(vault_encrypt_message_field, vault_decrypt_message_field);
      for (const RNS::Bytes& peer_hash : urns_message_store->get_conversations()) {
        for (const RNS::Bytes& msg_hash : urns_message_store->get_messages_for_conversation(peer_hash)) {
          LXMF::LXMessage msg = urns_message_store->load_message(msg_hash);
          if (msg.hash().size() == 0) continue; // failed to load - skip, don't lose the rest of the walk over one bad file
          urns_message_store->save_message(msg);
        }
      }
    }

    // Mirror of the above for vault_disable_flow() - decrypts every
    // stored message back to plaintext. The field-decrypt hook stays
    // wired throughout the walk (existing files still need it to load)
    // but the encrypt side is cleared first, forcing save_message() to
    // write the now-plaintext branch instead of re-encrypting - see
    // MessageStore.cpp's save_message()/set_field_cipher() for why a
    // falsy _field_encrypt alone is what selects that branch. Both hooks
    // are cleared once the walk finishes, so anything saved afterward
    // (this session, with the vault now off) is plaintext with no cipher
    // call at all, matching a device that never opted in.
    void vault_migrate_messages_decrypt() {
      if (!urns_message_store) return;
      urns_message_store->set_field_cipher(nullptr, vault_decrypt_message_field);
      for (const RNS::Bytes& peer_hash : urns_message_store->get_conversations()) {
        for (const RNS::Bytes& msg_hash : urns_message_store->get_messages_for_conversation(peer_hash)) {
          LXMF::LXMessage msg = urns_message_store->load_message(msg_hash);
          if (msg.hash().size() == 0) continue;
          urns_message_store->save_message(msg);
        }
      }
      urns_message_store->set_field_cipher(nullptr, nullptr);
    }
  #endif

  // Periodic propagation-node sync (msngr_sync_interval_idx, Off by
  // default) - polled from loop() (URNS.h, right after process_sync()
  // itself) same shape as LXMRouter::process_outbound()'s own periodic
  // announce check. Only fires when a node is actually active and no
  // sync is already in flight - request_messages_from_propagation_node()
  // itself is a no-op (logs+PR_FAILED) with no active node, but skipping
  // here avoids spamming that warning every tick once idle.
  unsigned long msngr_sync_last_ms = 0;

  void messenger_sync_process() {
    if (msngr_sync_interval_idx == 0) return; // Off
    if (msngr_active_prop_node_hash.size() != LXMF::PEER_HASH_SIZE) return; // no active node
    if (!urns_lxmf_router) return;

    LXMF::LXMRouter::PropagationSyncState state = urns_lxmf_router->get_sync_state();
    if (state != LXMF::LXMRouter::PR_IDLE && state != LXMF::LXMRouter::PR_COMPLETE && state != LXMF::LXMRouter::PR_FAILED) {
      return; // sync already in flight
    }

    unsigned long now = millis();
    unsigned long interval_ms = msngr_sync_interval_presets_s[msngr_sync_interval_idx] * 1000UL;
    if (now - msngr_sync_last_ms < interval_ms) return;
    msngr_sync_last_ms = now;

    urns_lxmf_router->request_messages_from_propagation_node();
  }

#endif
