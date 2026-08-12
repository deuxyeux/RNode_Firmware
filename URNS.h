// Onboard microReticulum node (Tier 1: identity + persistence only, no
// radio interface wired in yet). Experimental, branch-only feature - see
// HAS_URNS's own comment (Boards.h) for scope.
//
// Runs on a dedicated LittleFS-formatted data partition ("urns", see
// partitions/meshadventurer_s3_urns.csv) kept deliberately separate from
// the "spiffs"-labelled partition Console.h already owns (real SPIFFS
// format, web UI assets) - mounting the wrong one here would silently
// reformat it out from under Console.h.
//
// microStore's own PosixFileSystem::init() (its ESP32 branch) hardcodes
// LittleFS.begin(true, _basepath) with no partition-label argument, which
// defaults to partition label "spiffs" - i.e. Console's partition, not
// ours. So LittleFS is mounted here directly, by label, before microStore
// ever touches it, and PosixFileSystem::init() itself is never called -
// only its already-POSIX-correct open()/exists()/etc, which operate on
// whatever path string they're given and don't care who mounted it.

#include <LittleFS.h>
#include <microStore/FileSystem.h>
#include <microStore/Adapters/UniversalFileSystem.h>
#include "esp_mac.h"

// Config.h's own #define MTU 508 is a raw, unnamespaced macro that
// textually clobbers RNS::Type::Reticulum::MTU inside microReticulum's own
// headers - undefine it for just this include, then restore it. (A plain
// #define-based save/restore is circular here - MTU would expand to a
// saved-name token that itself expands back to MTU, which the
// preprocessor's self-reference guard then refuses to expand further -
// push_macro/pop_macro sidesteps that by saving the actual definition.)
#pragma push_macro("MTU")
#undef MTU
#include <microReticulum.h>
#include <LXMF/LXMRouter.h>
#pragma pop_macro("MTU")

#define URNS_BASE_PATH         "/urns"
#define URNS_PARTITION_LABEL   "urns"
#define URNS_IDENTITY_PATH     "/urns/identity"
#define URNS_DISPLAY_NAME_PATH "/urns/display_name"

// Fixed-footprint persistence stores that live on the urns partition
// alongside the identity file and LXMF MessageStore, used by the URNS >
// Free breakdown screen (Menu.h) - see project_urns_partition_growth
// memory for how these were discovered (both are BasicFileStore ring
// buffers of real segment files, sized in Identity.cpp/Transport.cpp):
//   - known_store: RNS::Identity's announce/peer cache, max 256KB
//     (RNS_KNOWN_DESTINATIONS_SEGMENT_SIZE * _COUNT)
//   - path_store: RNS::Transport's persisted path table, max 512KB
//     (RNS_PATH_TABLE_SEGMENT_SIZE * _COUNT)
// Both prefixes are built the same way Transport.cpp builds them
// internally (off Reticulum::_storagepath, which urns_init() sets to
// URNS_BASE_PATH) - kept as separate #defines here rather than reaching
// into Transport/Identity internals from a menu screen.
#define URNS_KNOWN_STORE_PATH  "/urns/known_store"
#define URNS_PATH_STORE_PATH   "/urns/path_store"
#define URNS_MESSAGES_PATH     "/urns/messages"

// Defined in RNode_Firmware.ino, after RTC.h's declarations are visible
// (RTC.h is #include'd from Utilities.h *after* URNS.h, so rtc_get_unixtime()
// etc. aren't callable directly from here). Seeds RNS::Utilities::OS's own
// millis()-based clock (OS::ltime()/OS::time(), which starts at 0 every
// boot with no automatic real-time source of its own - LXMF message
// timestamps come from this) from our onboard RTC, if present and valid -
// without this, every LXMF message reports 1970-01-01.
void urns_sync_time_from_rtc();

// Defined in RNode_Firmware.ino, after packet_queue/queue_cursor/etc are
// declared. Feeds a locally-originated packet into the exact same
// packet_queue/packet_starts/packet_lengths FIFO the KISS host path uses,
// so tx_queue_handler()'s existing CSMA/DIFS gate arbitrates both TX
// sources identically - no separate contention logic needed. Returns
// false (and drops the packet) if a host frame is currently mid-assembly
// (IN_FRAME) - writing into the shared circular buffer at that point
// would corrupt the host's in-progress frame.
bool urns_enqueue_outgoing(const uint8_t* data, uint16_t len);

