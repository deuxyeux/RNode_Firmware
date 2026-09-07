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

  #define MSNGR_PRESET_COUNT 3
  const char *const MSNGR_PRESETS[MSNGR_PRESET_COUNT] = { "Hi", "Bye", "SOS" };

  struct MessengerBookmark {
    bool in_use = false;
    uint8_t hash[LXMF::PEER_HASH_SIZE];
    char name[MSNGR_NAME_MAX_LEN + 1];
  };
  MessengerBookmark msngr_bookmarks[MSNGR_MAX_BOOKMARKS];
  uint8_t msngr_bookmark_count = 0;

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
    char snippet[24];
    bool incoming;
  };
  MessengerPeerMsgCacheRow msngr_peer_cache[MSNGR_PEER_MAX_MSG_ROWS];
  uint8_t msngr_peer_cache_count = 0;
  // Conversation's total message_count at the moment msngr_peer_cache was
  // last populated - lets messenger_refresh_peer_cache_if_stale() below
  // detect a new message arriving while MENU_STATE_MSNGR_PEER is sitting
  // open, without needing a flash read to do it.
  size_t msngr_peer_cache_message_count = 0;

  std::string msngr_msg_detail_cache_content;
  bool msngr_msg_detail_cache_incoming = false;
  bool msngr_msg_detail_cache_valid = false;

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
      snprintf(msngr_peer_cache[i].snippet, sizeof(msngr_peer_cache[i].snippet), "%s", meta.valid ? meta.content.c_str() : "?");
      msngr_peer_cache[i].incoming = meta.valid && meta.incoming;
    }
    msngr_peer_cache_count = n;
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
    msngr_msg_detail_cache_content = meta.valid ? meta.content : std::string("(unavailable)");
    msngr_msg_detail_cache_incoming = meta.valid && meta.incoming;
    msngr_msg_detail_cache_valid = true;
  }

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
    }
    std::string out;
    serializeJson(doc, out);
    RNS::Utilities::OS::write_file(MSNGR_BOOKMARKS_PATH, RNS::bytesFromString(out.c_str()));
  }

  void messenger_bookmarks_load() {
    for (uint8_t i = 0; i < MSNGR_MAX_BOOKMARKS; i++) msngr_bookmarks[i].in_use = false;
    msngr_bookmark_count = 0;

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
      msngr_bookmarks[i].in_use = true;
      i++;
    }
    msngr_bookmark_count = i;
  }

  bool messenger_bookmark_add(const RNS::Bytes &hash, const std::string &name) {
    if (messenger_bookmark_find(hash) >= 0) return true; // already bookmarked
    for (uint8_t i = 0; i < MSNGR_MAX_BOOKMARKS; i++) {
      if (msngr_bookmarks[i].in_use) continue;
      messenger_store_hash(msngr_bookmarks[i].hash, hash);
      messenger_store_name(msngr_bookmarks[i].name, name);
      msngr_bookmarks[i].in_use = true;
      msngr_bookmark_count++;
      messenger_bookmarks_save();
      return true;
    }
    return false; // pool full
  }

  void messenger_bookmark_remove(const RNS::Bytes &hash) {
    int8_t idx = messenger_bookmark_find(hash);
    if (idx < 0) return;
    msngr_bookmarks[idx].in_use = false;
    msngr_bookmark_count--;
    messenger_bookmarks_save();
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
    int8_t bm = messenger_bookmark_find(peer_hash);
    if (bm >= 0 && msngr_bookmarks[bm].name[0] != 0) return std::string(msngr_bookmarks[bm].name);
    if (urns_message_store) {
      std::string cached = urns_message_store->get_display_name(peer_hash);
      if (!cached.empty()) return cached;
    }
    int8_t an = messenger_announce_find(peer_hash);
    if (an >= 0 && msngr_announces[an].name[0] != 0) return std::string(msngr_announces[an].name);
    return peer_hash.toHex().substr(0, 16);
  }

  // Only ever sees announces from the delivery destinations of real LXMF
  // peers (aspect_filter "lxmf.delivery") - not the "rnode.onboard" Phase 1
  // test destination.
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
  // Generous - OPPORTUNISTIC delivery's proof has to travel from the
  // recipient back to us, potentially multiple LoRa hops each way, with
  // no guaranteed path warm already. PacketReceipt's own auto-computed
  // timeout (Packet::receipt_send(), Reticulum::get_first_hop_timeout())
  // fires independently of this - this is purely the UI's own "how long
  // will the Messenger screen wait before giving up on this specific
  // send", same reasoning as MSNGR_PING_PATH_TIMEOUT_MS/LINK_TIMEOUT_MS.
  #define MSNGR_SEND_DELIVERY_TIMEOUT_MS 60000
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
  #define MSNGR_SEND_RESULT_POPUP_MS 3000
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
  // Which method LXMessage::pack() actually resolved this send to
  // (LXMF::Type::Message::OPPORTUNISTIC or ::DIRECT) - read straight off
  // the local msg object in messenger_send_lxmf_resolved() right after
  // handle_outbound() returns, since handle_outbound() calls pack()
  // synchronously (LXMRouter.cpp) before queueing, so msg.method() is
  // already resolved by then even though the caller only ever *requested*
  // OPPORTUNISTIC - LXMRouter silently upgrades to DIRECT for anything
  // over LORA_ENCRYPTED_PACKET_MDU. Displayed on the MSNGR_SEND_PENDING
  // screen (Menu.h) so it's clear which path a given send actually took.
  uint8_t msngr_send_method = LXMF::Type::Message::OPPORTUNISTIC;
  unsigned long msngr_send_result_at_ms = 0; // set when state becomes DELIVERED/TIMEOUT/UNRESOLVED
  // Valid only while msngr_send_state == MSNGR_SEND_RESOLVING - the send
  // that's parked waiting for messenger_send_process() to find out whether
  // Identity::recall() ever comes good.
  RNS::Bytes msngr_send_pending_dest_hash;
  char msngr_send_pending_content[MSNGR_SEND_CONTENT_MAX_LEN + 1];
  unsigned long msngr_send_resolve_started_ms = 0;
  // Set by messenger_send_lxmf_resolved() whenever it actually saves a new
  // outgoing message - consumed by Menu.h's msngr_send_result_process()
  // (polled from loop() same as this file's own messenger_send_process())
  // to refresh the on-screen peer cache. A single mechanism for both the
  // immediate (identity already known) and deferred (post-RESOLVING) send
  // paths, since the latter completes from inside this file where Menu.h's
  // messenger_refresh_peer_cache() isn't visible yet (same layering split
  // as msngr_send_result_process() itself).
  bool msngr_send_needs_cache_refresh = false;

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
    // See messenger_on_delivery()'s own comment (this file) for why -
    // same flash-I/O-vs-DIO0-ISR hazard.
    LoRa->maskDio0();
    bool saved = urns_message_store->save_message(msg);
    LoRa->unmaskDio0();
    if (!saved) {
      DEBUG_LOG("[Messenger] delivered: save_message failed for %s\r\n", msg.hash().toHex().c_str());
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
  void messenger_send_lxmf_resolved(const RNS::Bytes &dest_hash, RNS::Identity &dest_identity, const char *content) {
    RNS::Destination dest(dest_identity, RNS::Type::Destination::OUT, RNS::Type::Destination::SINGLE, "lxmf", "delivery");
    LXMF::LXMessage msg(dest, urns_lxmf_router->delivery_destination(), RNS::bytesFromString(content),
      RNS::Bytes(), LXMF::Type::Message::OPPORTUNISTIC);
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
    msngr_send_state = MSNGR_SEND_PENDING;
    msngr_send_started_ms = millis();
    msngr_send_attempt = 1;

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
      if (millis() - msngr_send_started_ms > MSNGR_SEND_DELIVERY_TIMEOUT_MS) {
        msngr_send_state = MSNGR_SEND_TIMEOUT;
        msngr_send_result_at_ms = millis();
      }
    } else if (msngr_send_state == MSNGR_SEND_RESOLVING) {
      RNS::Identity dest_identity = RNS::Identity::recall(msngr_send_pending_dest_hash);
      if (dest_identity) {
        messenger_send_lxmf_resolved(msngr_send_pending_dest_hash, dest_identity, msngr_send_pending_content);
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
  uint8_t messenger_send_lxmf(const RNS::Bytes &dest_hash, const char *content) {
    // Let the confirm-click that triggered this Send finish playing
    // before the TX below can freeze it mid-note - see
    // buzzer_wait_for_melody()'s own comment (Utilities.h).
    #if HAS_BUZZER == true
      buzzer_wait_for_melody();
    #endif
    if (!urns_ready || !urns_message_store) return URNS_LXMF_SEND_NOT_READY;

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
      msngr_send_pending_dest_hash = dest_hash;
      strncpy(msngr_send_pending_content, content, MSNGR_SEND_CONTENT_MAX_LEN);
      msngr_send_pending_content[MSNGR_SEND_CONTENT_MAX_LEN] = 0;
      msngr_send_state = MSNGR_SEND_RESOLVING;
      msngr_send_resolve_started_ms = millis();
      return URNS_LXMF_SEND_RESOLVING;
    }

    messenger_send_lxmf_resolved(dest_hash, dest_identity, content);
    return URNS_LXMF_SEND_OK;
  }

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
  uint8_t msngr_ping_state = MSNGR_PING_IDLE;
  double msngr_ping_rtt = 0.0; // seconds, valid once msngr_ping_state == MSNGR_PING_SUCCESS
  unsigned long msngr_ping_phase_started_ms = 0;

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
    RNS::Destination dest(peer_identity, RNS::Type::Destination::OUT, RNS::Type::Destination::SINGLE, "lxmf", "delivery");
    msngr_ping_link = RNS::Link(dest, msngr_ping_link_established, msngr_ping_link_closed);
    msngr_ping_state = MSNGR_PING_ESTABLISHING;
    msngr_ping_phase_started_ms = millis();
  }

  // Kicks off a ping to dest_hash - called from Menu.h when the Ping
  // action is confirmed on MENU_STATE_MSNGR_PEER. Non-blocking: this only
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
  void messenger_ping_start(const RNS::Bytes &dest_hash) {
    msngr_ping_target_hash = dest_hash;
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
    RNS::Transport::register_announce_handler(msngr_announce_handler);
    messenger_bookmarks_load();

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

    DEBUG_LOG("[Messenger] ready, %u bookmark(s) loaded, max_retries=%u, retry_delay_s=%u, announce_at_start=%u, announce_interval_idx=%u\r\n",
      (unsigned)msngr_bookmark_count, (unsigned)msngr_max_retries, (unsigned)msngr_retry_delay_s, (unsigned)msngr_announce_at_start, (unsigned)msngr_announce_interval_idx);
  }

#endif
