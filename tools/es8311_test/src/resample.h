#pragma once
// Integer-ratio (4x) resampler between 32kHz and 8kHz. 32000/8000 is an
// exact integer ratio, so this is a plain decimate/interpolate-by-4 with a
// shared windowed-sinc anti-alias/anti-image lowpass filter (~3.4kHz
// cutoff, standard voice bandwidth, safely under 8kHz's 4kHz Nyquist).
// Bridges the ES8311's I2S rate (no valid MCLK-less clock-coefficient row
// at 8kHz - see pins.h) and Codec2's fixed 8kHz requirement.

#include <Arduino.h>

namespace Resample {

// out must hold at least inCount/4 samples. Returns samples written.
size_t downsample4x(const int16_t *in, size_t inCount, int16_t *out);

// out must hold at least (inCount*4) samples. Returns samples written
// (always inCount*4).
size_t upsample4x(const int16_t *in, size_t inCount, int16_t *out);

}  // namespace Resample
