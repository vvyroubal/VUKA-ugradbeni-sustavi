/*
 * bme280_regsim.h -- a simulated BME280 register map (Lab 3, QEMU).
 *
 * Ugradbeni racunalni sustavi -- Lab 3: BME280 (I2C)
 *
 * Same contract as one I2C register transaction on the real sensor:
 * read `len` bytes starting at register `reg` (the address
 * auto-increments, as in a burst read), or write one byte. Fixed
 * contents, identical for everyone (see README.md for the values and
 * the results they must give).
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

esp_err_t bme280_regsim_read(uint8_t reg, uint8_t *data, size_t len);
esp_err_t bme280_regsim_write(uint8_t reg, uint8_t val);
