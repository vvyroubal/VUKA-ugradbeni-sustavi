/*
 * Lab 4 -- ADC histograms, two-point calibration, PWM (starter)
 *
 * Ugradbeni racunalni sustavi -- Lab 4: ADC histograms, two-point
 * calibration, PWM
 *
 * app_main() talks to lab_adc.h's shared API only -- it never knows
 * whether it is linked against adc_sim (QEMU/CI,
 * CONFIG_LAB_SIM_ADC=y, the default here -- so this starter boots
 * and prints in QEMU with no changes) or the real adc_oneshot
 * driver in lab_adc_oneshot.c (hardware, CONFIG_LAB_SIM_ADC unset).
 * See labs/common/components/adc_sim/README.md for the Kconfig
 * selection mechanism and the sim's exact ground-truth numbers.
 *
 * Three TODOs in this file (nothing to do in lab_adc_oneshot.c --
 * that's peripheral-init plumbing, not this lab's learning
 * objective):
 *
 *   TODO 1 (manual T1): sample standard deviation + 16-bin
 *           histogram over the observed min..max range.
 *   TODO 2 (manual T2): two-point calibration formula (gain/offset
 *           from Kconfig).
 *   TODO 3 (manual T3): PWM duty sweep, driven by an esp_timer
 *           callback at ~50 Hz.
 *
 * Everything else (sampling loop, histogram printout,
 * LEDC/esp_timer bring-up, the "LAB4 OK ..." log lines) is given --
 * this boots and prints a banner out of the box; the three
 * "LAB4 OK ..." markers only carry correct values once the TODOs
 * above are filled in.
 */
#include <math.h>
#include <stdio.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/ledc.h"

#include "lab_adc.h"

static const char *TAG = "lab4";

/* --------------------- Part 1: histogram ---------------------- */

#define NUM_SAMPLES 1000
#define NUM_BINS      16
/* cap the widest histogram bar's printed width */
#define HIST_MAX_STARS 40

static int s_samples[NUM_SAMPLES];

/* --------------- Part 2: two-point calibration ---------------- */

/* Assumed ADC full-scale for the chosen attenuation
 * (ADC_ATTEN_DB_12 on the real driver, see lab_adc_oneshot.c) --
 * matches adc_sim's documented ground truth
 * (labs/common/components/adc_sim/README.md), so the ideal 1.5 V
 * code is round(1500/2500 * 4095) = 2457. */
#define ADC_FULLSCALE_MV 2500.0
#define ADC_MAX_RAW      4095

/* ------------------------ Part 3: PWM ------------------------- */

#define PWM_GPIO          GPIO_NUM_8
#define PWM_FREQ_HZ       5000
/* 10-bit -> duty range [0,1023] */
#define PWM_DUTY_RES      LEDC_TIMER_10_BIT
#define PWM_MAX_DUTY      1023
#define PWM_UPDATE_HZ     50    /* esp_timer callback rate */
#define PWM_UPDATE_PERIOD_US (1000000 / PWM_UPDATE_HZ)
#define PWM_SWEEP_STEPS   100   /* steps from 0 to max (or back) */
#define PWM_STEP          (PWM_MAX_DUTY / PWM_SWEEP_STEPS)

static esp_timer_handle_t s_pwm_timer;
static int s_pwm_duty = 0;
static int s_pwm_dir = 1;

static void pwm_sweep_cb(void *arg)
{
    (void)arg;

    /* TODO 3: sweep s_pwm_duty 0 -> PWM_MAX_DUTY -> 0, PWM_STEP per
     * call, reversing s_pwm_dir at each bound (clamp first, don't
     * just check equality -- PWM_STEP may not divide PWM_MAX_DUTY
     * evenly), then push the new duty out to the LEDC peripheral:
     *   ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0,
     *                 s_pwm_duty);
     *   ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
     * (ledc_set_duty() alone does not take effect --
     * ledc_update_duty() latches it.)
     *
     * Left as a no-op below: the duty never changes and the LED
     * never turns on until you fill this in. */
}

