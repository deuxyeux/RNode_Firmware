#pragma once

#include "LXMessage.h"
#include <microReticulum/Bytes.h>

#include <ArduinoJson.h>
#include <microStore/FileSystem.h>
#include <functional>
#include <string>
#include <vector>

namespace LXMF {

	// Fixed pool sizes to eliminate heap fragmentation. Overridable per-board
	// via build_flags (eg 2MB-PSRAM boards like heltec32v4pa_urns, where the
	// full 32/256 pool alone costs ~278KB of PSRAM). An earlier attempt at
	// shrinking this for heltec32v4pa_urns hit a reproducible TLSF
	// heap-corruption boot crash and was reverted; that turned out to be
	// stale build/sdkconfig-cache state, not a real bug in the shrink itself -
	// see project_urns_messagestore_psram_pool memory before touching this
	// again, in case it resurfaces.
	#ifndef LXMF_MAX_CONVERSATIONS
	#define LXMF_MAX_CONVERSATIONS 32
	#endif
	#ifndef LXMF_MAX_MESSAGES_PER_CONVERSATION
	#define LXMF_MAX_MESSAGES_PER_CONVERSATION 256
	#endif
	static constexpr size_t MAX_CONVERSATIONS = LXMF_MAX_CONVERSATIONS;
	static constexpr size_t MAX_MESSAGES_PER_CONVERSATION = LXMF_MAX_MESSAGES_PER_CONVERSATION;
	static constexpr size_t MESSAGE_HASH_SIZE = 32;  // SHA256 hash
	static constexpr size_t PEER_HASH_SIZE = 16;     // Truncated hash

	// Two-tier storage policy. The default `MessageStore` constructor
	// keeps everything on the main filesystem (the pre-tiered behavior),
	// which works fine until the partition fills up. Pyxis's LittleFS
	// partition is 1.875 MB and a sustained-receive soak can fill it
	// in ~30 min — at which point lfs_alloc panics with /0.
	//
	// To make the store sustainable, the consumer can supply a SECOND
	// filesystem via `set_archive_filesystem()` (eg microSD on T-Deck).
	// When set, save_message cull-walks each conversation after every
	// save: messages older than HOT_MESSAGES_PER_CONVERSATION are
	// MOVED (copy + delete) from the primary filesystem to the archive
	// filesystem. load_message tries primary first, then archive.
	//
	// The conversation's full hash list (up to MAX_MESSAGES_PER_CONVERSATION)
	// stays in the in-memory index either way, so scrollback past the
	// hot count just hits the archive. If the hash list itself fills,
	// add_message_hash evicts the oldest entirely (and deletes its
	// archive file too) — a hard cap.
	//
	// Without an archive filesystem set, cull just deletes — bounded
	// in-flash storage, but no historical scrollback.
	static constexpr size_t HOT_MESSAGES_PER_CONVERSATION = 50;

	// Hard retention limits, on top of (not instead of) the hot/archive
	// tiering above - HOT_MESSAGES_PER_CONVERSATION only culls *payload*
	// files while leaving the hash in the in-memory index forever (fine
	// when an archive filesystem can still serve scrollback, but this
	// board has none), so a long-running single conversation still grows
	// its index and eventually exhausts a small LittleFS partition (see
	// project_meshadventurer_s3_platformio_build / the "No more free
	// space" crashes it caused). These two limits do a *real* deletion -
	// hash removed from the index, payload file removed - not just a
	// hot/archive move:
	//   - RETAINED_MESSAGES_PER_CONVERSATION: at most this many messages
	//     kept per contact, oldest deleted first.
	//   - MAX_TOTAL_RETAINED_MESSAGES: at most this many messages kept in
	//     the whole store across all conversations - once at capacity,
	//     adding a new message evicts the oldest one somewhere else to
	//     make room, wrapping around rather than growing unbounded.
	// Enforced in save_message(), after every successful save - see
	// trim_conversation_to_retention()/trim_global_total().
	static constexpr size_t RETAINED_MESSAGES_PER_CONVERSATION = 50;
	// 250 (the originally requested total) doesn't fit the 512KB "urns"
	// LittleFS partition in practice - each message consumes a full 4KB
	// block regardless of its JSON payload's actual size, and the packed
	// message is stored hex-encoded (2x its raw size) on top of that. 100
	// leaves headroom for identity/path-table/index data alongside the
	// message payloads themselves. See project_message_store_fifo_retention
	// memory for the partition-capacity math and the pending "grow the
	// urns partition" follow-up (deferred - it wipes the node identity).
	static constexpr size_t MAX_TOTAL_RETAINED_MESSAGES = 100;

