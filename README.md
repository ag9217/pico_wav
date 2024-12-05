# Pico WAV

Custom written SD card driver with FAT file system interface to play WAV files via I2S on the RP2040. Debug UART console is also functional.

Hardware used includes an Adafruit Micro SD SPI breakout board on the pico's SPI0 and Adafruit MAX98360 I2S DAC on the pico's default I2S pins from [pico_audio_i2s](https://github.com/raspberrypi/pico-extras/tree/master/src/rp2_common/pico_audio_i2s).

The current implementation of the SD card driver is quite slow, therefore, during playback the WAV file is not played at full speed. The slow speed is due to the millisecond delays added at the end of SD card reads and writes. From experience, having no delay causes instability and incorrect reads on the bus.

### What currently works
- Debug console works as intended
- Can successfully read WAV files from an SD card
- I2S playback from SD card

### To-Do
- Write custom I2S driver (optional)
- Make SPI/SD driver faster
