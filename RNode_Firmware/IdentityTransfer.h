// KISS identity export/import (phase (d) of the vault plan,
// /home/nickie/.claude/plans/lucky-wandering-journal.md) - lets the
// onboard LXMF identity move to different hardware without hand-copying
// raw key bytes. Two opcodes (Framing.h): CMD_IDENTITY_EXPORT (host asks
// the device to export - the real gating and the fresh export passphrase
// are both entered ON-DEVICE, never sent over this wire) and
// CMD_IDENTITY_IMPORT (host sends an already-PIN-encrypted VaultBlob;
// the import passphrase is likewise entered on-device, only the
// ciphertext bytes ever touch the wire).
//
// Reuses Vault.h's existing VaultBlob container (vault_wrap_blob()/
// vault_unwrap_blob()) unchanged - transport-agnostic, the same format
// vault.vk/identity.vault already use on disk.
//
// Both flows block the caller (serial_callback(), RNode_Firmware.ino) for
// the whole on-device interaction - same precedent VaultUnlock.h's own
// vault_enroll_flow()/vault_disable_flow() already set for the *live*
// (not just boot-time) PIN Protection toggle in Settings. Export/import
// are rare, deliberate, physically-present operations; staying
// unresponsive to other traffic for the handful of seconds a human takes
// to enter a passphrase is an accepted tradeoff here, not an oversight.

#pragma once

// RNS::Identity's raw private-key blob size (Type::Identity::KEYSIZE/8,
// microReticulum/Type.h) - duplicated as a plain constant rather than
// reaching into Identity.h's namespaced constant this early, same
// "self-contained rather than depend on a sibling header's include order"
// reasoning as Vault.h's own path macros.
#define VAULT_IDENTITY_KEYSIZE_BYTES 64

// --- Plain hold-to-confirm/tap-to-cancel screen, reusing vault_unlock_
// active as the same exclusive-input gate vault_unlock_prompt() uses.
// vault_confirm_mode (VaultUnlock.h) tells that file's own release
// handlers to treat a release as CANCEL instead of doing PIN-digit-
// stepper things; CONFIRM is detected independently, by directly polling
// the raw hold state every loop iteration - the same live-submit
// technique vault_unlock_prompt() itself uses, and for the same reason
// (waiting for the release event to travel through the normal dispatch
// path felt unresponsive during that feature's own testing).
inline void vault_identity_confirm_draw(const char* title, const char* line1, const char* line2, const char* footer) {
	display.fillScreen(SSD1306_BLACK);
	display.setTextColor(SSD1306_WHITE);
	display.setFont(SMALL_FONT);
	display.setTextSize(1);
	display.setCursor(6, VAULT_HEADER_TEXT_Y);
	display.print(title);
	display.drawFastHLine(VAULT_CONTENT_X, VAULT_HEADER_HLINE_Y, VAULT_CONTENT_W, SSD1306_WHITE);
	display.setCursor(VAULT_CONTENT_X, VAULT_HEADER_HLINE_Y + 16);
	display.print(line1);
	display.setCursor(VAULT_CONTENT_X, VAULT_HEADER_HLINE_Y + 30);
	display.print(line2);
	display.drawFastHLine(VAULT_CONTENT_X, VAULT_FOOTER_HLINE_Y, VAULT_CONTENT_W, SSD1306_WHITE);
	display.setCursor(6, VAULT_FOOTER_TEXT_Y);
	display.print(footer);
	#if DISPLAY_IS_OLED
		display.display();
	#endif
}

inline bool vault_identity_confirm(const char* title, const char* line1, const char* line2) {
	vault_unlock_active = true;
	vault_confirm_mode = true;
	vault_confirm_cancelled = false;
	vault_hold_reset();

	bool confirmed = false;
	unsigned long last_draw = 0;
	while (true) {
		vault_wdt_reset();
		display_unblank(); // see vault_unlock_prompt()'s own comment (VaultUnlock.h)
		#if HAS_ENCODER == true
			encoder_process();
		#endif
		input_read();

		bool raw_pressed = false;
		#if HAS_ENCODER == true
			if (enc_btn_state == LOW) raw_pressed = true;
		#endif
		if (button_state == LOW) raw_pressed = true;
		unsigned long held_ms = vault_track_hold(raw_pressed);

		if (held_ms >= VAULT_SUBMIT_HOLD_MS) {
			confirmed = true;
			break;
		}
		if (vault_confirm_cancelled) {
			confirmed = false;
			break;
		}

		if ((millis() - last_draw) > 100) {
			char footer[28];
			if (raw_pressed) {
				snprintf(footer, sizeof(footer), "hold: %lu.%01lus/3s", held_ms / 1000, (held_ms % 1000) / 100);
			} else {
				snprintf(footer, sizeof(footer), "hold 3s:confirm tap:cancel");
			}
			vault_identity_confirm_draw(title, line1, line2, footer);
			last_draw = millis();
		}
		delay(10);
	}

	vault_unlock_active = false;
	vault_confirm_mode = false;
	if (confirmed) {
		// Still physically held when the loop exited (that's how the
		// hold-confirm path works) - the coming release needs swallowing
		// the same way vault_unlock_prompt()'s own live-submit exit does
		// (see that function's comment), so it doesn't leak into Menu.h's
		// normal long-press dispatch. NOT set on the cancel path - there
		// the release already happened (that's what set
		// vault_confirm_cancelled in the first place), so there's no
		// future stray release to suppress; doing so anyway would wrongly
		// eat the user's very next, unrelated click.
		vault_suppress_next_release = true;
	}
	vault_hold_reset();
	return confirmed;
}

