# adc_sim -- synthetic lab_adc.h implementation for QEMU/CI (no analog input
# exists under QEMU emulation).
#
# Kconfig-gated: the entire src file is wrapped in
#   #if CONFIG_LAB_SIM_ADC ... #endif
# so that when CONFIG_LAB_SIM_ADC is unset (real hardware build), this
# translation unit compiles to nothing and the project's own
# main/lab_adc_oneshot.c provides lab_adc_init()/lab_adc_read_raw() instead --
# see Kconfig and README.md in this directory for the full mechanism.
idf_component_register(SRCS "src/adc_sim.c"
                        REQUIRES lab_adc_defs)
