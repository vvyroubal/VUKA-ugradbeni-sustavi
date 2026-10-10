#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "lab1";
#define LED_GPIO GPIO_NUM_8   /* onboard LED (SuperMini/DevKitM) */

void app_main(void)
{
    ESP_LOGI(TAG, "Ugradbeni racunalni sustavi -- Lab 1");
    /* TODO 1: configure LED_GPIO as output
     *         (gpio_reset_pin + gpio_set_direction) */
    /* TODO 2: in the loop below, toggle the LED and print the
     *         tick count */
    int n = 0;
    for (;;) {
        /* TODO 2 here */
        (void)n;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
