// Audio.cpp - see Audio.h. ES8311 register sequence ported from
// tools/es8311_test/src/es8311_driver.cpp (itself ported from Espressif's
// esp-bsp es8311.c, Apache-2.0); resampler from resample.cpp (single-threaded
// here - it runs on the audio task, not a boot-time batch pass).

#include "Audio.h"

#if HAS_AUDIO == true

#include <Wire.h>
#include <math.h>
#include <stdarg.h>
#include "esp_heap_caps.h"
#include "codec2.h"
#include <ESP_I2S.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static void diag(const char *fmt, ...);
static bool audio_worker_ensure();
static TaskHandle_t worker_h = nullptr;
enum WorkerJob : uint8_t { WORKER_NONE = 0, WORKER_PLAY, WORKER_REC };
static volatile WorkerJob worker_job = WORKER_NONE;
static void *volatile worker_arg = nullptr;

namespace {

// ---- ES8311 (MCLK-less, 16-bit; valid rates 22050/32000/44100/48000/...) ----

constexpr uint8_t ES_ADDR_LOW = 0x18;
constexpr uint8_t ES_ADDR_HIGH = 0x19;
constexpr uint8_t REG_CHIPID1 = 0xFD;
constexpr uint8_t REG_CHIPID2 = 0xFE;

uint8_t es_addr = 0;

bool es_read(uint8_t reg, uint8_t &val) {
  Wire.beginTransmission(es_addr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((int)es_addr, 1) != 1) return false;
  val = Wire.read();
  return true;
}


bool es_write(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(es_addr);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

// Register sequence for MCLK-less (BCLK-as-MCLK), 16-bit, 32kHz - same
// coefficients for all the exact-match rates, see tools/es8311_test/src/
// es8311_driver.cpp for the derivation.
bool es_init() {
  if (!es_addr) return false;
  bool ok = true;
  ok &= es_write(0x00, 0x1F);
  vTaskDelay(pdMS_TO_TICKS(20));
  ok &= es_write(0x00, 0x00);
  ok &= es_write(0x00, 0x80);
  ok &= es_write(0x01, 0xBF);  // all clocks on, BCLK as MCLK source
  uint8_t r = 0;
  ok &= es_read(0x06, r); r &= ~(1 << 5); ok &= es_write(0x06, r);
  ok &= es_read(0x02, r);
  r = (r & 0x07) | ((1 - 1) << 5) | (3 << 3);  // preDiv=1, preMulti=3
  ok &= es_write(0x02, r);
  ok &= es_write(0x03, (0 << 6) | 0x10);  // fsMode 0, adcOsr 0x10
  ok &= es_write(0x04, 0x10);             // dacOsr
  ok &= es_write(0x05, ((1 - 1) << 4) | (1 - 1));  // adcDiv/dacDiv = 1
  ok &= es_read(0x06, r); r = (r & 0xE0) | (4 - 1); ok &= es_write(0x06, r);  // bclkDiv 4
  ok &= es_read(0x07, r); r = (r & 0xC0) | 0x00; ok &= es_write(0x07, r);     // lrckH
  ok &= es_write(0x08, 0xFF);                                                  // lrckL
  ok &= es_read(0x00, r); r &= 0xBF; ok &= es_write(0x00, r);  // slave mode
  ok &= es_write(0x09, 3 << 2);  // 16-bit DAC in
  ok &= es_write(0x0A, 3 << 2);  // 16-bit ADC out
  ok &= es_write(0x0D, 0x01);
  ok &= es_write(0x0E, 0x02);
  ok &= es_write(0x12, 0x00);
  ok &= es_write(0x13, 0x10);
  ok &= es_write(0x1C, 0x6A);
  ok &= es_write(0x37, 0x08);
  ok &= es_write(0x17, 0xC8);
  ok &= es_write(0x14, 0x1A);
  return ok;
}

// Percent -> DAC volume register (0x32: 0.5dB/step, 0xBF = 0dB, 0xFF = +32dB).
// The old linear pct*256/100 put 100% at +32dB (clipping) and 80% at +6dB.
// Now 100% = +6dB (reg 0xCB, the loudest level playback has always used) and
// each 10% step down is 3dB, so 10% = -21dB. 0% is a hard mute (reg 0).
void es_set_volume(uint8_t pct) {
  if (pct > 100) pct = 100;
  if (pct == 0) { es_write(0x32, 0); return; }
  int steps_down = (100 - pct + 5) / 10;  // nearest 10% step
  int reg = 0xCB - steps_down * 6;
  if (reg < 1) reg = 1;
  es_write(0x32, (uint8_t)reg);
}

void es_set_mic_gain(uint8_t g) { es_write(0x16, g); }  // 0..7 = 0..42dB in 6dB steps

void es_set_mute(bool mute) {
  uint8_t r = 0;
  es_read(0x31, r);
  if (mute) r |= (1 << 6) | (1 << 5); else r &= ~((1 << 6) | (1 << 5));
  es_write(0x31, r);
}

// ---- 32kHz <-> 8kHz windowed-sinc 4x resampler (3.4kHz cutoff) ----

constexpr int TAPS = 33;
constexpr int L = 4;
constexpr int MAX_PHASE_TAPS = (TAPS + L - 1) / L;

float filter_taps[TAPS];
struct Phase { float taps[MAX_PHASE_TAPS]; int count; int base; };
Phase phases[L];
bool filters_ready = false;

void design_filters() {
  const float fc = 3400.0f / 32000.0f;
  const int M = TAPS - 1;
  float sum = 0;
  for (int n = 0; n < TAPS; n++) {
    float x = n - M / 2.0f;
    float sinc = (x == 0.0f) ? 2.0f * fc : sinf(2.0f * (float)M_PI * fc * x) / ((float)M_PI * x);
    float window = 0.54f - 0.46f * cosf(2.0f * (float)M_PI * n / M);
    filter_taps[n] = sinc * window;
    sum += filter_taps[n];
  }
  for (int n = 0; n < TAPS; n++) filter_taps[n] /= sum;
  const int half = TAPS / 2;
  for (int p = 0; p < L; p++) {
    int t0 = ((half - p) % L + L) % L;
    phases[p].base = (p - half + t0) / L;
    int cnt = 0;
    for (int t = t0; t < TAPS; t += L) phases[p].taps[cnt++] = filter_taps[t];
    phases[p].count = cnt;
  }
  filters_ready = true;
}

inline int16_t clamp16(float v) {
  if (v > 32767.0f) return 32767;
  if (v < -32768.0f) return -32768;
  return (int16_t)v;
}

size_t downsample4x(const int16_t *in, size_t n, int16_t *out) {
  if (!filters_ready) design_filters();
  const int half = TAPS / 2;
  size_t out_n = n / 4;
  for (size_t o = 0; o < out_n; o++) {
    long center = (long)(o * 4);
    float acc = 0;
    for (int t = 0; t < TAPS; t++) {
      long idx = center - half + t;
      if (idx >= 0 && idx < (long)n) acc += filter_taps[t] * in[idx];
    }
    out[o] = clamp16(acc);
  }
  return out_n;
}

size_t upsample4x(const int16_t *in, size_t n, int16_t *out) {
  if (!filters_ready) design_filters();
  size_t out_n = n * 4;
  for (size_t o = 0; o < out_n; o++) {
    const Phase &ph = phases[o & 3];
    long m = (long)(o >> 2);
    float acc = 0;
    for (int j = 0; j < ph.count; j++) {
      long src = m + ph.base + j;
      if (src >= 0 && src < (long)n) acc += ph.taps[j] * in[src];
    }
    out[o] = clamp16(acc * 4.0f);
  }
  return out_n;
}

}  // namespace

bool audio_probe_done = false, audio_probe_found = false;
uint8_t audio_probe_addr = 0, audio_probe_id1 = 0, audio_probe_id2 = 0;

bool audio_probe(uint8_t *addr, uint8_t *id1, uint8_t *id2) {
  pinMode(PIN_AUDIO_EN, OUTPUT);
  digitalWrite(PIN_AUDIO_EN, HIGH);  // active-HIGH, confirmed on hardware
  delay(20);

  bool found = false;
  const uint8_t candidates[] = {ES_ADDR_LOW, ES_ADDR_HIGH};
  // A few attempts: a single missed ACK at boot would otherwise disable all
  // voice features until the next reboot.
  for (int attempt = 0; attempt < 3 && !found; attempt++) {
    if (attempt) delay(15);
    for (uint8_t a : candidates) {
      Wire.beginTransmission(a);
      if (Wire.endTransmission() == 0) {
        es_addr = a;
        found = true;
        break;
      }
    }
  }
  uint8_t c1 = 0, c2 = 0;
  if (found) {
    es_read(REG_CHIPID1, c1);
    es_read(REG_CHIPID2, c2);
  }
  if (addr) *addr = found ? es_addr : 0;
  if (id1) *id1 = c1;
  if (id2) *id2 = c2;

  digitalWrite(PIN_AUDIO_EN, LOW);
  if (found) audio_worker_ensure();
  audio_probe_done = true;
  audio_probe_found = found;
  audio_probe_addr = found ? es_addr : 0;
  audio_probe_id1 = c1;
  audio_probe_id2 = c2;
  return found;
}

size_t audio_c2_encoded_size(int codec2_mode, size_t samples32k) {
  struct CODEC2 *c2 = codec2_create(codec2_mode);
  if (!c2) return 0;
  size_t frames = (samples32k / 4) / codec2_samples_per_frame(c2);
  size_t bytes = frames * codec2_bytes_per_frame(c2);
  codec2_destroy(c2);
  return bytes;
}

size_t audio_c2_encode(int codec2_mode, const int16_t *pcm32k, size_t samples32k,
                       uint8_t *out, size_t out_cap) {
  if (!pcm32k || !out || samples32k < 4) return 0;
  struct CODEC2 *c2 = codec2_create(codec2_mode);
  if (!c2) return 0;
  const int spf = codec2_samples_per_frame(c2);
  const int bpf = codec2_bytes_per_frame(c2);
  size_t down_cap = samples32k / 4 + 1;
  int16_t *down = (int16_t *)heap_caps_malloc(down_cap * sizeof(int16_t), MALLOC_CAP_SPIRAM);
  if (!down) { codec2_destroy(c2); return 0; }
  size_t down_n = downsample4x(pcm32k, samples32k, down);
  size_t frames = down_n / spf;
  size_t written = 0;
  if (frames * bpf <= out_cap) {
    for (size_t f = 0; f < frames; f++) {
      codec2_encode(c2, out + f * bpf, (short *)(down + f * spf));
    }
    written = frames * bpf;
  }
  free(down);
  codec2_destroy(c2);
  return written;
}

size_t audio_c2_decode(int codec2_mode, const uint8_t *in, size_t in_len,
                       int16_t *pcm32k, size_t pcm_cap) {
  if (!in || !pcm32k) return 0;
  struct CODEC2 *c2 = codec2_create(codec2_mode);
  if (!c2) return 0;
  const int spf = codec2_samples_per_frame(c2);
  const int bpf = codec2_bytes_per_frame(c2);
  size_t frames = in_len / bpf;
  size_t n8 = frames * spf;
  size_t written = 0;
  if (n8 * 4 <= pcm_cap) {
    int16_t *dec = (int16_t *)heap_caps_malloc(n8 * sizeof(int16_t), MALLOC_CAP_SPIRAM);
    if (dec) {
      for (size_t f = 0; f < frames; f++) {
        codec2_decode(c2, (short *)(dec + f * spf), in + f * bpf);
      }
      written = upsample4x(dec, n8, pcm32k);
      free(dec);
    }
  }
  codec2_destroy(c2);
  return written;
}

// ---- LXMF mode table ----

static const AudioModeInfo kModes[] = {
  {3, CODEC2_MODE_700C, 4, 40}, {4, CODEC2_MODE_1200, 6, 40}, {5, CODEC2_MODE_1300, 7, 40},
  {6, CODEC2_MODE_1400, 7, 40}, {7, CODEC2_MODE_1600, 8, 40}, {8, CODEC2_MODE_2400, 6, 20},
  {9, CODEC2_MODE_3200, 8, 20},
};

const AudioModeInfo *audio_mode_info(uint8_t lxmf_mode) {
  for (const AudioModeInfo &m : kModes) if (m.lxmf_mode == lxmf_mode) return &m;
  return nullptr;
}

uint32_t audio_duration_ms(uint8_t lxmf_mode, size_t bytes) {
  const AudioModeInfo *m = audio_mode_info(lxmf_mode);
  if (!m) return 0;
  return (uint32_t)((bytes / m->bytes_per_frame) * m->ms_per_frame);
}

// ---- Playback task ----

namespace {

struct PlayJob {
  const AudioModeInfo *mode;
  uint8_t *data;
  size_t len;
};

volatile AudioState play_state = AUDIO_IDLE;
volatile bool play_stop_req = false;
volatile uint32_t play_total_ms = 0;
volatile uint32_t play_elapsed_ms = 0;
bool codec_powered = false;
I2SClass i2s_out;

void free_job(PlayJob *j) {
  if (j->data) free(j->data);
  delete j;
}

void play_task(void *arg) {
  PlayJob *j = (PlayJob *)arg;
  const int bpf_i = j->mode->bytes_per_frame;
  size_t frames = j->len / bpf_i;
  struct CODEC2 *c2 = frames ? codec2_create(j->mode->codec2_mode) : nullptr;
  int16_t *pcm = nullptr;
  size_t n32 = 0;
  if (c2) {
    const int spf = codec2_samples_per_frame(c2);
    size_t n8 = frames * spf;
    int16_t *dec = (int16_t *)heap_caps_malloc(n8 * sizeof(int16_t), MALLOC_CAP_SPIRAM);
    pcm = (int16_t *)heap_caps_malloc(n8 * 4 * sizeof(int16_t), MALLOC_CAP_SPIRAM);
    if (dec && pcm) {
      for (size_t f = 0; f < frames && !play_stop_req; f++) {
        codec2_decode(c2, (short *)(dec + f * spf), j->data + f * bpf_i);
        if ((f & 31) == 31) vTaskDelay(1);
      }
      if (!play_stop_req) n32 = upsample4x(dec, n8, pcm);
    }
    if (dec) free(dec);
    codec2_destroy(c2);
  }
  free(j->data);
  j->data = nullptr;
  diag("decoded frames=%u n32=%u stop=%d c2=%d", (unsigned)frames, (unsigned)n32, (int)play_stop_req, c2 != nullptr);

  if (n32 && !play_stop_req) {
    i2s_out.setPins(PIN_AUDIO_BCK, PIN_AUDIO_WS, PIN_AUDIO_DOUT, PIN_AUDIO_DIN);
    if (i2s_out.begin(I2S_MODE_STD, AUDIO_SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT,
                      I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT, I2S_ROLE_MASTER)) {
      play_state = AUDIO_PLAYING;
      const size_t CHUNK = 1024;
      size_t pos = 0;
      while (pos < n32 && !play_stop_req) {
        size_t n = n32 - pos < CHUNK ? n32 - pos : CHUNK;
        i2s_out.write((const void *)(pcm + pos), n * sizeof(int16_t));
        pos += n;
        play_elapsed_ms = (uint32_t)((uint64_t)pos * 1000 / AUDIO_SAMPLE_RATE);
      }
      // Let the DMA ring drain before tearing the clocks down.
      static int16_t silence[1024];
      for (int i = 0; i < 8 && !play_stop_req; i++) i2s_out.write((const void *)silence, sizeof(silence));
      i2s_out.end();
      diag("played %u samples ok", (unsigned)pos);
    } else {
      diag("i2s begin FAILED");
    }
  }
  if (pcm) free(pcm);
  audio_task_stack_hwm = uxTaskGetStackHighWaterMark(nullptr) * sizeof(StackType_t);
  delete j;
  play_state = AUDIO_DONE;
}

}  // namespace

uint32_t audio_task_stack_hwm = 0;
char audio_diag[160] = "";
volatile uint32_t audio_diag_seq = 0;
static void diag(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
static void diag(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(audio_diag, sizeof(audio_diag), fmt, ap);
  va_end(ap);
  audio_diag_seq++;
}

bool audio_play_lxmf(uint8_t lxmf_mode, uint8_t *data, size_t len, uint8_t volume_pct) {
  const AudioModeInfo *m = audio_mode_info(lxmf_mode);
  if (!m || !es_addr || !worker_h || play_state != AUDIO_IDLE || len < m->bytes_per_frame) {
    diag("play refused: mode=%d addr=0x%02X worker=%d state=%d len=%u", m != nullptr, es_addr, worker_h != nullptr,
         (int)play_state, (unsigned)len);
    if (data) free(data);
    return false;
  }
  // Codec register setup (Wire) happens here, on the caller's (loop) task.
  digitalWrite(PIN_AUDIO_EN, HIGH);
  vTaskDelay(pdMS_TO_TICKS(20));
  if (!es_init()) {
    diag("es_init FAILED addr=0x%02X", es_addr);
    digitalWrite(PIN_AUDIO_EN, LOW);
    free(data);
    return false;
  }
  diag("codec up addr=0x%02X vol=%u mode=%u len=%u", es_addr, volume_pct, lxmf_mode, (unsigned)len);
  es_set_volume(volume_pct);
  es_set_mute(false);
  codec_powered = true;

  PlayJob *j = new PlayJob{m, data, len};
  play_stop_req = false;
  play_elapsed_ms = 0;
  play_total_ms = audio_duration_ms(lxmf_mode, len);
  play_state = AUDIO_DECODING;
  worker_job = WORKER_PLAY;
  worker_arg = j;
  xTaskNotifyGive(worker_h);
  return true;
}

void audio_play_stop() {
  if (play_state == AUDIO_DECODING || play_state == AUDIO_PLAYING) play_stop_req = true;
}

AudioState audio_state() { return play_state; }
uint32_t audio_play_elapsed_ms() { return play_elapsed_ms; }
uint32_t audio_play_total_ms() { return play_total_ms; }

// ---- Recording task ----

namespace {

volatile AudioRecState rec_state = REC_IDLE;
volatile bool rec_stop_req = false;
volatile bool rec_discard_req = false;
volatile uint32_t rec_elapsed_ms = 0;
uint8_t *rec_clip = nullptr;
size_t rec_clip_len = 0;
I2SClass i2s_in;

void rec_task(void *) {
  const size_t max_samples = (size_t)AUDIO_REC_MAX_SECONDS * AUDIO_SAMPLE_RATE;
  int16_t *pcm = (int16_t *)heap_caps_malloc(max_samples * sizeof(int16_t), MALLOC_CAP_SPIRAM);
  size_t n = 0;
  bool manual_stop = false;
  if (pcm) {
    i2s_in.setPins(PIN_AUDIO_BCK, PIN_AUDIO_WS, PIN_AUDIO_DOUT, PIN_AUDIO_DIN);
    if (i2s_in.begin(I2S_MODE_STD, AUDIO_SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT,
                     I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT, I2S_ROLE_MASTER)) {
      const size_t CHUNK = 1024;
      while (n < max_samples && !rec_stop_req && !rec_discard_req) {
        size_t want = max_samples - n < CHUNK ? max_samples - n : CHUNK;
        size_t got = i2s_in.readBytes((char *)(pcm + n), want * sizeof(int16_t)) / sizeof(int16_t);
        if (got == 0) break;
        n += got;
        rec_elapsed_ms = (uint32_t)((uint64_t)n * 1000 / AUDIO_SAMPLE_RATE);
      }
      manual_stop = rec_stop_req;
      i2s_in.end();
    }
  }

  // Drop the codec's start-up settling (first ~64ms) and, if the user
  // stopped it, the trailing ~120ms where the stop click lands.
  size_t start = 2048, tail = manual_stop ? 3840 : 0;
  rec_state = REC_ENCODING;
  size_t out_len = 0;
  uint8_t *out = nullptr;
  if (pcm && !rec_discard_req && n > start + tail + AUDIO_SAMPLE_RATE / 4) {  // >= ~0.25s of audio
    int16_t *seg = pcm + start;
    size_t seg_n = n - start - tail;
    // Peak-normalize (Sideband does the same) - capped so room noise on a
    // silent clip isn't amplified into hiss.
    int32_t peak = 1;
    for (size_t i = 0; i < seg_n; i++) { int32_t a = seg[i] < 0 ? -(int32_t)seg[i] : seg[i]; if (a > peak) peak = a; }
    float gain = 29000.0f / (float)peak;
    if (gain > 8.0f) gain = 8.0f;
    if (gain > 1.05f) for (size_t i = 0; i < seg_n; i++) seg[i] = clamp16(seg[i] * gain);
    const AudioModeInfo *mi = audio_mode_info(AUDIO_REC_LXMF_MODE);
    size_t cap = ((size_t)AUDIO_REC_MAX_SECONDS * 1000 / mi->ms_per_frame + 2) * mi->bytes_per_frame;
    out = (uint8_t *)heap_caps_malloc(cap, MALLOC_CAP_SPIRAM);
    if (out) {
      out_len = audio_c2_encode(mi->codec2_mode, seg, seg_n, out, cap);
      if (!out_len) { free(out); out = nullptr; }
    }
  }
  if (pcm) free(pcm);
  diag("recorded %u samples, encoded %u bytes (manual=%d discard=%d)", (unsigned)n, (unsigned)out_len,
       (int)manual_stop, (int)rec_discard_req);
  audio_task_stack_hwm = uxTaskGetStackHighWaterMark(nullptr) * sizeof(StackType_t);
  if (rec_discard_req || !out) {
    if (out) free(out);
    rec_state = rec_discard_req ? REC_IDLE : REC_FAILED;
  } else {
    rec_clip = out;
    rec_clip_len = out_len;
    rec_state = REC_READY;
  }
}

}  // namespace

// ---- Persistent worker task ----
// One long-lived task owns the big Codec2 stack. It used to be created per
// play/record, which failed intermittently: a ~32KB contiguous internal-RAM
// block isn't reliably available once the heap has fragmented (BLE/WiFi/LoRa/
// LXMF), so xTaskCreate returned failure after the codec was already powered
// (silent Play). The stack is now reserved once, at boot, while the heap is
// still unfragmented, and jobs are handed over with a task notification.
// Internal RAM, not PSRAM: a PSRAM-stack task faults whenever the flash cache
// is disabled (LittleFS writes). Peak use measured ~16.5KB.
#define AUDIO_WORKER_STACK 24576

namespace {

void worker_task(void *) {
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    WorkerJob job = worker_job;
    worker_job = WORKER_NONE;
    if (job == WORKER_PLAY) play_task(worker_arg);
    else if (job == WORKER_REC) rec_task(nullptr);
  }
}

}  // namespace

static bool audio_worker_ensure() {
  if (worker_h) return true;
  size_t largest = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL);
  if (xTaskCreatePinnedToCore(worker_task, "audio", AUDIO_WORKER_STACK, nullptr, 1, &worker_h, 0) != pdPASS) {
    diag("worker create FAILED (largest internal block %u)", (unsigned)largest);
    return false;
  }
  diag("worker up, stack=%u (largest internal block was %u)", (unsigned)AUDIO_WORKER_STACK, (unsigned)largest);
  return true;
}

