// PIN/passphrase at-rest protection for the onboard LXMF identity
// (URNS_IDENTITY_PATH, see URNS.h) - step 1 of the vault feature (software-
// only; secure boot/flash encryption/eFuse HMAC are a deliberately separate
// later step, see /home/nickie/.claude/plans/lucky-wandering-journal.md).
//
// Key hierarchy: PIN/passphrase --PBKDF2-HMAC-SHA256--> KEK --AES-256-GCM
// unwrap--> random Vault Key (VK, 32B, generated once at opt-in)
// --HKDF-SHA256(context)--> per-purpose subkeys (identity-wrap key here;
// per-peer message keys are phase (c), MessageStore.cpp). The two-layer
// VK/KEK split means a PIN/passphrase change only needs to re-wrap the
// small VK blob (vault.vk, ~90 bytes), not re-encrypt the identity or
// (later) the whole message store.
//
// Opt-in only, default OFF (RNode Settings > Messenger > PIN Protection,
// Menu.h) - ADDR_CONF_VAULT_ENABLED (ROM.h) is read in setup() before
// urns_init() ever mounts the "urns" partition, same reasoning as
// ADDR_CONF_URNS/ADDR_CONF_URNS_TRANSPORT. When enabled, the boot-unlock
// screen (VaultUnlock.h) must call vault_try_unlock() successfully before
// urns_init() runs - see RNode_Firmware.ino's setup().

#pragma once

#include "ROM.h"

// Self-contained microReticulum.h include (RNS::Bytes, RNS::Cryptography::*,
// RNS::Utilities::OS::*) rather than relying on URNS.h having already done
// this - urns_init() (URNS.h) calls into this header's functions, so this
// header must be included BEFORE URNS.h (see Utilities.h's include order),
// which means it can't assume URNS.h's own include of this has already run.
// Same push_macro/pop_macro MTU dance as URNS.h's own include - see that
// file's comment for why a plain #define save/restore doesn't work here.
#pragma push_macro("MTU")
#undef MTU
#include <microReticulum.h>
// microReticulum.h doesn't pull these in itself (nothing used them before
// this feature) - PBKDF2 for the KDF, AES for AES-256-GCM, HKDF for
// per-purpose subkey derivation.
#include <microReticulum/Cryptography/PBKDF2.h>
#include <microReticulum/Cryptography/AES.h>
#include <microReticulum/Cryptography/HKDF.h>
#pragma pop_macro("MTU")

// Mirrors URNS_BASE_PATH/URNS_IDENTITY_PATH (URNS.h) - duplicated rather
// than shared because this header must be included before URNS.h (see
// above), so it can't reference URNS.h's macros. Must stay in sync with
// URNS.h's own definitions if either ever changes.
#define VAULT_BASE_PATH             "/urns"
#define VAULT_LEGACY_IDENTITY_PATH  "/urns/identity"
#define VAULT_VK_PATH               VAULT_BASE_PATH "/vault.vk"
#define VAULT_IDENTITY_PATH         VAULT_BASE_PATH "/identity.vault"

// ~10,000 iterations * ~84us/iteration (mbedtls hardware-SHA on ESP32-S3,
// confirmed on-device during phase (a) validation) is roughly a 0.85s
// unlock delay - see PBKDF2.cpp's own comment for the full throughput
// story (this software-only step does not defend against an attacker who
// has physically extracted the flash and can brute-force offline at
// whatever speed their own hardware allows; that's explicitly out of scope
// until the hardware-security phase).
#define VAULT_PBKDF2_ITERATIONS 10000

#define VAULT_SALT_SIZE  16
#define VAULT_NONCE_SIZE 12

// Whether PIN/passphrase protection is on (mirrors urns_enabled's own
// "read from EEPROM in setup(), default from the compiled constant"
// pattern) - false until setup() reads ADDR_CONF_VAULT_ENABLED.
bool vault_enabled = false;

// True once vault_key holds a real, password-unwrapped Vault Key for this
// boot session. vault_derive_subkey()/vault_load_identity_plaintext()
// refuse to run unless this is true.
bool vault_unlocked = false;

// The Vault Key (VK) - resident for the whole unlocked session (needed to
// lazily derive per-purpose subkeys on demand: the identity-wrap key here,
// and per-peer message keys in phase (c)). This is the necessary "working
// key" for an unlocked session, not a violation of "plaintext/keys in RAM
// only when necessary" - see the plan's RAM residency section.
RNS::Bytes vault_key;

