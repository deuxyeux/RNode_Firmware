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

// Driver for the DS3231MZ RTC (MeshPoE-S3, and any future HAS_RTC board).
// Talks to it over the default Wire bus - doesn't bring the bus up itself,
// see rtc_init() below for why. Exposes get/set of the RTC's time as a
// plain unix timestamp; the KISS CMD_TIME glue that calls into this lives
// in Utilities.h / RNode_Firmware.ino, alongside the other CMD_ handlers.

#include <Wire.h>

#define RTC_I2C_ADDR 0x68

// DEBUG_UART_BEGIN()/DEBUG_LOG() - see Utilities.h (DEBUG_UART_ENABLED)
// for what these do and how to turn them on/off, and Boards.h
// (HAS_DEBUG_UART) for which boards have a free UART to use. Not
// RTC-specific - defined in Utilities.h so any file can use them for
// future debug needs, not just this one.

bool rtc_present = false;

uint8_t rtc_bcd2bin(uint8_t val) { return val - 6 * (val >> 4); }
uint8_t rtc_bin2bcd(uint8_t val) { return val + 6 * (val / 10); }

// Howard Hinnant's days-from-civil / civil-from-days algorithms (public
// domain) - used instead of <time.h>'s mktime/gmtime so this driver has
// no libc timezone/DST behavior to worry about; the RTC and CMD_TIME both
// deal purely in UTC unix time.
int32_t rtc_days_from_civil(int32_t y, uint32_t m, uint32_t d) {
  y -= (m <= 2);
  int32_t era = (y >= 0 ? y : y - 399) / 400;
  uint32_t yoe = (uint32_t)(y - era * 400);
  uint32_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  uint32_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + (int32_t)doe - 719468;
}