// Defined in Messenger.h (included right after this file) - saves the
// message to urns_message_store, resolves/caches the sender's display
// name, and fires the inbound alert chirp. Forward-declared here so
// urns_init()'s delivery callback (below) can reach it despite Messenger.h
// not existing yet at this point in the include chain - same pattern as
// urns_sync_time_from_rtc()/urns_enqueue_outgoing() above.
void messenger_on_delivery(LXMF::LXMessage& msg);

// Outbound-proof-of-delivery counterpart - fires when static_proof_callback
// (lib/microLXMF's LXMRouter.cpp) confirms delivery of a message THIS
// device sent, not one it received. Forward-declared for the same reason
// as messenger_on_delivery() above.
void messenger_on_delivered(LXMF::LXMessage& msg);

// RX-side counterpart to urns_enqueue_outgoing() above - kiss_write_packet()
// (RNode_Firmware.ino) calls this instead of urns_lora_interface.handle_
// incoming() directly. It used to call handle_incoming() right there,
// synchronously - but kiss_write_packet() itself runs deep inside the
// radio driver's DIO0 GPIO interrupt handler (sx126x::handleDio0Rise() ->
// _onReceive -> receive_callback() -> kiss_write_packet(), none of it
// IRAM/ISR-safe beyond the driver's own tiny entry points), and
// handle_incoming() fans out into the *entire* Transport/LXMF/MessageStore
// stack (path table updates, announce dispatch, LXMF delivery, LittleFS
// writes) - none of which is safe to run from interrupt context. Real,
// reproduced-on-hardware symptom: if that processing (or, separately, a
// received packet's own interrupt) lands while the main loop task is
// mid-SPI-transaction on the *same* radio (e.g. endPacket()'s TX-done
// polling loop, sx126x.cpp), the two contend for Arduino-ESP32's SPI bus
// mutex - which is unconditionally a blocking, ISR-unaware
// xSemaphoreTake(..., portMAX_DELAY) with zero special-casing for being
// called from an interrupt. Confirmed to sometimes manifest as a hard
// crash (heap-poisoning assert, tlsf/multi_heap), and independently as a
// ~20s stall (LORA_MODEM_TIMEOUT_MS, sx126x.h) where a corrupted status
// read makes TX_DONE look like it never arrived even though the radio
// actually finished.
//
// Fix: do the absolute minimum here (a plain memcpy into our own buffer,
// no SPI, no heap, no Transport/LXMF code - just as ISR-unsafe-free as
// the pre-existing pbuf/host_write_len KISS-forwarding path this mirrors)
// and defer the real handle_incoming() call to urns_lxmf_loop(), which
// only ever runs from the main loop task (see its own comment). Single-
// slot, drop-if-still-pending - loop() reliably drains it many times
// faster than the radio could receive a second packet (a full LoRa
// symbol's airtime, let alone a whole packet, dwarfs one loop() lap), so
// this isn't a real capacity concern in practice, same reasoning as
// pbuf/host_write_len's own single-buffer reuse.
uint8_t urns_rx_staging_buf[MTU];
volatile uint16_t urns_rx_staging_len = 0;
volatile bool urns_rx_pending = false;

void urns_stage_incoming(const uint8_t* data, uint16_t len) {
  if (urns_rx_pending) return; // previous packet not drained yet - drop rather than corrupt it
  if (len > MTU) len = MTU;
  memcpy(urns_rx_staging_buf, data, len);
  urns_rx_staging_len = len;
  urns_rx_pending = true;
}

// Thin adapter: no radio logic of its own, just enqueues outgoing bytes
// onto the shared TX queue above. RX is fed the other way - see the
// urns_stage_incoming() call site in kiss_write_packet() (RNode_Firmware.ino).
class UrnsLoRaInterface : public RNS::InterfaceImpl {
public:
  UrnsLoRaInterface(const char* name = "UrnsLoRaInterface") : RNS::InterfaceImpl(name) {
    _IN = true;
    _OUT = true;
  }
  virtual ~UrnsLoRaInterface() {}

private:
  virtual bool send_outgoing(const RNS::Bytes& data) override {
    // TEMPORARY instrumentation - tracking down "reports SENT but no
    // radio activity" (project_microreticulum_onboard_node memory).
    DEBUG_LOG("[URNS] send_outgoing: called, size=%u\r\n", (unsigned)data.size());
    bool ok = urns_enqueue_outgoing(data.data(), (uint16_t)data.size());
    DEBUG_LOG("[URNS] send_outgoing: urns_enqueue_outgoing returned %s\r\n", ok ? "true" : "false");
    return ok;
  }
};

