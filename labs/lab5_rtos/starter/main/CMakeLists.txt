idf_component_register(SRCS "pipeline.c" "race.c" "inversion.c"
                        REQUIRES bme280_defs bme280_sim esp_timer)
