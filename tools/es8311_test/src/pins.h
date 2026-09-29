#pragma once

// MeshAdventurer-S3 (ESP32-S3) ES8311 wiring. Cross-checked against
// ../../RNode_Firmware/Boards.h's BOARD_MESHADVENTURER_S3 block (0xF2) -
// none of these are claimed by the real firmware's radio/display/RTC/
// buzzer/encoder/GPS/button pins.

// I2C control bus (ES8311 register interface). Same physical bus as the
// real firmware's SDA_OLED/SCL_OLED (Display.h) - shared with the OLED
// (0x3C) and a DS3231 RTC (0x68) on real hardware, though this isolated
// test never talks to either of those.
#define I2C_SDA 41
#define I2C_SCL 42

// I2S audio bus. No MCLK pin - the ES8311 derives its internal clock from
// BCLK instead (see es8311_driver.cpp).
#define I2S_BCK  12
#define I2S_WS   13
#define I2S_DOUT 14  // ESP32 -> codec (codec's DI pin, DAC/speaker path)
#define I2S_DIN  38  // codec -> ESP32 (codec's DO pin, ADC/mic path)

// Codec enable/shutdown control.
#define PIN_CODEC_EN 46
// UNVERIFIED assumption: active-high "enable" convention. If the I2C probe
// never ACKs despite correct wiring/address, flip this to LOW and re-flash.
#define CODEC_EN_ACTIVE HIGH

// ES8311 I2C address depends on the board's CE/AD0 strap - both are
// auto-probed at boot (see main.cpp).
#define ES8311_ADDR_CE_LOW  0x18
#define ES8311_ADDR_CE_HIGH 0x19

// Audio format. Only rates with an exact-match row in the ES8311's
// MCLK-less clock-coefficient table work: 22050, 32000, 44100, 48000,
// 64000, 88200 or 96000 Hz (see es8311_driver.cpp for why - 8000, 11025,
// 12000, 16000 and 24000 Hz do NOT work without a real MCLK). 32kHz mono
// 16-bit is a reasonable default for a voice bring-up test.
#define AUDIO_SAMPLE_RATE 32000

// OLED (SSD1306), same I2C bus as the codec (I2C_SDA/I2C_SCL above) - a
// third device on that bus alongside the ES8311, matching the real
// firmware's shared SDA_OLED/SCL_OLED (Display.h). No reset pin.
#define DISP_W 128
#define DISP_H 64
#define DISP_ADDR 0x3C
#define DISP_RST -1

// EC11 rotary encoder + push button, same pins as the real firmware's
// BOARD_MESHADVENTURER_S3 block (Boards.h: PIN_ENCODER_UP/DOWN/PRESS).
#define PIN_ENCODER_UP    9
#define PIN_ENCODER_DOWN  10
#define PIN_ENCODER_PRESS 11

// Record/playback buffer, allocated from PSRAM (see recorder.cpp) - this
// board's Octal PSRAM has 8MB total and nothing else in this isolated
// sketch competes for it, so 30s mono 16-bit @ AUDIO_SAMPLE_RATE (~1.9MB
// at 32kHz) is a comfortably safe default with room to raise if needed.
#define AUDIO_MAX_RECORD_SECONDS 30
