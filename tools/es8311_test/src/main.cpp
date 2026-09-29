// ES8311 hardware bring-up test for MeshAdventurer-S3 (ESP32-S3).
// Isolated throwaway firmware - see README.md.

#include <Arduino.h>
#include <Wire.h>
#include <ESP_I2S.h>
#include <string.h>
#include "esp_heap_caps.h"

#include "pins.h"
#include "es8311_driver.h"
#include "tone_gen.h"
#include "encoder.h"
#include "recorder.h"
#include "oled_ui.h"
#include "codec2_roundtrip.h"

static ES8311 codec;
static I2SClass i2s;
static ToneGenerator toneGen;  // not "tone" - collides with Arduino.h's global tone()
static bool oledOk = false;

enum class Mode { IDLE, TONE, MIC_METER, LOOPBACK, RECORDING, PLAYING };
static Mode mode = Mode::IDLE;
// Home menu: 0=Record, 1=Play Raw, 2=Play Codec2, 3=Mode (cycles on click).
static int homeCursor = 0;
static int codec2ModeIndex = Codec2RT::DEFAULT_MODE_INDEX;
static uint32_t modeStartMs = 0;

// Set just before entering Mode::PLAYING, read by doPlayingChunk() for the
// OLED label/duration - "Raw" vs "Codec2 <bps>" and their differing sample
// counts (recordedSamples() vs the round trip's decodedSamples).
static char playLabel[24] = "Raw";
static size_t playTotalSamples = 0;

static const size_t CHUNK_SAMPLES = 256;
static int16_t txBuf[CHUNK_SAMPLES];
static int16_t rxBuf[CHUNK_SAMPLES];

static void printHelp() {
  Serial.println();
  Serial.println("--- ES8311 bring-up commands ---");
  Serial.println("  ?  - this help");
  Serial.println("  i  - re-probe I2C bus / CHIPID");
  Serial.println("  r  - re-run ES8311 register init");
  Serial.println("  t  - toggle: play continuous 1kHz tone (speaker test)");
  Serial.println("  m  - toggle: live mic RMS level meter");
  Serial.println("  l  - toggle: mic -> speaker loopback (full duplex)");
  Serial.println("  v<0-100> - set DAC volume percent, e.g. v70");
  Serial.println("  any other key - stop current mode (back to idle)");
  Serial.println("  --- OLED + encoder ---");
  Serial.println("  turn encoder - move menu cursor (Record/Play Raw/Play Codec2/Mode)");
  Serial.println("  click encoder - select / cycle Codec2 mode / stop whatever is running");
  Serial.println("---------------------------------");
}

static bool initCodecAndBus() {
  pinMode(PIN_CODEC_EN, OUTPUT);
  digitalWrite(PIN_CODEC_EN, CODEC_EN_ACTIVE);
  Serial.println("[GPIO46] Codec enable pin driven " +
                  String(CODEC_EN_ACTIVE == HIGH ? "HIGH" : "LOW") +
                  " (assumed 'enable' convention - UNVERIFIED against "
                  "schematic/silkscreen; flip CODEC_EN_ACTIVE in pins.h if "
                  "the I2C probe below fails).");
  delay(30);  // let the codec's power rails settle before I2C traffic

  Wire.begin(I2C_SDA, I2C_SCL);
  if (!codec.probe()) return false;
  if (!codec.init(AUDIO_SAMPLE_RATE)) return false;

  codec.setMicGain(MIC_GAIN_24DB);
  codec.setVolume(70);
  return true;
}

static bool initI2S() {
  i2s.setPins(I2S_BCK, I2S_WS, I2S_DOUT, I2S_DIN /* mclk omitted -> -1 */);
  bool ok = i2s.begin(I2S_MODE_STD, AUDIO_SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT,
                       I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT, I2S_ROLE_MASTER);
  Serial.printf("[I2S] Full-duplex STD init %s (BCK=%d WS=%d DOUT=%d DIN=%d, "
                "%d Hz mono 16-bit)\n",
                ok ? "OK" : "FAILED", I2S_BCK, I2S_WS, I2S_DOUT, I2S_DIN,
                AUDIO_SAMPLE_RATE);
  return ok;
}

static void enterMode(Mode m) {
  // Leave the current mode cleanly before switching.
  if (mode == Mode::RECORDING) Recorder::stopRecording();
  if (mode == Mode::PLAYING) Recorder::stopPlayback();

  mode = m;
  modeStartMs = millis();
  switch (m) {
    case Mode::IDLE:
      Serial.println("[MODE] idle");
      if (oledOk) oled_draw_home(homeCursor, Recorder::hasRecording(), Codec2RT::MODES[codec2ModeIndex].name);
      break;
    case Mode::TONE:
      Serial.println("[MODE] tone-out (1kHz) - press any key to stop");
      break;
    case Mode::MIC_METER:
      Serial.println("[MODE] mic level meter - press any key to stop");
      break;
    case Mode::LOOPBACK:
      Serial.println("[MODE] mic->speaker loopback - press any key to stop");
      break;
    case Mode::RECORDING:
      Serial.println("[MODE] recording - click encoder (or any serial key) to stop");
      Recorder::startRecording();
      break;
    case Mode::PLAYING:
      // Recorder::startPlayback(source)/playLabel/playTotalSamples are set
      // by the caller (startRawPlayback()/runCodec2AndPlay() below) before
      // entering this mode, since the source (raw vs processed) has to be
      // chosen first.
      Serial.printf("[MODE] playing back (%s) - click encoder (or any serial key) to stop\n", playLabel);
      break;
  }
}

