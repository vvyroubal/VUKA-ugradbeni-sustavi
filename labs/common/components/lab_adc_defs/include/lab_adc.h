/*
 * lab_adc.h -- shared public API for the Lab 4 ADC driver family.
 *
 * Ugradbeni racunalni sustavi -- Lab 4: ADC histograms, two-point
 * calibration, PWM
 *
 * This header is the ONE contract both implementations must satisfy:
 *   - labs/common/components/adc_sim               -- synthetic data (QEMU / CI path)
 *   - labs/lab4_adc_pwm/{starter,solution}/main/lab_adc_oneshot.c
 *                                                    -- real adc_oneshot driver (hardware path)
 *
 * Exactly one of the two is compiled into any given firmware image, selected
 * purely by the Kconfig option CONFIG_LAB_SIM_ADC (see
 * labs/common/components/adc_sim/Kconfig and the top of lab_adc_oneshot.c) --
 * app_main() in main.c never knows or cares which one it is linked against.
 *
 * Both implementations read ADC1 channel 0 (GPIO0 on ESP32-C3), 12-bit
 * width, with an assumed/attenuation-matched full-scale of ~2.5 V -- see
 * adc_sim/README.md for the exact ground-truth numbers the sim uses and
 * lab_adc_oneshot.c for the real driver's attenuation choice.
 */
#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* One-time bring-up: adc_oneshot unit/channel config (real driver) or PRNG
 * seed reset (sim, so every boot reproduces the identical sample sequence).
 * Returns ESP_OK on success. */
esp_err_t lab_adc_init(void);

/* Blocking read of one 12-bit raw ADC code, range [0, 4095].
 * Returns ESP_OK on success. */
esp_err_t lab_adc_read_raw(int *raw);

#ifdef __cplusplus
}
#endif
