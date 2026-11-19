# bme280_regsim -- the BME280 register map in RAM, for Lab 3 under QEMU
# (no I2C bus there). The lab's own driver, main/bme280_i2c.c, reads and
# writes these registers instead of the bus when CONFIG_LAB3_REG_SIM=y,
# so all of its datasheet work (calibration unpacking, compensation)
# runs in the emulator too. See README.md.
idf_component_register(SRCS "src/bme280_regsim.c"
                       INCLUDE_DIRS "include"
                       REQUIRES bme280_defs)
