cmake_minimum_required(VERSION 3.13)

include(../pico_sdk_import.cmake)

project(pio-test)

pico_sdk_init()

add_executable(pio-test main.cc)

pico_generate_pio_header(pio-test ${CMAKE_CURRENT_LIST_DIR}/test.pio)
pico_enable_stdio_usb(pio-test 1)
pico_enable_stdio_uart(pio-test 0)

pico_add_extra_outputs(pio-test)
target_link_libraries(pio-test pico_stdlib hardware_pio)
