/*
 * Lab 5 -- provoked race condition, fixed with a mutex (starter)
 *
 * Ugradbeni racunalni sustavi -- Lab 5: FreeRTOS pipeline, race,
 * priority inversion
 *
 * Built when CONFIG_LAB5_DEMO_RACE=y (see main/Kconfig.projbuild).
 * Two equal-priority tasks each increment a shared, unprotected
 * counter RACE_ITER times. Every RACE_YIELD_EVERY-th iteration,
 * the read-modify-write is deliberately split around a taskYIELD():
 *
 *     uint32_t tmp = s_shared_counter;  // read
 *     taskYIELD();                      // hand off mid-RMW
 *     s_shared_counter = tmp + 1;       // write -- stomps on all
 *                                       // the other task did
 *
 * A bare `s_shared_counter++` would only *rarely* get interrupted
 * between its load and store on a single-core target -- forcing a
 * context switch mid-RMW makes the lost-update race reproducible
 * every run instead of an occasional flake. (Why not yield on
 * EVERY iteration? 2 x 100000 context switches take minutes under
 * QEMU; yielding every 1000th keeps the run fast while still losing
 * the other task's ~1000-increment batch at every yield point.)
 *
 * ONE TODO in this file (manual, T3): create the mutex used to
 * guard run 2. race_worker() below already treats a NULL mutex
 * handle as "no lock" (it skips the take/give calls), so until you
 * create it, run 2 repeats exactly the same race as run 1 -- both
 * runs print "LAB5 OK race_lost=<n>" with n>0.
 *
 * Run 1 (no lock):        "LAB5 OK race_lost=<n>"  (n>0, the race)
 * Run 2 (mutex-guarded):  "LAB5 OK race_lost=0"     once T3 is done
 */
#include "sdkconfig.h"
#if CONFIG_LAB5_DEMO_RACE

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static const char *TAG = "lab5";

#define RACE_ITER              100000
#define RACE_YIELD_EVERY          1000  /* split RMW every 1000th */
#define RACE_PRIO                    4
#define RACE_STACK_BYTES           2048

static SemaphoreHandle_t s_race_mutex;   /* guards run 2 only */

static volatile uint32_t s_shared_counter;
static volatile bool s_use_mutex;
static volatile bool s_race_start;
static volatile bool s_worker_done[2];

static void race_worker(void *arg)
{
    int id = (int)(intptr_t)arg;

    /* Start gate: a freshly created priority-4 worker preempts
     * app_main (priority 1) IMMEDIATELY -- without this gate,
     * worker 0 would run its entire loop to completion before
     * worker 1 even exists, and there would be no overlap (and
     * therefore no race) to observe. Both workers block here until
     * run_race() has created both, then wake on the same tick. */
    while (!s_race_start) {
        vTaskDelay(1);
    }

    for (int i = 0; i < RACE_ITER; i++) {
        if (s_use_mutex && s_race_mutex != NULL) {
            xSemaphoreTake(s_race_mutex, portMAX_DELAY);
        }

        uint32_t tmp = s_shared_counter;
        if (i % RACE_YIELD_EVERY == 0) {
            taskYIELD();  /* hand off mid-RMW -- tmp is now stale */
        }
        s_shared_counter = tmp + 1;

        if (s_use_mutex && s_race_mutex != NULL) {
            xSemaphoreGive(s_race_mutex);
        }
    }

    s_worker_done[id] = true;
    vTaskDelete(NULL);
}

/* Runs the two-task workload once (unprotected or mutex-guarded)
 * and returns the final counter value once both workers have
 * finished. */
static uint32_t run_race(bool use_mutex)
{
    s_shared_counter = 0;
    s_use_mutex = use_mutex;
    s_race_start = false;
    s_worker_done[0] = false;
    s_worker_done[1] = false;

    xTaskCreate(race_worker, "lab5_race0", RACE_STACK_BYTES,
                (void *)(intptr_t)0, RACE_PRIO, NULL);
    xTaskCreate(race_worker, "lab5_race1", RACE_STACK_BYTES,
                (void *)(intptr_t)1, RACE_PRIO, NULL);

    /* both exist now -- release them together */
    s_race_start = true;

    while (!s_worker_done[0] || !s_worker_done[1]) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    return s_shared_counter;
}

void app_main(void)
{
    ESP_LOGI(TAG, "Ugradbeni racunalni sustavi -- Lab 5 (race)");

    /* TODO 2 (manual, T3): create the mutex guarding run 2 below:
     *   s_race_mutex = xSemaphoreCreateMutex();
     * Left as NULL: race_worker() treats a NULL mutex handle as "no
     * lock" (see the guarded xSemaphoreTake/Give calls above), so
     * run 2 repeats exactly the same unprotected race as run 1
     * until you create it here. */

    const uint32_t expected = 2u * (uint32_t)RACE_ITER;

    uint32_t observed_unprotected = run_race(false);
    uint32_t lost_unprotected = (expected > observed_unprotected)
        ? (expected - observed_unprotected) : 0;
    ESP_LOGI(TAG, "LAB5 OK race_lost=%u",
             (unsigned)lost_unprotected);

    uint32_t observed_protected = run_race(true);
    uint32_t lost_protected = (expected > observed_protected)
        ? (expected - observed_protected) : 0;
    ESP_LOGI(TAG, "LAB5 OK race_lost=%u",
             (unsigned)lost_protected);

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

#endif /* CONFIG_LAB5_DEMO_RACE */
