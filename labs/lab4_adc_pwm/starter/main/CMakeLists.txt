idf_component_register(SRCS "main.c" "lab_adc_oneshot.c"
                        REQUIRES lab_adc_defs adc_sim esp_adc esp_driver_ledc esp_timer)
