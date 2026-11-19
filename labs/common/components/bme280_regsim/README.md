# bme280_regsim

The BME280 register map in RAM, for Lab 3 under QEMU. QEMU emulates no
I2C bus for the esp32c3, so with `CONFIG_LAB3_REG_SIM=y` (Lab 3's default,
`main/Kconfig.projbuild`) the lab's own driver `main/bme280_i2c.c` calls
`bme280_regsim_read()`/`bme280_regsim_write()` in place of its two I2C
transactions. Everything else in the driver -- the chip-id probe, the
calibration unpacking (TODO 1c) and the compensation formulas (TODO 2a-2c)
-- runs unchanged in the emulator. On the board (`sdkconfig.hw`,
`CONFIG_LAB3_REG_SIM` off) the driver uses the real bus.

Not to be confused with `bme280_sim`, which replaces the whole driver with
synthetic values (Labs 5 and 6).

## Contents and the results they must give

| Registers | Contents | Correct unpacking / result |
|---|---|---|
| `0xD0` | `0x60` | chip id |
| `0x88..0x8D` | `70 6B 43 67 18 FC` | `dig_T1=27504 dig_T2=26435 dig_T3=-1000` |
| `0x8E..0x9F` | `7D 8E 43 D6 D0 0B 27 0B 8C 00 F9 FF 8C 3C F8 C6 70 17` | `dig_P1=36477 dig_P2=-10685 dig_P3=3024 dig_P4=2855 dig_P5=140 dig_P6=-7 dig_P7=15500 dig_P8=-14600 dig_P9=6000` |
| `0xA1` | `4B` | `dig_H1=75` |
| `0xE1..0xE7` | `6A 01 00 13 E5 FC 1E` | `dig_H2=362 dig_H3=0 dig_H4=309 dig_H5=-50 dig_H6=30` |
| `0xF7..0xFE` | `65 5A C0 7E ED 00 6C 00` | `adc_P=415148 adc_T=519888 adc_H=27648` |

The T and P constants and raw values are the Bosch worked example (BMP280
datasheet sec. 3.12), so the compensation gives `t_fine=128422`,
`T=2508` (25.08 degC) and `P` = 1006.5 hPa; humidity comes out at
45.1 %RH. `dig_H5`'s high byte `0xFC` is negative: an unpacking without
sign extension gives `dig_H5=4046`.
