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
bool    gnss_time_valid()  { return gps_parser.time.isValid(); }
uint8_t gnss_time_hour()   { return gps_parser.time.hour(); }
uint8_t gnss_time_minute() { return gps_parser.time.minute(); }
uint8_t gnss_time_second() { return gps_parser.time.second(); }

// Calendar date (UTC) - same sentence/validity timing as gnss_time_*
// above (typically valid before a position fix). Used by RTC.h's
// rtc_sync_gps() to build a full unixtime out of GPS time alone, since
// rtc_set_unixtime() needs a date, not just a time-of-day.
bool     gnss_date_valid() { return gps_parser.date.isValid(); }
uint16_t gnss_date_year()  { return gps_parser.date.year(); }
uint8_t  gnss_date_month() { return gps_parser.date.month(); }
uint8_t  gnss_date_day()   { return gps_parser.date.day(); }

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

// "Module" rather than gnss_chip_name() while probing/absent - the chip
// name (Boards.h's GPS_MODEL) is just a compile-time label for whichever
// chip this board is wired for, not proof it's actually there.
const char *gnss_module_status_text() {
  if (!gnss_enabled)                            return "N/A";
  if (gnss_detect_state == GNSS_DETECT_PRESENT)  return gnss_chip_name();
  if (gnss_detect_state == GNSS_DETECT_ABSENT)   return "N/A";
  return "DETECTING...";
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
  #ifdef PIN_GPS_PPS
    pinMode(PIN_GPS_PPS, INPUT);
  #endif

  // gnss_enabled already reflects the resolved boot-time EEPROM state (set
  // by setup(), RNode_Firmware.ino, before this runs) - this just applies
  // it to the actual hardware.
  gnss_set_enabled(gnss_enabled);
}

// Non-blocking - drains only whatever GPS_SERIAL.available() already has
// buffered, never waits for more. Called every loop() iteration
// (RNode_Firmware.ino), same as encoder_process()/menu_button_process().
void gnss_update() {
  while (gnss_enabled && GPS_SERIAL.available()) {
    gps_parser.encode(GPS_SERIAL.read());
  }

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
}
