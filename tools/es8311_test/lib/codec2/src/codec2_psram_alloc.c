// PSRAM-backed allocator for Codec2's persistent per-instance state
// (struct CODEC2 itself, Pn/Sn_/w/Sn/bpf_buf, and - via debug_alloc.h's
// MALLOC/FREE macros routing to these when __EMBEDDED__ is defined
// (library.json's build flags) - everything codec2_create()/destroy()
// allocates once and holds for the instance's lifetime.
//
// Deliberately NOT applied to mbest.c's per-frame VQ search scratch
// (700C mode's rate_K_mbest_encode(), called every ~20ms) - that's tiny,
// transient (freed before the next frame), and a hot path where PSRAM's
// higher access latency would cost more than the (negligible, since it
// never accumulates) memory saved by moving it. This file only covers
// the persistent state that actually eats into the real RNode_Firmware's
// constrained internal DRAM budget for as long as a Codec2 instance lives.

#include <stdlib.h>
#include "esp_heap_caps.h"

void *codec2_malloc(size_t size) {
  return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}

void *codec2_calloc(size_t nmemb, size_t size) {
  return heap_caps_calloc(nmemb, size, MALLOC_CAP_SPIRAM);
}

void codec2_free(void *ptr) {
  // ESP-IDF's heap allocator is unified across capability regions - free()
  // correctly releases heap_caps_malloc()'d memory regardless of which
  // capability it was allocated with, same as codec2_malloc's own
  // heap_caps_malloc/free pairing throughout this codebase already relies
  // on (recorder.cpp, codec2_roundtrip.cpp).
  free(ptr);
}
