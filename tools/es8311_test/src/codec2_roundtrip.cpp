#include "codec2_roundtrip.h"
#include "resample.h"
#include "esp_heap_caps.h"

#include "codec2.h"

namespace Codec2RT {

// Real codec2.h CODEC2_MODE_* constants (0/1/2/3/4/5/8 - 6 and 7 are
// unused/reserved upstream). Sideband's LXMF.AM_CODEC2_* wire-protocol
// byte values (0x01-0x09) are a *different* numbering - this table only
// needs to produce the right codec2Mode int for codec2_create(), the
// LXMF-side mapping is out of scope for this standalone test firmware.
const Mode MODES[MODE_COUNT] = {
    {"700C", CODEC2_MODE_700C}, {"1200", CODEC2_MODE_1200}, {"1300", CODEC2_MODE_1300},
    {"1400", CODEC2_MODE_1400}, {"1600", CODEC2_MODE_1600}, {"2400", CODEC2_MODE_2400},
    {"3200", CODEC2_MODE_3200},
};

Result roundTrip(const int16_t *rawSamples32k, size_t rawCount, int modeIndex,
                  int16_t *outSamples32k, size_t outCapacity) {
  Result r{false, 0, 0};
  if (modeIndex < 0 || modeIndex >= MODE_COUNT || rawCount == 0) return r;

  // Downsample 32kHz -> 8kHz. Scratch buffers all come from PSRAM - this
  // is a one-shot batch pass, not a hot loop, so heap_caps_malloc/free per
  // call is fine.
  uint32_t t0 = millis();
  size_t downCapacity = rawCount / 4 + 1;
  int16_t *down8k = (int16_t *)heap_caps_malloc(downCapacity * sizeof(int16_t), MALLOC_CAP_SPIRAM);
  if (!down8k) return r;
  size_t downCount = Resample::downsample4x(rawSamples32k, rawCount, down8k);
  uint32_t t1 = millis();

  // Internal-DRAM before/after codec2_create() - confirms in practice
  // (not just in theory) how much of Codec2's persistent per-instance
  // state actually landed in PSRAM vs internal DRAM, per
  // codec2_psram_alloc.c/kiss_fft.h/nlp.c's redirect - internal DRAM is
  // the scarce resource in the real RNode_Firmware, not overall heap.
  size_t internalBefore = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
  size_t psramBefore = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
  struct CODEC2 *c2 = codec2_create(MODES[modeIndex].codec2Mode);
  if (!c2) {
    free(down8k);
    return r;
  }
  size_t internalAfter = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
  size_t psramAfter = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
  Serial.printf("[C2] codec2_create() cost: internal DRAM -%d bytes, PSRAM -%d bytes\n",
                (int)(internalBefore - internalAfter), (int)(psramBefore - psramAfter));

  int spf = codec2_samples_per_frame(c2);
  int bpf = codec2_bytes_per_frame(c2);
  size_t numFrames = downCount / spf;

  uint8_t *bitstream = (uint8_t *)heap_caps_malloc(numFrames * bpf, MALLOC_CAP_SPIRAM);
  int16_t *decoded8k = (int16_t *)heap_caps_malloc(numFrames * spf * sizeof(int16_t), MALLOC_CAP_SPIRAM);
  if (!bitstream || !decoded8k) {
    codec2_destroy(c2);
    free(down8k);
    if (bitstream) free(bitstream);
    if (decoded8k) free(decoded8k);
    return r;
  }

  for (size_t f = 0; f < numFrames; f++) {
    codec2_encode(c2, bitstream + f * bpf, (short *)(down8k + f * spf));
  }
  uint32_t t2 = millis();
  for (size_t f = 0; f < numFrames; f++) {
    codec2_decode(c2, (short *)(decoded8k + f * spf), bitstream + f * bpf);
  }
  uint32_t t3 = millis();

  codec2_destroy(c2);
  free(down8k);

  size_t decoded8kCount = numFrames * spf;
  size_t up32kCount = decoded8kCount * 4;
  if (up32kCount > outCapacity) {
    // Defensive only - callers are expected to size outCapacity >=
    // rawCount, and the round trip never produces more samples than it
    // started with after frame-boundary truncation.
    up32kCount = outCapacity - (outCapacity % 4);
    decoded8kCount = up32kCount / 4;
  }

  size_t written = Resample::upsample4x(decoded8k, decoded8kCount, outSamples32k);
  uint32_t t4 = millis();
  free(decoded8k);
  free(bitstream);

  Serial.printf("[C2] timing: downsample=%ums encode=%ums decode=%ums upsample=%ums total=%ums\n",
                (unsigned)(t1 - t0), (unsigned)(t2 - t1), (unsigned)(t3 - t2), (unsigned)(t4 - t3),
                (unsigned)(t4 - t0));

  r.ok = true;
  r.encodedBytes = numFrames * bpf;
  r.decodedSamples = written;
  return r;
}

}  // namespace Codec2RT