bool audio_record_start() {
  if (!es_addr || !worker_h || play_state != AUDIO_IDLE) return false;
  if (rec_state == REC_RECORDING || rec_state == REC_ENCODING) return false;
  audio_record_discard();  // drop any previous clip
  digitalWrite(PIN_AUDIO_EN, HIGH);
  vTaskDelay(pdMS_TO_TICKS(20));
  if (!es_init()) {
    diag("record: es_init FAILED");
    digitalWrite(PIN_AUDIO_EN, LOW);
    return false;
  }
  es_set_mic_gain(4);     // 24dB, as validated in the bring-up project
  es_set_volume(0);
  es_set_mute(true);      // no speaker output while the mic is live
  codec_powered = true;
  rec_stop_req = false;
  rec_discard_req = false;
  rec_elapsed_ms = 0;
  rec_state = REC_RECORDING;
  worker_job = WORKER_REC;
  worker_arg = nullptr;
  xTaskNotifyGive(worker_h);
  return true;
}

void audio_record_stop() {
  if (rec_state == REC_RECORDING) rec_stop_req = true;
}

void audio_record_discard() {
  if (rec_state == REC_RECORDING) {
    rec_discard_req = true;  // task ends on its own and returns to IDLE
  } else if (rec_state == REC_ENCODING) {
    rec_discard_req = true;
  } else {
    if (rec_clip) { free(rec_clip); rec_clip = nullptr; }
    rec_clip_len = 0;
    rec_state = REC_IDLE;
  }
}

