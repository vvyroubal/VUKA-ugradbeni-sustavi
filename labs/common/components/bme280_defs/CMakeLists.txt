# bme280_defs -- header-only component.
#
# Owns the public bme280.h API (register map, calibration struct, function
# prototypes) shared by both implementations of that API in this course:
#   - bme280_sim        (synthetic data, used in QEMU / CI)
#   - lab3 solution's main/bme280_i2c.c (real i2c_master driver, hardware)
#
# No SRCS: this component contributes only a header search path, never any
# object code of its own.
idf_component_register(INCLUDE_DIRS "include")
