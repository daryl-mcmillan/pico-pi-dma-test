cmake_minimum_required(VERSION 3.13)

include(../pico_sdk_import.cmake)

project(dma-test)

pico_sdk_init()

add_executable(dma-test main.cc)

pico_generate_pio_header(dma-test ${CMAKE_CURRENT_LIST_DIR}/multiply.pio)
pico_enable_stdio_usb(dma-test 1)
pico_enable_stdio_uart(dma-test 0)

pico_add_extra_outputs(dma-test)
target_link_libraries(dma-test pico_stdlib hardware_dma hardware_pio)
