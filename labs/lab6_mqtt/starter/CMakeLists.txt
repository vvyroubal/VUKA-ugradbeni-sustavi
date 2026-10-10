cmake_minimum_required(VERSION 3.16)

# Two extra component trees:
#   ../../common/components  -> bme280_sim / bme280_defs (shared telemetry source)
#   $IDF_PATH/.../protocol_examples_common -> example_connect() network bring-up
#     (openeth + DHCP under QEMU; see labs/common/run_qemu_net.sh).
set(EXTRA_COMPONENT_DIRS
    ../../common/components
    $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(lab6)
