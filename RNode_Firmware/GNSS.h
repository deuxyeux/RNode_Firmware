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

// Driver for HAS_GPS boards' GNSS receiver (currently: Heltec T096's UC6580,
// Heltec T114's L76K, and MeshAdventurer-S3's optional ATGM336H/AT6558 -
// GPS_MODEL_UC6580/GPS_MODEL_L76K/GPS_MODEL_AT6558, Boards.h). Streams and
// parses standard NMEA sentences off GPS_SERIAL via TinyGPSPlus - no
// chip-specific command sequences, since plain NMEA-over-UART is all this
// basic implementation needs. The menu glue that reads the gnss_*
// accessors below lives in Menu.h, same split RTC.h uses for its own rtc_*
// functions.

#include <TinyGPSPlus.h>

TinyGPSPlus gps_parser;

// Board default (GNSS_ENABLED_DEFAULT, Boards.h) - overridden at boot from
// EEPROM (see setup(), RNode_Firmware.ino, same erased-byte/never-touched
// convention as sound_enabled/SOUND_ENABLED_DEFAULT). True on boards with a
// built-in receiver (T096/T114), so it just works out of the box with no
// menu trip required; false on MeshAdventurer-S3, where the ATGM336H is an
// optional add-on most builds don't have installed.
bool gnss_enabled = GNSS_ENABLED_DEFAULT;

// Manual, session-only override for the generic OLED path's GNSS info panel
// (draw_disp_area(), Display.h) - set true only by the "Show GNSS Banner"
// GNSS-menu action (Menu.h), never persisted to EEPROM and never cleared
// except by reboot. Only actually changes anything while the radio is off:
// it lets the panel open at all in that state (Display.h's outer gate is
// otherwise radio_online-only), where it then stays shown full-time since
// there's no airtime/channel-load data to alternate with anyway. The
// instant radio_online goes true again, Display.h falls back to the normal
// airtime/GNSS alternation on its own - this flag doesn't suppress that.
// An earlier version tied this to radio_online/gnss_enabled automatically
// instead of a menu action, which collided with the idle-status carousel
// (checks passed/hardware OK/version, Display.h) that keeps redrawing over
// the same screen region for the device's whole uptime, not just at boot -
// an explicit user action sidesteps that by skipping the carousel outright
// while pinned, rather than trying to schedule around it.
bool gnss_banner_forced = false;

const char *gnss_chip_name() {
  #if GPS_MODEL == GPS_MODEL_UC6580
    return "UC6580";
  #elif GPS_MODEL == GPS_MODEL_L76K
    return "L76K";
  #elif GPS_MODEL == GPS_MODEL_AT6558
    return "AT6558";
  #else
    return "UNKNOWN";
  #endif
}

bool     gnss_has_fix()         { return gps_parser.location.isValid(); }
uint32_t gnss_satellite_count() { return gps_parser.satellites.isValid() ? gps_parser.satellites.value() : 0; }
double   gnss_latitude()        { return gps_parser.location.lat(); }
double   gnss_longitude()       { return gps_parser.location.lng(); }
double   gnss_altitude_meters() { return gps_parser.altitude.meters(); }

// GPS time (UTC) - broadcast in the same sentences as satellite count, but
// receivers typically sync time before ever achieving a position fix, so
// this populating while satellites/location stay at 0 is a useful
// diagnostic: it confirms sentence parsing is working end-to-end and the
// chip just isn't seeing enough sky yet, rather than a deeper parsing
// problem. Independent of gnss_has_fix() - check its own validity, not
// location's.
//
// Same TinyGPSTime::commit() quirk as gnss_date_valid() below (isValid()
// goes true the moment any sentence's time term is processed at all, even
// if the term itself was blank - a Void-status sentence sent before the
// receiver has a real UTC time), so isValid() alone can read true with
// hour()/minute()/second() all still at the "00:00:00" sentinel. Unlike
// day() for the date sentinel, 00:00:00 is also a legitimate real UTC
// time once a day - but only for the one second the world is actually at
// UTC midnight, so treating an exact zero as "not yet real" is the same
// safe-in-practice tradeoff gnss_date_valid() already makes, not a new
// one. Without this, the tz-shifted display banners (Display.h,
// gnss_local_time()) would render this stale sentinel as a real time in
// whatever timezone is configured (e.g. a device at UTC+3 briefly reading
// "03:00:00" instead of "N/A" right after boot, before any real fix).
bool    gnss_time_valid()  { return gps_parser.time.isValid() && !(gps_parser.time.hour() == 0 && gps_parser.time.minute() == 0 && gps_parser.time.second() == 0); }
uint8_t gnss_time_hour()   { return gps_parser.time.hour(); }
uint8_t gnss_time_minute() { return gps_parser.time.minute(); }
uint8_t gnss_time_second() { return gps_parser.time.second(); }