// Logs every announce Transport processes, from any destination/aspect
// (nullptr filter) - not just ones addressed to urns_destination. This is
// the only way to observe RX at all right now; nothing else logs on a
// received announce.
class UrnsAnnounceHandler : public RNS::AnnounceHandler {
public:
  UrnsAnnounceHandler() : RNS::AnnounceHandler(nullptr) {}
  virtual ~UrnsAnnounceHandler() {}
  virtual void received_announce(const RNS::Bytes& destination_hash, const RNS::Identity& announced_identity, const RNS::Bytes& app_data) override {
    DEBUG_LOG("[URNS] RX announce from %s (identity %s)\r\n", destination_hash.toHex().c_str(), announced_identity ? announced_identity.hash().toHex().c_str() : "unknown");
  }
};

microStore::FileSystem urns_filesystem;
RNS::Reticulum urns_reticulum({RNS::Type::NONE});
RNS::Identity urns_identity({RNS::Type::NONE});
RNS::Interface urns_lora_interface({RNS::Type::NONE});
RNS::Destination urns_destination({RNS::Type::NONE});
RNS::HAnnounceHandler urns_announce_handler(new UrnsAnnounceHandler());
LXMF::LXMRouter::Ptr urns_lxmf_router;

bool urns_ready = false;

// Master switch (ADDR_CONF_URNS, ROM.h; RNode Settings > URNS > Enabled).
// Defaults true - the onboard node ran unconditionally before this toggle
// existed, so a never-touched EEPROM byte should keep matching that,
// same "leave the compiled default alone unless explicitly written"
// convention as gnss_enabled (GNSS.h). Only takes effect at boot -
// urns_init()/urns_radio_bringup() (RNode_Firmware.ino) are gated on it;
// there's no live start/stop path (matches ESP-NOW's Enabled field, not
// GNSS's live-toggle one), so flipping it in the menu requires a reboot.
bool urns_enabled = true;

// Whether this node participates in RNS transport (relays other nodes'
// traffic - RNS::Reticulum::transport_enabled(), called from urns_init()
// below). Same "read once at boot, no live start/stop path" convention
// as urns_enabled above (ADDR_CONF_URNS_TRANSPORT, ROM.h; RNode Settings
// > URNS > Transport Mode). Defaults OFF - see that #define's own
// comment for why.
bool urns_transport_enabled = false;

#define URNS_LXMF_SEND_OK          0
#define URNS_LXMF_SEND_NOT_READY   1
#define URNS_LXMF_SEND_NO_IDENTITY 2

inline const char* urns_lxmf_send_result_text(uint8_t result) {
  switch (result) {
    case URNS_LXMF_SEND_OK:          return "SENT";
    case URNS_LXMF_SEND_NOT_READY:   return "NOT READY";
    case URNS_LXMF_SEND_NO_IDENTITY: return "UNKNOWN DEST";
    default:                         return "ERROR";
  }
}

// Same MD5-of-BT-MAC scheme bt_devname (Bluetooth.h) already uses for its
// "RNode XXXX" Bluetooth name - MD5 hash of the 6 raw BT MAC bytes, last 2
// hash bytes as hex. This is also the exact 4-digit ID shown in the boot
// banner carousel (Display.h, bt_dh[14]/bt_dh[15]) - reusing the algorithm
// here (rather than the already-computed bt_dh global) means the LXMF
// display name matches what's printed on the device's own screen even if
// Bluetooth is disabled and bt_setup_hw() never ran.
std::string urns_compute_default_display_name() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_BT);
  unsigned char* hash = MD5::make_hash((char*)mac, sizeof(mac));
  char name[16];
  snprintf(name, sizeof(name), "RNode %02X%02X", hash[14], hash[15]);
  free(hash);
  return std::string(name);
}

// Overrideable via a plain text file on the urns LittleFS partition
// (URNS-managed storage, per the design decision - independent of EEPROM's
// fixed-field conventions). No menu text-entry UI writes this yet; falls
// back to the MAC-derived default whenever the file is absent or empty.
std::string urns_lxmf_display_name() {
  RNS::Bytes stored;
  if (RNS::Utilities::OS::read_file(URNS_DISPLAY_NAME_PATH, stored) > 0) {
    std::string name = stored.toString();
    while (!name.empty() && (name.back() == '\n' || name.back() == '\r')) name.pop_back();
    if (!name.empty()) return name;
  }
  return urns_compute_default_display_name();
}

