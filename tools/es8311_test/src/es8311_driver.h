#pragma once
// Minimal ES8311 register driver for MCLK-less operation (the codec derives
// its internal clock from BCLK, since this board's MCLK pin is not wired).
//
// Register map, init sequence and clock-coefficient values are ported from
// Espressif's esp-bsp "es8311" component
// (https://github.com/espressif/esp-bsp, components/es8311, Apache-2.0),
// fetched 2026-09-28, specialized down to the single "16-bit, MCLK-less"
// clock configuration this test needs - see es8311_driver.cpp for the
// coefficient-table derivation.

#include <Arduino.h>
#include <Wire.h>

namespace ES8311Reg {
  constexpr uint8_t RESET     = 0x00;
  constexpr uint8_t CLK_MGR1  = 0x01;
  constexpr uint8_t CLK_MGR2  = 0x02;
  constexpr uint8_t CLK_MGR3  = 0x03;
  constexpr uint8_t CLK_MGR4  = 0x04;
  constexpr uint8_t CLK_MGR5  = 0x05;
  constexpr uint8_t CLK_MGR6  = 0x06;
  constexpr uint8_t CLK_MGR7  = 0x07;
  constexpr uint8_t CLK_MGR8  = 0x08;
  constexpr uint8_t SDP_IN    = 0x09;  // DAC serial port (word length/format)
  constexpr uint8_t SDP_OUT   = 0x0A;  // ADC serial port (word length/format)
  constexpr uint8_t SYSTEM_0D = 0x0D;
  constexpr uint8_t SYSTEM_0E = 0x0E;
  constexpr uint8_t SYSTEM_12 = 0x12;
  constexpr uint8_t SYSTEM_13 = 0x13;
  constexpr uint8_t SYSTEM_14 = 0x14;
  constexpr uint8_t ADC_16    = 0x16;
  constexpr uint8_t ADC_17    = 0x17;
  constexpr uint8_t ADC_1C    = 0x1C;
  constexpr uint8_t DAC_31    = 0x31;
  constexpr uint8_t DAC_32    = 0x32;
  constexpr uint8_t DAC_37    = 0x37;
  constexpr uint8_t CHIPID1   = 0xFD;
  constexpr uint8_t CHIPID2   = 0xFE;
}

// ADC (mic) PGA gain steps - raw values match ES8311Reg::ADC_16's scale,
// matching Espressif's es8311_mic_gain_t enum (0=0dB ... 7=42dB).
enum ES8311MicGain : uint8_t {
  MIC_GAIN_0DB = 0, MIC_GAIN_6DB, MIC_GAIN_12DB, MIC_GAIN_18DB,
  MIC_GAIN_24DB, MIC_GAIN_30DB, MIC_GAIN_36DB, MIC_GAIN_42DB
};

class ES8311 {
public:
  explicit ES8311(TwoWire &wire = Wire) : _wire(wire), _addr(0) {}

  // Probes 0x18 then 0x19 for an I2C ACK, then reads back CHIPID1/2
  // (0xFD/0xFE) as a stronger confirmation. Prints diagnostics to Serial
  // either way. Returns true and latches the working address on success.
  bool probe();

  // Full register init sequence for MCLK-less (BCLK-as-MCLK) operation at
  // 16-bit resolution. sampleRate must be one of the rates documented in
  // es8311_driver.cpp as having a valid clock-coefficient row in this mode
  // - anything else fails cleanly and prints why. Requires probe() to have
  // succeeded first. Leaves the DAC unmuted at 0% volume - call setVolume()
  // afterwards.
  bool init(uint32_t sampleRate);

  void setVolume(uint8_t percent);      // DAC digital volume, 0-100
  void setMicGain(ES8311MicGain gain);  // ADC/mic PGA gain
  void setMute(bool mute);

  bool readReg(uint8_t reg, uint8_t &val);

  uint8_t address() const { return _addr; }

private:
  bool writeReg(uint8_t reg, uint8_t val);

  TwoWire &_wire;
  uint8_t _addr;
};