static void doToneChunk() {
  for (size_t i = 0; i < CHUNK_SAMPLES; i++) txBuf[i] = toneGen.next();
  i2s.write((const void *)txBuf, sizeof(txBuf));
}

static void doMicMeterChunk() {
  size_t got = i2s.readBytes((char *)rxBuf, sizeof(rxBuf));
  size_t n = got / sizeof(int16_t);
  if (n == 0) return;

  static uint32_t lastPrint = 0;
  static double sumSq = 0;
  static size_t count = 0;
  for (size_t i = 0; i < n; i++) {
    sumSq += (double)rxBuf[i] * (double)rxBuf[i];
    count++;
  }
  if (millis() - lastPrint >= 100) {  // ~10Hz
    lastPrint = millis();
    double rms = count ? sqrt(sumSq / count) : 0;
    sumSq = 0;
    count = 0;
    int bars = (int)constrain(map((long)rms, 0, 8000, 0, 40), 0, 40);
    Serial.print("[MIC] rms=");
    Serial.print((int)rms);
    Serial.print("\t");
    for (int i = 0; i < bars; i++) Serial.print('#');
    Serial.println();
  }
}

static void doLoopbackChunk() {
  size_t got = i2s.readBytes((char *)rxBuf, sizeof(rxBuf));
  if (got > 0) {
    i2s.write((const void *)rxBuf, got);
  }
}

static void doRecordingChunk() {
  size_t got = i2s.readBytes((char *)rxBuf, sizeof(rxBuf));
  size_t n = got / sizeof(int16_t);
  if (n == 0) return;

  double sumSq = 0;
  for (size_t i = 0; i < n; i++) sumSq += (double)rxBuf[i] * (double)rxBuf[i];
  double rms = sqrt(sumSq / n);
  int level = (int)constrain(map((long)rms, 0, 8000, 0, 100), 0, 100);

  size_t written = Recorder::writeSamples(rxBuf, n);

  static uint32_t lastDraw = 0;
  if (oledOk && millis() - lastDraw >= 150) {
    lastDraw = millis();
    uint32_t maxMs = (uint32_t)((uint64_t)Recorder::maxSamples() * 1000 / Recorder::sampleRate());
    oled_draw_recording(millis() - modeStartMs, maxMs, level);
  }

  if (written < n) {
    Serial.println("[REC] buffer full - stopping");
    homeCursor = 1;  // land on Play Raw after filling up
    enterMode(Mode::IDLE);
  }
}

static void doPlayingChunk() {
  size_t n = Recorder::readSamples(txBuf, CHUNK_SAMPLES);
  if (n == 0) {
    Serial.println("[PLAY] finished");
    enterMode(Mode::IDLE);
    return;
  }
  i2s.write((const void *)txBuf, n * sizeof(int16_t));

  static uint32_t lastDraw = 0;
  if (oledOk && millis() - lastDraw >= 150) {
    lastDraw = millis();
    uint32_t totalMs = (uint32_t)((uint64_t)playTotalSamples * 1000 / Recorder::sampleRate());
    uint32_t elapsedMs = (uint32_t)((uint64_t)Recorder::playbackPosition() * 1000 / Recorder::sampleRate());
    oled_draw_playing(elapsedMs, totalMs, playLabel);
  }
}

static void startRawPlayback() {
  strncpy(playLabel, "Raw", sizeof(playLabel));
  playTotalSamples = Recorder::recordedSamples();
  Recorder::startPlayback(Recorder::PlaySource::RAW);
  enterMode(Mode::PLAYING);
}

static void runCodec2AndPlay() {
  const char *modeName = Codec2RT::MODES[codec2ModeIndex].name;
  if (oledOk) oled_draw_processing(modeName);
  Serial.printf("[C2] encoding+decoding at %s...\n", modeName);

  size_t rawCount = Recorder::rawLength();
  size_t cap = Recorder::maxSamples();
  int16_t *scratch = (int16_t *)heap_caps_malloc(cap * sizeof(int16_t), MALLOC_CAP_SPIRAM);
  if (!scratch) {
    Serial.println("[C2] scratch allocation FAILED");
    return;
  }

  Codec2RT::Result res = Codec2RT::roundTrip(Recorder::rawSamples(), rawCount, codec2ModeIndex, scratch, cap);
  if (!res.ok) {
    Serial.println("[C2] round trip FAILED");
    free(scratch);
    return;
  }

  Recorder::setProcessedResult(scratch, res.decodedSamples);
  free(scratch);

  float seconds = (float)rawCount / Recorder::sampleRate();
  float bitrateKbps = seconds > 0 ? (res.encodedBytes * 8.0f / 1000.0f) / seconds : 0;
  Serial.printf("[C2] %u bytes encoded, ~%.2f kbps, %u samples decoded\n", (unsigned)res.encodedBytes,
                bitrateKbps, (unsigned)res.decodedSamples);

  if (oledOk) {
    oled_draw_codec2_result(res.encodedBytes, bitrateKbps, (uint32_t)(seconds * 1000));
    delay(1200);  // let the compression numbers stay readable before playback starts
  }

  snprintf(playLabel, sizeof(playLabel), "Codec2 %s", modeName);
  playTotalSamples = res.decodedSamples;
  Recorder::startPlayback(Recorder::PlaySource::PROCESSED);
  enterMode(Mode::PLAYING);
}

