#pragma once
// Simple PCM record/playback buffers, allocated once from PSRAM. Mono
// int16_t samples at whatever rate begin() is called with (AUDIO_SAMPLE_RATE
// in practice - see pins.h). Holds two buffers: the raw as-recorded audio,
// and an optional "processed" result (e.g. a Codec2 encode/decode round
// trip, see codec2_roundtrip.h) so both can be played back and compared
// without re-processing.

#include <Arduino.h>

namespace Recorder {

// Allocates both sample buffers (maxSeconds worth each, from PSRAM).
// Returns false if either allocation fails (check Serial output either way).
bool begin(uint32_t sampleRate, uint32_t maxSeconds);

void startRecording();
// Appends up to `count` samples to the raw buffer; returns how many were
// actually written (less than count, possibly 0, once the buffer fills -
// caller should stop recording when this returns less than count).
size_t writeSamples(const int16_t *samples, size_t count);
void stopRecording();

// Raw (as-recorded) buffer direct access, for batch processing (e.g.
// Codec2 encode) - separate from the streaming playback API below so the
// two don't fight over playback position state.
const int16_t *rawSamples();
size_t rawLength();

// Stores a processed result (e.g. a Codec2 round trip) in the second
// buffer, for "Play Codec2"-style comparison playback. Truncates to
// maxSamples() if count is larger. Returns false if count is 0 or nothing
// is allocated.
bool setProcessedResult(const int16_t *samples, size_t count);
size_t processedLength();

enum class PlaySource { RAW, PROCESSED };
void startPlayback(PlaySource src = PlaySource::RAW);
// Fills up to `count` samples from the current playback position in
// whichever source startPlayback() selected; returns how many were
// actually read (0 once playback has reached the end).
size_t readSamples(int16_t *out, size_t count);
void stopPlayback();

bool hasRecording();
size_t recordedSamples();  // raw buffer length
size_t maxSamples();       // per-buffer capacity
uint32_t sampleRate();
size_t playbackPosition();

}  // namespace Recorder
