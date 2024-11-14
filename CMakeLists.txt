set(PICO_SDK_PATH /home/armand/tools/pico-sdk/)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include_directories(lib/console)
include_directories(lib/fs)
include_directories(lib/log)
include_directories(drivers/)

project(pico_wav)

pico_sdk_init()

add_executable(pico_wav
        src/main.c
        lib/console/console.c
        lib/console/commands.c
        lib/log/log.c
        drivers/sd.c
        lib/fs/fat.c
        )

target_link_libraries(pico_wav
        pico_stdlib              # for core functionality
        hardware_spi
        pico_audio_i2s
        pico_multicore
        )

target_compile_definitions(pico_wav PRIVATE
        # compile time configuration of I2S
        PICO_AUDIO_I2S_MONO_INPUT=1
        #define for our example code
        USE_AUDIO_I2S=1
)

# create map/bin/hex file etc.
pico_add_extra_outputs(pico_wav)

pico_enable_stdio_usb(pico_wav 1)
pico_enable_stdio_uart(pico_wav 1)