// Sums file sizes under `path`, recursing into subdirectories (used by
// e.g. "/urns/messages", which has "m/"+"c/<peer>/" subdirs - see
// MessageStore.cpp's initialize_storage()). Bounded by MessageStore's own
// caps (RETAINED_MESSAGES_PER_CONVERSATION/MAX_TOTAL_RETAINED_MESSAGES,
// MessageStore.h) plus a handful of fixed segment files elsewhere, so this
// is a few dozen to ~150 cheap directory-entry reads worst case - fine to
// call once when a menu screen is entered, but NEVER from a draw/redraw
// path (draw_settings_menu_disp() runs every loop() iteration - an
// unthrottled expensive call there is exactly what crashed the "Free" row
// itself before it got a cache, see project_urns_partition_growth memory).
size_t urns_dir_size_recursive(const char *path) {
  // Arduino's LittleFS wrapper (VFSImpl::open(), vfs_api.cpp) prepends its
  // own mountpoint ("/urns", set via LittleFS.begin()'s basePath arg) to
  // whatever path it's given - callers pass paths relative to that root,
  // not the absolute "/urns/..." form RNS::Utilities::OS/microStore use
  // elsewhere in this file. Strip it if present so this can still be
  // called with the natural URNS_*_PATH macros (all absolute) without
  // double-prefixing into a nonexistent "/urns/urns/..." path - which is
  // exactly what silently made every bucket read 0.0KB before this fix,
  // since LittleFS.open() just returns an invalid File on a miss rather
  // than erroring loudly. Recursive calls pass entry.path(), which the
  // same wrapper already hands back relative (no "/urns" prefix) - the
  // strncmp below is a no-op for those.
  const char *rel_path = path;
  size_t base_len = strlen(URNS_BASE_PATH);
  if (strncmp(path, URNS_BASE_PATH, base_len) == 0) rel_path = path + base_len;

  File f = LittleFS.open(rel_path);
  if (!f) return 0;
  if (!f.isDirectory()) {
    size_t sz = f.size();
    f.close();
    return sz;
  }
  size_t total = 0;
  File entry = f.openNextFile();
  while (entry) {
    if (entry.isDirectory()) {
      total += urns_dir_size_recursive(entry.path());
    } else {
      total += entry.size();
    }
    entry.close();
    entry = f.openNextFile();
  }
  f.close();
  return total;
}