	/**
	 * @brief Message persistence and conversation management for LXMF
	 *
	 * Stores LXMF messages on the filesystem organized by conversation (peer).
	 * Maintains an index of conversations and message order for efficient retrieval.
	 *
	 * Storage structure:
	 *   <base_path>/
	 *     conversations.json         - Conversation index
	 *     messages/<hash>.json       - Individual message files
	 *     conversations/<peer_hash>/ - Per-conversation metadata
	 *
	 * Usage:
	 *   MessageStore store("/path/to/storage");
	 *   store.save_message(message);
	 *
	 *   auto conversations = store.get_conversations();
	 *   auto messages = store.get_messages_for_conversation(peer_hash);
	 *   LXMessage msg = store.load_message(message_hash);
	 */
	class MessageStore {

	public:
		/**
		 * @brief Conversation metadata with fixed-size message hash storage
		 */
		// Cache of the peer's last-known LXMF display name. The
		// authoritative source is Identity::recall_app_data(peer_hash),
		// which is in-memory only and lost on reboot — without this
		// cache the conversation list falls back to truncated hashes
		// every cold start until the peer re-announces.
		static constexpr size_t MAX_DISPLAY_NAME_LEN = 47;  // 47 + nul = 48

		struct ConversationInfo {
			// Fixed arrays eliminate ~6KB Bytes metadata overhead per conversation
			// (256 messages × 24 bytes metadata = 6.1KB saved per conversation)
			uint8_t peer_hash[PEER_HASH_SIZE];
			uint8_t message_hashes[MAX_MESSAGES_PER_CONVERSATION][MESSAGE_HASH_SIZE];
			size_t message_count = 0;          // Number of messages in this conversation
			double last_activity = 0.0;        // Timestamp of most recent message
			size_t unread_count = 0;           // Number of unread messages
			uint8_t last_message_hash[MESSAGE_HASH_SIZE];
			char display_name[MAX_DISPLAY_NAME_LEN + 1] = {0};  // Last seen, nul-terminated

			// Helper methods for accessing fixed arrays as Bytes
			RNS::Bytes peer_hash_bytes() const { return RNS::Bytes(peer_hash, PEER_HASH_SIZE); }
			RNS::Bytes message_hash_bytes(size_t idx) const {
				if (idx >= message_count) return RNS::Bytes();
				return RNS::Bytes(message_hashes[idx], MESSAGE_HASH_SIZE);
			}
			RNS::Bytes last_message_hash_bytes() const { return RNS::Bytes(last_message_hash, MESSAGE_HASH_SIZE); }

			void set_peer_hash(const RNS::Bytes& b) {
				size_t len = std::min(b.size(), PEER_HASH_SIZE);
				memcpy(peer_hash, b.data(), len);
				if (len < PEER_HASH_SIZE) memset(peer_hash + len, 0, PEER_HASH_SIZE - len);
			}
			void set_last_message_hash(const RNS::Bytes& b) {
				size_t len = std::min(b.size(), MESSAGE_HASH_SIZE);
				memcpy(last_message_hash, b.data(), len);
				if (len < MESSAGE_HASH_SIZE) memset(last_message_hash + len, 0, MESSAGE_HASH_SIZE - len);
			}
			bool peer_hash_equals(const RNS::Bytes& b) const {
				if (b.size() != PEER_HASH_SIZE) return false;
				return memcmp(peer_hash, b.data(), PEER_HASH_SIZE) == 0;
			}