AudioRecState audio_record_state() { return rec_state; }
uint32_t audio_record_elapsed_ms() { return rec_elapsed_ms; }
size_t audio_record_clip_bytes() { return rec_state == REC_READY ? rec_clip_len : 0; }
uint32_t audio_record_clip_ms() { return rec_state == REC_READY ? audio_duration_ms(AUDIO_REC_LXMF_MODE, rec_clip_len) : 0; }

bool audio_record_preview(uint8_t volume_pct) {
  if (rec_state != REC_READY || !rec_clip) return false;
  uint8_t *copy = (uint8_t *)heap_caps_malloc(rec_clip_len, MALLOC_CAP_SPIRAM);
  if (!copy) return false;
  memcpy(copy, rec_clip, rec_clip_len);
  return audio_play_lxmf(AUDIO_REC_LXMF_MODE, copy, rec_clip_len, volume_pct);
}

bool audio_record_take(uint8_t **data, size_t *len, uint8_t *lxmf_mode) {
  if (rec_state != REC_READY || !rec_clip) return false;
  *data = rec_clip;
  *len = rec_clip_len;
  *lxmf_mode = AUDIO_REC_LXMF_MODE;
  rec_clip = nullptr;
  rec_clip_len = 0;
  rec_state = REC_IDLE;
  return true;
}

void audio_poll() {
  if (play_state == AUDIO_DONE) {
    play_state = AUDIO_IDLE;
  }
  // The codec only needs to be powered while playing or capturing; once
  // the record task moves on to encoding (or either side finishes), power
  // it down.
  bool need = (play_state == AUDIO_DECODING || play_state == AUDIO_PLAYING || rec_state == REC_RECORDING);
  if (codec_powered && !need) {
    es_set_mute(true);
    digitalWrite(PIN_AUDIO_EN, LOW);
    codec_powered = false;
  }
}

#endif  // HAS_AUDIO
