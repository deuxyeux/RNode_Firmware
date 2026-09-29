#pragma once
// Tiny helper to run two independent jobs concurrently on ESP32-S3's two
// cores: the caller's own job runs directly on whichever core called
// runSplit() (Arduino's loopTask, where all our audio processing runs, is
// pinned to core 1 by default - see platformio.ini's own comment on
// ARDUINO_LOOP_STACK_SIZE), while a second job runs on a fresh FreeRTOS
// task pinned to core 0 (otherwise idle during these batch passes, since
// this test firmware has no WiFi/BLE using it). Blocks until both finish.
//
// Only safe for jobs with no shared mutable state - each job must only
// read shared inputs and write to its own disjoint output range (see
// resample.cpp's use: two halves of the same output buffer, never
// overlapping).

#include <Arduino.h>

namespace DualCore {

using JobFn = void (*)(void *ctx);

void runSplit(JobFn coreLocalJob, void *coreLocalCtx, JobFn otherCoreJob, void *otherCoreCtx);

}  // namespace DualCore