void urns_init() {
  DEBUG_LOG("[URNS] step 1: mounting LittleFS\r\n");
  if (!LittleFS.begin(true, URNS_BASE_PATH, 10, URNS_PARTITION_LABEL)) {
    DEBUG_LOG("[URNS] Failed to mount partition, onboard node disabled\r\n");
    return;
  }
  DEBUG_LOG("[URNS] step 2: mounted, constructing filesystem adapter\r\n");

  urns_filesystem = microStore::Adapters::UniversalFileSystem(URNS_BASE_PATH);
  DEBUG_LOG("[URNS] step 3: registering filesystem\r\n");
  RNS::Utilities::OS::register_filesystem(urns_filesystem);

  DEBUG_LOG("[URNS] step 4: constructing Reticulum\r\n");
  urns_reticulum = RNS::Reticulum();
  // Must come AFTER constructing Reticulum, not before - its constructor
  // unconditionally does strncpy(_storagepath, ".", ...) (Reticulum.cpp,
  // an apparent "CBA TEST" leftover, not the commented-out python-style
  // line right above it), clobbering whatever storagepath() was set to
  // beforehand. Confirmed via live Serial0 instrumentation after a long
  // chase down microStore's own file-open path (this was the actual root
  // cause of the recall()/remember() failures, not the transport_enabled
  // gate or the relative-vs-absolute prefix - both real, both fixed, but
  // neither mattered until this was too, since Identity::_known_store's
  // "/known_store/" ends up hung off _storagepath either way, and it was
  // always "." here). See project_microreticulum_onboard_node memory for
  // the full trace.
  DEBUG_LOG("[URNS] step 5: setting storagepath\r\n");
  RNS::Reticulum::storagepath(URNS_BASE_PATH);
  DEBUG_LOG("[URNS] step 6: transport_enabled(%d)\r\n", (int)urns_transport_enabled);
  urns_reticulum.transport_enabled(urns_transport_enabled);
  // Enable RNS's remote-management wiring (Transport::start(), triggered
  // by reticulum.start() just below) so Provisioning::Provisioner
  // (provisioning_init(), called from setup() right after urns_init()
  // returns) is reachable over a Link, not just local KISS. Safe to turn
  // on unconditionally: Transport::remote_management_allowed() (the
  // ALLOW_LIST gate on the "remote.management" destination) defaults to
  // an empty set, so no remote peer can actually invoke /provision until
  // the operator adds one - and that allow-list is itself only settable
  // via local KISS provisioning first (CMD_PROVISION_REQ, Provisioning.h),
  // the same bootstrap order microReticulum_Firmware's web console uses.
  RNS::Reticulum::remote_management_enabled(true);
  DEBUG_LOG("[URNS] step 7: reticulum.start()\r\n");
  urns_reticulum.start();
  DEBUG_LOG("[URNS] step 8: reticulum started\r\n");

  urns_sync_time_from_rtc();

  if (RNS::Utilities::OS::file_exists(URNS_IDENTITY_PATH)) {
    DEBUG_LOG("[URNS] step 9: loading persisted identity\r\n");
    urns_identity = RNS::Identity::from_file(URNS_IDENTITY_PATH);
    DEBUG_LOG("[URNS] step 10: loaded persisted identity\r\n");
  } else {
    DEBUG_LOG("[URNS] step 9: generating new identity\r\n");
    urns_identity = RNS::Identity();
    DEBUG_LOG("[URNS] step 10: saving new identity\r\n");
    urns_identity.to_file(URNS_IDENTITY_PATH);
    DEBUG_LOG("[URNS] step 11: saved new identity\r\n");
  }

  DEBUG_LOG("[URNS] step 12: registering LoRa interface\r\n");
  urns_lora_interface = new UrnsLoRaInterface("UrnsLoRaInterface");
  urns_lora_interface.mode(RNS::Type::Interface::MODE_FULL);
  RNS::Transport::register_interface(urns_lora_interface);
  urns_lora_interface.start();

  DEBUG_LOG("[URNS] step 13: creating destination\r\n");
  urns_destination = RNS::Destination(urns_identity, RNS::Type::Destination::IN, RNS::Type::Destination::SINGLE, "rnode", "onboard");
  urns_destination.set_proof_strategy(RNS::Type::Destination::PROVE_ALL);

  DEBUG_LOG("[URNS] step 14: registering announce handler\r\n");
  RNS::Transport::register_announce_handler(urns_announce_handler);

  DEBUG_LOG("[URNS] step 15: constructing LXMF router\r\n");
  // No MessageStore yet (Phase 1: hardcoded test destination, no
  // conversation/destination list) - LXMRouter doesn't need one, it's a
  // fully separate opt-in component (confirmed by reading LXMRouter.cpp -
  // it never references MessageStore internally).
  urns_lxmf_router = std::make_shared<LXMF::LXMRouter>(urns_identity, URNS_BASE_PATH "/lxmf", false);
  urns_lxmf_router->register_delivery_callback([](LXMF::LXMessage& msg) {
    DEBUG_LOG("[URNS] LXMF RX from %s: %s\r\n", msg.source_hash().toHex().c_str(), msg.content().toString().c_str());
    messenger_on_delivery(msg);
  });
  urns_lxmf_router->register_delivered_callback([](LXMF::LXMessage& msg) {
    DEBUG_LOG("[URNS] LXMF delivery proof received for %s\r\n", msg.hash().toHex().c_str());
    messenger_on_delivered(msg);
  });

  std::string display_name = urns_lxmf_display_name();
  urns_lxmf_router->set_display_name(display_name);
  DEBUG_LOG("[URNS] step 15b: LXMF display name set to \"%s\"\r\n", display_name.c_str());

  urns_ready = true;
  DEBUG_LOG("[URNS] step 16: ready, identity hash: %s\r\n", urns_identity.hash().toHex().c_str());
}

