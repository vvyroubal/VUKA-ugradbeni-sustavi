/*
 * bme280_i2c.c -- real BME280 driver over I2C (starter).
 *
 * Ugradbeni racunalni sustavi -- Lab 3: BME280 (I2C)
 *
 * Implements bme280.h using ESP-IDF v5.5's new i2c_master driver
 * (driver/i2c_master.h). Compiled only for the real-hardware build
 * (see labs/common/components/bme280_sim/README.md for the Kconfig
 * mechanism); with the default sim-on config this file compiles to
 * nothing and you won't see your changes here take effect until you
 * build with CONFIG_LAB_SIM_BME280 unset (sdkconfig.hw fragment).
 *
 * The I2C bus/device bring-up (an ESP-IDF API concern, not a
 * datasheet one) is already filled in below. Your job is the two
 * things that actually come straight out of the Bosch BME280
 * datasheet:
 *
 *   TODO 1 (register layer, sec. 5.3/5.4 + Table 16):
 *           reg_read()/reg_write() (the actual I2C transactions)
 *           and the calibration-block byte-unpacking in
 *           read_calibration().
 *   TODO 2 (compensation formulas, sec. 4.2.3): compensate_T()/
 *           compensate_P()/compensate_H(), the Bosch integer math
 *           that turns raw ADC counts into engineering units via
 *           the shared t_fine intermediate.
 *
 * Wiring (adjust I2C_SDA_GPIO/I2C_SCL_GPIO below to your board):
 *   BME280 VDD -> 3V3   BME280 SDA -> GPIO5 (default)
 *   BME280 GND -> GND   BME280 SCL -> GPIO6 (default)
 *   BME280 SDO -> GND (selects I2C address 0x76, matching
 *                      BME280_I2C_ADDR)
 *   BME280 CSB -> VDD (selects I2C mode, not SPI)
 */
#include "sdkconfig.h"
#if !CONFIG_LAB_SIM_BME280

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "bme280.h"

static const char *TAG = "bme280_i2c";

#define I2C_SDA_GPIO   GPIO_NUM_5
#define I2C_SCL_GPIO   GPIO_NUM_6
#define I2C_SCL_HZ     100000
#define I2C_TIMEOUT_MS 1000

static i2c_master_bus_handle_t s_bus;
static i2c_master_dev_handle_t s_dev;
static bme280_calib_t s_calib;
static int32_t s_t_fine;

/* ------------------- register-layer helpers ------------------- */

static esp_err_t reg_read(uint8_t reg, uint8_t *data, size_t len)
{
    (void)reg;
    (void)data;
    (void)len;
    /* TODO 1a: this is a write-then-repeated-start-read: write the
     * single register address byte `reg`, then read `len` bytes
     * into `data`. ESP-IDF's i2c_master driver does exactly this
     * pattern in one call:
     *   return i2c_master_transmit_receive(
     *       s_dev, &reg, 1, data, len, I2C_TIMEOUT_MS);
     */
    return ESP_ERR_NOT_SUPPORTED;
}

static esp_err_t reg_write(uint8_t reg, uint8_t val)
{
    (void)reg;
    (void)val;
    /* TODO 1b: a register write is {reg, val} transmitted as one
     * buffer:
     *   uint8_t buf[2] = { reg, val };
     *   return i2c_master_transmit(s_dev, buf, sizeof(buf),
     *                              I2C_TIMEOUT_MS);
     */
    return ESP_ERR_NOT_SUPPORTED;
}

/* --------------------- calibration readout -------------------- */

static esp_err_t read_calibration(void)
{
    uint8_t b1[BME280_REG_CALIB00_LEN];
    esp_err_t err = reg_read(BME280_REG_CALIB00, b1, sizeof(b1));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "calib block 1 read failed: %s",
                 esp_err_to_name(err));
        return err;
    }

    uint8_t b2[BME280_REG_CALIB26_LEN];
    err = reg_read(BME280_REG_CALIB26, b2, sizeof(b2));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "calib block 2 read failed: %s",
                 esp_err_to_name(err));
        return err;
    }

    /* TODO 1c: unpack b1/b2 into s_calib. Per datasheet Table 16,
     * block 1 is twelve little-endian 16-bit pairs (dig_T1..dig_T3,
     * dig_P1..dig_P9), one reserved byte, then dig_H1; block 2 is
     * dig_H2 (16-bit), dig_H3 (8-bit), then dig_H4/dig_H5 packed as
     * two 12-bit values across three bytes (E4, E5, E6), then
     * dig_H6 (8-bit). Example for the first field:
     *   s_calib.dig_T1 = (uint16_t)(b1[1] << 8 | b1[0]);
     * Mind the signedness in bme280.h's bme280_calib_t --
     * dig_T1/dig_P1 are unsigned, dig_H1/dig_H3 are unsigned 8-bit,
     * dig_H6 is SIGNED 8-bit, everything else is signed 16-bit (or
     * the packed 12-bit dig_H4/dig_H5). When scaling the signed
     * high byte of dig_H4/dig_H5 into place, use `* 16` rather than
     * `<< 4`: left-shifting a negative signed value is undefined
     * behaviour in C (Bosch's reference driver multiplies too).
     */
    (void)b1;
    (void)b2;

    return ESP_OK;
}

/* ------------ Bosch integer compensation formulas --------------
 * TODO 2: transliterate these 1:1 from the BME280 datasheet, sec.
 * 4.2.3. t_fine is a shared intermediate: compensate_T() must run
 * first (it sets s_t_fine), then compensate_P()/compensate_H()
 * (which read it).
 *
 * Once implemented, validate against the datasheet's worked
 * example: dig_T1=27504, dig_T2=26435, dig_T3=-1000, adc_T=519888
 * should give t_fine=128422 and T=2508 (25.08 degC).
 * -------------------------------------------------------------- */