// Frames a reply the same way every other multi-byte KISS command in this
// firmware does (see e.g. kiss_indicate_provision_response(),
// Provisioning.h) - FEND, command byte, escaped payload, FEND. An empty
// blob signals failure/cancellation to the host script - there's no
// separate error code, matching CMD_PROVISION_RSP's own "just send
// whatever handle_message() produced" shape.
inline void kiss_indicate_identity_export(const RNS::Bytes& blob) {
	#if HAS_ESPNOW == true
		kiss_select_interface(0);
	#endif
	serial_write(FEND);
	serial_write(CMD_IDENTITY_EXPORT);
	const uint8_t* data = blob.data();
	size_t len = blob.size();
	for (size_t i = 0; i < len; i++) escaped_serial_write(data[i]);
	serial_write(FEND);
}

// Single-byte ack (0x01/0x00) - sent BEFORE hard_reset() on a successful
// import so the companion script can confirm the write actually
// committed before the port drops for the reboot, instead of inferring
// success purely from "the device reset."
inline void kiss_indicate_identity_import_result(bool ok) {
	#if HAS_ESPNOW == true
		kiss_select_interface(0);
	#endif
	serial_write(FEND);
	serial_write(CMD_IDENTITY_IMPORT);
	serial_write(ok ? (uint8_t)0x01 : (uint8_t)0x00);
	serial_write(FEND);
}

// Menu.h's PIN Protection is the only other caller of vault_wrap_blob()
// today (Vault.h's vault_enable()) - this reuses the exact same container
// format/iteration count for the export blob, just under a throwaway
// passphrase instead of the device's own boot PIN.
inline void vault_identity_export_flow() {
	if (vault_enabled) {
		// Re-verify the CURRENT PIN, not just "already unlocked" - guards
		// an unattended-but-unlocked device (see the plan's own "Export
		// gate" note). vault_verify_password() doesn't touch session
		// state, so this can't disturb an already-unlocked vault_key.
		RNS::Bytes pw;
		vault_unlock_prompt("Confirm PIN", pw);
		bool ok = vault_verify_password(pw);
		RNS::secure_zero(pw);
		if (!ok) {
			vault_unlock_draw("Wrong PIN", "Export cancelled");
			vault_wdt_safe_delay(1500);
			kiss_indicate_identity_export(RNS::Bytes::NONE);
			return;
		}
	}

	RNS::Bytes exp_pw1;
	vault_unlock_prompt("Export PIN", exp_pw1);
	if (exp_pw1.size() < VAULT_PIN_MIN_LEN) {
		RNS::secure_zero(exp_pw1);
		vault_unlock_draw("Too short", "Export cancelled");
		vault_wdt_safe_delay(1500);
		kiss_indicate_identity_export(RNS::Bytes::NONE);
		return;
	}
	RNS::Bytes exp_pw2;
	vault_unlock_prompt("Confirm Export", exp_pw2);
	bool match = (exp_pw1.size() == exp_pw2.size() && exp_pw1.compare(exp_pw2) == 0);
	RNS::secure_zero(exp_pw2);
	if (!match) {
		RNS::secure_zero(exp_pw1);
		vault_unlock_draw("Mismatch", "Export cancelled");
		vault_wdt_safe_delay(1500);
		kiss_indicate_identity_export(RNS::Bytes::NONE);
		return;
	}

	RNS::Bytes identity_plain;
	try {
		if (vault_enabled) {
			identity_plain = vault_load_identity_plaintext();
		} else if (RNS::Utilities::OS::read_file(VAULT_LEGACY_IDENTITY_PATH, identity_plain) == 0) {
			throw std::runtime_error("no identity on disk");
		}
	} catch (const std::exception&) {
		RNS::secure_zero(exp_pw1);
		vault_unlock_draw("Error", "No identity found");
		vault_wdt_safe_delay(1500);
		kiss_indicate_identity_export(RNS::Bytes::NONE);
		return;
	}

	RNS::Bytes blob = vault_wrap_blob(identity_plain, exp_pw1, VAULT_PBKDF2_ITERATIONS);
	RNS::secure_zero(identity_plain);
	RNS::secure_zero(exp_pw1);

	kiss_indicate_identity_export(blob);
	vault_unlock_draw("Exported", "Identity sent");
	vault_wdt_safe_delay(1500);
}

