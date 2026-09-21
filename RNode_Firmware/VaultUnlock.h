// PIN entry UI for Vault.h - a blocking, pre-loop() screen used both at
// boot (device locked, vault_enabled true - see setup()'s call to
// vault_unlock_boot_screen()) and from the Menu.h "PIN Protection" toggle
// (enrolling a new PIN, or re-confirming the current one before disabling
// protection). Genuinely new territory in this codebase - no prior
// blocking pre-loop() UI existed before this - so it owns its own cursor/
// input state entirely separately from Menu.h's not-yet-initialized state
// machine, redirecting into it via the vault_unlock_active gate in
// Encoder.h's encoder_process() and RNode_Firmware.ino's button_event().
//
// PIN-only for this pass (numeric digit entry) - full passphrase/QWERTY
// entry is a deferred fast-follow that reuses this same vault_try_unlock()/
// vault_enable()/vault_disable() plumbing, just with a different input
// widget.
//
// Interaction model - a single-digit "stepper" (not a spatial grid): one
// digit is dialed at a time and confirmed/discarded via tap/hold tiers,
// working identically in spirit on both input modalities this feature
// targets (MeshAdventurer-S3's encoder, MeshPoE-S3's single button):
//   Encoder:      rotate = change the current digit's value (0-9, wraps)
//                 short press  (<400ms)   = confirm digit, advance
//                 medium press (400-1500ms) = backspace last digit
//                 long press   (>1500ms)  = submit (if PIN long enough)
//   Button-only:  short tap  (<400ms)     = increment current digit (0-9, wraps)
//                 short hold (400-1200ms) = confirm digit, advance
//                 med hold   (1200-3000ms) = backspace last digit
//                 long hold  (>3000ms)    = submit (if PIN long enough)
#pragma once

// RNode_Firmware.ino's own #include <esp_task_wdt.h> comes AFTER
// #include "Utilities.h" (which pulls this header in) - can't rely on it,
// needed directly here for vault_wdt_reset() below.
#if MCU_VARIANT == MCU_ESP32 && HAS_URNS == true
	#include <esp_task_wdt.h>
#endif

// Forward declarations rather than #include "Input.h"/"Encoder.h" - those
// headers aren't included yet at this point in Utilities.h's chain
// (VaultUnlock.h comes before them, since urns_init() needs Vault.h/
// VaultUnlock.h first), and pulling them in early risks depending on board
// pin definitions in an order that isn't guaranteed this early. Both
// functions, and the raw debounce-state globals below, are safe to
// reference this early regardless - input_init()/encoder_init() both run
// well before urns_init() in setup(). Comparing against LOW directly
// rather than ENC_PRESSED/PRESSED (both just #define LOW anyway) avoids
// needing those macros declared too.
void input_read();
#if HAS_ENCODER == true
	void encoder_process();
	extern int enc_btn_state;
	extern unsigned long enc_btn_down_last;
#endif
extern int button_state;
extern unsigned long button_down_last;

// Phase (c) message-store migration - forward declared for the same
// reason as input_read()/encoder_process() above: Messenger.h (where
// urns_message_store/MessageStore.h actually live) is included well
// after this file in Utilities.h's chain. Called from
// vault_enroll_flow()/vault_disable_flow() below right after
// vault_enable()/vault_disable() itself commits, to bring the whole
// existing message store into sync with the new field-cipher state - see
// each function's own comment (Messenger.h) for the bulk re-save walk.
// No-ops if there's no message store yet (HAS_LXMF false, or messenger_
// init() hasn't run) - same reasoning as messenger_has_unread()'s own
// urns_message_store null check.
#if HAS_LXMF == true
	void vault_migrate_messages_encrypt();
	void vault_migrate_messages_decrypt();
#endif