static void handleCommand(char c) {
  switch (c) {
    case '?':
      printHelp();
      break;
    case 'i': {
      bool wasOk = codec.probe();
      Serial.println(wasOk ? "[CMD] I2C probe OK" : "[CMD] I2C probe FAILED");
      break;
    }
    case 'r': {
      bool ok = codec.init(AUDIO_SAMPLE_RATE);
      if (ok) {
        codec.setMicGain(MIC_GAIN_24DB);
        codec.setVolume(70);
      }
      Serial.println(ok ? "[CMD] ES8311 re-init OK" : "[CMD] ES8311 re-init FAILED");
      break;
    }
    case 't':
      enterMode(mode == Mode::TONE ? Mode::IDLE : Mode::TONE);
      break;
    case 'm':
      enterMode(mode == Mode::MIC_METER ? Mode::IDLE : Mode::MIC_METER);
      break;
    case 'l':
      enterMode(mode == Mode::LOOPBACK ? Mode::IDLE : Mode::LOOPBACK);
      break;
    case 'v': {
      // handled in loop() below where the numeric suffix is read
      break;
    }
    default:
      if (mode != Mode::IDLE) enterMode(Mode::IDLE);
      break;
  }
}

static void handleEncoder() {
  int8_t rot = encoder_read_rotation();
  bool click = encoder_read_click();

  if (mode == Mode::IDLE && rot != 0) {
    homeCursor = constrain(homeCursor + rot, 0, 3);
    if (oledOk) oled_draw_home(homeCursor, Recorder::hasRecording(), Codec2RT::MODES[codec2ModeIndex].name);
  }

  if (click) {
    if (mode != Mode::IDLE) {
      enterMode(Mode::IDLE);
      return;
    }
    switch (homeCursor) {
      case 0:
        enterMode(Mode::RECORDING);
        break;
      case 1:
        if (Recorder::hasRecording()) startRawPlayback();
        else Serial.println("[CMD] no recording yet");
        break;
      case 2:
        if (Recorder::hasRecording()) runCodec2AndPlay();
        else Serial.println("[CMD] no recording yet");
        break;
      case 3:
        codec2ModeIndex = (codec2ModeIndex + 1) % Codec2RT::MODE_COUNT;
        Serial.printf("[CMD] Codec2 mode -> %s\n", Codec2RT::MODES[codec2ModeIndex].name);
        if (oledOk) oled_draw_home(homeCursor, Recorder::hasRecording(), Codec2RT::MODES[codec2ModeIndex].name);
        break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);  // give the USB-CDC host time to enumerate before the first prints
  Serial.println("\n\n=== ES8311 bring-up test (MeshAdventurer-S3) ===");

  bool codecOk = initCodecAndBus();
  bool i2sOk = codecOk && initI2S();

  if (!codecOk || !i2sOk) {
    Serial.println("[SETUP] Init did not fully succeed - fix the issue above, "
                    "then press 'i' or 'r' to retry without a full reboot.");
  }

  toneGen.begin(AUDIO_SAMPLE_RATE, 1000.0f, 0.28f);  // 1kHz, ~28% amplitude

  // OLED shares the codec's I2C bus (Wire.begin(I2C_SDA, I2C_SCL) already
  // ran in initCodecAndBus() above) - oled_init() must not re-run
  // Wire.begin() with default pins, see oled_ui.cpp.
  oledOk = oled_init();

  encoder_begin();
  Recorder::begin(AUDIO_SAMPLE_RATE, AUDIO_MAX_RECORD_SECONDS);

  if (oledOk) oled_draw_home(homeCursor, Recorder::hasRecording(), Codec2RT::MODES[codec2ModeIndex].name);

  printHelp();
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'v') {
      int percent = Serial.parseInt();
      codec.setVolume((uint8_t)constrain(percent, 0, 100));
      Serial.printf("[CMD] volume set to %d%%\n", constrain(percent, 0, 100));
    } else if (c != '\n' && c != '\r') {
      handleCommand(c);
    }
  }

  handleEncoder();

  switch (mode) {
    case Mode::TONE:      doToneChunk();      break;
    case Mode::MIC_METER: doMicMeterChunk();  break;
    case Mode::LOOPBACK:  doLoopbackChunk();  break;
    case Mode::RECORDING: doRecordingChunk(); break;
    case Mode::PLAYING:   doPlayingChunk();   break;
    case Mode::IDLE:      delay(5);           break;
  }
}