inline void vault_random_bytes(RNS::Bytes& out, size_t n) {
	out.clear();
	uint8_t* buf = out.writable(n);
	#if MCU_VARIANT == MCU_ESP32
		size_t i = 0;
		while (i < n) {
			uint32_t r = esp_random();
			size_t chunk = (n - i < 4) ? (n - i) : 4;
			memcpy(buf + i, &r, chunk);
			i += chunk;
		}
	#else
		// Not reached in this step - the Messenger/HAS_URNS feature this
		// belongs to is ESP32-S3-only today. Kept so this header stays
		// portable/compilable rather than hard-erroring on other MCUs.
		for (size_t i = 0; i < n; i++) buf[i] = (uint8_t)random(256);
	#endif
}

// Self-describing container: magic/version/kdf params/salt/nonce/payload,
// where payload is exactly what AES_256_GCM::encrypt() returns (ciphertext
// with its tag already appended) - reused as-is by AES_256_GCM::decrypt(),
// no separate ciphertext/tag bookkeeping needed.
inline RNS::Bytes vault_wrap_blob(const RNS::Bytes& plaintext, const RNS::Bytes& password, uint32_t iterations) {
	RNS::Bytes salt, nonce;
	vault_random_bytes(salt, VAULT_SALT_SIZE);
	vault_random_bytes(nonce, VAULT_NONCE_SIZE);

	RNS::Bytes kek = RNS::Cryptography::pbkdf2_hmac_sha256(password, salt, iterations, 32);
	RNS::Bytes payload = RNS::Cryptography::AES_256_GCM::encrypt(plaintext, kek, nonce);
	RNS::secure_zero(kek);

	RNS::Bytes blob;
	blob.append((const uint8_t*)"RNVB", 4);
	blob.append((uint8_t)1); // version
	blob.append((uint8_t)1); // kdf_id: PBKDF2-HMAC-SHA256
	uint8_t iter_be[4] = {
		(uint8_t)((iterations >> 24) & 0xFF), (uint8_t)((iterations >> 16) & 0xFF),
		(uint8_t)((iterations >> 8) & 0xFF), (uint8_t)(iterations & 0xFF),
	};
	blob.append(iter_be, 4);
	blob.append(salt);
	blob.append(nonce);
	uint32_t payload_len = (uint32_t)payload.size();
	uint8_t len_be[4] = {
		(uint8_t)((payload_len >> 24) & 0xFF), (uint8_t)((payload_len >> 16) & 0xFF),
		(uint8_t)((payload_len >> 8) & 0xFF), (uint8_t)(payload_len & 0xFF),
	};
	blob.append(len_be, 4);
	blob.append(payload);
	return blob;
}

// Throws std::runtime_error on a malformed blob, or (via AES_256_GCM::
// decrypt()) on a wrong password / tag verification failure - callers must
// treat any exception here as "wrong password," not a specific error to
// surface differently.
inline RNS::Bytes vault_unwrap_blob(const RNS::Bytes& blob, const RNS::Bytes& password) {
	const size_t header_size = 4 + 1 + 1 + 4 + VAULT_SALT_SIZE + VAULT_NONCE_SIZE + 4;
	if (blob.size() < header_size) {
		throw std::runtime_error("vault blob truncated");
	}
	if (blob.left(4).compare((const uint8_t*)"RNVB", 4) != 0) {
		throw std::runtime_error("vault blob bad magic");
	}
	// blob[4] version, blob[5] kdf_id - only one of each defined so far,
	// nothing to branch on yet.
	uint32_t iterations = ((uint32_t)blob[6] << 24) | ((uint32_t)blob[7] << 16) | ((uint32_t)blob[8] << 8) | (uint32_t)blob[9];
	RNS::Bytes salt = blob.mid(10, VAULT_SALT_SIZE);
	RNS::Bytes nonce = blob.mid(10 + VAULT_SALT_SIZE, VAULT_NONCE_SIZE);
	size_t len_pos = 10 + VAULT_SALT_SIZE + VAULT_NONCE_SIZE;
	uint32_t payload_len = ((uint32_t)blob[len_pos] << 24) | ((uint32_t)blob[len_pos + 1] << 16)
		| ((uint32_t)blob[len_pos + 2] << 8) | (uint32_t)blob[len_pos + 3];
	if (blob.size() != header_size + payload_len) {
		throw std::runtime_error("vault blob length mismatch");
	}
	RNS::Bytes payload = blob.mid(header_size, payload_len);

	RNS::Bytes kek = RNS::Cryptography::pbkdf2_hmac_sha256(password, salt, iterations, 32);
	RNS::Bytes plaintext = RNS::Cryptography::AES_256_GCM::decrypt(payload, kek, nonce);
	RNS::secure_zero(kek);
	return plaintext;
}