			/**
			 * @brief Add a message hash to this conversation
			 * @param hash Message hash to add
			 * @return True if added, false if already exists or pool full
			 */
			bool add_message_hash(const RNS::Bytes& hash);

			/**
			 * @brief Check if conversation has a specific message
			 * @param hash Message hash to check
			 * @return True if message exists in this conversation
			 */
			bool has_message(const RNS::Bytes& hash) const;

			/**
			 * @brief Remove a message hash from this conversation
			 * @param hash Message hash to remove
			 * @return True if removed, false if not found
			 */
			bool remove_message_hash(const RNS::Bytes& hash);

			/**
			 * @brief Clear all data in this conversation info
			 */
			void clear();
		};

		/**
		 * @brief Fixed-size slot for conversation storage
		 */
		struct ConversationSlot {
			bool in_use = false;
			uint8_t peer_hash[PEER_HASH_SIZE];
			ConversationInfo info;

			// Helper methods
			RNS::Bytes peer_hash_bytes() const { return RNS::Bytes(peer_hash, PEER_HASH_SIZE); }
			void set_peer_hash(const RNS::Bytes& b) {
				size_t len = std::min(b.size(), PEER_HASH_SIZE);
				memcpy(peer_hash, b.data(), len);
				if (len < PEER_HASH_SIZE) memset(peer_hash + len, 0, PEER_HASH_SIZE - len);
			}
			bool peer_hash_equals(const RNS::Bytes& b) const {
				if (b.size() != PEER_HASH_SIZE) return false;
				return memcmp(peer_hash, b.data(), PEER_HASH_SIZE) == 0;
			}

			/**
			 * @brief Clear this slot and mark as not in use
			 */
			void clear();
		};

		/**
		 * @brief Lightweight message metadata for fast loading
		 *
		 * Contains only fields needed for chat list display, avoiding
		 * expensive msgpack unpacking.
		 */
		struct MessageMetadata {
			RNS::Bytes hash;
			std::string content;
			double timestamp;
			bool incoming;
			int state;  // Type::Message::State as int
			bool valid;  // True if loaded successfully
			// FIELD_AUDIO summary (AudioField.h) recorded at save time so list
			// views don't need the full packed blob. mode 0 = no audio.
			uint8_t audio_mode = 0;
			uint32_t audio_bytes = 0;
		};

	public:
		/**
		 * @brief Construct MessageStore
		 *
		 * @param base_path Base directory for message storage
		 */
		MessageStore(const std::string& base_path);

		~MessageStore();

	public:
		/**
		 * @brief Configure an archive filesystem for older messages
		 *
		 * When set, save_message cull-walks each conversation after every
		 * save: messages older than HOT_MESSAGES_PER_CONVERSATION are
		 * moved from the primary (hot) filesystem to this archive
		 * filesystem. load_message falls back to the archive on a miss.
		 *
		 * @param fs        Archive filesystem (eg microSD on T-Deck).
		 *                  Pass an empty filesystem (default-constructed)
		 *                  to disable archiving.
		 * @param base_path Subdirectory on the archive filesystem to use
		 *                  for the message store. Defaults to the same
		 *                  base_path used for the hot filesystem.
		 */
		void set_archive_filesystem(microStore::FileSystem fs, const std::string& base_path = "");

		/**
		 * @brief Whether an archive filesystem is configured
		 */
		bool has_archive() const;

		// Optional per-field encryption hook for "content"/"packed" - the
		// two fields that carry actual message text/payload (see the "enc"
		// marker in save_message()/load_message()/load_message_metadata()).
		// MessageStore itself stays free of any PIN/passphrase-vault
		// dependency (that lives in RNode_Firmware/Vault.h, well above this
		// portable library) - the caller supplies encrypt/decrypt as plain
		// callbacks, keyed by peer hash so it can derive a per-peer subkey.
		// Unset by default: every field is stored exactly as before
		// (plaintext UTF-8 content / hex packed), so a consumer that never
		// calls this sees zero behavior change. encrypt_fn/decrypt_fn may
		// throw - callers of save_message()/load_message()/
		// load_message_metadata() already handle exceptions from those.
		using FieldEncryptFn = std::function<RNS::Bytes(const RNS::Bytes& peer_hash, const RNS::Bytes& plaintext)>;
		using FieldDecryptFn = std::function<RNS::Bytes(const RNS::Bytes& peer_hash, const RNS::Bytes& ciphertext)>;
		void set_field_cipher(FieldEncryptFn encrypt_fn, FieldDecryptFn decrypt_fn);

