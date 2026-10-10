# lab_adc_defs -- header-only component.
#
# Owns the public lab_adc.h API (lab_adc_init()/lab_adc_read_raw()) shared by
# both implementations of that API in this course:
#   - adc_sim                                       (synthetic data, used in QEMU / CI)
#   - lab4 solution's main/lab_adc_oneshot.c         (real adc_oneshot driver, hardware)
#
# No SRCS: this component contributes only a header search path, never any
# object code of its own.
idf_component_register(INCLUDE_DIRS "include")
