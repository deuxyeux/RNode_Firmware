#include "resample.h"
#include "dualcore.h"
#include <math.h>

namespace {

constexpr int TAPS = 33;
float filter[TAPS];
bool filterReady = false;

void designFilter() {
  // Windowed-sinc lowpass (Hamming window), cutoff at 3400Hz relative to
  // the 32kHz rate - standard voice bandwidth, comfortably under 8kHz's
  // 4kHz Nyquist after decimation.
  const float fc = 3400.0f / 32000.0f;  // normalized cutoff (fraction of Fs)
  const int M = TAPS - 1;
  float sum = 0;
  for (int n = 0; n < TAPS; n++) {
    float x = n - M / 2.0f;
    float sinc = (x == 0.0f) ? 2.0f * fc : sinf(2.0f * (float)M_PI * fc * x) / ((float)M_PI * x);
    float window = 0.54f - 0.46f * cosf(2.0f * (float)M_PI * n / M);
    filter[n] = sinc * window;
    sum += filter[n];
  }
  for (int n = 0; n < TAPS; n++) filter[n] /= sum;  // unity DC gain
  filterReady = true;
}

// Polyphase decomposition of the same prototype filter for interpolate-by-4
// (see upsample4x). Splitting into 4 phases (one per output-sample-mod-4)
// turns "33 taps, mostly skipped via a per-tap idx%4==0 check" into "~8-9
// taps, always real, no modulo" - both algebraically identical (verified
// host-side against the old brute-force form before this went anywhere
// near the device) and the actual speed win, since Xtensa has no hardware
// integer division/modulo and the old form paid for one on every tap of
// every output sample.
constexpr int L = 4;
constexpr int MAX_PHASE_TAPS = (TAPS + L - 1) / L;  // 9

struct Phase {
  float taps[MAX_PHASE_TAPS];
  int count;
  int baseOffset;  // real-sample index = (o>>2) + baseOffset + j
};
Phase phases[L];
bool phasesReady = false;

void designPolyphase() {
  if (!filterReady) designFilter();
  const int half = TAPS / 2;
  for (int p = 0; p < L; p++) {
    int t0 = ((half - p) % L + L) % L;  // first tap index with this phase's residue
    int baseOffset = (p - half + t0) / L;  // exact - t0's construction guarantees divisibility
    int cnt = 0;
    for (int t = t0; t < TAPS; t += L) phases[p].taps[cnt++] = filter[t];
    phases[p].count = cnt;
    phases[p].baseOffset = baseOffset;
  }
  phasesReady = true;
}

inline int16_t clampSample(float v) {
  if (v > 32767.0f) return 32767;
  if (v < -32768.0f) return -32768;
  return (int16_t)v;
}

struct Range {
  const int16_t *in;
  size_t inCount;
  int16_t *out;
  size_t outStart, outEnd;
};

void downsampleRange(void *ctx) {
  Range *r = (Range *)ctx;
  const int half = TAPS / 2;
  for (size_t o = r->outStart; o < r->outEnd; o++) {
    long center = (long)(o * 4);
    float acc = 0;
    for (int t = 0; t < TAPS; t++) {
      long idx = center - half + t;
      if (idx >= 0 && idx < (long)r->inCount) acc += filter[t] * r->in[idx];
    }
    r->out[o] = clampSample(acc);
  }
}

void upsampleRange(void *ctx) {
  Range *r = (Range *)ctx;
  for (size_t o = r->outStart; o < r->outEnd; o++) {
    int p = (int)(o & 3);
    long m = (long)(o >> 2);
    const Phase &ph = phases[p];
    float acc = 0;
    for (int j = 0; j < ph.count; j++) {
      long srcIdx = m + ph.baseOffset + j;
      if (srcIdx >= 0 && srcIdx < (long)r->inCount) acc += ph.taps[j] * r->in[srcIdx];
    }
    r->out[o] = clampSample(acc * 4.0f);  // *4 compensates zero-stuffing attenuation
  }
}

}  // namespace

size_t Resample::downsample4x(const int16_t *in, size_t inCount, int16_t *out) {
  if (!filterReady) designFilter();
  size_t outCount = inCount / 4;
  size_t mid = outCount / 2;
  Range a{in, inCount, out, 0, mid};
  Range b{in, inCount, out, mid, outCount};
  // Halves are independent (disjoint output ranges, shared read-only
  // input) - safe to run concurrently on both cores, no synchronization
  // needed beyond DualCore::runSplit()'s own join.
  DualCore::runSplit(downsampleRange, &a, downsampleRange, &b);
  return outCount;
}

size_t Resample::upsample4x(const int16_t *in, size_t inCount, int16_t *out) {
  if (!phasesReady) designPolyphase();
  size_t outCount = inCount * 4;
  size_t mid = outCount / 2;
  Range a{in, inCount, out, 0, mid};
  Range b{in, inCount, out, mid, outCount};
  DualCore::runSplit(upsampleRange, &a, upsampleRange, &b);
  return outCount;
}