#define VAULT_PIN_MAX_LEN 8
#define VAULT_PIN_MIN_LEN 4
// Live submit threshold - fires the instant a hold crosses this duration,
// without waiting for release (see vault_unlock_prompt()'s own comment for
// why, and why a "wait for release" step follows once it fires). Matches
// the footer caption ("hold 3s:confirm", vault_unlock_draw()).
#define VAULT_SUBMIT_HOLD_MS 3000

bool vault_unlock_active = false;
// Set whenever vault_unlock_prompt() returns with the triggering control
// still physically held (the live-submit path always exits this way, by
// design - see that function's own comment). Consumed exactly once by
// Encoder.h/RNode_Firmware.ino's normal release dispatch to swallow the
// stray release that's still coming, instead of letting a 3s+ duration
// land in menu_encoder_button()/button_event()'s long-press-opens-Settings
// tier the instant control returns to normal handling.
bool vault_suppress_next_release = false;

char vault_pin_buf[VAULT_PIN_MAX_LEN + 1] = {0};
uint8_t vault_pin_len = 0;
uint8_t vault_pin_current_digit = 0;
bool vault_pin_submitted = false;
bool vault_ui_dirty = true;
// One-shot per press - set once the live-hold submit check fires, so it
// doesn't keep re-firing every loop iteration for the rest of the hold.
bool vault_pin_submit_fired = false;

// Set by IdentityTransfer.h's vault_identity_confirm() while its own
// plain hold-to-confirm/tap-to-cancel screen owns input - reuses the same
// vault_unlock_active exclusive-input gate the PIN entry above uses
// (Encoder.h/RNode_Firmware.ino dispatch on that single flag), this just
// tells the release handlers below which of the two owns it right now.
// The CONFIRM case itself is detected the same way vault_unlock_prompt()'s
// own live-submit is - a direct raw-hold poll inside vault_identity_
// confirm()'s own loop - these handlers only need to cover CANCEL (a
// release before the hold threshold is reached).
bool vault_confirm_mode = false;
bool vault_confirm_cancelled = false;

inline void vault_pin_reset() {
	vault_pin_len = 0;
	vault_pin_buf[0] = 0;
	vault_pin_current_digit = 0;
	vault_pin_submitted = false;
	vault_pin_submit_fired = false;
	vault_ui_dirty = true;
}

inline void vault_unlock_confirm_digit() {
	if (vault_pin_len < VAULT_PIN_MAX_LEN) {
		vault_pin_buf[vault_pin_len++] = '0' + vault_pin_current_digit;
		vault_pin_buf[vault_pin_len] = 0;
		vault_pin_current_digit = 0;
		// Standard "confirm select" click (buzzer_encoder_click_melody(),
		// used throughout Menu.h for the same purpose) - the generic hold-
		// warning tone is suppressed during vault_unlock_active (Encoder.h's
		// own comment on that), so this is the only audio feedback digit
		// entry gets.
		buzzer_encoder_click_melody();
		// Auto-submit at the max length - confirmed via live testing that
		// requiring a separate long-hold gesture here was a dead end users
		// couldn't discover on their own (the screen gave no indication a
		// different gesture was needed once "confirm digit" stopped doing
		// anything visible at the cap). Shorter PINs (down to
		// VAULT_PIN_MIN_LEN) still need the explicit long-hold submit -
		// see vault_unlock_try_submit() - only the "already at max, nothing
		// left to enter" case is auto-resolved.
		if (vault_pin_len == VAULT_PIN_MAX_LEN) {
			vault_pin_submitted = true;
		}
	}
	vault_ui_dirty = true;
}

inline void vault_unlock_backspace() {
	if (vault_pin_len > 0) {
		vault_pin_len--;
		vault_pin_buf[vault_pin_len] = 0;
	}
	vault_ui_dirty = true;
}

inline void vault_unlock_try_submit() {
	if (vault_pin_len >= VAULT_PIN_MIN_LEN) {
		vault_pin_submitted = true;
	}
	vault_ui_dirty = true;
}