// Reclaims space from expired path/announce entries - see Transport::
// cull_new_path_table()/Identity::cull_known_destinations()'s own
// comments (Transport.h/Identity.h) for why this doesn't happen on its
// own: BasicFileStore::compact() (FileStore.h) already correctly skips
// is_ttl_expired() records when it runs, but it only ever runs
// automatically as a last resort (every segment full, a write needs
// room) - not proactively. Without this, entries the Path Table menu
// shows as "Expired" (Menu.h) just sit there forever instead of ever
// being reclaimed. Named urns_cull_stores() (not "housekeep") to match
// ~/Development/microReticulum_Firmware's cull_path_table()/
// cull_known_destinations() naming convention for this kind of
// operation. Called from urns_lxmf_loop() (below), inside its existing
// DIO0 mask - compaction is real flash I/O, same hazard class as
// everything else masked there - and self-throttles to once an hour -
// PATHFINDER_E (Type.h) is a full week, so anything more frequent would
// just be wasted I/O for no benefit.
#define URNS_CULL_INTERVAL_MS (60UL * 60UL * 1000UL)
void urns_cull_stores() {
  static unsigned long last_cull_ms = 0;
  unsigned long now_ms = millis();
  if (last_cull_ms != 0 && (now_ms - last_cull_ms) < URNS_CULL_INTERVAL_MS) return;
  last_cull_ms = now_ms;
  unsigned before_paths = (unsigned)RNS::Transport::new_path_table().size();
  bool path_ok = RNS::Transport::cull_new_path_table();
  unsigned after_paths = (unsigned)RNS::Transport::new_path_table().size();
  bool known_ok = RNS::Identity::cull_known_destinations();
  DEBUG_LOG("[URNS] cull_stores: path_ok=%d paths_before=%u paths_after=%u known_ok=%d\r\n",
            (int)path_ok, before_paths, after_paths, (int)known_ok);
}

// Serviced every loop() iteration once urns_ready - outbound queue
// draining (which itself goes through urns_enqueue_outgoing() same as
// everything else) and inbound message processing.
void urns_lxmf_loop() {
  if (!urns_ready) return;
  // Masked for this whole block, not just around Messenger.h's
  // MessageStore save (messenger_on_delivery(), which used to carry its
  // own narrower mask/unmask around just that call). handle_incoming()
  // fans out into Transport::inbound() - which validates and PERSISTS
  // announces (Identity::remember() -> known_store) and path table
  // updates (Transport::_path_store) synchronously, entirely outside
  // Messenger.h's guarded call - before process_inbound() ever reaches
  // the LXMF delivery callback those masks were protecting. Same root
  // hazard as feedback_dio0_isr_does_spi_work/feedback_sx126x_tx_rx_spi_
  // mutex_race: a DIO0 RX interrupt landing mid-flash-write tries SPI
  // from real ISR context, hard-aborting via FreeRTOS's own blocking-
  // semaphore assert - confirmed live as "received an announce and
  // immediately crashed" (project_urns_partition_growth memory's sibling
  // investigation), i.e. exactly the known_store write this masks now.
  // maskDio0()/unmaskDio0() aren't nesting-safe (a bare detachInterrupt/
  // attachInterrupt pair, not a counter) - unmasking partway through
  // (the old inner guard) would have re-armed the interrupt for whatever
  // of this block still had to run after it, so that inner guard is
  // removed now that this single outer one covers the whole pipeline.
  LoRa->maskDio0();
  // Drains urns_stage_incoming()'s single-slot staging buffer - the real
  // handle_incoming() call, deliberately moved here (main loop task) out
  // of the DIO0 ISR that used to call it directly. See that function's
  // own comment (above) for why.
  if (urns_rx_pending) {
    urns_lora_interface.handle_incoming(RNS::Bytes(urns_rx_staging_buf, urns_rx_staging_len));
    urns_rx_pending = false;
  }
  urns_lxmf_router->process_outbound();
  urns_lxmf_router->process_inbound();
  urns_cull_stores();
  LoRa->unmaskDio0();
}

// One-shot smoke test (Tier 2) - call once from RNode_Firmware.ino's
// loop() a few seconds after boot, not from urns_init() itself, so the
// announce actually goes out through tx_queue_handler()'s normal CSMA
// gate instead of racing radio bring-up.
void urns_announce() {
  if (!urns_ready) return;
  DEBUG_LOG("[URNS] Announcing destination %s\r\n", urns_destination.hash().toHex().c_str());
  urns_destination.announce(RNS::bytesFromString("URNS-TEST"));

  // Announces the LXMF delivery destination itself, carrying the display
  // name set above (LXMRouter::announce() builds app_data from
  // _display_name automatically when no explicit app_data is passed). The
  // router's own constructor was given announce_at_start=false, so without
  // this, peers can still receive messages from us (opportunistic delivery
  // only needs their identity, not the reverse) but never learn our name -
  // which is exactly the blank/hash-only name field originally reported.
  DEBUG_LOG("[URNS] Announcing LXMF delivery destination (display name: \"%s\")\r\n", urns_lxmf_router->display_name().c_str());
  urns_lxmf_router->announce();
}
