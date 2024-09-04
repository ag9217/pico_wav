set(PICO_SDK_PATH /home/armand/tools/pico-sdk/)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(pico_mp3)

pico_sdk_init()

add_executable(pico_mp3
        main.c
        console.c
        commands.c
        log.c
        )

target_link_libraries(pico_mp3
        pico_stdlib              # for core functionality
        pico_multicore
        )

# create map/bin/hex file etc.
pico_add_extra_outputs(pico_mp3)

pico_enable_stdio_usb(pico_mp3 1)
pico_enable_stdio_uart(pico_mp3 1)