// Encoder.h forward-declares these two and dispatches to them (instead of
// menu_encoder_rotate()/menu_encoder_button()) whenever vault_unlock_active
// is set.
inline void vault_unlock_encoder_rotate(int8_t dir) {
	if (vault_confirm_mode) return; // hold/tap only - nothing to change
	vault_pin_current_digit = (uint8_t)((vault_pin_current_digit + dir + 10) % 10);
	vault_ui_dirty = true;
}

inline void vault_unlock_encoder_button(unsigned long duration) {
	if (vault_confirm_mode) { vault_confirm_cancelled = true; return; }
	// Submit threshold raised from 1500ms to 3000ms to match the button-
	// only path's own tiers below - "hold 3s" is now the same real-world
	// gesture (and the same footer caption, vault_unlock_draw()) on both
	// input methods.
	if (duration < 400) vault_unlock_confirm_digit();
	else if (duration < 3000) vault_unlock_backspace();
	else vault_unlock_try_submit();
}

// RNode_Firmware.ino's button_event() dispatches EVENT_BUTTON_CLICK here
// (instead of into its normal display-blank/menu/BT/sleep tiers) whenever
// vault_unlock_active is set - the only path on encoder-less boards.
inline void vault_unlock_button_press(unsigned long duration) {
	if (vault_confirm_mode) { vault_confirm_cancelled = true; return; }
	if (duration < 400) vault_unlock_encoder_rotate(1);
	else if (duration < 1200) vault_unlock_confirm_digit();
	else if (duration < 3000) vault_unlock_backspace();
	else vault_unlock_try_submit();
}

// Mirrors Menu.h's own standard-128x64-board layout constants (that
// header's #else branch, MENU_HEADER_TEXT_Y/MENU_HEADER_HLINE_Y/
// MENU_LIST_FOOTER_TEXT_Y/MENU_LIST_FOOTER_HLINE_Y/MENU_CONTENT_W) rather
// than using them directly - Menu.h isn't included yet at this point in
// Utilities.h's chain (Vault.h/VaultUnlock.h come first, since urns_init()
// needs them). Both of this feature's target boards are the standard
// (non-T096/T114) SSD1306 128x64 category those constants are tuned for,
// where MENU_GFX is just an alias for `display` too - drawing straight to
// `display` here matches what MENU_GFX would resolve to anyway. Keep in
// sync with Menu.h's own values if either ever changes.
#define VAULT_HEADER_TEXT_Y 4
#define VAULT_HEADER_HLINE_Y 7
#define VAULT_FOOTER_TEXT_Y 62
#define VAULT_FOOTER_HLINE_Y 55
#define VAULT_CONTENT_X 4
#define VAULT_CONTENT_W 120

// Fixed-position digit slots (not sliding as digits are entered, unlike
// the original version - confirmed via live user feedback that a slick,
// menu-consistent look needed static positions) - VAULT_PIN_MAX_LEN slots,
// each VAULT_SLOT_PITCH px apart, centered as one block on the 128px-wide
// screen. 10px digit glyph (bm_pin_digits, Graphics.h) + 4px gap = 14px
// pitch; 8 slots = 112px total, comfortably inside the 120px content width
// with room either side.
#define VAULT_SLOT_PITCH 14
#define VAULT_DIGIT_W 10
#define VAULT_DIGIT_H 16

inline int16_t vault_slot_center_x(uint8_t slot) {
	int16_t total_w = VAULT_PIN_MAX_LEN * VAULT_SLOT_PITCH;
	int16_t start_x = 64 - total_w / 2;
	return start_x + slot * VAULT_SLOT_PITCH + VAULT_SLOT_PITCH / 2;
}