inline RNS::Bytes vault_derive_subkey(const char* context) {
	if (!vault_unlocked) {
		throw std::runtime_error("vault not unlocked");
	}
	return RNS::Cryptography::hkdf(32, vault_key, RNS::Bytes::NONE, RNS::Bytes(context));
}

inline void vault_lock() {
	RNS::secure_zero(vault_key);
	vault_unlocked = false;
}

// MessageStore.h's set_field_cipher() hook (phase (c)) - per-peer subkey
// (not a single whole-store key) so a change to one conversation's
// history never needs touching another's, same reasoning as the VK/KEK
// split for a PIN change. Context string intentionally includes the
// peer's hex hash, not just a fixed "message" label - see HKDF.cpp's
// context-forwarding fix (phase (a)) for why two different contexts must
// never collide. Envelope is nonce[12] || AES_256_GCM ciphertext-with-tag
// (mirrors vault_load_identity_plaintext()'s identity.vault layout, minus
// the outer RNVB header - no KDF/salt needed per-field since the subkey
// is already derived from the resident VK, not a password).
inline RNS::Bytes vault_encrypt_message_field(const RNS::Bytes& peer_hash, const RNS::Bytes& plaintext) {
	RNS::Bytes subkey = vault_derive_subkey(("rnode-vault/msg/" + peer_hash.toHex()).c_str());
	RNS::Bytes nonce;
	vault_random_bytes(nonce, VAULT_NONCE_SIZE);
	RNS::Bytes payload = RNS::Cryptography::AES_256_GCM::encrypt(plaintext, subkey, nonce);
	RNS::secure_zero(subkey);
	RNS::Bytes envelope;
	envelope.append(nonce);
	envelope.append(payload);
	return envelope;
}

// Throws std::runtime_error/std::invalid_argument on a truncated envelope
// or (via AES_256_GCM::decrypt()) tag verification failure - callers
// (MessageStore.cpp) already treat any exception here as "can't decrypt
// this field."
inline RNS::Bytes vault_decrypt_message_field(const RNS::Bytes& peer_hash, const RNS::Bytes& envelope) {
	if (envelope.size() <= VAULT_NONCE_SIZE) {
		throw std::runtime_error("message field envelope truncated");
	}
	RNS::Bytes nonce = envelope.left(VAULT_NONCE_SIZE);
	RNS::Bytes payload = envelope.mid(VAULT_NONCE_SIZE);
	RNS::Bytes subkey = vault_derive_subkey(("rnode-vault/msg/" + peer_hash.toHex()).c_str());
	RNS::Bytes plaintext = RNS::Cryptography::AES_256_GCM::decrypt(payload, subkey, nonce);
	RNS::secure_zero(subkey);
	return plaintext;
}

// Reads vault.vk and unwraps it under the given password. On success,
// leaves vault_key/vault_unlocked set for the rest of this boot session.
// Returns false (does not throw) on any failure - wrong password, missing
// file, corrupt blob - so callers (VaultUnlock.h's retry loop) don't need
// their own try/catch.
inline bool vault_try_unlock(const RNS::Bytes& password) {
	if (!RNS::Utilities::OS::file_exists(VAULT_VK_PATH)) {
		return false;
	}
	RNS::Bytes vk_blob;
	if (RNS::Utilities::OS::read_file(VAULT_VK_PATH, vk_blob) == 0) {
		return false;
	}
	try {
		RNS::Bytes vk = vault_unwrap_blob(vk_blob, password);
		if (vk.size() != 32) {
			RNS::secure_zero(vk);
			return false;
		}
		vault_key = vk;
		vault_unlocked = true;
		return true;
	}
	catch (const std::exception&) {
		return false;
	}
}

