// Copyright (C) 2026, Mark Qvist

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

// Driver for optional I2C environment sensors - Bosch BMP280 (temperature +
// pressure) and BME280 (temperature + humidity + pressure). Auto-detected at
// boot by probing both of the family's two possible I2C addresses (0x76/
// 0x77 - selected on the breakout board by how its SDO pin is strapped, not
// something firmware can know in advance) rather than requiring the user to
// configure a model/address anywhere - see sensors_init() below.
//
// Doesn't bring up its own I2C bus - only ever compiled in on boards where
// HAS_SENSORS (Utilities.h) is true, which only happens where display_init()
// (Display.h) has already brought up Wire for the board's I2C OLED panel.
// Same bus-sharing convention RTC.h already uses.

#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>

#define SENSOR_MODEL_NONE   0x00
#define SENSOR_MODEL_BMP280 0x01
#define SENSOR_MODEL_BME280 0x02

#define SENSOR_I2C_ADDR_PRIMARY   0x76
#define SENSOR_I2C_ADDR_ALTERNATE 0x77

Adafruit_BME280 bme280_sensor;
Adafruit_BMP280 bmp280_sensor;

uint8_t sensor_model   = SENSOR_MODEL_NONE;
bool    sensor_present = false;

const char *sensor_chip_name() {
  switch (sensor_model) {
    case SENSOR_MODEL_BME280: return "BME280";
    case SENSOR_MODEL_BMP280: return "BMP280";
    default:                  return "NONE";
  }
}

// Probes a single address for both known chip types - BME280 first (its
// begin() only succeeds if the chip ID register actually reads back 0x60,
// Adafruit_BME280.cpp), then BMP280 (same exact-chip-ID check, against
// BMP280_CHIPID/0x58, Adafruit_BMP280.cpp) - so a real chip always wins over
// a false positive, and an empty address correctly probes as absent
// (Adafruit_I2CDevice::begin() itself first checks for a NACK before either
// driver ever gets to the chip ID check).
bool sensors_probe_addr(uint8_t addr) {
  if (bme280_sensor.begin(addr, &Wire)) {
    sensor_model = SENSOR_MODEL_BME280;
    return true;
  }
  if (bmp280_sensor.begin(addr, BMP280_CHIPID)) {
    sensor_model = SENSOR_MODEL_BMP280;
    return true;
  }
  return false;
}

// Run once at boot (see setup(), RNode_Firmware.ino) - no periodic re-scan,
// same one-shot-presence-check reasoning as rtc_init() (RTC.h): a sensor
// that's physically wired in doesn't come and go at runtime.
bool sensors_init() {
  sensor_model   = SENSOR_MODEL_NONE;
  sensor_present = sensors_probe_addr(SENSOR_I2C_ADDR_PRIMARY) || sensors_probe_addr(SENSOR_I2C_ADDR_ALTERNATE);

  DEBUG_LOG("[Sensors] init: present=%d model=%s\r\n", sensor_present, sensor_chip_name());

  return sensor_present;
}

// Plain on-demand reads, not cached - each call goes straight to the chip
// over I2C, same as rtc_get_unixtime() (RTC.h). Only ever called from the
// Settings menu's Sensors page while it's actually open (Menu.h), so the
// extra bus traffic per redraw is negligible in practice. Callers are
// expected to check sensor_present (and, for humidity, sensor_model ==
// SENSOR_MODEL_BME280) first - same "check *_has_fix()/*_present before
// reading" convention as GNSS.h/RTC.h.
float sensor_temperature_c() {
  if (sensor_model == SENSOR_MODEL_BME280) return bme280_sensor.readTemperature();
  return bmp280_sensor.readTemperature();
}

// Only ever valid on a BME280 - the BMP280 has no humidity element.
float sensor_humidity_percent() {
  return bme280_sensor.readHumidity();
}

// Raw Pascals, straight from whichever driver is active - the unit both
// libraries' own readPressure() already return, and what CMD_SENSOR
// (Framing.h/Utilities.h) sends over the wire, so it stays lossless there.
float sensor_pressure_pa() {
  return (sensor_model == SENSOR_MODEL_BME280) ? bme280_sensor.readPressure() : bmp280_sensor.readPressure();
}

float sensor_pressure_hpa() {
  return sensor_pressure_pa() / 100.0f;
}