static esp_err_t pwm_init_and_start(void)
{
    ledc_timer_config_t timer_cfg = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = PWM_DUTY_RES,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    esp_err_t err = ledc_timer_config(&timer_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ledc_timer_config failed: %s",
                 esp_err_to_name(err));
        return err;
    }

    ledc_channel_config_t chan_cfg = {
        .gpio_num = PWM_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    err = ledc_channel_config(&chan_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ledc_channel_config failed: %s",
                 esp_err_to_name(err));
        return err;
    }

    const esp_timer_create_args_t timer_args = {
        .callback = &pwm_sweep_cb,
        .name = "pwm_sweep",
    };
    err = esp_timer_create(&timer_args, &s_pwm_timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_timer_create failed: %s",
                 esp_err_to_name(err));
        return err;
    }

    return esp_timer_start_periodic(s_pwm_timer,
                                    PWM_UPDATE_PERIOD_US);
}

/* -------------------------- app_main -------------------------- */

void app_main(void)
{
    ESP_LOGI(TAG, "Ugradbeni racunalni sustavi -- Lab 4");

    ESP_ERROR_CHECK(lab_adc_init());

    /* --- Part 1: sample + histogram + mean/sd --- */

    int raw_min = ADC_MAX_RAW;
    int raw_max = 0;
    long sum = 0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        int raw = 0;
        ESP_ERROR_CHECK(lab_adc_read_raw(&raw));
        s_samples[i] = raw;
        sum += raw;
        if (raw < raw_min) {
            raw_min = raw;
        }
        if (raw > raw_max) {
            raw_max = raw;
        }
    }

    double mean = (double)sum / NUM_SAMPLES;

    /* TODO 1: compute the sample standard deviation and the 16-bin
     * histogram over the OBSERVED [raw_min, raw_max) range
     * (manual, T1):
     *   - sd = sqrt( sum((x_i - mean)^2) / NUM_SAMPLES )
     *   - each sample's bin index =
     *       (int)((x_i - raw_min) / bin_width),
     *     clamped to NUM_BINS-1 for the one sample exactly at
     *     raw_max (bin_width is already computed below for you)
     * Left as a no-op below: sd stays 0.0 and every bin_counts[]
     * stays 0 (an empty-looking histogram) until you fill this
     * in. */
    double sd = 0.0;
    int bin_counts[NUM_BINS] = {0};
    double bin_width = (raw_max > raw_min)
        ? (double)(raw_max - raw_min) / NUM_BINS
        : 1.0;

    int max_count = 0;
    for (int b = 0; b < NUM_BINS; b++) {
        if (bin_counts[b] > max_count) {
            max_count = bin_counts[b];
        }
    }

    printf("Lab 4 -- ADC raw histogram "
           "(%d samples, %d bins, range %d..%d)\n",
           NUM_SAMPLES, NUM_BINS, raw_min, raw_max);
    for (int b = 0; b < NUM_BINS; b++) {
        double lo = raw_min + bin_width * b;
        double hi = lo + bin_width;
        int stars = bin_counts[b];
        if (max_count > HIST_MAX_STARS) {
            stars = (bin_counts[b] * HIST_MAX_STARS) / max_count;
        }
        printf("[%6.1f, %6.1f) | ", lo, hi);
        for (int s = 0; s < stars; s++) {
            putchar('#');
        }
        printf(" (%d)\n", bin_counts[b]);
    }

    ESP_LOGI(TAG, "LAB4 OK mean=%.1f sd=%.1f", mean, sd);

    /* --- Part 2: two-point calibration --- */

    /* TODO 2: apply the two-point calibration formula using
     * CONFIG_LAB4_CAL_GAIN_PPM / CONFIG_LAB4_CAL_OFFSET (manual,
     * T2; see main/Kconfig.projbuild for the derivation):
     *   corrected_raw = (mean - CONFIG_LAB4_CAL_OFFSET)
     *                   * CONFIG_LAB4_CAL_GAIN_PPM / 1e6
     *   voltage_v = corrected_raw
     *               * (ADC_FULLSCALE_MV / ADC_MAX_RAW) / 1000.0
     * Left as a no-op below: voltage_v stays 0.000 until you fill
     * this in. */
    double voltage_v = 0.0;

    ESP_LOGI(TAG, "LAB4 OK volt=%.3f", voltage_v);

    /* --- Part 3: PWM breathing LED --- */

    ESP_ERROR_CHECK(pwm_init_and_start());
    ESP_LOGI(TAG, "LAB4 OK pwm=running");

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