// Requires vault_unlocked. Decrypts identity.vault under the identity-wrap
// subkey and returns the raw 64-byte private key blob (Identity's own
// on-disk format, see Identity.cpp's get_private_key()/load_private_key())
// for the caller to build an RNS::Identity from in memory - never written
// back to disk in plaintext.
inline RNS::Bytes vault_load_identity_plaintext() {
	if (!vault_unlocked) {
		throw std::runtime_error("vault not unlocked");
	}
	RNS::Bytes id_file;
	if (RNS::Utilities::OS::read_file(VAULT_IDENTITY_PATH, id_file) == 0) {
		throw std::runtime_error("identity.vault missing or empty");
	}
	if (id_file.size() <= VAULT_NONCE_SIZE) {
		throw std::runtime_error("identity.vault truncated");
	}
	RNS::Bytes nonce = id_file.left(VAULT_NONCE_SIZE);
	RNS::Bytes payload = id_file.mid(VAULT_NONCE_SIZE);
	RNS::Bytes id_key = vault_derive_subkey("rnode-vault/identity/v1");
	RNS::Bytes plaintext = RNS::Cryptography::AES_256_GCM::decrypt(payload, id_key, nonce);
	RNS::secure_zero(id_key);
	return plaintext;
}

// Turns PIN protection on: generates a fresh VK, wraps it under a
// password-derived KEK, migrates the existing plaintext identity
// (URNS_IDENTITY_PATH, written by urns_init()'s load-or-create block - must
// already exist, since urns_init() always creates one if missing) into
// identity.vault, and sets ADDR_CONF_VAULT_ENABLED. Two-phase commit
// (write to .tmp, verify readback, rename, only then delete the old
// plaintext file) so a power loss mid-migration can't leave neither a
// valid plaintext nor a valid encrypted identity. Leaves the vault
// unlocked with this session's VK on success (the caller just proved the
// password by choosing it, no need to make them re-enter it).
inline bool vault_enable(const RNS::Bytes& password) {
	if (vault_enabled) {
		return false;
	}
	if (!RNS::Utilities::OS::file_exists(VAULT_LEGACY_IDENTITY_PATH)) {
		return false;
	}

	RNS::Bytes vk;
	vault_random_bytes(vk, 32);
	RNS::Bytes vk_blob = vault_wrap_blob(vk, password, VAULT_PBKDF2_ITERATIONS);

	RNS::Bytes identity_plain;
	if (RNS::Utilities::OS::read_file(VAULT_LEGACY_IDENTITY_PATH, identity_plain) == 0) {
		RNS::secure_zero(vk);
		return false;
	}

	RNS::Bytes id_key = RNS::Cryptography::hkdf(32, vk, RNS::Bytes::NONE, RNS::Bytes("rnode-vault/identity/v1"));
	RNS::Bytes id_nonce;
	vault_random_bytes(id_nonce, VAULT_NONCE_SIZE);
	RNS::Bytes id_payload = RNS::Cryptography::AES_256_GCM::encrypt(identity_plain, id_key, id_nonce);
	RNS::secure_zero(id_key);
	RNS::secure_zero(identity_plain);

	RNS::Bytes id_file;
	id_file.append(id_nonce);
	id_file.append(id_payload);

	const char* vk_tmp = VAULT_VK_PATH ".tmp";
	const char* id_tmp = VAULT_IDENTITY_PATH ".tmp";
	RNS::Utilities::OS::write_file(vk_tmp, vk_blob);
	RNS::Utilities::OS::write_file(id_tmp, id_file);

	RNS::Bytes verify_vk, verify_id;
	RNS::Utilities::OS::read_file(vk_tmp, verify_vk);
	RNS::Utilities::OS::read_file(id_tmp, verify_id);
	bool verified = (verify_vk.size() == vk_blob.size() && verify_vk.compare(vk_blob) == 0
		&& verify_id.size() == id_file.size() && verify_id.compare(id_file) == 0);
	if (!verified) {
		RNS::Utilities::OS::remove_file(vk_tmp);
		RNS::Utilities::OS::remove_file(id_tmp);
		RNS::secure_zero(vk);
		return false;
	}

	RNS::Utilities::OS::rename_file(vk_tmp, VAULT_VK_PATH);
	RNS::Utilities::OS::rename_file(id_tmp, VAULT_IDENTITY_PATH);
	RNS::Utilities::OS::remove_file(VAULT_LEGACY_IDENTITY_PATH);

	#if HAS_EEPROM
		EEPROM.write(ADDR_CONF_VAULT_ENABLED, VAULT_ENABLE_BYTE);
		EEPROM.commit();
	#endif

	vault_enabled = true;
	vault_key = vk;
	vault_unlocked = true;
	return true;
}

