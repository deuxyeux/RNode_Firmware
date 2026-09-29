#pragma once
// Small header-only sine-wave sample generator for the tone-out test.

#include <Arduino.h>
#include <math.h>

class ToneGenerator {
public:
  ~ToneGenerator() {
    if (_buf) free(_buf);
  }

  // Generates one full cycle at toneHz for the given sampleRate as int16_t
  // PCM, sized to fit an exact integer number of samples per cycle so the
  // buffer loops with no phase discontinuity. amplitude is 0..1 of full
  // scale (int16_t range).
  bool begin(uint32_t sampleRate, float toneHz, float amplitude) {
    uint16_t n = (uint16_t)roundf(sampleRate / toneHz);
    if (n < 2 || n > 4096) return false;
    int16_t *buf = (int16_t *)malloc(n * sizeof(int16_t));
    if (!buf) return false;
    if (_buf) free(_buf);
    _buf = buf;
    _n = n;
    _pos = 0;
    int16_t peak = (int16_t)(amplitude * 32767.0f);
    for (uint16_t i = 0; i < _n; i++) {
      _buf[i] = (int16_t)(peak * sinf(2.0f * (float)M_PI * i / _n));
    }
    return true;
  }

  int16_t next() {
    if (!_buf) return 0;
    int16_t s = _buf[_pos];
    _pos = (uint16_t)((_pos + 1) % _n);
    return s;
  }

private:
  int16_t *_buf = nullptr;
  uint16_t _n = 0;
  uint16_t _pos = 0;
};
