/*
 * Lab 3 -- BME280 over I2C, from the datasheet (starter)
 *
 * Ugradbeni racunalni sustavi -- Lab 3: BME280 (I2C)
 *
 * app_main() talks to bme280.h's shared API only -- it never knows
 * whether it is linked against bme280_sim (QEMU/CI,
 * CONFIG_LAB_SIM_BME280=y, the default here -- so this starter
 * boots and prints in QEMU with no changes) or the real i2c_master
 * driver in bme280_i2c.c (hardware, CONFIG_LAB_SIM_BME280 unset),
 * which still has TODOs to fill in -- see bme280_i2c.c and the
 * manual.
 *
 * Every 2 s: read one T/P/H sample and print
 *   "LAB3 OK T=<c.c>C P=<hhhh.h>hPa H=<rr.r>%"
 *
 * Nothing to implement in THIS file -- the lab's TODOs are all in
 * bme280_i2c.c (register layer + compensation formulas).
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