// Draws straight to `display` (Display.h) - see the comment above for why
// not MENU_GFX. `footer` overrides the default turn/tap instructions with
// a transient message (e.g. "Wrong PIN" retry/throttle screens) - pass
// nullptr during normal entry to show the default instructions instead.
inline void vault_unlock_draw(const char* title, const char* footer = nullptr) {
	display.fillScreen(SSD1306_BLACK);
	display.setTextColor(SSD1306_WHITE);
	display.setFont(SMALL_FONT);
	display.setTextSize(1);

	// Header: title left, "(N/8)" count right - same layout/format as the
	// Messenger keyboard's own byte/hex-progress counter
	// (draw_menu_msngr_keyboard_disp(), Menu.h).
	display.setCursor(6, VAULT_HEADER_TEXT_Y);
	display.print(title);
	{
		char count_buf[16];
		snprintf(count_buf, sizeof(count_buf), " (%u/%u)", (unsigned)vault_pin_len, (unsigned)VAULT_PIN_MAX_LEN);
		int16_t cx1, cy1; uint16_t count_w, count_h;
		display.getTextBounds(count_buf, 0, 0, &cx1, &cy1, &count_w, &count_h);
		display.setCursor(VAULT_CONTENT_X + VAULT_CONTENT_W - (int16_t)count_w, VAULT_HEADER_TEXT_Y);
		display.print(count_buf);
	}
	display.drawFastHLine(VAULT_CONTENT_X, VAULT_HEADER_HLINE_Y, VAULT_CONTENT_W, SSD1306_WHITE);

	// Digit slots, vertically centered in the content region (between the
	// header and footer dividers) and horizontally centered as one fixed-
	// position block. Confirmed digits mask as a small filled dot (never
	// show the actual value); the current, not-yet-confirmed digit shows
	// in the clear via the bold digit font so the user can see what
	// they're about to confirm; not-yet-reached slots show a short dash.
	const int16_t content_top = VAULT_HEADER_HLINE_Y + 1;
	const int16_t content_bottom = VAULT_FOOTER_HLINE_Y - 1;
	const int16_t content_mid_y = (content_top + content_bottom) / 2;
	for (uint8_t i = 0; i < VAULT_PIN_MAX_LEN; i++) {
		int16_t cx = vault_slot_center_x(i);
		if (i < vault_pin_len) {
			display.fillCircle(cx, content_mid_y, 3, SSD1306_WHITE);
		} else if (i == vault_pin_len) {
			uint16_t offset = (uint16_t)vault_pin_current_digit * 32; // 2 bytes/row * 16 rows
			display.drawBitmap(cx - VAULT_DIGIT_W / 2, content_mid_y - VAULT_DIGIT_H / 2,
				bm_pin_digits + offset, VAULT_DIGIT_W, VAULT_DIGIT_H, SSD1306_WHITE);
		} else {
			display.drawFastHLine(cx - 3, content_mid_y, 6, SSD1306_WHITE);
		}
	}

	display.drawFastHLine(VAULT_CONTENT_X, VAULT_FOOTER_HLINE_Y, VAULT_CONTENT_W, SSD1306_WHITE);
	display.setCursor(6, VAULT_FOOTER_TEXT_Y);
	if (footer != nullptr) {
		display.print(footer);
	} else {
		// Per user request: shortened to just the submit gesture - the
		// per-digit confirm/advance tap doesn't need footer real estate,
		// its own click (buzzer_encoder_click_melody(), see
		// vault_unlock_confirm_digit()) is feedback enough.
		#if HAS_ENCODER == true
			display.print("turn:val hold 3s:confirm");
		#else
			display.print("tap:val hold 3s:confirm");
		#endif
	}

	#if DISPLAY_IS_OLED
		display.display();
	#endif
}

// loop() feeds the app-level 25s task watchdog (esp_task_wdt, configured
// with trigger_panic=true in setup()) once per iteration - see
// RNode_Firmware.ino:3113. None of this header's blocking loops ever
// return to loop(), so left unfed they panic-reboot the device mid-flow -
// confirmed live: enrolling an 8-digit PIN (two full prompts, real human
// entry time) reliably exceeded 25s and rebooted before vault_enable()
// ever got to commit ADDR_CONF_VAULT_ENABLED, which is exactly why it came
// back up still OFF. Used both in the input-polling loop below and in
// vault_unlock_boot_screen()'s retry delays (up to 60s otherwise).
inline void vault_wdt_reset() {
	#if MCU_VARIANT == MCU_ESP32 && HAS_URNS == true
		esp_task_wdt_reset();
	#endif
}