// Calendar date (UTC) - same sentence/validity timing as gnss_time_*
// above (typically valid before a position fix). Used by RTC.h's
// rtc_sync_gps() to build a full unixtime out of GPS time alone, since
// rtc_set_unixtime() needs a date, not just a time-of-day.
// TinyGPSDate::commit() (TinyGPS++.cpp) marks the date valid whenever an
// RMC sentence's date term was processed at all, even if the term itself
// was blank/zero (e.g. a Void-status RMC sent before the receiver has
// computed a real UTC date) - it never checks the decoded value, so
// isValid() alone can read true while year()/month()/day() come back as
// the "2000-00-00" sentinel (raw date value 0). day() is 1-31 in any real
// date, never 0, so that's a safe general guard against this specific
// library quirk - also protects rtc_sync_gps() (RTC.h), which trusts this
// same wrapper before setting the RTC.
bool     gnss_date_valid() { return gps_parser.date.isValid() && gps_parser.date.day() != 0; }
uint16_t gnss_date_year()  { return gps_parser.date.year(); }
uint8_t  gnss_date_month() { return gps_parser.date.month(); }
uint8_t  gnss_date_day()   { return gps_parser.date.day(); }

// Howard Hinnant's days-from-civil / civil-from-days algorithms (public
// domain) - a self-contained copy for GNSS.h's own use, deliberately not
// shared with RTC.h's identical rtc_days_from_civil()/rtc_civil_from_days()
// (RTC.h only compiles in at all when HAS_RTC is true, so this file can't
// depend on it - see system_time_utc()'s own comment, Utilities.h, for why
// GNSS needs to work as a time source independently of any RTC chip).
int32_t gnss_days_from_civil(int32_t y, uint32_t m, uint32_t d) {
  y -= (m <= 2);
  int32_t era = (y >= 0 ? y : y - 399) / 400;
  uint32_t yoe = (uint32_t)(y - era * 400);
  uint32_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  uint32_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + (int32_t)doe - 719468;
}