void rtc_civil_from_days(int32_t z, int32_t &y, uint32_t &m, uint32_t &d) {
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

// Doesn't call Wire.begin() itself - on both current HAS_RTC boards
// (MeshPoE-S3, MeshAdventurer-S3), the RTC is on the same physical bus as
// the OLED (SDA_OLED/SCL_OLED, Boards.h/Display.h), and display_init()
// (Display.h) already brings it up before rtc_init() runs (see setup(),
// RNode_Firmware.ino) - a second Wire.begin() here would just be a
// redundant re-init of the same bus. A future HAS_RTC board with no
// display of its own would need its own explicit Wire.begin() somewhere
// before rtc_init() runs.
bool rtc_init() {
  Wire.beginTransmission(RTC_I2C_ADDR);
  rtc_present = (Wire.endTransmission() == 0);

  DEBUG_LOG("[RTC] init: present=%d\r\n", rtc_present);

  return rtc_present;
}

// Returns 0 if the RTC isn't present or couldn't be read.
uint32_t rtc_get_unixtime() {
  if (!rtc_present) return 0;

  Wire.beginTransmission(RTC_I2C_ADDR);
  Wire.write((uint8_t)0x00);
  if (Wire.endTransmission() != 0) return 0;
  if (Wire.requestFrom((int)RTC_I2C_ADDR, 7) != 7) return 0;

  uint8_t sec_r = Wire.read();
  uint8_t min_r = Wire.read();
  uint8_t hour_r = Wire.read();
  Wire.read(); // day-of-week, unused
  uint8_t date_r = Wire.read();
  uint8_t month_r = Wire.read();
  uint8_t year_r = Wire.read();

  uint8_t seconds = rtc_bcd2bin(sec_r & 0x7F);
  uint8_t minutes = rtc_bcd2bin(min_r & 0x7F);

  uint8_t hours;
  if (hour_r & 0x40) { // 12-hour mode
    hours = rtc_bcd2bin(hour_r & 0x1F) % 12;
    if (hour_r & 0x20) hours += 12; // PM
  } else {
    hours = rtc_bcd2bin(hour_r & 0x3F);
  }

  uint32_t date = rtc_bcd2bin(date_r & 0x3F);
  uint32_t month = rtc_bcd2bin(month_r & 0x1F);
  int32_t year = 2000 + rtc_bcd2bin(year_r) + ((month_r & 0x80) ? 100 : 0);

  int32_t days = rtc_days_from_civil(year, month, date);

  return (uint32_t)days * 86400UL + (uint32_t)hours * 3600UL + (uint32_t)minutes * 60UL + seconds;
}

// True once the RTC holds a plausible time - a DS3231 that's never been
// set (or lost its backup battery) reads back 2000-01-01 00:00:00, so
// year == 2000 is what "never set" looks like in practice. Used to gate
// the Date/Time info page (Display.h) so it doesn't show a meaningless
// default time.
bool rtc_time_valid() {
  if (!rtc_present) return false;

  uint32_t epoch = rtc_get_unixtime();
  int32_t days = (int32_t)(epoch / 86400UL);
  int32_t y; uint32_t m, d;
  rtc_civil_from_days(days, y, m, d);

  return y != 2000;
}

// Display-only UTC offset (set via tz_conf_save(), Utilities.h, from the
// RTC menu's Timezone field) - the RTC chip, rtc_set_unixtime()/
// rtc_get_unixtime(), CMD_TIME, and rtc_sync_ntp() all stay strictly UTC;
// this offset is applied only when rendering time for a human to read
// (Menu.h's RTC page, Display.h's Date/Time info page). Deliberately not a
// full timezone - no DST rules, no IANA database, just a fixed
// minutes-from-UTC shift, per [[project note: "simple time display
// offset" requested over full timezone support]].
//
// Stored as raw+64 quarter-hours (not a plain signed value) so 0x00/0xFF
// (unset/erased EEPROM) fall outside the valid range and unambiguously
// mean "never configured" - same convention as vsr_conf_save()/
// bvs_conf_save() (Utilities.h).
#define TZ_OFFSET_QH_MIN    -48  // UTC-12:00
#define TZ_OFFSET_QH_MAX     56  // UTC+14:00
#define TZ_OFFSET_RAW_ZERO   64  // raw EEPROM byte encoding UTC+00:00 (qh=0)

int8_t rtc_get_tz_offset_qh() {
  #if HAS_EEPROM
    uint8_t raw = EEPROM.read(eeprom_addr(ADDR_CONF_TZ));
  #elif MCU_VARIANT == MCU_NRF52
    uint8_t raw = eeprom_read(eeprom_addr(ADDR_CONF_TZ));
  #endif
  int16_t qh = (int16_t)raw - TZ_OFFSET_RAW_ZERO;
  if (qh < TZ_OFFSET_QH_MIN || qh > TZ_OFFSET_QH_MAX) return 0; // unset/erased -> UTC
  return (int8_t)qh;
}

// Shifts a UTC unix timestamp by the configured display offset - the
// result is NOT a real unix time (it's "local wall-clock seconds", the
// same trick this driver's epoch/civil-calendar math already works on
// regardless) - only ever feed it to rtc_days_from_civil()/
// rtc_civil_from_days() for display, never back into rtc_set_unixtime()
// or over the wire.
uint32_t rtc_apply_tz_offset(uint32_t utc_epoch) {
  return (uint32_t)((int64_t)utc_epoch + (int64_t)rtc_get_tz_offset_qh() * 900);
}

bool rtc_set_unixtime(uint32_t epoch) {
  if (!rtc_present) return false;

  int32_t days = (int32_t)(epoch / 86400UL);
  uint32_t rem = epoch % 86400UL;
  uint8_t hours = rem / 3600; rem %= 3600;
  uint8_t minutes = rem / 60;
  uint8_t seconds = rem % 60;

  int32_t year; uint32_t month, date;
  rtc_civil_from_days(days, year, month, date);

  uint8_t century = 0;
  int32_t yy = year - 2000;
  if (yy >= 100) { century = 0x80; yy -= 100; }

  // DS3231 doesn't interpret the day-of-week register, it's just stored
  // and left to alarms/the host to make sense of - any consistent 1-7
  // mapping works; 1970-01-01 (days=0) was a Thursday.
  uint8_t dow = (uint8_t)(((days % 7) + 7 + 3) % 7) + 1;

  Wire.beginTransmission(RTC_I2C_ADDR);
  Wire.write((uint8_t)0x00);
  Wire.write(rtc_bin2bcd(seconds));
  Wire.write(rtc_bin2bcd(minutes));
  Wire.write(rtc_bin2bcd(hours)); // bit 6 = 0 -> 24-hour mode
  Wire.write(dow);
  Wire.write(rtc_bin2bcd((uint8_t)date));
  Wire.write(rtc_bin2bcd((uint8_t)month) | century);
  Wire.write(rtc_bin2bcd((uint8_t)yy));

  return Wire.endTransmission() == 0;
}

#if MCU_VARIANT == MCU_ESP32 && (HAS_WIFI == true || HAS_ETHERNET == true)

#define NTP_SYNC_TIMEOUT_MS 5000
// NTP_SERVER itself lives in Config.h (not RTC-specific, and overridable
// via build flag - e.g. -DNTP_SERVER='"time.cloudflare.com"' - for
// swapping servers without editing source, same pattern Boards.h's
// HAS_ENCODER diagnostic guard already uses).

// rtc_sync_ntp()'s return value - a reason code rather than a plain bool,
// so callers (the KISS CMD_NTP_SYNC reply, the Settings menu's Sync NTP
// result popup) can actually say *why* a sync failed instead of just
// "failed" - RTC not present, no network, the NTP request itself timing
// out (bad DNS/no route/server unreachable), and the RTC write failing are
// all different problems with different fixes.
#define NTP_SYNC_OK             0
#define NTP_SYNC_ERR_NO_RTC     1
#define NTP_SYNC_ERR_NO_NET     2
#define NTP_SYNC_ERR_TIMEOUT    3
#define NTP_SYNC_ERR_RTC_WRITE  4

// Optional progress hook - see rtc_sync_ntp()'s status_cb param. Plain
// text in, no return value; RTC.h stays UI-agnostic (doesn't know or care
// whether anything's listening), the Settings menu (Menu.h) is what
// actually draws something when it passes one in.
typedef void (*rtc_sync_status_cb_t)(const char *status);

// One-shot, on-demand sync only - no periodic/automatic resync anywhere in
// this firmware. Uses the ESP32 core's built-in SNTP client (configTime()/
// getLocalTime()) instead of a hand-rolled UDP NTP client - the minimal
// path to a synced clock. Blocks the caller for up to NTP_SYNC_TIMEOUT_MS
// if the sync doesn't complete in time (see callers - both are explicit,
// rarely-used user actions, not something on a hot path). status_cb, if
// given, is called with a short human-readable stage name as the sync
// progresses - the KISS-triggered path (RNode_Firmware.ino) doesn't pass
// one, only the Settings menu (Menu.h) does, to show live status.
uint8_t rtc_sync_ntp(rtc_sync_status_cb_t status_cb = nullptr) {
  if (!rtc_present) { DEBUG_LOG("[NTP] no RTC present\r\n"); return NTP_SYNC_ERR_NO_RTC; }

  if (status_cb) status_cb("CONNECTING");
  bool net_up = false;
  #if HAS_WIFI == true
    net_up = net_up || wifi_is_connected();
  #endif
  #if HAS_ETHERNET == true
    net_up = net_up || ethernet_is_connected();
  #endif
  if (!net_up) { DEBUG_LOG("[NTP] no network up\r\n"); return NTP_SYNC_ERR_NO_NET; }

  // Manual resolve first, purely for diagnostics - configTime()/
  // getLocalTime() below do their own resolution internally regardless,
  // this just tells us (via DEBUG_LOG) whether a timeout is a DNS problem
  // or the actual NTP exchange itself.
  if (status_cb) status_cb("RESOLVING");
  IPAddress resolved_ip;
  bool dns_ok = (Network.hostByName(NTP_SERVER, resolved_ip) == 1);
  DEBUG_LOG("[NTP] DNS %s -> %s\r\n", NTP_SERVER, dns_ok ? resolved_ip.toString().c_str() : "FAILED");

  if (status_cb) status_cb("SYNCING");
  DEBUG_LOG("[NTP] starting sync, timeout=%ums\r\n", (unsigned)NTP_SYNC_TIMEOUT_MS);
  configTime(0, 0, NTP_SERVER); // UTC, no DST - matches this driver's UTC-only design
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, NTP_SYNC_TIMEOUT_MS)) {
    DEBUG_LOG("[NTP] getLocalTime() timed out\r\n");
    return NTP_SYNC_ERR_TIMEOUT;
  }

  time_t now;
  time(&now);
  DEBUG_LOG("[NTP] synced, epoch=%lu\r\n", (unsigned long)now);
  if (!rtc_set_unixtime((uint32_t)now)) { DEBUG_LOG("[NTP] RTC write failed\r\n"); return NTP_SYNC_ERR_RTC_WRITE; }

  return NTP_SYNC_OK;
}

#endif