inline void vault_wdt_safe_delay(unsigned long ms) {
	unsigned long start = millis();
	while (millis() - start < ms) {
		vault_wdt_reset();
		delay(10);
	}
}

// Blip-tolerant hold tracking, independent of Encoder.h/Input.h's own
// enc_btn_state/button_state debounce - a defensive layer against brief
// contact bounce during a sustained hold (not just at the press/release
// edges, which the existing single-stage ~25ms debounce already handles).
// Confirmed via a live raw-signal diagnostic during development that
// bounce was NOT actually the cause of a since-fixed "requires release"
// bug (the raw signal never dropped even once across a full 3s hold) -
// kept anyway as a reasonable robustness margin for noisier hardware,
// just no longer load-bearing for that specific bug. Rather than a single
// "pressed since" timestamp that a one-frame blip could invalidate, this
// only treats a hold as truly broken once the underlying debounced state
// has read "not pressed" for a full VAULT_HOLD_GRACE_MS stretch.
#define VAULT_HOLD_GRACE_MS 150
unsigned long vault_hold_anchor_ms = 0;
bool vault_hold_active = false;
unsigned long vault_last_seen_pressed_ms = 0;

// Feed with the CURRENT raw-debounced pressed/not-pressed reading (from
// either input path) every loop iteration. Returns how long the hold has
// been considered continuously active (0 if not currently considered
// held).
inline unsigned long vault_track_hold(bool currently_pressed) {
	unsigned long now = millis();
	if (currently_pressed) {
		vault_last_seen_pressed_ms = now;
		if (!vault_hold_active) {
			vault_hold_active = true;
			vault_hold_anchor_ms = now;
		}
	} else if (vault_hold_active && (now - vault_last_seen_pressed_ms) > VAULT_HOLD_GRACE_MS) {
		vault_hold_active = false;
	}
	return vault_hold_active ? (now - vault_hold_anchor_ms) : 0;
}

inline void vault_hold_reset() {
	vault_hold_active = false;
	vault_last_seen_pressed_ms = 0;
	vault_hold_anchor_ms = 0;
}

