#include "recorder.h"
#include "esp_heap_caps.h"
#include <string.h>

namespace {
  int16_t *buf = nullptr;          // raw as-recorded buffer
  size_t bufCapacity = 0;          // in samples, per buffer
  size_t bufLen = 0;               // valid recorded samples

  int16_t *procBuf = nullptr;      // processed (e.g. Codec2 round trip) buffer
  size_t procLen = 0;

  size_t playPos = 0;
  Recorder::PlaySource playSource = Recorder::PlaySource::RAW;
  uint32_t rate = 0;
  bool recording = false;
  bool playing = false;
}  // namespace

bool Recorder::begin(uint32_t sampleRate, uint32_t maxSeconds) {
  rate = sampleRate;
  bufCapacity = (size_t)sampleRate * maxSeconds;

  buf = (int16_t *)heap_caps_malloc(bufCapacity * sizeof(int16_t), MALLOC_CAP_SPIRAM);
  procBuf = (int16_t *)heap_caps_malloc(bufCapacity * sizeof(int16_t), MALLOC_CAP_SPIRAM);
  if (!buf || !procBuf) {
    Serial.printf("[REC] PSRAM allocation of %u bytes x2 FAILED\n",
                   (unsigned)(bufCapacity * sizeof(int16_t)));
    bufCapacity = 0;
    return false;
  }
  Serial.printf("[REC] PSRAM buffers OK: %u samples each (%.1fs @ %u Hz, %u KB x2)\n",
                (unsigned)bufCapacity, (float)maxSeconds, (unsigned)sampleRate,
                (unsigned)(bufCapacity * sizeof(int16_t) / 1024));
  return true;
}

void Recorder::startRecording() {
  bufLen = 0;
  procLen = 0;  // a new recording invalidates any prior processed result
  recording = true;
  playing = false;
}

size_t Recorder::writeSamples(const int16_t *samples, size_t count) {
  if (!recording || !buf) return 0;
  size_t space = bufCapacity - bufLen;
  size_t n = count < space ? count : space;
  memcpy(buf + bufLen, samples, n * sizeof(int16_t));
  bufLen += n;
  return n;
}

void Recorder::stopRecording() {
  recording = false;
}

const int16_t *Recorder::rawSamples() { return buf; }
size_t Recorder::rawLength() { return bufLen; }

bool Recorder::setProcessedResult(const int16_t *samples, size_t count) {
  if (!procBuf || count == 0) return false;
  if (count > bufCapacity) count = bufCapacity;
  memcpy(procBuf, samples, count * sizeof(int16_t));
  procLen = count;
  return true;
}

size_t Recorder::processedLength() { return procLen; }

void Recorder::startPlayback(PlaySource src) {
  playPos = 0;
  playSource = src;
  playing = true;
  recording = false;
}

size_t Recorder::readSamples(int16_t *out, size_t count) {
  if (!playing) return 0;
  const int16_t *src = playSource == PlaySource::RAW ? buf : procBuf;
  size_t srcLen = playSource == PlaySource::RAW ? bufLen : procLen;
  if (!src) return 0;
  size_t remaining = srcLen - playPos;
  size_t n = count < remaining ? count : remaining;
  memcpy(out, src + playPos, n * sizeof(int16_t));
  playPos += n;
  return n;
}

void Recorder::stopPlayback() {
  playing = false;
}

bool Recorder::hasRecording() { return bufLen > 0; }
size_t Recorder::recordedSamples() { return bufLen; }
size_t Recorder::maxSamples() { return bufCapacity; }
uint32_t Recorder::sampleRate() { return rate; }
size_t Recorder::playbackPosition() { return playPos; }
