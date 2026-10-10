/*
 * Lab 2 -- direct-register GPIO, interrupts, timers (starter)
 *
 * Ugradbeni racunalni sustavi -- Lab 2: GPIO/IRQ/timer
 *
 * Three features to implement (see the manual for details):
 *   1. LED on GPIO8 toggled by DIRECT REGISTER ACCESS -- no
 *      gpio_set_level(). The register-address macros below are
 *      already correct (verified against the ESP32-C3 TRM); TODO 1
 *      is to actually USE them to enable the pin as an output and
 *      to set/clear it.
 *   2. Button on GPIO9 (BOOT button) with a falling-edge GPIO ISR
 *      (IRAM_ATTR). TODO 2 is the 30 ms software debounce by
 *      timestamp comparison (esp_timer_get_time() is ISR-safe).
 *   3. A GPTimer at 1 Hz whose ISR also gives to the shared queue.
 *      TODO 3 is the alarm configuration (alarm_count /
 *      reload_count / auto-reload) plus enabling and starting the
 *      timer.
 *
 * The main task blocks on the queue and must print:
 *   "LAB2 OK tick=<n>"  on every GPTimer alarm
 *   "LAB2 OK btn=<n>"   on every debounced button press
 *
 * This starter builds and boots in QEMU printing the banner below
 * even before the TODOs are filled in -- it just won't tick or
 * debounce yet.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_attr.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "lab2";

#define LED_GPIO    GPIO_NUM_8  /* onboard LED, SuperMini/DevKitM */
#define BUTTON_GPIO GPIO_NUM_9  /* BOOT button, active low */
#define DEBOUNCE_US (30 * 1000) /* 30 ms debounce window */

/* ----------------------------------------------------------------
 * Direct-register GPIO access (ESP32-C3 TRM, "GPIO Controller"
 * chapter). GPIO_BASE = 0x6000_4000.
 *
 *   GPIO_OUT_W1TS_REG    (+0x0008): write 1 bits to SET   GPIO_OUT
 *   GPIO_OUT_W1TC_REG    (+0x000C): write 1 bits to CLEAR GPIO_OUT
 *   GPIO_ENABLE_W1TS_REG (+0x0024): write 1 bits to enable output
 *
 * These are volatile-pointer macros so every access is a real bus
 * write, not something the compiler can reorder or elide.
 * ------------------------------------------------------------- */
#define GPIO_BASE   0x60004000u
#define GPIO_OUT_W1TS_REG \
    (*(volatile uint32_t *)(GPIO_BASE + 0x0008u))
#define GPIO_OUT_W1TC_REG \
    (*(volatile uint32_t *)(GPIO_BASE + 0x000Cu))
#define GPIO_ENABLE_W1TS_REG \
    (*(volatile uint32_t *)(GPIO_BASE + 0x0024u))

/* Event queue shared by both ISRs and the main task. */
typedef enum {
    LAB2_EVT_TICK,
    LAB2_EVT_BUTTON,
} lab2_evt_type_t;

typedef struct {
    lab2_evt_type_t type;
} lab2_evt_t;

static QueueHandle_t s_evtq;

/* ------------------- LED (direct register) ------------------- */

static void led_init(void)
{
    /* Mux the pin to plain GPIO and clear any previous driver
     * config. This one-time setup step legitimately uses the
     * driver API; toggling the LED afterwards must not. */
    gpio_reset_pin(LED_GPIO);

    /* TODO 1a: enable LED_GPIO as an output by writing to
     *          GPIO_ENABLE_W1TS_REG (write 1u << LED_GPIO). */

    /* TODO 1b: drive the pin low at start-up via
     *          GPIO_OUT_W1TC_REG. */
}

static inline void led_set(bool on)
{
    (void)on;
    /* TODO 1c: if `on`, write (1u << LED_GPIO) to
     *          GPIO_OUT_W1TS_REG; else write (1u << LED_GPIO) to
     *          GPIO_OUT_W1TC_REG.
     *          No gpio_set_level() calls allowed here. */
}

/* ------------------ Button (ISR + debounce) ------------------ */

static void IRAM_ATTR button_isr_handler(void *arg)
{
    (void)arg;

    /* TODO 2: 30 ms debounce by timestamp comparison.
     *   - keep a `static int64_t s_last_accepted_us` across calls
     *   - int64_t now = esp_timer_get_time(); (ISR-safe on ESP-IDF)
     *   - if (now - s_last_accepted_us < DEBOUNCE_US)
     *         return; // bounce
     *   - s_last_accepted_us = now;
     * Without this, every electrical bounce on the falling edge
     * would enqueue its own (spurious) button event. */

    lab2_evt_t evt = { .type = LAB2_EVT_BUTTON };
    BaseType_t hpw = pdFALSE;
    xQueueSendFromISR(s_evtq, &evt, &hpw);
    if (hpw) {
        portYIELD_FROM_ISR();
    }
}

static void button_init(void)
{
    gpio_reset_pin(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(BUTTON_GPIO, GPIO_INTR_NEGEDGE);

    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(BUTTON_GPIO,
                                         button_isr_handler, NULL));
    gpio_intr_enable(BUTTON_GPIO);
}

/* ---------------------- GPTimer (1 Hz) ----------------------- */

static bool IRAM_ATTR timer_alarm_cb(gptimer_handle_t timer,
        const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    (void)timer;
    (void)edata;
    (void)user_ctx;

    lab2_evt_t evt = { .type = LAB2_EVT_TICK };
    BaseType_t hpw = pdFALSE;
    xQueueSendFromISR(s_evtq, &evt, &hpw);
    return hpw == pdTRUE;
}

static void timer_init_1hz(void)
{
    gptimer_handle_t timer = NULL;
    gptimer_config_t tcfg = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, /* 1 MHz -> 1 tick = 1 us */
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&tcfg, &timer));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_alarm_cb,
    };
    ESP_ERROR_CHECK(
        gptimer_register_event_callbacks(timer, &cbs, NULL));

    /* TODO 3: build a gptimer_alarm_config_t for a 1 Hz alarm and
     * apply it:
     *   gptimer_alarm_config_t acfg = {
     *       // resolution_hz -> 1 alarm per second
     *       .alarm_count = 1000000,
     *       .reload_count = 0,
     *       .flags.auto_reload_on_alarm = true,
     *   };
     *   ESP_ERROR_CHECK(gptimer_set_alarm_action(timer, &acfg));
     *   ESP_ERROR_CHECK(gptimer_enable(timer));
     *   ESP_ERROR_CHECK(gptimer_start(timer));
     * Until this is filled in, the timer is created but never
     * fires. */
    (void)timer;
}

/* --------------------------- main ---------------------------- */

void app_main(void)
{
    ESP_LOGI(TAG, "Ugradbeni racunalni sustavi -- Lab 2");

    s_evtq = xQueueCreate(8, sizeof(lab2_evt_t));

    led_init();
    button_init();
    timer_init_1hz();

    int ticks = 0;
    int btns = 0;
    lab2_evt_t evt;

    for (;;) {
        if (xQueueReceive(s_evtq, &evt, portMAX_DELAY) != pdTRUE) {
            continue;
        }
        switch (evt.type) {
        case LAB2_EVT_TICK:
            led_set((ticks % 2) == 0);
            ESP_LOGI(TAG, "LAB2 OK tick=%d", ticks);
            ticks++;
            break;
        case LAB2_EVT_BUTTON:
            ESP_LOGI(TAG, "LAB2 OK btn=%d", btns);
            btns++;
            break;
        }
    }
}