		/**
		 * @brief Cache a peer's LXMF display name in the conversation
		 *        index. The name is persisted to conv.json so it
		 *        survives reboots — without this the conversation list
		 *        falls back to truncated hashes every cold start until
		 *        the peer re-announces.
		 *
		 * @param peer_hash    Peer's destination hash
		 * @param display_name Resolved display name (eg from
		 *                     LXMF::display_name_from_app_data on
		 *                     Identity::recall_app_data result)
		 * @return True if the name was stored / updated. No-op if the
		 *         conversation isn't in the pool yet, or if the cached
		 *         name is already identical.
		 */
		bool set_display_name(const RNS::Bytes& peer_hash,
		                      const std::string& display_name);

		/**
		 * @brief Read the cached display name for a peer.
		 * @return Empty string if none cached.
		 */
		std::string get_display_name(const RNS::Bytes& peer_hash) const;

		/**
		 * @brief Save a message to storage
		 *
		 * Saves the message and updates the conversation index.
		 * Messages are organized by peer (the other party in the conversation).
		 *
		 * @param message Message to save
		 * @return True if saved successfully
		 */
		bool save_message(const LXMessage& message);

		/**
		 * @brief Load a message from storage
		 *
		 * @param message_hash Hash of the message to load
		 * @return LXMessage object (or empty if not found)
		 */
		LXMessage load_message(const RNS::Bytes& message_hash);

		/**
		 * @brief Load only message metadata (fast path for chat list)
		 *
		 * Reads content/timestamp/state directly from JSON without msgpack unpacking.
		 * Much faster than load_message() for displaying message lists.
		 *
		 * @param message_hash Hash of the message to load
		 * @return MessageMetadata struct (check .valid field)
		 */
		MessageMetadata load_message_metadata(const RNS::Bytes& message_hash);

		/**
		 * @brief Update message state in storage
		 *
		 * Updates just the state field of a stored message.
		 *
		 * @param message_hash Hash of the message to update
		 * @param state New state value
		 * @return True if updated successfully
		 */
		bool update_message_state(const RNS::Bytes& message_hash, Type::Message::State state);

		/**
		 * @brief Delete a message from storage
		 *
		 * Removes the message file and updates the conversation index.
		 *
		 * @param message_hash Hash of the message to delete
		 * @return True if deleted successfully
		 */
		bool delete_message(const RNS::Bytes& message_hash);

		/**
		 * @brief Get list of all conversation peer hashes
		 *
		 * Returns peer hashes sorted by last activity (most recent first).
		 *
		 * @return Vector of peer hashes
		 */
		std::vector<RNS::Bytes> get_conversations();

		/**
		 * @brief Get conversation info for a peer
		 *
		 * @param peer_hash Hash of the peer
		 * @return ConversationInfo (or empty if not found)
		 */
		ConversationInfo get_conversation_info(const RNS::Bytes& peer_hash);

		/**
		 * @brief Get all message hashes for a conversation
		 *
		 * Returns messages in chronological order (oldest first).
		 *
		 * @param peer_hash Hash of the peer
		 * @return Vector of message hashes
		 */
		std::vector<RNS::Bytes> get_messages_for_conversation(const RNS::Bytes& peer_hash);

		/**
		 * @brief Mark all messages in conversation as read
		 *
		 * @param peer_hash Hash of the peer
		 */
		void mark_conversation_read(const RNS::Bytes& peer_hash);

		/**
		 * @brief Delete entire conversation
		 *
		 * Removes all messages and conversation metadata.
		 *
		 * @param peer_hash Hash of the peer
		 * @return True if deleted successfully
		 */
		bool delete_conversation(const RNS::Bytes& peer_hash);

