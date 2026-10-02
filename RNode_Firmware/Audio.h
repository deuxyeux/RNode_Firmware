// Audio.h - ES8311 codec + Codec2 voice support (HAS_AUDIO boards only).
//
// Ported from the isolated bring-up project tools/es8311_test/ (see that
// directory's README for the hardware findings). Phase 0: codec power/probe,
// MCLK-less I2S bring-up values, 32kHz<->8kHz resampler and one-shot Codec2
// encode/decode. No UI. Everything here is blocking and must run on a
// dedicated task, never loop() - Codec2 needs ~32KB of stack.

#ifndef AUDIO_H
#define AUDIO_H

#include "Boards.h"

#if HAS_AUDIO == true

#include <Arduino.h>

#define AUDIO_SAMPLE_RATE 32000  // only rates with an MCLK-less coefficient row work
#define AUDIO_CODEC2_RATE 8000

// Power the codec, probe it on the shared Wire bus (Display.h already owns
// Wire.begin), read the chip ID, and power it back down. Returns true if the
// ES8311 ACKed; addr/id1/id2 (each optional) report what was seen. Safe to
// call once at boot. Does not log - Serial is the KISS channel, the caller
// uses DEBUG_LOG.
// Last audio_probe() result, kept so a later loop() can report it - DEBUG_LOG
// output from setup() is dropped before loop() starts.
extern bool audio_probe_done, audio_probe_found;
extern uint8_t audio_probe_addr, audio_probe_id1, audio_probe_id2;

bool audio_probe(uint8_t *addr = nullptr, uint8_t *id1 = nullptr, uint8_t *id2 = nullptr);

// Number of Codec2 frames' worth of bytes produced for `samples32k` input
// samples in the given codec2.h CODEC2_MODE_* mode, or 0 on bad mode.
size_t audio_c2_encoded_size(int codec2_mode, size_t samples32k);

// Resample 32k -> 8k, then Codec2-encode. Returns bytes written to `out`
// (0 on failure/insufficient capacity). Scratch buffers come from PSRAM.
size_t audio_c2_encode(int codec2_mode, const int16_t *pcm32k, size_t samples32k,
                       uint8_t *out, size_t out_cap);

// Codec2-decode, then resample 8k -> 32k. Returns 32kHz samples written to
// `pcm32k` (0 on failure/insufficient capacity).
size_t audio_c2_decode(int codec2_mode, const uint8_t *in, size_t in_len,
                       int16_t *pcm32k, size_t pcm_cap);


// ---- LXMF FIELD_AUDIO (0x07) mode bytes (LXMF.AM_CODEC2_*) ----
// Raw Codec2 frames, concatenated, no header (Sideband audioproc.py).
struct AudioModeInfo {
  uint8_t lxmf_mode;
  int codec2_mode;       // codec2.h CODEC2_MODE_*
  uint8_t bytes_per_frame;
  uint8_t ms_per_frame;
};
// Returns nullptr for modes this device can't decode (450/450PWB/Opus/...).
const AudioModeInfo *audio_mode_info(uint8_t lxmf_mode);
// Playback length in ms for `bytes` of a given LXMF mode (0 if unsupported).
uint32_t audio_duration_ms(uint8_t lxmf_mode, size_t bytes);

// ---- Playback ----
// Codec register I/O shares Wire with the display, so audio_codec_up()/
// _down()/audio_poll() must only be called from the loop task. The decode+
// I2S work runs on its own core-0 task and never touches Wire.
enum AudioState : uint8_t { AUDIO_IDLE = 0, AUDIO_DECODING, AUDIO_PLAYING, AUDIO_DONE };

// Start playing `len` bytes of LXMF-mode Codec2 frames. `data` must come from
// heap_caps_malloc (PSRAM ok); ownership transfers to the audio task (it is
// freed there, also on failure). Returns false if busy/unsupported (data is
// freed in that case too).
bool audio_play_lxmf(uint8_t lxmf_mode, uint8_t *data, size_t len, uint8_t volume_pct);
void audio_play_stop();
AudioState audio_state();
uint32_t audio_play_elapsed_ms();
uint32_t audio_play_total_ms();
// Loop-task housekeeping: powers the codec down once playback finished.
void audio_poll();
// Stack high-water mark (bytes free at worst) of the last audio task run.
extern uint32_t audio_task_stack_hwm;
// Last playback diagnostics (set by the audio task / audio_play_lxmf, read by
// the heartbeat in Messenger.h - Audio.cpp can't DEBUG_LOG itself).
extern char audio_diag[160];
extern volatile uint32_t audio_diag_seq;

#endif  // HAS_AUDIO
#endif  // AUDIO_H
