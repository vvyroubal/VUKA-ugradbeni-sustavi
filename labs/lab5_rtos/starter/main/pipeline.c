/*
 * Lab 5 -- FreeRTOS pipeline: sensor -> processing -> logger
 * (starter)
 *
 * Ugradbeni racunalni sustavi -- Lab 5: FreeRTOS pipeline, race,
 * priority inversion
 *
 * Compiled in when CONFIG_LAB5_DEMO_PIPELINE=y (the default -- so
 * this starter boots and prints the banner in QEMU with no
 * changes). A classic three-stage FreeRTOS pipeline glued together
 * with two queues:
 *
 *  sensor_task (prio 5, 10 Hz)     bme280_read() -> raw_q
 *    -> processing_task (prio 4)  8-sample moving average -> avg_q
 *      -> logger_task (prio 3)    "LAB5 OK avg=<t.t>" @ 1 Hz
 *
 * At t ~= 30 s, logger_task additionally prints each task's stack
 * high-water mark: "LAB5 OK hwm_<task>=<bytes>".
 *
 * ONE TODO in this file (manual, T2): wire the pipeline together by
 * creating the two queues in app_main(). Every task below already
 * treats a NULL queue handle as "nothing to do yet, try again
 * shortly" and idles harmlessly -- so this boots and prints the
 * banner, but no "LAB5 OK ..." marker ever appears, until you
 * create both queues.
 *
 * app_main() talks to bme280.h's shared API only -- same
 * sim/hardware split as lab3 (see
 * labs/common/components/bme280_sim/README.md). Lab 5 is
 * QEMU-complete by design: only the sim path is ever exercised
 * here.
 */
#include "sdkconfig.h"
#if CONFIG_LAB5_DEMO_PIPELINE

#include <stdbool.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "bme280.h"

static const char *TAG = "lab5";

#define SENSOR_PERIOD_MS         100    /* 10 Hz */
#define SENSOR_PRIO                 5
#define PROCESSING_PRIO              4
#define LOGGER_PRIO                  3

#define TASK_STACK_BYTES          4096

#define RAW_QUEUE_DEPTH               4
#define AVG_QUEUE_DEPTH                4
#define MOVING_AVG_WINDOW               8
/* 10 Hz samples -> 1 Hz logging */
#define LOG_EVERY_N                    10
#define HWM_PRINT_US        (30LL * 1000000LL)   /* t ~= 30 s */

/* sensor_task -> processing_task, float (degC) */
static QueueHandle_t s_raw_q;
/* processing_task -> logger_task, float (degC) */
static QueueHandle_t s_avg_q;

static TaskHandle_t s_sensor_handle;
static TaskHandle_t s_processing_handle;
static TaskHandle_t s_logger_handle;

static void sensor_task(void *arg)
{
    (void)arg;
    for (;;) {
        float temp_c = 0.0f, press_hpa = 0.0f, hum_pct = 0.0f;
        if (bme280_read(&temp_c, &press_hpa, &hum_pct) == ESP_OK
            && s_raw_q != NULL) {
            xQueueSend(s_raw_q, &temp_c, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(SENSOR_PERIOD_MS));
    }
}

static void processing_task(void *arg)
{
    (void)arg;
    static float ring[MOVING_AVG_WINDOW];
    int idx = 0;
    int count = 0;
    float sum = 0.0f;

    for (;;) {
        float raw;
        if (s_raw_q == NULL) {
            /* Queue not created yet (TODO 1): block on the tick,
             * NOT with a bare `continue` -- that would busy-spin at
             * priority 4, starve the idle task, and trip the Task
             * Watchdog every 5 s. */
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }
        if (xQueueReceive(s_raw_q, &raw,
                          pdMS_TO_TICKS(200)) != pdTRUE) {
            continue;
        }

        if (count < MOVING_AVG_WINDOW) {
            count++;
        } else {
            sum -= ring[idx];
        }
        ring[idx] = raw;
        sum += raw;
        idx = (idx + 1) % MOVING_AVG_WINDOW;

        float avg = sum / (float)count;
        if (s_avg_q != NULL) {
            xQueueSend(s_avg_q, &avg, 0);
        }
    }
}

static void logger_task(void *arg)
{
    (void)arg;
    int n = 0;
    bool hwm_printed = false;

    for (;;) {
        float avg;
        if (s_avg_q == NULL) {
            /* Queue not created yet (TODO 1): block on the tick,
             * NOT with a bare `continue` -- see processing_task
             * above. */
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }
        if (xQueueReceive(s_avg_q, &avg,
                          pdMS_TO_TICKS(200)) != pdTRUE) {
            continue;
        }

        n++;
        if (n % LOG_EVERY_N == 0) {
            ESP_LOGI(TAG, "LAB5 OK avg=%.1f", avg);
        }

        if (!hwm_printed && esp_timer_get_time() >= HWM_PRINT_US) {
            ESP_LOGI(TAG, "LAB5 OK hwm_sensor=%u",
                     (unsigned)uxTaskGetStackHighWaterMark(
                         s_sensor_handle));
            ESP_LOGI(TAG, "LAB5 OK hwm_processing=%u",
                     (unsigned)uxTaskGetStackHighWaterMark(
                         s_processing_handle));
            ESP_LOGI(TAG, "LAB5 OK hwm_logger=%u",
                     (unsigned)uxTaskGetStackHighWaterMark(
                         s_logger_handle));
            hwm_printed = true;
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG,
             "Ugradbeni racunalni sustavi -- Lab 5 (pipeline)");

    ESP_ERROR_CHECK(bme280_init());

    /* TODO 1 (manual, T2): wire the pipeline together -- create the
     * two queues that carry raw sensor samples to processing_task,
     * and the resulting moving average to logger_task:
     *   s_raw_q = xQueueCreate(RAW_QUEUE_DEPTH, sizeof(float));
     *   s_avg_q = xQueueCreate(AVG_QUEUE_DEPTH, sizeof(float));
     * Left as NULL below: every task above already treats a NULL
     * queue handle as "nothing to do yet" and idles harmlessly, so
     * this boots and prints the banner but never reaches the
     * "LAB5 OK avg=..." / "LAB5 OK hwm_..." markers until you
     * create both queues here. */

    xTaskCreate(sensor_task, "lab5_sensor", TASK_STACK_BYTES, NULL,
                SENSOR_PRIO, &s_sensor_handle);
    xTaskCreate(processing_task, "lab5_proc", TASK_STACK_BYTES,
                NULL, PROCESSING_PRIO, &s_processing_handle);
    xTaskCreate(logger_task, "lab5_logger", TASK_STACK_BYTES, NULL,
                LOGGER_PRIO, &s_logger_handle);
}

#endif /* CONFIG_LAB5_DEMO_PIPELINE */