// Blocking - returns once the user submits a PIN of at least
// VAULT_PIN_MIN_LEN digits. Polls encoder_process()/input_read() directly
// rather than waiting for loop() (which hasn't started yet during the boot
// call site) - both are already safe to call this early: encoder_init()/
// input_init() both run well before this in setup(), and neither function
// touches anything else that isn't already up.
inline void vault_unlock_prompt(const char* title, RNS::Bytes& out_password) {
	vault_unlock_active = true;
	vault_pin_reset();
	vault_hold_reset();

	unsigned long last_draw = 0;
	while (!vault_pin_submitted) {
		vault_wdt_reset();
		// Keeps Display.h's normal idle-blank timer (last_unblank_event)
		// alive while this screen is up - this loop draws directly
		// (vault_unlock_draw()) rather than through display_unblank()'s
		// usual button/encoder-event path, so without this the timer never
		// resets during however long PIN/passphrase entry takes. Confirmed
		// via live testing: without it, the screen blanked instantly the
		// moment control returned to normal operation after a multi-prompt
		// flow (export/import), since the idle timeout had already elapsed
		// in the background the whole time this was drawing something.
		display_unblank();
		#if HAS_ENCODER == true
			encoder_process();
		#endif
		input_read();

		// Live submit check - fires the instant a (blip-tolerant, see
		// vault_track_hold()) hold crosses VAULT_SUBMIT_HOLD_MS, without
		// waiting for release (per user request: release-based submit felt
		// unresponsive). Fed the OR of both input paths' raw debounced
		// state so either physical control can trigger it. held_ms also
		// drives the live footer countdown below.
		bool raw_pressed = false;
		#if HAS_ENCODER == true
			if (enc_btn_state == LOW) raw_pressed = true;
		#endif
		if (button_state == LOW) raw_pressed = true;
		unsigned long held_ms = vault_track_hold(raw_pressed);
		bool is_held = (held_ms > 0) || raw_pressed;
		if (!vault_pin_submit_fired && held_ms >= VAULT_SUBMIT_HOLD_MS) {
			vault_unlock_try_submit();
			vault_pin_submit_fired = true;
		}

		if (vault_ui_dirty || (millis() - last_draw) > 100) {
			if (is_held) {
				char hold_status[24];
				if (held_ms >= VAULT_SUBMIT_HOLD_MS) {
					snprintf(hold_status, sizeof(hold_status), "SUBMITTING...");
				} else {
					snprintf(hold_status, sizeof(hold_status), "hold: %lu.%01lus/3s",
						held_ms / 1000, (held_ms % 1000) / 100);
				}
				vault_unlock_draw(title, hold_status);
			} else {
				// Static turn/tap instructions in the footer (default, footer=
				// nullptr) plus the live "(N/8)" count in the header - see
				// vault_unlock_draw() - together already convey what a per-
				// frame digit-count status used to spell out here.
				vault_unlock_draw(title);
			}
			vault_ui_dirty = false;
			last_draw = millis();
		}
		delay(10);
	}

	out_password.assign((const uint8_t*)vault_pin_buf, vault_pin_len);
	// Wipe the plaintext PIN characters from this buffer now that they've
	// been copied out - secure_zero() on out_password itself is the
	// caller's job once it's done with the derived key.
	memset(vault_pin_buf, 0, sizeof(vault_pin_buf));
	vault_pin_len = 0;

	// The caller's real work (vault_try_unlock()/vault_enable()/
	// vault_disable(), ~1s of PBKDF2+AES-GCM+file I/O) draws nothing of
	// its own - without this, the screen just sits frozen on the last
	// countdown frame for that whole second, which is exactly what looked
	// like "stuck, requires release" during testing (confirmed via the
	// raw p=/h=/u= diagnostic: the hold value itself was correct and
	// frozen at 3.0s because the loop had already exited, not because
	// anything was actually stuck).
	vault_unlock_draw(title, "Verifying...");

	// Deliberately returns immediately, without waiting for release -
	// confirmed via live testing (raw p=/h=/u= diagnostic) that the loop
	// above already exits correctly the instant a hold crosses
	// VAULT_SUBMIT_HOLD_MS; the earlier-suspected "requires release" bug
	// was actually vault_unlock_active staying held by a deliberate
	// wait-for-physical-release step in each top-level caller
	// (vault_unlock_boot_screen()/vault_enroll_flow()/vault_disable_flow()),
	// added to stop a still-held button's eventual release from leaking
	// into Menu.h's normal long-press handling (e.g. "open Settings").
	// Removed per explicit user request to prioritize immediate response.
	// The still-held button's eventual release is now swallowed once by
	// vault_suppress_next_release instead (Encoder.h/RNode_Firmware.ino),
	// rather than blocked on here - non-blocking, and covers the exact
	// same stray-release-into-Settings risk this used to guard against.
	vault_unlock_active = false;
	vault_suppress_next_release = true;
}

