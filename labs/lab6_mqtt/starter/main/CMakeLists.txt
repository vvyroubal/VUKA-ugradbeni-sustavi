# The CA certificate is generated per run by broker/gen_certs.sh and is NOT
# committed (see .gitignore). Drop a clearly-marked placeholder if certs/ca.crt
# is absent so a fresh checkout still builds; running broker/gen_certs.sh
# overwrites it with the real CA cert you embed for TLS verification.
set(_ca_cert "${CMAKE_CURRENT_SOURCE_DIR}/certs/ca.crt")
if(NOT EXISTS "${_ca_cert}")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/certs")
    file(WRITE "${_ca_cert}"
"-----BEGIN CERTIFICATE-----\n\
PLACEHOLDER-CA-NOT-VALID-run-broker/gen_certs.sh-to-regenerate\n\
-----END CERTIFICATE-----\n")
endif()

idf_component_register(SRCS "main.c"
                       REQUIRES bme280_defs bme280_sim
                                protocol_examples_common
                                mqtt esp-tls nvs_flash esp_event esp_netif
                       EMBED_TXTFILES "certs/ca.crt")
