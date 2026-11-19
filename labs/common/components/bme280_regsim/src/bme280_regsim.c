/*
 * bme280_regsim.c -- a simulated BME280 register map (Lab 3, QEMU).
 *
 * Ugradbeni racunalni sustavi -- Lab 3: BME280 (I2C)
 *
 * Only the registers Lab 3 reads have contents; everything else
 * reads 0x00. Writes are stored, so a read-back sees them, but they
 * do not start a conversion: the measurement block is fixed.
 */
#include <string.h>
#include "bme280.h"
#include "bme280_regsim.h"

/* Calibration block 1, 0x88..0xA1 (datasheet Table 16), little-
 * endian pairs. dig_T1..T3 and dig_P1..P9 are the constants of the
 * Bosch worked example (BMP280 datasheet sec. 3.12). */
static const uint8_t k_calib1[BME280_REG_CALIB00_LEN] = {
    0x70, 0x6B,  /* dig_T1 =  27504 */
    0x43, 0x67,  /* dig_T2 =  26435 */
    0x18, 0xFC,  /* dig_T3 =  -1000 */
    0x7D, 0x8E,  /* dig_P1 =  36477 */
    0x43, 0xD6,  /* dig_P2 = -10685 */
    0xD0, 0x0B,  /* dig_P3 =   3024 */
    0x27, 0x0B,  /* dig_P4 =   2855 */
    0x8C, 0x00,  /* dig_P5 =    140 */
    0xF9, 0xFF,  /* dig_P6 =     -7 */
    0x8C, 0x3C,  /* dig_P7 =  15500 */
    0xF8, 0xC6,  /* dig_P8 = -14600 */
    0x70, 0x17,  /* dig_P9 =   6000 */
    0x00,        /* 0xA0, reserved  */
    0x4B,        /* dig_H1 =     75 */
};

/* Calibration block 2, 0xE1..0xE7. dig_H4/dig_H5 are 12-bit fields
 * sharing 0xE5; dig_H5's high byte 0xFC is negative, so an unpacking
 * that forgets the sign extension gives 4046 instead of -50. */
static const uint8_t k_calib2[BME280_REG_CALIB26_LEN] = {
    0x6A, 0x01,  /* dig_H2 = 362                        */
    0x00,        /* dig_H3 = 0                          */
    0x13,        /* 0xE4: dig_H4[11:4]                  */
    0xE5,        /* 0xE5: dig_H5[3:0] | dig_H4[3:0]     */
    0xFC,        /* 0xE6: dig_H5[11:4] -> H4 309, H5 -50 */
    0x1E,        /* dig_H6 = 30                         */
};

/* Measurement block 0xF7..0xFE: adc_P = 415148 and adc_T = 519888
 * (the Bosch example again), adc_H = 0x6C00 = 27648. */
static const uint8_t k_data[BME280_REG_DATA_LEN] = {
    0x65, 0x5A, 0xC0,  /* press_msb/lsb/xlsb */
    0x7E, 0xED, 0x00,  /* temp_msb/lsb/xlsb  */
    0x6C, 0x00,        /* hum_msb/lsb        */
};

static uint8_t s_regs[256];
static int s_ready;

static void regsim_init(void)
{
    memcpy(&s_regs[BME280_REG_CALIB00], k_calib1, sizeof(k_calib1));
    memcpy(&s_regs[BME280_REG_CALIB26], k_calib2, sizeof(k_calib2));
    memcpy(&s_regs[BME280_REG_PRESS_MSB], k_data, sizeof(k_data));
    s_regs[BME280_REG_CHIP_ID] = BME280_CHIP_ID_VALUE;
    s_ready = 1;
}

esp_err_t bme280_regsim_read(uint8_t reg, uint8_t *data, size_t len)
{
    if (!s_ready) {
        regsim_init();
    }
    if (data == NULL || (size_t)reg + len > sizeof(s_regs)) {
        return ESP_ERR_INVALID_ARG;
    }
    memcpy(data, &s_regs[reg], len);
    return ESP_OK;
}

esp_err_t bme280_regsim_write(uint8_t reg, uint8_t val)
{
    if (!s_ready) {
        regsim_init();
    }
    if (reg == BME280_REG_RESET || reg == BME280_REG_STATUS) {
        return ESP_OK;              /* no effect in the simulation */
    }
    s_regs[reg] = val;
    return ESP_OK;
}
