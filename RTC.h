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

// Driver for the DS3231MZ RTC (MeshPoE-S3, and any future HAS_RTC board -
// see Boards.h for the pin_rtc_sda/pin_rtc_scl each such board must
// define). Exposes get/set of the RTC's time as a plain unix timestamp;
// the KISS CMD_TIME glue that calls into this lives in Utilities.h /
// RNode_Firmware.ino, alongside the other CMD_ handlers.

#include <Wire.h>

#define RTC_I2C_ADDR 0x68

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

bool rtc_init() {
  Wire.begin(pin_rtc_sda, pin_rtc_scl);

  Wire.beginTransmission(RTC_I2C_ADDR);
  rtc_present = (Wire.endTransmission() == 0);

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
#define NTP_SERVER "pool.ntp.org"

// One-shot, on-demand sync only - no periodic/automatic resync anywhere in
// this firmware. Uses the ESP32 core's built-in SNTP client (configTime()/
// getLocalTime()) instead of a hand-rolled UDP NTP client - the minimal
// path to a synced clock. Blocks the caller for up to NTP_SYNC_TIMEOUT_MS
// if the sync doesn't complete in time (see callers - both are explicit,
// rarely-used user actions, not something on a hot path).
bool rtc_sync_ntp() {
  if (!rtc_present) return false;

  bool net_up = false;
  #if HAS_WIFI == true
    net_up = net_up || wifi_is_connected();
  #endif
  #if HAS_ETHERNET == true
    net_up = net_up || ethernet_is_connected();
  #endif
  if (!net_up) return false;

  configTime(0, 0, NTP_SERVER); // UTC, no DST - matches this driver's UTC-only design
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, NTP_SYNC_TIMEOUT_MS)) return false;

  time_t now;
  time(&now);
  return rtc_set_unixtime((uint32_t)now);
}

#endif
