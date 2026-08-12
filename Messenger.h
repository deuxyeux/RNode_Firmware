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

  // For messenger_rns_log_to_debug_uart()'s mutex - not reliably pulled in
  // transitively just via Arduino.h in this build (lib/microLXMF's own
  // LXStamper.cpp needs the same explicit includes for its own FreeRTOS
  // task/mutex use).
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
  // test destination URNS.h's own UrnsAnnounceHandler logs everything for.
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
    #if MCU_VARIANT == MCU_ESP32
      // User confirmed via a sustained automated stress test that heap
      // decline correlates with message-receive count, not just elapsed
      // time - this is the single call site every received message goes
      // through, so measure exactly how much each one costs and whether
      // it's recovered, instead of continuing to guess where in the
      // pipeline (save_message, app_data recall, LXMRouter's own link/
      // resource handling before this callback even runs) it's going.
      unsigned free_before = ESP.getFreeHeap();
    #endif
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

    #if MCU_VARIANT == MCU_ESP32
      unsigned free_after = ESP.getFreeHeap();
      DEBUG_LOG("[HeapDelta] on_delivery before=%u after=%u delta=%ld\r\n",
        free_before, free_after, (long)free_before - (long)free_after);
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
  #define MSNGR_SEND_IDLE      0
  #define MSNGR_SEND_PENDING   1 // enqueued, awaiting delivery proof
  #define MSNGR_SEND_DELIVERED 2
  #define MSNGR_SEND_TIMEOUT   3 // no proof within MSNGR_SEND_DELIVERY_TIMEOUT_MS
  // Generous - OPPORTUNISTIC delivery's proof has to travel from the
  // recipient back to us, potentially multiple LoRa hops each way, with
  // no guaranteed path warm already. PacketReceipt's own auto-computed
  // timeout (Packet::receipt_send(), Reticulum::get_first_hop_timeout())
  // fires independently of this - this is purely the UI's own "how long
  // will the Messenger screen wait before giving up on this specific
  // send", same reasoning as MSNGR_PING_PATH_TIMEOUT_MS/LINK_TIMEOUT_MS.
  #define MSNGR_SEND_DELIVERY_TIMEOUT_MS 60000
  // How long the terminal Delivered/No Confirmation result stays on
  // screen before auto-returning to the peer screen with no input needed -
  // same auto-dismiss convention as the Announce/GPS-Sync/NTP-Sync popups
  // (Menu.h's menu_popup_process()), just implemented locally here since
  // this is a dedicated live-status screen (MENU_STATE_MSNGR_SEND_RESULT),
  // not the generic MENU_STATE_STATUS_POPUP.
  #define MSNGR_SEND_RESULT_POPUP_MS 3000

  uint8_t msngr_send_state = MSNGR_SEND_IDLE;
  RNS::Bytes msngr_send_message_hash;
  unsigned long msngr_send_started_ms = 0;
  unsigned long msngr_send_result_at_ms = 0; // set when state becomes DELIVERED/TIMEOUT

  // Registered via LXMRouter::register_delivered_callback() (URNS.h) -
  // fires once for every outbound message this router gets delivery proof
  // for, not just the one the UI happens to be tracking, so this must
  // check the hash before touching UI state (a proof for some earlier,
  // already-timed-out send arriving late shouldn't resurrect/overwrite
  // whatever's currently being tracked).
  void messenger_on_delivered(LXMF::LXMessage &msg) {
    if (msngr_send_state == MSNGR_SEND_PENDING && msg.hash() == msngr_send_message_hash) {
      msngr_send_state = MSNGR_SEND_DELIVERED;
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
  void messenger_send_process() {
    if (msngr_send_state == MSNGR_SEND_PENDING) {
      if (millis() - msngr_send_started_ms > MSNGR_SEND_DELIVERY_TIMEOUT_MS) {
        msngr_send_state = MSNGR_SEND_TIMEOUT;
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
      DEBUG_LOG("[Messenger] send: no known identity for %s\r\n", dest_hash.toHex().c_str());
      return URNS_LXMF_SEND_NO_IDENTITY;
    }

    RNS::Destination dest(dest_identity, RNS::Type::Destination::OUT, RNS::Type::Destination::SINGLE, "lxmf", "delivery");
    LXMF::LXMessage msg(dest, urns_lxmf_router->delivery_destination(), RNS::bytesFromString(content),
      RNS::Bytes(), LXMF::Type::Message::OPPORTUNISTIC);
    urns_lxmf_router->handle_outbound(msg);
    // See messenger_on_delivery()'s own comment (this file) for why -
    // same flash-I/O-vs-DIO0-ISR hazard.
    LoRa->maskDio0();
    bool send_saved = urns_message_store->save_message(msg);
    LoRa->unmaskDio0();
    if (!send_saved) {
      DEBUG_LOG("[Messenger] send: save_message failed for %s\r\n", msg.hash().toHex().c_str());
    }

    // Start delivery-proof tracking - see its own declaration above for
    // why the UI can't just show "Sent" here: handle_outbound() only
    // enqueued/transmitted the packet, static_proof_callback() (LXMRouter.
    // cpp) fires later, asynchronously, if and when the recipient's proof
    // makes it back.
    msngr_send_message_hash = msg.hash();
    msngr_send_state = MSNGR_SEND_PENDING;
    msngr_send_started_ms = millis();

    DEBUG_LOG("[Messenger] send: queued message to %s\r\n", dest_hash.toHex().c_str());
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
  #define MSNGR_PING_NO_IDENTITY  5 // mirrors URNS_LXMF_SEND_NO_IDENTITY
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
  // starts path resolution (or the link itself, if a path's already
  // known) - messenger_ping_process() carries it the rest of the way, and
  // MENU_STATE_MSNGR_PING_RESULT's own draw code just reads
  // msngr_ping_state/msngr_ping_rtt fresh on every redraw.
  void messenger_ping_start(const RNS::Bytes &dest_hash) {
    msngr_ping_target_hash = dest_hash;
    msngr_ping_rtt = 0.0;
    msngr_ping_teardown_pending = false;
    msngr_ping_link = RNS::Link({RNS::Type::NONE});

    if (!urns_ready || !RNS::Identity::recall(dest_hash)) {
      msngr_ping_state = MSNGR_PING_NO_IDENTITY;
      return;
    }

    if (RNS::Transport::has_path(dest_hash)) {
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
      if (RNS::Transport::has_path(msngr_ping_target_hash)) {
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
    // RNS::doLog() (lib/microReticulum's Log.cpp) writes to the native USB
    // CDC `Serial` by default - the same port the binary KISS protocol
    // uses. On this board that meant RNS's own ERROR/INFO/DEBUG calls
    // (used throughout MessageStore.cpp, LXMRouter.cpp, Transport.cpp,
    // ...) were either invisible (no KISS host attached) or, worse, mixed
    // straight into the KISS byte stream a real host *was* reading (a
    // latent corruption risk, not just a visibility gap). Tee them to
    // Serial0 instead - the dedicated debug UART this board actually has
    // free (see HAS_DEBUG_UART, Boards.h) - via the log framework's own
    // callback hook. This is what actually surfaced the real
    // MessageStore-never-initializes bug (see remove_message_hash()'s own
    // comment, MessageStore.cpp) - keeping it registered permanently.
    //
    // This callback isn't only ever called from loop()'s own task:
    // LXStamper's async stamp-generation worker (lib/microLXMF's
    // LXStamper.cpp) runs on its own FreeRTOS task, explicitly
    // xTaskCreatePinnedToCore()'d to core 0 specifically so it doesn't
    // compete with loop() on core 1 - and generate_stamp() itself calls
    // INFO()/DEBUG() while it runs, landing here from that other core.
    // DEBUG_LOG() itself no longer touches Serial0 directly from the
    // caller's own task (see Utilities.h, debug_log_guarded()) - it just
    // queues the line for a dedicated consumer task to print, so no
    // extra guarding needed here regardless of which task calls this.
    void messenger_rns_log_to_debug_uart(const char *msg, RNS::LogLevel level) {
      DEBUG_LOG("[RNS] %s\r\n", msg);
    }

    // User-requested: a periodic Serial0 line (independent of anything
    // else logging) so a physical TX LED on the debug UART adapter keeps
    // blinking on a known cadence while the device is alive - the last
    // blink before it stops is the freeze moment, without needing a
    // laptop actively capturing when it happens. Free heap included since
    // it's already right here and a slow leak trending toward the freeze
    // would show up in this same trail.
    #define MSNGR_HEARTBEAT_INTERVAL_MS 5000
    unsigned long msngr_heartbeat_last_ms = 0;

    // g_loop_checkpoint (RNode_Firmware.ino) - see its own comment for why
    // this piggy-backs on the heartbeat's existing single-threaded Serial0
    // write instead of a separate concurrent printer.
    #if MCU_VARIANT == MCU_ESP32
      extern volatile const char* g_loop_checkpoint;
    #endif

    void messenger_heartbeat_process() {
      unsigned long now = millis();
      if (now - msngr_heartbeat_last_ms < MSNGR_HEARTBEAT_INTERVAL_MS) return;
      msngr_heartbeat_last_ms = now;
      #if MCU_VARIANT == MCU_ESP32
        // DEBUG_LOG() no longer writes to Serial0 from this task directly
        // (Utilities.h, debug_log_guarded()) - it queues the line for a
        // dedicated consumer task, so a hung Serial0 write can no longer
        // block loopTask here the way it was confirmed to do previously.
        // psram=X/Y confirms whether PSRAM is actually merged into the
        // general allocator (psramAddToHeap(), esp32-hal-misc.c) or just
        // sitting there unused - see this session's PSRAM investigation.
        // radio_online/dcd/airtime_lock/current_rssi/total_channel_util
        // (Config.h) - user reported the radio visibly stopped sending/
        // receiving for close to a minute *before* a crash, with no
        // hang symptom yet during that window (heartbeats kept printing
        // normally). Tracking these here to see whether the radio was
        // already reporting a stuck/bad state (e.g. permanently "channel
        // busy") in the heartbeats leading up to the next such crash,
        // rather than only learning about it after the fact from a
        // watchdog panic that doesn't capture any of this.
        DEBUG_LOG("[Heartbeat] alive, uptime=%lus heap=%u psram_free=%u psram_size=%u radio_online=%d dcd=%d airtime_lock=%d current_rssi=%d total_channel_util=%.2f loc=%s\r\n",
          now / 1000, (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getFreePsram(), (unsigned)ESP.getPsramSize(),
          (int)radio_online, (int)dcd, (int)airtime_lock, current_rssi, total_channel_util, g_loop_checkpoint);
      #else
        DEBUG_LOG("[Heartbeat] alive, uptime=%lus heap=%u\r\n", now / 1000, (unsigned)ESP.getFreeHeap());
      #endif
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
    #if HAS_DEBUG_UART == true
      RNS::set_log_callback(messenger_rns_log_to_debug_uart);
      // The default runtime level (Log.cpp: `LogLevel _level = LOG_TRACE;`)
      // is the single most verbose setting this framework has - every
      // packet, announce, and resource tick throughout Transport/
      // LXMRouter/MessageStore logs at INFO/VERBOSE/DEBUG/TRACE, and each
      // one is now a synchronous, blocking Serial0.printf() at 115200
      // baud (see messenger_rns_log_to_debug_uart()'s own comment) sitting
      // directly in loop()'s critical path. Confirmed on real hardware:
      // normal radio/mesh traffic under the full TRACE firehose made the
      // whole device sluggish - buttons/menu barely responsive, not
      // frozen outright but clearly starved. NOTICE keeps genuinely
      // actionable state changes (errors, warnings, established/closed
      // links, etc.) without the ordinary per-packet chatter. Bump this
      // back up temporarily (RNS::loglevel(RNS::LOG_TRACE) or similar) for
      // a future deep-debugging session - it's what found the
      // MessageStore-never-initializes bug - just don't leave it there.
      RNS::loglevel(RNS::LOG_NOTICE);
    #endif
    urns_message_store = new LXMF::MessageStore(URNS_BASE_PATH "/messages");
    RNS::Transport::register_announce_handler(msngr_announce_handler);
    messenger_bookmarks_load();
    DEBUG_LOG("[Messenger] ready, %u bookmark(s) loaded\r\n", (unsigned)msngr_bookmark_count);
  }

#endif
