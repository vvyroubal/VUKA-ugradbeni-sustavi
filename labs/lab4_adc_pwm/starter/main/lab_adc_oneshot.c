/*
 * lab_adc_oneshot.c -- real ADC driver over adc_oneshot (ESP32-C3,
 * hardware).
 *
 * Ugradbeni racunalni sustavi -- Lab 4: ADC histograms, two-point
 * calibration, PWM
 *
 * Implements lab_adc.h using ESP-IDF v5.5's adc_oneshot driver
 * (esp_adc/adc_oneshot.h). Compiled only for the real-hardware
 * build; see labs/common/components/adc_sim/README.md for the
 * Kconfig mechanism ("#if !CONFIG_LAB_SIM_ADC" below is the mirror
 * image of adc_sim.c's "#if CONFIG_LAB_SIM_ADC").
 *
 * There is nothing to fill in here -- this file is peripheral-init
 * "plumbing" (ADC unit/channel bring-up), not this lab's learning
 * objective. The lab's TODOs (histogram binning, calibration
 * formula, PWM duty sweep) all live in main.c and apply equally
 * whichever ADC implementation is linked in.
 *
 * Wiring: feed a ~1.5 V signal (e.g. a potentiometer across
 * 3V3/GND, or a bench supply) into GPIO0 (ADC1 channel 0 on
 * ESP32-C3).
 *
 * Configuration:
 *   - ADC1 channel 0 (GPIO0), 12-bit width (codes 0..4095).
 *   - ADC_ATTEN_DB_12: extends the measurable input range to
 *     roughly 0..2.5 V on ESP32-C3, comfortably covering a ~1.5 V
 *     nominal signal with headroom -- this 2.5 V full-scale
 *     assumption is also what adc_sim.c's synthetic transfer
 *     function and this lab's two-point calibration formula
 *     (main.c) are built around.
 */
#include "sdkconfig.h"
#if !CONFIG_LAB_SIM_ADC

#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "lab_adc.h"

static const char *TAG = "lab_adc_oneshot";

#define LAB_ADC_UNIT     ADC_UNIT_1
#define LAB_ADC_CHANNEL  ADC_CHANNEL_0   /* GPIO0 on ESP32-C3 */
#define LAB_ADC_ATTEN    ADC_ATTEN_DB_12
#define LAB_ADC_BITWIDTH ADC_BITWIDTH_12

static adc_oneshot_unit_handle_t s_adc_handle;

esp_err_t lab_adc_init(void)
{
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = LAB_ADC_UNIT,
    };
    esp_err_t err = adc_oneshot_new_unit(&init_cfg, &s_adc_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "adc_oneshot_new_unit failed: %s",
                 esp_err_to_name(err));
        return err;
    }

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = LAB_ADC_ATTEN,
        .bitwidth = LAB_ADC_BITWIDTH,
    };
    err = adc_oneshot_config_channel(s_adc_handle, LAB_ADC_CHANNEL,
                                     &chan_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "adc_oneshot_config_channel failed: %s",
                 esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG,
        "ADC1 channel 0 (GPIO0) ready, 12-bit, ADC_ATTEN_DB_12");
    return ESP_OK;
}

esp_err_t lab_adc_read_raw(int *raw)
{
    return adc_oneshot_read(s_adc_handle, LAB_ADC_CHANNEL, raw);
}

#endif /* !CONFIG_LAB_SIM_ADC */
