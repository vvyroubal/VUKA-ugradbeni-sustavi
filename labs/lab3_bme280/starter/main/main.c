/*
 * Lab 3 -- BME280 over I2C, from the datasheet (starter)
 *
 * Ugradbeni racunalni sustavi -- Lab 3: BME280 (I2C)
 *
 * Nothing to change in this file: all of the lab's TODOs are in
 * bme280_i2c.c. app_main() initialises the sensor and, every 2 s,
 * prints one sample:
 *   "LAB3 OK T=<c.c>C P=<hhhh.h>hPa H=<rr.r>%"
 * In QEMU the values come from the simulated registers through
 * YOUR driver code: all zeros until TODO 1c and 2a-2c are done.
 */
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "bme280.h"

static const char *TAG = "lab3";

void app_main(void)
{
    ESP_LOGI(TAG, "Ugradbeni racunalni sustavi -- Lab 3");

    ESP_ERROR_CHECK(bme280_init());

    for (;;) {
        float temp_c = 0.0f, press_hpa = 0.0f, hum_pct = 0.0f;
        esp_err_t err = bme280_read(&temp_c, &press_hpa, &hum_pct);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "LAB3 OK T=%.1fC P=%.1fhPa H=%.1f%%",
                     temp_c, press_hpa, hum_pct);
        } else {
            ESP_LOGE(TAG, "bme280_read failed: %s",
                     esp_err_to_name(err));
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
