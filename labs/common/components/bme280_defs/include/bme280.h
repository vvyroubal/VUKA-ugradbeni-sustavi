/*
 * bme280.h -- shared public API for the Lab 3 BME280 driver family.
 *
 * Ugradbeni racunalni sustavi -- Lab 3: BME280 (I2C)
 *
 * This header is the ONE contract both implementations must satisfy:
 *   - labs/common/components/bme280_sim   -- synthetic data (QEMU / CI path)
 *   - labs/lab3_bme280/{starter,solution}/main/bme280_i2c.c
 *                                          -- real i2c_master driver (hardware path)
 *
 * Exactly one of the two is compiled into any given firmware image, selected
 * purely by the Kconfig option CONFIG_LAB_SIM_BME280 (see
 * labs/common/components/bme280_sim/Kconfig and the top of bme280_i2c.c) --
 * app_main() in main.c never knows or cares which one it is linked against.
 *
 * All register-map constants, offsets and the calibration struct's field
 * widths/signedness below are transcribed from the Bosch BME280 datasheet
 * (Rev. 1.1), "Register description" (sec. 5.3) and "Trimming parameter
 * readout" (sec. 4.2.2).
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------
 * I2C bus address (datasheet sec. 5.2). SDO pin tied to GND -> 0x76 (the
 * value used throughout this lab); tied to VDDIO -> 0x77.
 * --------------------------------------------------------------------- */
#define BME280_I2C_ADDR             0x76

/* ---------------------------------------------------------------------
 * Register map (datasheet Table 18 / sec. 5.3).
 * --------------------------------------------------------------------- */
#define BME280_REG_CHIP_ID          0xD0    /* read-only, always reads 0x60 */
#define BME280_CHIP_ID_VALUE        0x60

#define BME280_REG_RESET            0xE0    /* write 0xB6 to soft-reset */
#define BME280_RESET_VALUE          0xB6

/* Calibration block 1: dig_T1..dig_T3, dig_P1..dig_P9, dig_H1 (26 bytes). */
#define BME280_REG_CALIB00          0x88
#define BME280_REG_CALIB00_LEN      26      /* 0x88 .. 0xA1 inclusive */

/* Calibration block 2: dig_H2..dig_H6 (7 bytes, packed 12-bit H4/H5). */
#define BME280_REG_CALIB26          0xE1
#define BME280_REG_CALIB26_LEN      7       /* 0xE1 .. 0xE7 inclusive */

#define BME280_REG_CTRL_HUM         0xF2    /* humidity oversampling; latched by next ctrl_meas write */
#define BME280_REG_STATUS           0xF3
#define BME280_REG_CTRL_MEAS        0xF4    /* temp/press oversampling + power mode */
#define BME280_REG_CONFIG           0xF5    /* t_sb / filter / spi3w_en */

/* Burst-readable data block: press(3) + temp(3) + hum(2) = 8 bytes. */
#define BME280_REG_PRESS_MSB        0xF7
#define BME280_REG_PRESS_LSB        0xF8
#define BME280_REG_PRESS_XLSB       0xF9
#define BME280_REG_TEMP_MSB         0xFA
#define BME280_REG_TEMP_LSB         0xFB
#define BME280_REG_TEMP_XLSB        0xFC
#define BME280_REG_HUM_MSB          0xFD
#define BME280_REG_HUM_LSB          0xFE
#define BME280_REG_DATA_LEN         8       /* 0xF7 .. 0xFE inclusive */

/* status register bits (sec. 5.3, ctrl/status table) */
#define BME280_STATUS_MEASURING     0x08
#define BME280_STATUS_IM_UPDATE     0x01

/* ctrl_hum / ctrl_meas oversampling encodings (sec. 5.4.3 / 5.4.5, Table 20/24) */
#define BME280_OSRS_SKIPPED         0x00
#define BME280_OSRS_X1              0x01
#define BME280_OSRS_X2              0x02
#define BME280_OSRS_X4              0x03
#define BME280_OSRS_X8              0x04
#define BME280_OSRS_X16             0x05

/* ctrl_meas power-mode encoding (sec. 3.3, Table 25) */
#define BME280_MODE_SLEEP           0x00
#define BME280_MODE_FORCED          0x01
#define BME280_MODE_NORMAL          0x03

/* ---------------------------------------------------------------------
 * Calibration ("trimming") data, unpacked from the two register blocks
 * above. Field widths and signedness are exactly as specified by the
 * datasheet's compensation formulas (sec. 4.2.2 / 4.2.3):
 *   T: dig_T1 unsigned short, dig_T2/dig_T3 signed short
 *   P: dig_P1 unsigned short, dig_P2..dig_P9 signed short
 *   H: dig_H1 unsigned char, dig_H2 signed short, dig_H3 unsigned char,
 *      dig_H4/dig_H5 signed short (12-bit packed), dig_H6 signed char
 * --------------------------------------------------------------------- */
typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
} bme280_calib_t;

/* ---------------------------------------------------------------------
 * Public API -- identical whether backed by bme280_sim or the real
 * i2c_master driver (main/bme280_i2c.c).
 * --------------------------------------------------------------------- */

/* One-time bring-up: bus/device init (real driver) or PRNG/clock seed (sim),
 * chip-ID probe + calibration readout (real driver only). Returns ESP_OK on
 * success, ESP_ERR_NOT_FOUND if the chip-ID probe fails (real driver), or
 * the underlying I2C error otherwise. */
esp_err_t bme280_init(void);

/* Blocking read of one T/H/P sample, in engineering units:
 *   *temp_c     degrees Celsius (e.g. 22.3)
 *   *press_hpa  hectopascals   (e.g. 1013.2)
 *   *hum_pct    percent RH     (e.g. 45.0)
 * Returns ESP_OK on success. */
esp_err_t bme280_read(float *temp_c, float *press_hpa, float *hum_pct);

#ifdef __cplusplus
}
#endif