void gnss_civil_from_days(int32_t z, int32_t &y, uint32_t &m, uint32_t &d) {
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

// Generic GNSS-seeded system clock - independent of HAS_URNS (unlike the
// urns_sync_time_from_gnss() consumer below, RNode_Firmware.ino, which now
// just reads this) and independent of HAS_RTC (system_time_utc(),
// Utilities.h, prefers a real RTC chip when present and only falls back to
// this). Millis()-anchored: seeded once from a GNSS fix, then free-runs off
// millis() between fixes rather than re-reading raw GNSS date/time fields
// on every call, which would go stale/unavailable the moment the receiver
// loses fix or is duty-cycle-slept (gnss_duty_cycle_update() above).
bool gnss_system_time_synced = false;
uint32_t gnss_system_time_epoch_at_sync = 0; // unix seconds, UTC, at the sync moment
uint32_t gnss_system_time_millis_at_sync = 0;

// Called from gnss_update() below, right after it decodes a fresh NMEA
// sentence - not re-armed after the first success, same one-shot reasoning
// urns_sync_time_from_gnss() originally had: GPS time doesn't drift the way
// a free-running local oscillator does, so there's no ongoing correction to
// make once synced.
void gnss_sync_system_time() {
  if (gnss_system_time_synced) return;
  if (!gnss_date_valid() || !gnss_time_valid()) return;

  int32_t days = gnss_days_from_civil(gnss_date_year(), gnss_date_month(), gnss_date_day());
  gnss_system_time_epoch_at_sync = (uint32_t)days * 86400UL + (uint32_t)gnss_time_hour() * 3600UL +
    (uint32_t)gnss_time_minute() * 60UL + gnss_time_second();
  gnss_system_time_millis_at_sync = millis();
  gnss_system_time_synced = true;
}

bool gnss_system_time_valid() { return gnss_system_time_synced; }

uint32_t gnss_system_time_now() {
  if (!gnss_system_time_synced) return 0;
  return gnss_system_time_epoch_at_sync + (millis() - gnss_system_time_millis_at_sync) / 1000UL;
}

// Module presence auto-detection - relevant on any HAS_GPS board, but
// especially MeshAdventurer-S3's ATGM336H, an optional add-on most builds
// don't have installed (GNSS_ENABLED_DEFAULT false there, Boards.h): a user
// who turns GPS on with no module actually wired would otherwise just see
// permanently-zero Fix/Satellites forever, indistinguishable from "no sky
// view yet". NMEA is a one-way broadcast (no ping/ack to probe with), so
// presence can only be inferred from what's actually arriving on the UART -
// specifically, at least one *complete, checksum-valid* NMEA sentence
// (gps_parser.passedChecksum()) since detection last (re)started, not
// merely any raw byte (gps_parser.charsProcessed()). Confirmed on real
// hardware that raw bytes alone are far too weak a signal: a floating/
// unconnected RX pin with no module wired at all still picks up stray
// electrical noise and racks up a nonzero char count with zero valid
// sentences ever assembled - a real module reliably produces complete
// sentences within a second or two of power-up, noise essentially never
// does. passedChecksum() is (like charsProcessed()) a monotonic
// since-boot counter TinyGPSPlus never resets, so a plain "is it > 0"
// check would wrongly stay PRESENT forever
// after a module that was once seen gets unplugged and GPS is toggled
// off/on again; a baseline snapshot at the start of each detection run
// avoids that.
#define GNSS_DETECT_PROBING 0 // still within its attempt window, no verdict yet
#define GNSS_DETECT_PRESENT 1 // saw a new valid NMEA sentence since detection started
#define GNSS_DETECT_ABSENT  2 // GNSS_DETECT_MAX_ATTEMPTS elapsed with nothing
#define GNSS_DETECT_ATTEMPT_MS   1000 // how often gnss_update() re-checks
#define GNSS_DETECT_MAX_ATTEMPTS 5    // ~5s total before giving up

uint8_t  gnss_detect_state = GNSS_DETECT_PROBING;
uint32_t gnss_detect_baseline_sentences = 0;
uint8_t  gnss_detect_attempts = 0;
unsigned long gnss_detect_last_attempt_ms = 0;

// Called whenever GPS transitions to enabled (gnss_set_enabled() below,
// which covers both gnss_init() at boot and the Settings menu's Enabled
// toggle) - a fresh detection run starts from a fresh baseline every time,
// so a module that gets plugged in after a prior ABSENT verdict, or
// unplugged after a prior PRESENT one, is re-evaluated rather than stuck on
// a stale result.
void gnss_detect_reset() {
  gnss_detect_state = GNSS_DETECT_PROBING;
  gnss_detect_baseline_sentences = gps_parser.passedChecksum();
  gnss_detect_attempts = 0;
  gnss_detect_last_attempt_ms = millis();
}

// Duty-cycled acquisition - lets a board that can actually gate GNSS power
// (GNSS_DUTY_CYCLE_CAPABLE, Boards.h - true wherever a board-specific GPS
// block defined PIN_GPS_EN and/or PIN_GPS_STANDBY) sleep the receiver most
// of the time instead of streaming continuously. Opt-in via the Settings
// menu's GNSS > Update Interval field (Menu.h); GNSS_UPDATE_INTERVAL_
// CONTINUOUS (0, the default on every board) bypasses all of this and
// keeps today's always-on behavior. Boards with no gating pin at all never
// get anything but Continuous - see the per-board audit in Boards.h.
//
// Three states, driven from gnss_duty_cycle_update() below (called from
// gnss_update(), so still non-blocking, still polled every loop()
// iteration):
//   ACTIVE - powered, searching (identical to the always-on behavior).
//   HOLD   - fix acquired; stays awake GNSS_FIX_HOLD_MS longer so the
//            receiver can finish downloading ephemeris/almanac before
//            sleeping (makes the *next* wake a warm/hot start instead of
//            cold) and so any immediate menu/RTC-sync read sees a fresh
//            fix.
//   SLEEP  - receiver powered down (gnss_duty_pins_sleep() below decides
//            soft-standby vs. hard EN-off per board); wakes back to ACTIVE
//            once millis() reaches gnss_duty_wake_at_ms.
#define GNSS_PSTATE_ACTIVE 0
#define GNSS_PSTATE_HOLD   1
#define GNSS_PSTATE_SLEEP  2

#define GNSS_UPDATE_INTERVAL_CONTINUOUS 0UL // seconds - duty-cycle disabled

#define GNSS_FIX_HOLD_MS               10000UL // stay awake this long past a fix before sleeping
#define GNSS_SEARCH_TIMEOUT_MS         90000UL // give up a search with no fix after this long
#define GNSS_SEARCH_TIMEOUT_BACKOFF_MS 45000UL // tightened timeout once a search has already failed once since the last lock
#define GNSS_INITIAL_FIX_TIMEOUT_MS   180000UL // generous timeout while no fix has been obtained yet since GNSS was enabled - a true cold start (no ephemeris/almanac) can run well past the steady-state timeouts above, and the user turned GNSS on expecting to wait for it
#define GNSS_FAILED_SEARCH_RETRY_MS   (5UL * 60UL * 1000UL) // retry this soon (not the full interval) after a failed search
#define GNSS_LOCK_EWMA_WEIGHT          0.2f     // smoothing weight for gnss_duty_predicted_lock_ms

// Selectable Update Interval presets (Menu.h) - a small curated list
// rather than freeform seconds, matching the existing UX for this kind of
// setting (Display Timeout/Brightness). GNSS_DUTY_CYCLE_HARD_ONLY boards
// (T096/T1 - PIN_GPS_EN only, so every wake is a cold start) drop the
// 1-minute preset, since a cycle that short would mostly just be spent
// waiting out the cold start.
#if GNSS_DUTY_CYCLE_HARD_ONLY == true
  const uint32_t gnss_update_interval_presets_s[] = { 0, 300, 900, 3600 };
  #define GNSS_UPDATE_INTERVAL_PRESET_COUNT 4
#else
  const uint32_t gnss_update_interval_presets_s[] = { 0, 60, 300, 900, 3600 };
  #define GNSS_UPDATE_INTERVAL_PRESET_COUNT 5
#endif

uint32_t gnss_update_interval_s = GNSS_UPDATE_INTERVAL_DEFAULT; // 0 = Continuous, seconds otherwise
uint8_t  gnss_pstate = GNSS_PSTATE_ACTIVE;
unsigned long gnss_pstate_entered_ms = 0;
unsigned long gnss_duty_wake_at_ms = 0;
uint32_t gnss_duty_predicted_lock_ms = 0; // 0 = not yet seeded
uint8_t  gnss_duty_lock_count = 0;        // successful locks since the last gnss_duty_reset() - the first is a cold-start outlier, excluded from the predictor (same reasoning Meshtastic's GPSUpdateScheduling uses)
uint8_t  gnss_duty_consecutive_failures = 0;

// Age of the last valid position, in ms - a thin wrapper over TinyGPSPlus's
// own per-field staleness tracking (TinyGPSLocation::age(), TinyGPS++.h),
// not a separately-maintained timestamp: location.isValid() latches true
// forever after the first fix and never resets (confirmed against the
// vendored library), so gnss_has_fix()/gnss_latitude()/etc. already keep
// returning the last-known fix on their own once the receiver stops
// updating - .age() is just what turns that into a "how stale" figure for
// the Settings menu's Fix row (Menu.h).
uint32_t gnss_location_age_ms() { return gps_parser.location.age(); }

// Whether a duty-cycle sleep should cut hard power (PIN_GPS_EN) rather
// than just soft-standby (PIN_GPS_STANDBY). Boards with only one of the
// two pins have no choice to make; boards with both (currently only
// Heltec32_v4) pick based on how long the sleep will be - short intervals
// favor the cheap, fast-rewake standby path, long ones favor cutting
// power outright once standby leakage no longer beats a full power cycle.
// No calibration data for our specific chips exists, unlike Meshtastic's
// empirically-fit power-curve threshold, so this is a simple fixed
// cutover rather than a formula.
#define GNSS_SOFTSLEEP_HARDSLEEP_CUTOVER_S (15UL * 60UL)

bool gnss_duty_sleep_should_be_hard() {
  #if defined(PIN_GPS_EN) && defined(PIN_GPS_STANDBY)
    return gnss_update_interval_s >= GNSS_SOFTSLEEP_HARDSLEEP_CUTOVER_S;
  #elif defined(PIN_GPS_EN)
    return true;
  #else
    return false;
  #endif
}

// Pin-only power control for duty-cycle wake/sleep - unlike
// gnss_set_enabled() below, never touches GPS_SERIAL or gnss_detect_state:
// a duty-cycle wake is a module already confirmed present (detection only
// ever runs once, on the first activation - see gnss_duty_reset()), so
// there's nothing to re-probe, and leaving GPS_SERIAL open the whole time
// costs nothing while the receiver is silent.
void gnss_duty_pins_wake() {
  #ifdef PIN_GPS_EN
    digitalWrite(PIN_GPS_EN, LOW); // active LOW
  #endif
  #ifdef PIN_GPS_STANDBY
    digitalWrite(PIN_GPS_STANDBY, HIGH); // force wake
  #endif
}

void gnss_duty_pins_sleep() {
  bool hard = gnss_duty_sleep_should_be_hard();
  #ifdef PIN_GPS_EN
    if (hard) digitalWrite(PIN_GPS_EN, HIGH); // deassert, cut power
  #endif
  #ifdef PIN_GPS_STANDBY
    // Moot when EN is also being cut (hard==true on a both-pins board) -
    // harmless to leave as-is, same reasoning as gnss_set_enabled()'s own
    // comment on Heltec32_v4's overlap between the two pins.
    if (!hard) digitalWrite(PIN_GPS_STANDBY, LOW); // allow sleep
  #endif
}

// Resets all duty-cycle bookkeeping to a fresh ACTIVE search - called
// whenever GNSS transitions to enabled (gnss_set_enabled(), alongside
// gnss_detect_reset()), so a freshly-enabled module always starts hunting
// immediately rather than picking up mid-cycle state from a previous
// session.
void gnss_duty_reset() {
  gnss_pstate = GNSS_PSTATE_ACTIVE;
  gnss_pstate_entered_ms = millis();
  gnss_duty_predicted_lock_ms = 0;
  gnss_duty_lock_count = 0;
  gnss_duty_consecutive_failures = 0;
}

// Shared by gnss_init() (boot) and the Settings menu's Enabled toggle
// (Menu.h, MENU_STATE_GNSS_EDIT) - the single place that actually power-
// cycles the receiver, so both paths stay in sync. Two independent,
// separately-guarded mechanisms, since they're mechanically different and
// a board may have either, both, or neither:
//  - PIN_GPS_EN: a hard power switch (T096's UC6580) - active LOW.
//  - PIN_GPS_STANDBY: a soft sleep/wake line (T114's L76K, and other
//    L76-family/clone chips) - HIGH forces the chip awake, LOW allows it
//    to sleep (confirmed against Meshtastic's GPS.cpp writePinStandby()
//    and the T114 variant.h's own comment) - independent of any shared
//    VEXT display-power rail, so toggling this never affects the display.
void gnss_set_enabled(bool en) {
  gnss_enabled = en;
  #ifdef PIN_GPS_EN
    digitalWrite(PIN_GPS_EN, en ? LOW : HIGH); // active LOW
  #endif
  #ifdef PIN_GPS_STANDBY
    digitalWrite(PIN_GPS_STANDBY, en ? HIGH : LOW); // HIGH=force wake, LOW=allow sleep
  #endif
  if (en) {
    // nRF52's GPS_SERIAL (a Uart object, cores/nRF5/Uart.cpp) binds its
    // pins at compile time via the constructor args baked into the core's
    // variant.cpp - .begin() there only takes a baud rate. ESP32's
    // HardwareSerial instead routes pins through the GPIO matrix at
    // .begin() time, so PIN_GPS_RX/TX must be passed explicitly there.
    #if MCU_VARIANT == MCU_ESP32
      GPS_SERIAL.begin(GPS_BAUD_RATE, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    #else
      GPS_SERIAL.begin(GPS_BAUD_RATE);
    #endif
    gnss_detect_reset();
    gnss_duty_reset();
  } else {
    GPS_SERIAL.end();
  }
}

void gnss_init() {
  #if BOARD_MODEL == BOARD_HELTEC_T114
    // No dedicated GPS enable pin on this board - it shares PIN_VEXT_EN
    // (Boards.h) with the display. Asserted once here, unconditionally
    // (not tied to gnss_enabled/gnss_set_enabled() below - the power-saving
    // toggle uses PIN_GPS_STANDBY instead, precisely so it can never cut
    // power to the display too).
    pinMode(PIN_VEXT_EN, OUTPUT);
    digitalWrite(PIN_VEXT_EN, HIGH);
  #endif
  #ifdef PIN_GPS_RESET
    // Deassert only - no active reset pulse in this basic implementation,
    // just needs to not be held in reset. Active LOW, needs a >100ms LOW
    // hold to actually reset the chip (see Boards.h) - HIGH is inert.
    pinMode(PIN_GPS_RESET, OUTPUT);
    digitalWrite(PIN_GPS_RESET, HIGH);
  #endif
  #ifdef PIN_GPS_EN
    pinMode(PIN_GPS_EN, OUTPUT);
  #endif
  #ifdef PIN_GPS_STANDBY
    // Never explicitly configured before this - digitalWrite() calls to it
    // (gnss_set_enabled()/gnss_duty_pins_wake()/gnss_duty_pins_sleep()) had
    // no guaranteed electrical effect without this, on any core that
    // doesn't implicitly force OUTPUT mode on first digitalWrite().
    pinMode(PIN_GPS_STANDBY, OUTPUT);
  #endif
  #ifdef PIN_GPS_PPS
    pinMode(PIN_GPS_PPS, INPUT);
  #endif

  // gnss_enabled already reflects the resolved boot-time EEPROM state (set
  // by setup(), RNode_Firmware.ino, before this runs) - this just applies
  // it to the actual hardware.
  gnss_set_enabled(gnss_enabled);
}

// Enters SLEEP and schedules the next wake. On a successful lock (failed
// == false), the wake is scheduled interval_ms minus the predicted lock
// time before the nominal deadline, so a fix is ready by the time it's
// actually due rather than starting the search only at the deadline
// itself. On a failed search, retries sooner (GNSS_FAILED_SEARCH_RETRY_MS,
// capped at the configured interval) rather than waiting out the full
// interval blind - an indoor/no-sky node gets another attempt soon instead
// of going dark for a potentially long configured interval.
void gnss_duty_schedule_sleep(unsigned long now, bool failed) {
  gnss_duty_pins_sleep();
  gnss_pstate = GNSS_PSTATE_SLEEP;
  gnss_pstate_entered_ms = now;

  unsigned long interval_ms = gnss_update_interval_s * 1000UL;
  unsigned long sleep_ms;
  if (failed) {
    sleep_ms = min(interval_ms, (unsigned long)GNSS_FAILED_SEARCH_RETRY_MS);
  } else {
    sleep_ms = (interval_ms > gnss_duty_predicted_lock_ms) ? (interval_ms - gnss_duty_predicted_lock_ms) : 0;
  }
  gnss_duty_wake_at_ms = now + sleep_ms;
}

// Drives the ACTIVE/HOLD/SLEEP state machine (see the block comment above
// gnss_duty_reset()) - only called while gnss_enabled, from gnss_update()
// below. A no-op (forces/keeps ACTIVE) whenever duty-cycling isn't in play
// at all, either because the board can't gate GNSS power
// (GNSS_DUTY_CYCLE_CAPABLE, Boards.h) or the user has it set to
// Continuous - covers both the default state and a live switch back to
// Continuous while asleep, which needs to force the receiver back awake
// immediately rather than leaving it powered down.
void gnss_duty_cycle_update() {
  if (gnss_update_interval_s == GNSS_UPDATE_INTERVAL_CONTINUOUS || !GNSS_DUTY_CYCLE_CAPABLE) {
    if (gnss_pstate != GNSS_PSTATE_ACTIVE) gnss_duty_pins_wake();
    gnss_pstate = GNSS_PSTATE_ACTIVE;
    return;
  }

  unsigned long now = millis();

  if (gnss_pstate == GNSS_PSTATE_ACTIVE) {
    if (gnss_has_fix()) {
      unsigned long lock_ms = now - gnss_pstate_entered_ms;
      gnss_duty_lock_count++;
      if (gnss_duty_lock_count >= 2) {
        gnss_duty_predicted_lock_ms = (gnss_duty_predicted_lock_ms == 0)
          ? lock_ms
          : (uint32_t)((1.0f - GNSS_LOCK_EWMA_WEIGHT) * gnss_duty_predicted_lock_ms + GNSS_LOCK_EWMA_WEIGHT * lock_ms);
      }
      gnss_duty_consecutive_failures = 0;
      gnss_pstate = GNSS_PSTATE_HOLD;
      gnss_pstate_entered_ms = now;
    } else {
      // No fix at all yet since GNSS was enabled (gnss_duty_lock_count == 0,
      // reset alongside gnss_duty_reset()) gets a longer, dedicated timeout
      // regardless of gnss_duty_consecutive_failures - a true cold start can
      // legitimately run past the steady-state search timeouts below, and
      // the user enabled GNSS expecting to wait for that first fix.
      unsigned long timeout;
      if (gnss_duty_lock_count == 0) {
        timeout = GNSS_INITIAL_FIX_TIMEOUT_MS;
      } else {
        timeout = (gnss_duty_consecutive_failures > 0) ? GNSS_SEARCH_TIMEOUT_BACKOFF_MS : GNSS_SEARCH_TIMEOUT_MS;
      }
      if (now - gnss_pstate_entered_ms >= timeout) {
        gnss_duty_consecutive_failures++;
        gnss_duty_schedule_sleep(now, true);
      }
    }
  } else if (gnss_pstate == GNSS_PSTATE_HOLD) {
    if (now - gnss_pstate_entered_ms >= GNSS_FIX_HOLD_MS) {
      gnss_duty_schedule_sleep(now, false);
    }
  } else { // GNSS_PSTATE_SLEEP
    if ((long)(now - gnss_duty_wake_at_ms) >= 0) {
      gnss_duty_pins_wake();
      gnss_pstate = GNSS_PSTATE_ACTIVE;
      gnss_pstate_entered_ms = now;
    }
  }
}

// Settings menu (Menu.h) label for the GNSS page's universal Duty State
// row - distinguishes ACTIVE vs. HOLD (fix already acquired, holding
// awake) rather than folding both into one collapsed word, and reads a
// static "ACTIVE"/"OFF" on boards that can't actually duty-cycle (see
// gnss_duty_cycle_update() above - gnss_pstate never leaves ACTIVE there).
const char *gnss_pstate_text() {
  // gnss_duty_cycle_update() (and thus gnss_pstate itself) only ever runs
  // while gnss_enabled (see gnss_update()) - disabling GNSS just freezes
  // gnss_pstate wherever it was (defaulting to ACTIVE at declaration/
  // reset), which would otherwise misleadingly read as "still searching"
  // while the receiver is actually fully powered off.
  if (!gnss_enabled) return "OFF";
  switch (gnss_pstate) {
    case GNSS_PSTATE_ACTIVE: return "ACTIVE";
    case GNSS_PSTATE_HOLD:   return "HOLD";
    case GNSS_PSTATE_SLEEP:  return "SLEEP";
    default:                 return "?";
  }
}

// Verbose GNSS diagnostics (Menu.h's MENU_STATE_GNSS_DIAG/_GNSS_DIAG_SATS)
// - opt-in per board (HAS_GNSS_DEBUG_MENU, Boards.h) since it adds real
// per-byte parsing cost and pulls in PDOP/VDOP/satellites-in-view via
// GSA/GSV, which stock TinyGPSPlus doesn't parse (only GGA/RMC are
// handled internally - see TinyGPS++.cpp's endOfTermHandler()) and which
// aren't guaranteed to even be in a given module's default NMEA output
// set. TinyGPSCustom's sentence-name match is talker-ID-literal ("GPGSA"/
// "GPGSV" exactly) - a multi-constellation module reporting as $GNGSA/
// $GNGSV instead of $GPxxx will leave these permanently !isValid(), which
// every wrapper below turns into a harmless "N/A"/empty result rather
// than stale or garbage data.
#if HAS_GNSS_DEBUG_MENU == true
  TinyGPSCustom gnss_pdop_field(gps_parser, "GPGSA", 15);
  TinyGPSCustom gnss_vdop_field(gps_parser, "GPGSA", 17);

  // Satellites-in-view (PRN/elevation/azimuth/SNR), assembled from GPGSV -
  // split across multiple sentences (up to 4 satellites each), reassembled
  // below in gnss_gsv_update(). GNSS_SAT_VIEW_MAX is a display cap, not a
  // protocol limit - same "bound the iteration/stack cost, let the menu
  // scroll" reasoning as MENU_URNS_PATH_MAX_ROWS (Menu.h).
  #define GNSS_SAT_VIEW_MAX 16
  TinyGPSCustom gnss_gsv_total_msgs(gps_parser, "GPGSV", 1);
  TinyGPSCustom gnss_gsv_msg_number(gps_parser, "GPGSV", 2);
  TinyGPSCustom gnss_gsv_sat_number[4];
  TinyGPSCustom gnss_gsv_elevation[4];
  TinyGPSCustom gnss_gsv_azimuth[4];
  TinyGPSCustom gnss_gsv_snr[4];

  // TinyGPSCustom::begin() just links each field into gps_parser's custom-
  // element chain - one-time setup, called once from setup() (RNode_
  // Firmware.ino), same "wire it once, poll forever" shape as the rest of
  // this file's initialization.
  void gnss_gsv_fields_init() {
    for (uint8_t i = 0; i < 4; i++) {
      gnss_gsv_sat_number[i].begin(gps_parser, "GPGSV", 4 + 4 * i);
      gnss_gsv_elevation[i].begin(gps_parser, "GPGSV", 5 + 4 * i);
      gnss_gsv_azimuth[i].begin(gps_parser, "GPGSV", 6 + 4 * i);
      gnss_gsv_snr[i].begin(gps_parser, "GPGSV", 7 + 4 * i);
    }
  }

  struct gnss_sat_view_t {
    uint8_t  prn;
    uint8_t  elevation_deg;
    uint16_t azimuth_deg;
    uint8_t  snr_db;
  };

  gnss_sat_view_t gnss_sat_view_staging[GNSS_SAT_VIEW_MAX];   // being rebuilt this round
  uint8_t         gnss_sat_view_staging_count = 0;
  gnss_sat_view_t gnss_sat_view_committed[GNSS_SAT_VIEW_MAX]; // last complete round - what the menu reads
  uint8_t         gnss_sat_view_committed_count = 0;

  // Called from gnss_update()'s existing per-byte loop, after encode() -
  // non-blocking, only does work when a new GPGSV sentence just completed
  // (isUpdated() on the message-number field). Reset-and-rebuild: message
  // number 1 starts a fresh staging pass (previous round's leftovers
  // discarded); every sentence appends up to 4 non-empty satellite slots;
  // once the message number reaches the round's own reported total, the
  // staging buffer is committed as the current view snapshot. GSV round
  // cadence/satellite count varies with sky visibility and receiver update
  // rate, so this only ever trusts what's reported live each round, never
  // a fixed sentence count.
  void gnss_gsv_update() {
    if (!gnss_gsv_msg_number.isUpdated()) return;
    uint8_t msg_num = (uint8_t)atoi(gnss_gsv_msg_number.value());
    uint8_t total_msgs = (uint8_t)atoi(gnss_gsv_total_msgs.value());
    if (msg_num == 0 || total_msgs == 0) return; // malformed/empty term this round, skip

    if (msg_num == 1) gnss_sat_view_staging_count = 0;

    for (uint8_t i = 0; i < 4 && gnss_sat_view_staging_count < GNSS_SAT_VIEW_MAX; i++) {
      const char *prn_str = gnss_gsv_sat_number[i].value();
      if (prn_str[0] == 0) continue; // slot unused in this sentence (the last sentence of a round is often partial)
      gnss_sat_view_t &s = gnss_sat_view_staging[gnss_sat_view_staging_count++];
      s.prn           = (uint8_t)atoi(prn_str);
      s.elevation_deg = (uint8_t)atoi(gnss_gsv_elevation[i].value());
      s.azimuth_deg   = (uint16_t)atoi(gnss_gsv_azimuth[i].value());
      s.snr_db        = (uint8_t)atoi(gnss_gsv_snr[i].value());
    }

    if (msg_num == total_msgs) {
      memcpy(gnss_sat_view_committed, gnss_sat_view_staging, sizeof(gnss_sat_view_t) * gnss_sat_view_staging_count);
      gnss_sat_view_committed_count = gnss_sat_view_staging_count;
    }
  }

  // Fix quality/mode text - TinyGPSLocation::FixQuality()/FixMode()
  // (TinyGPS++.h:57-58/67-68), richer than the plain has-fix bool above.
  const char *gnss_fix_quality_text() {
    if (!gps_parser.location.isValid()) return "N/A";
    switch (gps_parser.location.FixQuality()) {
      case TinyGPSLocation::Invalid:   return "Invalid";
      case TinyGPSLocation::GPS:       return "GPS";
      case TinyGPSLocation::DGPS:      return "DGPS";
      case TinyGPSLocation::PPS:       return "PPS";
      case TinyGPSLocation::RTK:       return "RTK";
      case TinyGPSLocation::FloatRTK:  return "Float RTK";
      case TinyGPSLocation::Estimated: return "Estimated";
      case TinyGPSLocation::Manual:    return "Manual";
      case TinyGPSLocation::Simulated: return "Simulated";
      default:                         return "Unknown";
    }
  }
  const char *gnss_fix_mode_text() {
    if (!gps_parser.location.isValid()) return "N/A";
    switch (gps_parser.location.FixMode()) {
      case TinyGPSLocation::N: return "No Fix";
      case TinyGPSLocation::A: return "Autonomous";
      case TinyGPSLocation::D: return "Differential";
      case TinyGPSLocation::E: return "Estimated";
      default:                 return "Unknown";
    }
  }

  // HDOP is already stock-parsed (gps_parser.hdop, GGA term 8) - just
  // needs a validity guard and a quality-band label, neither of which
  // exists anywhere else in this codebase. Bands are our own convention
  // (not library-provided): <1 excellent, 1-2 good, 2-5 moderate, 5-10
  // fair, >10 poor.
  bool   gnss_hdop_valid() { return gps_parser.hdop.isValid(); }
  double gnss_hdop()       { return gps_parser.hdop.hdop(); }
  const char *gnss_hdop_band_text() {
    if (!gnss_hdop_valid()) return "N/A";
    double h = gnss_hdop();
    if (h < 1.0)  return "Excellent";
    if (h < 2.0)  return "Good";
    if (h < 5.0)  return "Moderate";
    if (h < 10.0) return "Fair";
    return "Poor";
  }

  bool   gnss_pdop_valid() { return gnss_pdop_field.isValid(); }
  double gnss_pdop()       { return atof(gnss_pdop_field.value()); }
  bool   gnss_vdop_valid() { return gnss_vdop_field.isValid(); }
  double gnss_vdop()       { return atof(gnss_vdop_field.value()); }

  // Speed/course - km/h chosen (not knots/mph) to match this file's
  // existing metric-only convention (gnss_altitude_meters() already
  // reports meters, not feet); there's no unit-selection setting anywhere
  // in Config.h/Menu.h to key off of instead.
  bool   gnss_speed_valid()  { return gps_parser.speed.isValid(); }
  double gnss_speed_kmph()   { return gps_parser.speed.kmph(); }
  bool   gnss_course_valid() { return gps_parser.course.isValid(); }
  double gnss_course_deg()   { return gps_parser.course.deg(); }
  const char *gnss_course_cardinal() {
    return gnss_course_valid() ? TinyGPSPlus::cardinal(gnss_course_deg()) : "N/A";
  }

  // Link/parse health - passedChecksum()/failedChecksum()/charsProcessed()
  // already exist unconditionally on gps_parser (TinyGPSPlus core, not a
  // TinyGPSCustom field); wrapped here rather than alongside the always-
  // available wrappers above since nothing outside the diagnostics screen
  // calls them.
  uint32_t gnss_chars_processed() { return gps_parser.charsProcessed(); }
  uint32_t gnss_checksum_passed() { return gps_parser.passedChecksum(); }
  uint32_t gnss_checksum_failed() { return gps_parser.failedChecksum(); }
  uint8_t  gnss_checksum_pass_rate_pct() {
    uint32_t total = gnss_checksum_passed() + gnss_checksum_failed();
    return total == 0 ? 0 : (uint8_t)((gnss_checksum_passed() * 100UL) / total);
  }

  uint32_t gnss_duty_predicted_lock_s() { return gnss_duty_predicted_lock_ms / 1000; }
  // Seconds until gnss_duty_wake_at_ms, 0 if not currently asleep or
  // already past due - same signed-subtraction idiom gnss_duty_cycle_
  // update() itself uses to stay millis()-wraparound-safe.
  uint32_t gnss_duty_wake_countdown_s() {
    if (gnss_pstate != GNSS_PSTATE_SLEEP) return 0;
    long remain_ms = (long)(gnss_duty_wake_at_ms - millis());
    return remain_ms > 0 ? (uint32_t)(remain_ms / 1000) : 0;
  }

  // Satellites-in-view accessors - Menu.h reads these row-by-row rather
  // than being handed the struct array directly, same narrow-accessor
  // convention as every other gnss_* wrapper in this file.
  uint8_t  gnss_sat_view_count()              { return gnss_sat_view_committed_count; }
  uint8_t  gnss_sat_view_prn(uint8_t i)       { return gnss_sat_view_committed[i].prn; }
  uint8_t  gnss_sat_view_elevation(uint8_t i) { return gnss_sat_view_committed[i].elevation_deg; }
  uint16_t gnss_sat_view_azimuth(uint8_t i)   { return gnss_sat_view_committed[i].azimuth_deg; }
  uint8_t  gnss_sat_view_snr(uint8_t i)       { return gnss_sat_view_committed[i].snr_db; }
#endif // HAS_GNSS_DEBUG_MENU

// Non-blocking - drains only whatever GPS_SERIAL.available() already has
// buffered, never waits for more. Called every loop() iteration
// (RNode_Firmware.ino), same as encoder_process()/menu_button_process().
void gnss_update() {
  while (gnss_enabled && GPS_SERIAL.available()) {
    if (gps_parser.encode(GPS_SERIAL.read())) {
      // Fires only on a freshly-decoded sentence, not idle polling. Cheap/
      // no-op most of the time even here: gnss_sync_system_time() early-
      // returns immediately once already synced, and most sentence types
      // (e.g. GSV) don't carry a date/time field anyway.
      gnss_sync_system_time();
      // Pushes the now-synced clock into RNS::Utilities::OS/microStore too,
      // on top of gnss_system_time_* above - see its own comment
      // (RNode_Firmware.ino) for why HAS_URNS boards need that separately.
      #if HAS_URNS == true && HAS_RTC == false
        urns_sync_time_from_gnss();
      #endif
    }
  }

  #if HAS_GNSS_DEBUG_MENU == true
    if (gnss_enabled) gnss_gsv_update();
  #endif

  if (gnss_enabled && gnss_detect_state == GNSS_DETECT_PROBING) {
    if (gps_parser.passedChecksum() > gnss_detect_baseline_sentences) {
      gnss_detect_state = GNSS_DETECT_PRESENT;
    } else if (millis() - gnss_detect_last_attempt_ms >= GNSS_DETECT_ATTEMPT_MS) {
      gnss_detect_last_attempt_ms = millis();
      gnss_detect_attempts++;
      if (gnss_detect_attempts >= GNSS_DETECT_MAX_ATTEMPTS) {
        gnss_detect_state = GNSS_DETECT_ABSENT;
      }
    }
  }

  if (gnss_enabled) gnss_duty_cycle_update();
}
