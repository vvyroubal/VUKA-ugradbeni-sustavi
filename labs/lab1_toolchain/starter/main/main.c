#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "lab1";
#define LED_GPIO GPIO_NUM_8   /* SuperMini LED (DevKitM: manual) */

void app_main(void)
{
    ESP_LOGI(TAG, "Ugradbeni racunalni sustavi -- Lab 1");
    /* TODO 1: configure LED_GPIO as output
     *         (gpio_reset_pin + gpio_set_direction with
     *         GPIO_MODE_OUTPUT) */
    /* TODO 2: in the loop below, each pass: set the LED to n % 2
     *         (gpio_set_level), log "LAB1 OK n=%d" with
     *         ESP_LOGI, then n++ */
    int n = 0;
    for (;;) {
        /* TODO 2 here */
        (void)n;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