static int32_t compensate_T(int32_t adc_T)
{
    (void)adc_T;
    /* TODO 2a: 32-bit integer formula from datasheet sec. 4.2.3
     * ("BME280_compensate_T_int32"). Structure: compute var1
     * (linear term from dig_T1/dig_T2) and var2 (quadratic term
     * from dig_T1/dig_T3), store their sum in s_t_fine
     * (compensate_P/compensate_H depend on it), and return the
     * temperature in centi-degrees Celsius derived from s_t_fine.
     * Transliterate the datasheet listing exactly -- every shift
     * and cast matters.
     *
     * Self-check with the datasheet's worked example:
     * dig_T1=27504, dig_T2=26435, dig_T3=-1000, adc_T=519888 must
     * give t_fine=128422 and a return value of 2508
     * (= 25.08 degC). */
    s_t_fine = 0;
    return 0;
}

static uint32_t compensate_P(int32_t adc_P)
{
    (void)adc_P;
    /* TODO 2b: 64-bit Q24.8 fixed-point formula from the datasheet,
     * using s_t_fine set by compensate_T() above. See the solution
     * or the datasheet sec. 4.2.3 for the full var1/var2/p
     * derivation. */
    return 0;
}

static uint32_t compensate_H(int32_t adc_H)
{
    (void)adc_H;
    /* TODO 2c: Q22.10 fixed-point formula from the datasheet, using
     * s_t_fine set by compensate_T() above. See the solution or the
     * datasheet sec. 4.2.3 for the full derivation. */
    return 0;
}

/* ----------------------------- API ---------------------------- */

esp_err_t bme280_init(void)
{
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    esp_err_t err = i2c_new_master_bus(&bus_cfg, &s_bus);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_new_master_bus failed: %s",
                 esp_err_to_name(err));
        return err;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BME280_I2C_ADDR,
        .scl_speed_hz = I2C_SCL_HZ,
    };
    err = i2c_master_bus_add_device(s_bus, &dev_cfg, &s_dev);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_master_bus_add_device failed: %s",
                 esp_err_to_name(err));
        return err;
    }

    uint8_t chip_id = 0;
    err = reg_read(BME280_REG_CHIP_ID, &chip_id, 1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "chip-id read failed: %s",
                 esp_err_to_name(err));
        return err;
    }
    if (chip_id != BME280_CHIP_ID_VALUE) {
        ESP_LOGE(TAG, "unexpected chip id 0x%02X (want 0x%02X)",
                 chip_id, BME280_CHIP_ID_VALUE);
        return ESP_ERR_NOT_FOUND;
    }

    err = read_calibration();
    if (err != ESP_OK) {
        return err;
    }

    /* Humidity oversampling x1. Must be written before
     * ctrl_meas. */
    err = reg_write(BME280_REG_CTRL_HUM, BME280_OSRS_X1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ctrl_hum write failed: %s",
                 esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG,
             "BME280 found (chip id 0x%02X), calibration loaded",
             chip_id);
    return ESP_OK;
}

esp_err_t bme280_read(float *temp_c, float *press_hpa,
                      float *hum_pct)
{
    /* osrs_t=x1 (bits 7:5=001), osrs_p=x1 (bits 4:2=001),
     * mode=forced (bits 1:0=01) */
    uint8_t ctrl_meas = (BME280_OSRS_X1 << 5) |
                        (BME280_OSRS_X1 << 2) |
                        BME280_MODE_FORCED;
    esp_err_t err = reg_write(BME280_REG_CTRL_MEAS, ctrl_meas);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ctrl_meas write failed: %s",
                 esp_err_to_name(err));
        return err;
    }

    /* Forced-mode conversion time for x1/x1/x1 oversampling is a
     * few ms (datasheet sec. 9.1); poll the "measuring" status bit
     * with a small fixed budget rather than trusting a single
     * guessed delay. */
    for (int i = 0; i < 20; i++) {
        uint8_t status = 0;
        err = reg_read(BME280_REG_STATUS, &status, 1);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "status read failed: %s",
                     esp_err_to_name(err));
            return err;
        }
        if ((status & BME280_STATUS_MEASURING) == 0) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    /* Zero-initialized so the file compiles under
     * -Werror=maybe-uninitialized even while reg_read() is still
     * the TODO stub that never fills it in. */
    uint8_t data[BME280_REG_DATA_LEN] = {0};
    err = reg_read(BME280_REG_PRESS_MSB, data, sizeof(data));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "data burst read failed: %s",
                 esp_err_to_name(err));
        return err;
    }

    int32_t adc_P = ((int32_t)data[0] << 12) |
                    ((int32_t)data[1] << 4) | (data[2] >> 4);
    int32_t adc_T = ((int32_t)data[3] << 12) |
                    ((int32_t)data[4] << 4) | (data[5] >> 4);
    int32_t adc_H = ((int32_t)data[6] << 8) | data[7];

    /* must run first: sets s_t_fine */
    int32_t T = compensate_T(adc_T);
    uint32_t P = compensate_P(adc_P);
    uint32_t H = compensate_H(adc_H);

    if (temp_c) {
        *temp_c = T / 100.0f;
    }
    if (press_hpa) {
        *press_hpa = (P / 256.0f) / 100.0f;
    }
    if (hum_pct) {
        *hum_pct = H / 1024.0f;
    }
    return ESP_OK;
}

#endif /* !CONFIG_LAB_SIM_BME280 */