		/**
		 * @brief Get total number of stored messages
		 *
		 * @return Message count
		 */
		size_t get_message_count() const;

		/**
		 * @brief Get total number of conversations
		 *
		 * @return Conversation count
		 */
		size_t get_conversation_count() const;

		/**
		 * @brief Get total unread message count across all conversations
		 *
		 * @return Unread message count
		 */
		size_t get_unread_count() const;

		/**
		 * @brief Clear all stored messages and conversations
		 *
		 * WARNING: This permanently deletes all data.
		 *
		 * @return True if cleared successfully
		 */
		bool clear_all();

	private:
		/**
		 * @brief Initialize storage directories
		 *
		 * Creates base_path, messages/, and conversations/ directories if needed.
		 *
		 * @return True if initialized successfully
		 */
		bool initialize_storage();

		/**
		 * @brief Load conversation index from disk
		 *
		 * Loads conversations.json into _conversations_pool.
		 */
		bool load_index();
		bool load_index_file(const std::string& index_path);

		/**
		 * @brief Save conversation index to disk
		 *
		 * Persists _conversations_pool to conversations.json.
		 *
		 * @return True if saved successfully
		 */
		bool save_index(bool empty = false);

		/**
		 * @brief Get filesystem path for a message file
		 *
		 * @param message_hash Hash of the message
		 * @return Full path to message JSON file
		 */
		std::string get_message_path(const RNS::Bytes& message_hash) const;
		bool recover_message_payload(const std::string& message_path,
		                             const RNS::Bytes& expected_hash);
		bool recover_archived_message_payload(const RNS::Bytes& expected_hash);

		/**
		 * @brief Whether any conversation's message list still references
		 *        this hash.
		 *
		 * get_message_path() truncates a message's real hash to 6 bytes for
		 * the filename, so two different messages can in principle collide
		 * on the same path - recover_message_payload() must never silently
		 * overwrite a generation that's still a live message just because
		 * its hash doesn't match the new one. But a generation nothing
		 * references anymore (eg a payload written by a save_message() call
		 * that failed before it could commit the conversation index -
		 * orphaned, not a collision) is safe to discard and replace. This
		 * distinguishes the two cases.
		 *
		 * @param message_hash Hash read back from an on-disk generation
		 * @return True if some conversation's index still lists this hash
		 */
		bool message_hash_is_referenced(const RNS::Bytes& message_hash) const;

		/**
		 * @brief Get filesystem path for conversation directory
		 *
		 * @param peer_hash Hash of the peer
		 * @return Full path to conversation directory
		 */
		std::string get_conversation_path(const RNS::Bytes& peer_hash) const;

		/**
		 * @brief Determine peer hash from message
		 *
		 * For incoming messages: peer = source
		 * For outgoing messages: peer = destination
		 *
		 * @param message The message
		 * @param our_hash Our local identity hash
		 * @return Peer hash
		 */
		RNS::Bytes get_peer_hash(const LXMessage& message, const RNS::Bytes& our_hash) const;

		/**
		 * @brief Find a conversation slot by peer hash
		 *
		 * @param peer_hash Hash of the peer
		 * @return Pointer to ConversationSlot or nullptr if not found
		 */
		ConversationSlot* find_conversation(const RNS::Bytes& peer_hash);
		const ConversationSlot* find_conversation(const RNS::Bytes& peer_hash) const;

		/**
		 * @brief Get or create a conversation slot for a peer
		 *
		 * @param peer_hash Hash of the peer
		 * @return Pointer to ConversationSlot or nullptr if pool is full
		 */
		ConversationSlot* get_or_create_conversation(const RNS::Bytes& peer_hash);

		/**
		 * @brief Count number of active conversations in pool
		 *
		 * @return Number of in-use conversation slots
		 */
		size_t count_conversations() const;

