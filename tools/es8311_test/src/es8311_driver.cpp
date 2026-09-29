#include "es8311_driver.h"
#include "pins.h"

// Coefficient row for MCLK-less (BCLK-as-MCLK) mode at 16-bit resolution.
//
// In this mode the ES8311's internal "virtual MCLK" is computed as
// sample_rate * resolution_bits * 2 (es8311_clock_config() in Espressif's
// es8311.c), then looked up in a fixed table of (mclk, rate) -> divider
// coefficients with no interpolation - only an exact match works.
//
// Walked every row of that table (es8311.c, espressif/esp-bsp master,
// fetched 2026-09-28) against mclk == rate*32 for 16-bit: the rates with a
// matching row are 22050, 32000, 44100, 48000, 64000, 88200 and 96000 Hz,
// and every one of them happens to resolve to the *same* coefficient values
// below. 8000, 11025, 12000, 16000 and 24000 Hz have no matching row in
// MCLK-less mode and cannot be used without wiring a real MCLK.
struct Es8311Coeff {
  uint8_t preDiv, preMulti, adcDiv, dacDiv, fsMode, lrckH, lrckL, bclkDiv, adcOsr, dacOsr;
};
static constexpr Es8311Coeff kMclklessCoeff16Bit = {
  /* preDiv   */ 0x01, /* preMulti */ 0x03, /* adcDiv */ 0x01, /* dacDiv */ 0x01,
  /* fsMode   */ 0x00, /* lrckH    */ 0x00, /* lrckL  */ 0xFF, /* bclkDiv */ 0x04,
  /* adcOsr   */ 0x10, /* dacOsr   */ 0x10,
};

static bool rateSupportedMclkless16Bit(uint32_t rate) {
  switch (rate) {
    case 22050: case 32000: case 44100: case 48000:
    case 64000: case 88200: case 96000:
      return true;
    default:
      return false;
  }
}

bool ES8311::writeReg(uint8_t reg, uint8_t val) {
  _wire.beginTransmission(_addr);
  _wire.write(reg);
  _wire.write(val);
  return _wire.endTransmission() == 0;
}

bool ES8311::readReg(uint8_t reg, uint8_t &val) {
  _wire.beginTransmission(_addr);
  _wire.write(reg);
  if (_wire.endTransmission(false) != 0) {  // repeated start, keep the bus
    return false;
  }
  if (_wire.requestFrom((int)_addr, 1) != 1) {
    return false;
  }
  val = _wire.read();
  return true;
}

bool ES8311::probe() {
  const uint8_t candidates[] = {ES8311_ADDR_CE_LOW, ES8311_ADDR_CE_HIGH};
  for (uint8_t addr : candidates) {
    _wire.beginTransmission(addr);
    if (_wire.endTransmission() == 0) {
      _addr = addr;
      Serial.printf("[I2C] ES8311 ACKed at 0x%02X\n", addr);
      uint8_t id1 = 0, id2 = 0;
      if (readReg(ES8311Reg::CHIPID1, id1) && readReg(ES8311Reg::CHIPID2, id2)) {
        Serial.printf("[I2C] CHIPID = 0x%02X 0x%02X (informational - expected values "
                       "not independently re-verified against the datasheet this "
                       "session, just print and eyeball for consistency across runs)\n",
                       id1, id2);
      } else {
        Serial.println("[I2C] CHIPID read-back failed despite ACK - continuing anyway");
      }
      return true;
    }
  }
  Serial.println("[I2C] ES8311 not found at 0x18 or 0x19 - check wiring, or try "
                  "flipping CODEC_EN_ACTIVE in pins.h if GPIO46's enable polarity "
                  "assumption is wrong");
  _addr = 0;
  return false;
}

