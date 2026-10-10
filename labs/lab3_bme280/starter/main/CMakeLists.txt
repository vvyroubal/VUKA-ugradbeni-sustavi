idf_component_register(SRCS "main.c" "bme280_i2c.c"
                        REQUIRES bme280_defs bme280_sim driver)