// Mirrors vault_enable()'s own identity-wrap step (Vault.h) when the
// vault is on - same subkey context, same two-phase .tmp/verify/rename
// commit - or a plain write when it's off, matching how a from-scratch
// identity is written today (URNS.h's load-or-create block).
inline bool vault_identity_commit(const RNS::Bytes& identity_plain) {
	if (vault_enabled) {
		RNS::Bytes id_key = vault_derive_subkey("rnode-vault/identity/v1");
		RNS::Bytes id_nonce;
		vault_random_bytes(id_nonce, VAULT_NONCE_SIZE);
		RNS::Bytes id_payload = RNS::Cryptography::AES_256_GCM::encrypt(identity_plain, id_key, id_nonce);
		RNS::secure_zero(id_key);
		RNS::Bytes id_file;
		id_file.append(id_nonce);
		id_file.append(id_payload);

		const char* id_tmp = VAULT_IDENTITY_PATH ".tmp";
		RNS::Utilities::OS::write_file(id_tmp, id_file);
		RNS::Bytes verify;
		RNS::Utilities::OS::read_file(id_tmp, verify);
		if (verify.size() != id_file.size() || verify.compare(id_file) != 0) {
			RNS::Utilities::OS::remove_file(id_tmp);
			return false;
		}
		RNS::Utilities::OS::rename_file(id_tmp, VAULT_IDENTITY_PATH);
		return true;
	} else {
		const char* id_tmp = VAULT_LEGACY_IDENTITY_PATH ".tmp";
		RNS::Utilities::OS::write_file(id_tmp, identity_plain);
		RNS::Bytes verify;
		RNS::Utilities::OS::read_file(id_tmp, verify);
		if (verify.size() != identity_plain.size() || verify.compare(identity_plain) != 0) {
			RNS::Utilities::OS::remove_file(id_tmp);
			return false;
		}
		RNS::Utilities::OS::rename_file(id_tmp, VAULT_LEGACY_IDENTITY_PATH);
		return true;
	}
}

inline void vault_identity_import_flow(const RNS::Bytes& import_blob) {
	RNS::Bytes pw;
	vault_unlock_prompt("Import PIN", pw);

	RNS::Bytes identity_plain;
	try {
		identity_plain = vault_unwrap_blob(import_blob, pw);
	} catch (const std::exception&) {
		RNS::secure_zero(pw);
		vault_unlock_draw("Wrong PIN", "Import failed");
		vault_wdt_safe_delay(1500);
		kiss_indicate_identity_import_result(false);
		return;
	}
	RNS::secure_zero(pw);

	if (identity_plain.size() != VAULT_IDENTITY_KEYSIZE_BYTES) {
		RNS::secure_zero(identity_plain);
		vault_unlock_draw("Invalid", "Not an identity");
		vault_wdt_safe_delay(1500);
		kiss_indicate_identity_import_result(false);
		return;
	}

	// Destructive/irreversible - see feedback_destructive_diagnostic_needs_
	// confirmation - always gated behind an explicit on-device confirm,
	// never wired to commit silently just because a well-formed blob and
	// correct passphrase arrived over KISS.
	bool confirmed = vault_identity_confirm("Replace Identity?", "Dest hash changes.", "Cannot be undone.");
	if (!confirmed) {
		RNS::secure_zero(identity_plain);
		vault_unlock_draw("Cancelled", "Not imported");
		vault_wdt_safe_delay(1500);
		kiss_indicate_identity_import_result(false);
		return;
	}

	bool committed = vault_identity_commit(identity_plain);
	RNS::secure_zero(identity_plain);

	if (!committed) {
		vault_unlock_draw("Error", "Could not import");
		vault_wdt_safe_delay(1500);
		kiss_indicate_identity_import_result(false);
		return;
	}

	kiss_indicate_identity_import_result(true);
	vault_unlock_draw("Imported", "Restarting...");
	// The already-running session's urns_identity/urns_destination/
	// Transport routing state are all built around the OLD identity
	// (URNS.h, urns_init()) - swapping the file on disk doesn't change
	// any of that live state, so a clean reboot (same as every other
	// "big state change" in this firmware) is required, not optional.
	// The delay just lets the ack above actually flush over serial before
	// the port drops.
	vault_wdt_safe_delay(1500);
	hard_reset();
}

// Called from serial_callback() (RNode_Firmware.ino) once a complete
// CMD_IDENTITY_EXPORT frame arrives - payload is a bare trigger,
// contents ignored (matches CMD_SENSOR's own "command byte alone
// triggers an immediate reply" shape, Framing.h).
inline void on_identity_export_request() {
	DEBUG_LOG("[IDTX] export request received, urns_ready=%d vault_enabled=%d\r\n", (int)urns_ready, (int)vault_enabled);
	if (!urns_ready) {
		kiss_indicate_identity_export(RNS::Bytes::NONE);
		return;
	}
	vault_identity_export_flow();
}

// Called from serial_callback() once a complete CMD_IDENTITY_IMPORT frame
// has been unescaped into identity_import_buf.
inline void on_identity_import_request(const uint8_t* buf, size_t len) {
	if (!urns_ready) {
		kiss_indicate_identity_import_result(false);
		return;
	}
	RNS::Bytes blob(buf, len);
	vault_identity_import_flow(blob);
}
