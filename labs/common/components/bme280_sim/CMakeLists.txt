# bme280_sim -- synthetic bme280.h implementation for QEMU/CI (no I2C bus
# exists under QEMU emulation).
#
# Kconfig-gated: the entire src file is wrapped in
#   #if CONFIG_LAB_SIM_BME280 ... #endif
# so that when CONFIG_LAB_SIM_BME280 is unset (real hardware build), this
# translation unit compiles to nothing and the project's own
# main/bme280_i2c.c provides bme280_init()/bme280_read() instead -- see
# Kconfig and README.md in this directory for the full mechanism.
idf_component_register(SRCS "src/bme280_sim.c"
                        REQUIRES bme280_defs esp_timer)
