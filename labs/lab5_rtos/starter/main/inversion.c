/*
 * Lab 5 -- measured priority inversion: binary semaphore vs. mutex
 * (starter)
 *
 * Ugradbeni racunalni sustavi -- Lab 5: FreeRTOS pipeline, race,
 * priority inversion
 *
 * Compiled in when CONFIG_LAB5_DEMO_INVERSION=y (see
 * main/Kconfig.projbuild). Three tasks, staggered start (10 ms
 * apart -- a multiple of the default 10 ms FreeRTOS tick, so wake
 * order is deterministic under QEMU):
 *
 *   low_task    (prio 2, wakes at t=20 ms) takes the lock,
 *               busy-holds it for ~50 ms.
 *   medium_task (prio 3, wakes at t=30 ms) busy-hogs the CPU for
 *               ~200 ms. It never touches the lock -- it is what
 *               starves low_task of CPU time while low_task still
 *               holds it.
 *   high_task   (prio 5, wakes at t=40 ms) blocks trying to take
 *               the same lock; the time it spends blocked is
 *               measured with esp_timer_get_time().
 *
 * ONE TODO in this file (manual, T4): create the two lock
 * primitives. lock_take()/lock_give() below already treat a NULL
 * handle as "no lock" (they skip straight through), so until you
 * create both, high_task never actually blocks and both runs print
 * a near-zero microsecond count.
 *
 * Run 1: plain binary semaphore (no priority inheritance) --
 *                                          "LAB5 OK inv_sem_us=<n>"
 * Run 2: mutex (FreeRTOS priority inheritance) --
 *                                          "LAB5 OK inv_mux_us=<m>"
 * Expect n >> m once T4 is done. (busy_wait_ms() is wall-clock
 * based, so time low_task spends preempted counts toward its
 * ~50 ms hold budget; in run 1, n is effectively
 * (MEDIUM_START_MS + MEDIUM_HOG_MS) - HIGH_START_MS, independent
 * of LOW_HOLD_MS.)
 */
#include "sdkconfig.h"
#if CONFIG_LAB5_DEMO_INVERSION

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "lab5";

#define LOW_PRIO                      2
#define MEDIUM_PRIO                    3
#define HIGH_PRIO                       5
#define INV_STACK_BYTES               2048

#define LOW_START_MS                    20
#define MEDIUM_START_MS                 30
#define HIGH_START_MS                    40

#define LOW_HOLD_MS                      50
#define MEDIUM_HOG_MS                   200

typedef enum {
    INV_PRIM_SEMAPHORE = 0,
    INV_PRIM_MUTEX,
} inv_primitive_t;

/* plain binary semaphore -- run 1 */
static SemaphoreHandle_t s_inv_sem;
/* mutex, priority inheritance -- run 2 */
static SemaphoreHandle_t s_inv_mutex;

static volatile inv_primitive_t s_primitive;
static volatile bool s_low_done, s_medium_done, s_high_done;
static volatile int64_t s_block_us;

static SemaphoreHandle_t active_lock(void)
{
    return (s_primitive == INV_PRIM_MUTEX) ? s_inv_mutex
                                           : s_inv_sem;
}

static void lock_take(void)
{
    SemaphoreHandle_t h = active_lock();
    if (h != NULL) {
        xSemaphoreTake(h, portMAX_DELAY);
    }
}

static void lock_give(void)
{
    SemaphoreHandle_t h = active_lock();
    if (h != NULL) {
        xSemaphoreGive(h);
    }
}

/* Pure CPU spin (no vTaskDelay) for approximately `ms` milliseconds
 * -- only a strictly higher-priority ready task can preempt this.
 * Wall-clock based: time spent preempted still counts toward the
 * budget, so a task that gets preempted past its deadline returns
 * as soon as it next gets the CPU. */
static void busy_wait_ms(uint32_t ms)
{
    int64_t start = esp_timer_get_time();
    int64_t budget_us = (int64_t)ms * 1000;
    while ((esp_timer_get_time() - start) < budget_us) {
        /* spin */
    }
}

static void low_task(void *arg)
{
    (void)arg;
    vTaskDelay(pdMS_TO_TICKS(LOW_START_MS));

    lock_take();
    busy_wait_ms(LOW_HOLD_MS);
    lock_give();

    s_low_done = true;
    vTaskDelete(NULL);
}

static void medium_task(void *arg)
{
    (void)arg;
    vTaskDelay(pdMS_TO_TICKS(MEDIUM_START_MS));

    busy_wait_ms(MEDIUM_HOG_MS);

    s_medium_done = true;
    vTaskDelete(NULL);
}

static void high_task(void *arg)
{
    (void)arg;
    vTaskDelay(pdMS_TO_TICKS(HIGH_START_MS));

    int64_t t0 = esp_timer_get_time();
    lock_take();
    int64_t t1 = esp_timer_get_time();
    lock_give();

    s_block_us = t1 - t0;
    s_high_done = true;
    vTaskDelete(NULL);
}

static int64_t run_inversion(inv_primitive_t primitive)
{
    s_primitive = primitive;
    s_low_done = false;
    s_medium_done = false;
    s_high_done = false;
    s_block_us = 0;

    xTaskCreate(low_task, "lab5_inv_lo", INV_STACK_BYTES, NULL,
                LOW_PRIO, NULL);
    xTaskCreate(medium_task, "lab5_inv_med", INV_STACK_BYTES, NULL,
                MEDIUM_PRIO, NULL);
    xTaskCreate(high_task, "lab5_inv_hi", INV_STACK_BYTES, NULL,
                HIGH_PRIO, NULL);

    while (!s_low_done || !s_medium_done || !s_high_done) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    return s_block_us;
}

void app_main(void)
{
    ESP_LOGI(TAG,
             "Ugradbeni racunalni sustavi -- Lab 5 (inversion)");

    /* TODO 3 (manual, T4): create the two lock primitives used by
     * the two runs below:
     *   s_inv_sem = xSemaphoreCreateBinary();
     *   if (s_inv_sem != NULL) {
     *       // binary sems start EMPTY -- must be given once before
     *       // the first take, unlike a mutex (created already
     *       // "unlocked")
     *       xSemaphoreGive(s_inv_sem);
     *   }
     *   s_inv_mutex = xSemaphoreCreateMutex();
     * Left as NULL: lock_take()/lock_give() above already treat a
     * NULL handle as "no lock" and return immediately, so high_task
     * never actually blocks and both runs print a near-zero
     * microsecond count until you create both primitives here. */

    int64_t block_sem_us = run_inversion(INV_PRIM_SEMAPHORE);
    ESP_LOGI(TAG, "LAB5 OK inv_sem_us=%lld",
             (long long)block_sem_us);

    int64_t block_mux_us = run_inversion(INV_PRIM_MUTEX);
    ESP_LOGI(TAG, "LAB5 OK inv_mux_us=%lld",
             (long long)block_mux_us);

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

#endif /* CONFIG_LAB5_DEMO_INVERSION */