	private:
		/**
		 * @brief Cull-walk a conversation, archiving messages older than
		 *        HOT_MESSAGES_PER_CONVERSATION.
		 *
		 * For each in-memory hash beyond the hot count, copy the file
		 * from the primary filesystem to the archive (if set) and
		 * delete the primary copy. If no archive is set, just delete.
		 * The hash stays in the in-memory list either way.
		 */
		void cull_conversation_to_hot(const RNS::Bytes& peer_hash);

		/**
		 * @brief Hard-delete this conversation's oldest messages (hash +
		 *        payload, not just a hot/archive move) down to
		 *        RETAINED_MESSAGES_PER_CONVERSATION. No-op if already at
		 *        or under the limit. Batches all evictions from one call
		 *        into a single index commit rather than one per message.
		 *
		 * @param reserve Trim as if this many more messages are about to
		 *                be added - eg reserve=1 when called *before*
		 *                adding a new message, so the conversation lands
		 *                exactly at the limit afterward instead of one
		 *                over. Called with reserve=1 pre-write (this is
		 *                what actually makes room, since a critically
		 *                full filesystem fails the new message's own
		 *                payload/index write before any post-write
		 *                cleanup would ever run) and reserve=0 post-write
		 *                as a cheap no-op safety net.
		 */
		void trim_conversation_to_retention(const RNS::Bytes& peer_hash, size_t reserve = 0);

		/**
		 * @brief Hard-delete the oldest messages store-wide (hash +
		 *        payload) down to MAX_TOTAL_RETAINED_MESSAGES, taking
		 *        them from whichever conversation has been least
		 *        recently active (an approximation of true global
		 *        chronological order - exact would need each message's
		 *        own timestamp, which isn't cheap to compare without
		 *        loading every candidate payload from flash). No-op if
		 *        already at or under the limit.
		 *
		 * @param reserve See trim_conversation_to_retention()'s own doc.
		 */
		void trim_global_total(size_t reserve = 0);

		/**
		 * @brief Move a single message file from hot → archive.
		 * @return True if archive successful, false otherwise. Hot copy
		 *         is removed iff archive succeeded (or no archive_fs).
		 */
		bool archive_one_message(const RNS::Bytes& message_hash);
		bool update_archived_message_state(const RNS::Bytes& message_hash,
		                                   Type::Message::State state);

		/**
		 * @brief Build the archive-side path for a message hash.
		 */
		std::string get_archive_message_path(const RNS::Bytes& message_hash) const;

		/**
		 * @brief Read a file from the archive filesystem.
		 * @return Number of bytes read; 0 on miss / no archive.
		 *
		 * Not const because microStore::FileSystem's accessors are
		 * non-const (they assert _impl and forward through a shared_ptr).
		 */
		size_t read_archive_file(const char* path, RNS::Bytes& out);

		/**
		 * @brief Write a file to the archive filesystem.
		 * @return Number of bytes written; 0 on failure / no archive.
		 */
		size_t write_archive_file(const char* path, const RNS::Bytes& data);

	private:
		std::string _base_path;
		ConversationSlot _conversations_pool[MAX_CONVERSATIONS];
		bool _initialized;

		// Optional archive filesystem (eg microSD). When `_archive_fs`
		// is truthy, save_message cull-walks each conversation and
		// older messages flow to `<_archive_path>/messages/<hash>.json`.
		microStore::FileSystem _archive_fs;
		std::string _archive_path;

		// Set via set_field_cipher() - both null (default-constructed
		// std::function, falsy) until a caller opts in.
		FieldEncryptFn _field_encrypt;
		FieldDecryptFn _field_decrypt;

		// Reusable JSON document to reduce heap fragmentation
		// Note: This class is assumed to be used from a single thread (main loop).
		// If called from multiple threads, this would need per-thread documents or locking.
		JsonDocument _json_doc;

		// Reusable rollback snapshot for save_message(). ConversationInfo is
		// larger than 8 KiB, so placing it in the save_message stack frame
		// overflows callers such as Pyxis's LVGL task. MessageStore already has
		// a single-threaded contract because _json_doc is shared, so reuse one
		// object-owned snapshot instead of allocating or copying it on the stack.
		ConversationInfo _transaction_snapshot;
	};

}  // namespace LXMF
