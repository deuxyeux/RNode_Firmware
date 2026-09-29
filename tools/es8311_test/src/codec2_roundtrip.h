#pragma once
// Batch encode+decode round trip through Codec2, for gauging how the
// actual bitrate LXMF/Sideband uses for voice messages sounds. Not a
// real-time streaming path - runs as one blocking pass over an already
// -recorded 32kHz buffer (Codec2 has run in real time on far weaker MCUs
// than this for over a decade, so batch is not a real-time-budget concern
// here; simplicity matters more for a listening-quality test).

#include <Arduino.h>

namespace Codec2RT {

struct Mode {
  const char *name;  // bps label, matches LXMF's AM_CODEC2_* naming
  int codec2Mode;    // real codec2.h CODEC2_MODE_* constant
};

extern const Mode MODES[7];
constexpr int MODE_COUNT = 7;
// "2400" - Sideband's own "Low-bandwidth Voice" default (sbapp/main.py:2476
// in the local ~/Development/Sideband checkout), not 1200.
constexpr int DEFAULT_MODE_INDEX = 5;

struct Result {
  bool ok;
  size_t encodedBytes;    // compressed bitstream size
  size_t decodedSamples;  // at 32kHz, ready to hand to I2S playback
};

// rawSamples32k/rawCount: the original recording. outSamples32k must have
// room for at least rawCount samples (the round trip never produces more
// samples than it started with, after frame-boundary truncation).
Result roundTrip(const int16_t *rawSamples32k, size_t rawCount, int modeIndex,
                  int16_t *outSamples32k, size_t outCapacity);

}  // namespace Codec2RT