// Boot-time retry loop - called from setup() when vault_enabled is true,
// before urns_init() runs. Never returns until vault_try_unlock() succeeds;
// there is no "cancel" path, matching every other boot-blocking condition
// this firmware already has (e.g. OTA recovery). Soft, increasing retry
// delay (no hardware backstop exists in this software-only step - see
// Vault.h's own comment on VAULT_PBKDF2_ITERATIONS - so this is UX
// friction, not real brute-force protection, and is disclosed as such).
inline void vault_unlock_boot_screen() {
	uint8_t attempts = 0;
	while (true) {
		RNS::Bytes password;
		vault_unlock_prompt("Enter PIN", password);
		bool ok = vault_try_unlock(password);
		RNS::secure_zero(password);
		if (ok) {
			return;
		}
		attempts++;
		if (attempts <= 3) {
			vault_unlock_draw("Wrong PIN", "Try again");
			vault_wdt_safe_delay(800);
		} else {
			unsigned long throttle_ms = (unsigned long)1000 << (attempts - 3 > 6 ? 6 : attempts - 3);
			if (throttle_ms > 60000) throttle_ms = 60000;
			vault_unlock_draw("Wrong PIN", "Please wait...");
			vault_wdt_safe_delay(throttle_ms);
		}
	}
}

// Menu.h's PIN Protection "On" flow - prompts once, then again to confirm,
// and only calls vault_enable() if both entries match.
inline bool vault_enroll_flow() {
	RNS::Bytes pw1;
	vault_unlock_prompt("Set PIN", pw1);
	if (pw1.size() < VAULT_PIN_MIN_LEN) {
		RNS::secure_zero(pw1);
		return false;
	}

	RNS::Bytes pw2;
	vault_unlock_prompt("Confirm PIN", pw2);
	bool match = (pw1.size() == pw2.size() && pw1.compare(pw2) == 0);
	RNS::secure_zero(pw2);
	if (!match) {
		RNS::secure_zero(pw1);
		vault_unlock_draw("Mismatch", "PIN not changed");
		vault_wdt_safe_delay(1500);
		return false;
	}

	bool ok = vault_enable(pw1);
	RNS::secure_zero(pw1);
	if (!ok) {
		vault_unlock_draw("Error", "Could not enable");
		vault_wdt_safe_delay(1500);
		return false;
	}
	#if HAS_LXMF == true
		// vault_enable() already left vault_key/vault_unlocked set for this
		// session (see its own comment) - safe to encrypt existing message
		// history now, without asking the user to re-enter the PIN.
		vault_unlock_draw("Migrating...", "Encrypting messages");
		vault_migrate_messages_encrypt();
	#endif
	return true;
}

// Menu.h's PIN Protection "Off" flow - re-verifies the current PIN (not
// just "already unlocked") before decrypting the identity back to
// plaintext - see Vault.h's vault_disable() for why.
inline bool vault_disable_flow() {
	RNS::Bytes pw;
	vault_unlock_prompt("Enter PIN", pw);

	// Verify BEFORE touching the message store - vault_disable() only
	// commits/locks at the very end of its own success path (too late to
	// derive per-peer message subkeys from by then), so a real password
	// check has to happen up front here instead of relying on
	// vault_disable()'s own. Without this, a wrong-PIN attempt could still
	// flatten the whole message store to plaintext even though "disable"
	// never actually succeeds - see vault_verify_password()'s own comment
	// (Vault.h).
	if (!vault_verify_password(pw)) {
		RNS::secure_zero(pw);
		vault_unlock_draw("Wrong PIN", "Not disabled");
		vault_wdt_safe_delay(1500);
		return false;
	}

	#if HAS_LXMF == true
		vault_unlock_draw("Migrating...", "Decrypting messages");
		vault_migrate_messages_decrypt();
	#endif

	bool ok = vault_disable(pw);
	RNS::secure_zero(pw);
	if (!ok) {
		// Password was just verified above, so this should be unreachable
		// in practice - but if it somehow still fails, don't leave the
		// message store silently decrypted while vault_enabled stays true;
		// put it back rather than leaving a state PIN Protection ==ON
		// doesn't actually cover.
		#if HAS_LXMF == true
			vault_migrate_messages_encrypt();
		#endif
		vault_unlock_draw("Error", "Not disabled");
		vault_wdt_safe_delay(1500);
	}
	return ok;
}