// Read-only password check against vault.vk - does NOT touch vault_key/
// vault_unlocked (the session's already-established key, if any, is left
// exactly as it was). Needed by callers (VaultUnlock.h's
// vault_disable_flow()) that must gate a side effect (decrypting the
// whole message store) on "is this really the right PIN," evaluated
// BEFORE calling vault_disable() itself - vault_disable() only commits/
// locks at the very end of its own success path, by which point
// vault_key has already been zeroed and is too late to derive per-peer
// message subkeys from. Running the message-store decrypt unconditionally
// ahead of a real password check would let a wrong-PIN attempt still
// flatten the store to plaintext even though "disable" never actually
// succeeded - this exists specifically to close that gap.
inline bool vault_verify_password(const RNS::Bytes& password) {
	if (!RNS::Utilities::OS::file_exists(VAULT_VK_PATH)) {
		return false;
	}
	RNS::Bytes vk_blob;
	if (RNS::Utilities::OS::read_file(VAULT_VK_PATH, vk_blob) == 0) {
		return false;
	}
	try {
		RNS::Bytes vk = vault_unwrap_blob(vk_blob, password);
		bool ok = (vk.size() == 32);
		RNS::secure_zero(vk);
		return ok;
	}
	catch (const std::exception&) {
		return false;
	}
}

// Turns PIN protection off: re-verifies the password against vault.vk
// (not just "already unlocked," in case the device was left unattended
// while unlocked), decrypts the identity back to plaintext at
// URNS_IDENTITY_PATH (same two-phase commit as vault_enable()), removes
// the vault files, and clears ADDR_CONF_VAULT_ENABLED. Confirmed product
// decision: disabling always decrypts back to plaintext (symmetric with
// enabling), gated behind a confirmation prompt at the call site (Menu.h),
// not behind a hard refusal here.
inline bool vault_disable(const RNS::Bytes& password) {
	if (!RNS::Utilities::OS::file_exists(VAULT_VK_PATH)) {
		return false;
	}
	RNS::Bytes vk_blob;
	if (RNS::Utilities::OS::read_file(VAULT_VK_PATH, vk_blob) == 0) {
		return false;
	}
	RNS::Bytes vk;
	try {
		vk = vault_unwrap_blob(vk_blob, password);
	}
	catch (const std::exception&) {
		return false;
	}
	if (vk.size() != 32) {
		RNS::secure_zero(vk);
		return false;
	}

	RNS::Bytes id_file;
	if (RNS::Utilities::OS::read_file(VAULT_IDENTITY_PATH, id_file) == 0 || id_file.size() <= VAULT_NONCE_SIZE) {
		RNS::secure_zero(vk);
		return false;
	}
	RNS::Bytes id_nonce = id_file.left(VAULT_NONCE_SIZE);
	RNS::Bytes id_payload = id_file.mid(VAULT_NONCE_SIZE);
	RNS::Bytes id_key = RNS::Cryptography::hkdf(32, vk, RNS::Bytes::NONE, RNS::Bytes("rnode-vault/identity/v1"));
	RNS::Bytes identity_plain;
	try {
		identity_plain = RNS::Cryptography::AES_256_GCM::decrypt(id_payload, id_key, id_nonce);
	}
	catch (const std::exception&) {
		RNS::secure_zero(id_key);
		RNS::secure_zero(vk);
		return false;
	}
	RNS::secure_zero(id_key);

	const char* id_plain_tmp = VAULT_LEGACY_IDENTITY_PATH ".tmp";
	RNS::Utilities::OS::write_file(id_plain_tmp, identity_plain);
	RNS::Bytes verify;
	RNS::Utilities::OS::read_file(id_plain_tmp, verify);
	bool verified = (verify.size() == identity_plain.size() && verify.compare(identity_plain) == 0);
	RNS::secure_zero(identity_plain);
	if (!verified) {
		RNS::Utilities::OS::remove_file(id_plain_tmp);
		RNS::secure_zero(vk);
		return false;
	}

	RNS::Utilities::OS::rename_file(id_plain_tmp, VAULT_LEGACY_IDENTITY_PATH);
	RNS::Utilities::OS::remove_file(VAULT_IDENTITY_PATH);
	RNS::Utilities::OS::remove_file(VAULT_VK_PATH);
	RNS::secure_zero(vk);

	#if HAS_EEPROM
		EEPROM.write(ADDR_CONF_VAULT_ENABLED, VAULT_DISABLE_BYTE);
		EEPROM.commit();
	#endif

	vault_enabled = false;
	vault_lock();
	return true;
}
