#include "dualcore.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

namespace {

struct Trampoline {
  DualCore::JobFn fn;
  void *ctx;
  SemaphoreHandle_t done;
};

void taskTrampoline(void *arg) {
  Trampoline *t = (Trampoline *)arg;
  t->fn(t->ctx);
  xSemaphoreGive(t->done);
  vTaskDelete(nullptr);
}

}  // namespace

void DualCore::runSplit(JobFn coreLocalJob, void *coreLocalCtx, JobFn otherCoreJob, void *otherCoreCtx) {
  SemaphoreHandle_t done = xSemaphoreCreateBinary();
  // Lives on this function's stack - safe because runSplit() doesn't
  // return until after xSemaphoreTake() below, so the spawned task's one
  // read of `t` (right at taskTrampoline's start) always happens before
  // this stack frame could go away.
  Trampoline t{otherCoreJob, otherCoreCtx, done};
  xTaskCreatePinnedToCore(taskTrampoline, "dualcore_job", 8192, &t, 1, nullptr, 0);

  coreLocalJob(coreLocalCtx);  // do our own half while the core-0 task runs

  xSemaphoreTake(done, portMAX_DELAY);
  vSemaphoreDelete(done);
}
