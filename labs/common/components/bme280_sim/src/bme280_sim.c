/*
 * bme280_sim.c -- synthetic bme280.h implementation (QEMU / CI path).
 *
 * Ugradbeni racunalni sustavi -- Lab 3: BME280 (I2C)
 *
 * Entirely wrapped in "#if CONFIG_LAB_SIM_BME280" so that, when the sim is
 * disabled (real-hardware build), this translation unit contributes no
 * symbols at all -- see Kconfig and each lab3_bme280 project's
 * main/bme280_i2c.c, which is wrapped the mirror-image way
 * ("#if !CONFIG_LAB_SIM_BME280").
 * Exactly one of the two ever defines bme280_init()/bme280_read() in a
 * given firmware image; the linker would fail with duplicate symbols if
 * both (or neither) did.
 *
 * Data model (deterministic, no RNG so captures/logs are reproducible):
 *   - temperature: slow sine, 20-25 degC, ~90 s period
 *   - pressure:    slow sine around 1013 hPa, +/-1.5 hPa, ~150 s period
 *   - humidity:    slow sine around 45 %RH, +/-3 %RH, ~120 s period
 * All three use esp_timer_get_time() (microseconds since boot) as the sole
 * time base, so behaviour is identical from one boot to the next.
 */
#include "sdkconfig.h"
#if CONFIG_LAB_SIM_BME280

#include <math.h>
#include "esp_timer.h"
#include "bme280.h"

#define SIM_TEMP_MID_C     22.5f
#define SIM_TEMP_AMPL_C     2.5f     /* 20.0 .. 25.0 degC */
#define SIM_TEMP_PERIOD_US (90.0 * 1e6)

#define SIM_PRESS_MID_HPA   1013.0f
#define SIM_PRESS_AMPL_HPA     1.5f
#define SIM_PRESS_PERIOD_US (150.0 * 1e6)

#define SIM_HUM_MID_PCT     45.0f
#define SIM_HUM_AMPL_PCT     3.0f
#define SIM_HUM_PERIOD_US  (120.0 * 1e6)

static const double TWO_PI = 6.28318530717958647692;

esp_err_t bme280_init(void)
{
    /* Nothing to bring up: no bus, no chip-ID probe, no calibration block --
     * the whole point of the sim is to stand in for hardware that is not
     * present under QEMU. */
    return ESP_OK;
}

esp_err_t bme280_read(float *temp_c, float *press_hpa, float *hum_pct)
{
    int64_t now_us = esp_timer_get_time();

    double phase_t = TWO_PI * ((double)now_us / SIM_TEMP_PERIOD_US);
    double phase_p = TWO_PI * ((double)now_us / SIM_PRESS_PERIOD_US);
    double phase_h = TWO_PI * ((double)now_us / SIM_HUM_PERIOD_US);

    if (temp_c) {
        *temp_c = SIM_TEMP_MID_C + SIM_TEMP_AMPL_C * (float)sin(phase_t);
    }
    if (press_hpa) {
        *press_hpa = SIM_PRESS_MID_HPA + SIM_PRESS_AMPL_HPA * (float)sin(phase_p);
    }
    if (hum_pct) {
        *hum_pct = SIM_HUM_MID_PCT + SIM_HUM_AMPL_PCT * (float)sin(phase_h);
    }
    return ESP_OK;
}

#endif /* CONFIG_LAB_SIM_BME280 */
