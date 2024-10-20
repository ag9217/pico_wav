set(PICO_SDK_PATH /home/armand/tools/pico-sdk/)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

include_directories(lib/console)
include_directories(lib/fs)
include_directories(lib/log)
include_directories(drivers/)

project(pico_mp3)

pico_sdk_init()

add_executable(pico_mp3
        src/main.c
        lib/console/console.c
        lib/console/commands.c
        lib/log/log.c
        drivers/sd.c
        lib/fs/fat.c
        )

target_link_libraries(pico_mp3
        pico_stdlib              # for core functionality
        hardware_spi
        pico_multicore
        )

# create map/bin/hex file etc.
pico_add_extra_outputs(pico_mp3)

pico_enable_stdio_usb(pico_mp3 1)
pico_enable_stdio_uart(pico_mp3 1)