bool ES8311::init(uint32_t sampleRate) {
  if (!_addr) {
    Serial.println("[ES8311] init() called before a successful probe()");
    return false;
  }
  if (!rateSupportedMclkless16Bit(sampleRate)) {
    Serial.printf("[ES8311] %lu Hz has no clock-coefficient match in MCLK-less "
                   "16-bit mode (valid: 22050/32000/44100/48000/64000/88200/96000 Hz)\n",
                   (unsigned long)sampleRate);
    return false;
  }
  const Es8311Coeff &c = kMclklessCoeff16Bit;
  bool ok = true;

  // Reset -> power-on command (es8311_init()).
  ok &= writeReg(ES8311Reg::RESET, 0x1F);
  delay(20);
  ok &= writeReg(ES8311Reg::RESET, 0x00);
  ok &= writeReg(ES8311Reg::RESET, 0x80);

  // Clock config: enable all internal clocks, select BCLK as the MCLK
  // source (reg01 bit7) instead of the (unconnected) MCLK pin
  // (es8311_clock_config(), mclk_from_mclk_pin=false path).
  ok &= writeReg(ES8311Reg::CLK_MGR1, 0xBF);  // 0x3F (enable all) | BIT(7)

  // sclk_inverted = false -> clear reg06 bit5, preserving the other bits.
  uint8_t reg06 = 0;
  ok &= readReg(ES8311Reg::CLK_MGR6, reg06);
  reg06 &= ~(1 << 5);
  ok &= writeReg(ES8311Reg::CLK_MGR6, reg06);

  // Clock dividers (es8311_sample_frequency_config()).
  uint8_t reg02 = 0;
  ok &= readReg(ES8311Reg::CLK_MGR2, reg02);
  reg02 = (reg02 & 0x07) | ((c.preDiv - 1) << 5) | (c.preMulti << 3);
  ok &= writeReg(ES8311Reg::CLK_MGR2, reg02);

  ok &= writeReg(ES8311Reg::CLK_MGR3, (c.fsMode << 6) | c.adcOsr);
  ok &= writeReg(ES8311Reg::CLK_MGR4, c.dacOsr);
  ok &= writeReg(ES8311Reg::CLK_MGR5, ((c.adcDiv - 1) << 4) | (c.dacDiv - 1));

  ok &= readReg(ES8311Reg::CLK_MGR6, reg06);
  reg06 = (reg06 & 0xE0) | (c.bclkDiv < 19 ? (uint8_t)(c.bclkDiv - 1) : c.bclkDiv);
  ok &= writeReg(ES8311Reg::CLK_MGR6, reg06);

  uint8_t reg07 = 0;
  ok &= readReg(ES8311Reg::CLK_MGR7, reg07);
  reg07 = (reg07 & 0xC0) | c.lrckH;
  ok &= writeReg(ES8311Reg::CLK_MGR7, reg07);
  ok &= writeReg(ES8311Reg::CLK_MGR8, c.lrckL);

  // Format: force slave mode (ESP32 is the I2S master), 16-bit on both the
  // DAC-in (SDP_IN) and ADC-out (SDP_OUT) serial ports (es8311_fmt_config()
  // / es8311_resolution_config(ES8311_RESOLUTION_16) -> bits[4:2] = 3).
  uint8_t reg00 = 0;
  ok &= readReg(ES8311Reg::RESET, reg00);
  reg00 &= 0xBF;
  ok &= writeReg(ES8311Reg::RESET, reg00);
  ok &= writeReg(ES8311Reg::SDP_IN, 3 << 2);
  ok &= writeReg(ES8311Reg::SDP_OUT, 3 << 2);

  // Power-up sequence (es8311_init()'s tail).
  ok &= writeReg(ES8311Reg::SYSTEM_0D, 0x01);  // power up analog circuitry
  ok &= writeReg(ES8311Reg::SYSTEM_0E, 0x02);  // enable analog PGA + ADC modulator
  ok &= writeReg(ES8311Reg::SYSTEM_12, 0x00);  // power up DAC
  ok &= writeReg(ES8311Reg::SYSTEM_13, 0x10);  // enable output to HP/line drive
  ok &= writeReg(ES8311Reg::ADC_1C, 0x6A);     // ADC EQ bypass, DC-offset cancel
  ok &= writeReg(ES8311Reg::DAC_37, 0x08);     // bypass DAC equalizer

  // Mic config (es8311_microphone_config(dev, /*digital_mic=*/false)):
  // analog mic input, max PGA gain from reg0x14, plus its accompanying
  // ADC digital gain write - both hardcoded this way in the upstream driver.
  ok &= writeReg(ES8311Reg::ADC_17, 0xC8);
  ok &= writeReg(ES8311Reg::SYSTEM_14, 0x1A);

  setMute(false);

  Serial.printf("[ES8311] init %s (rate=%lu Hz, 16-bit, MCLK-less)\n",
                ok ? "OK" : "FAILED (a register write or read-back NACKed)",
                (unsigned long)sampleRate);
  return ok;
}

void ES8311::setVolume(uint8_t percent) {
  if (percent > 100) percent = 100;
  uint8_t reg32 = percent == 0 ? 0 : (uint8_t)(((uint16_t)percent * 256 / 100) - 1);
  writeReg(ES8311Reg::DAC_32, reg32);
}

void ES8311::setMicGain(ES8311MicGain gain) {
  writeReg(ES8311Reg::ADC_16, (uint8_t)gain);
}

void ES8311::setMute(bool mute) {
  uint8_t reg31 = 0;
  readReg(ES8311Reg::DAC_31, reg31);
  if (mute) {
    reg31 |= (1 << 6) | (1 << 5);
  } else {
    reg31 &= ~((1 << 6) | (1 << 5));
  }
  writeReg(ES8311Reg::DAC_31, reg31);
}
