#include "console.h"
#include "log.h"

#define PICO_SPI_RX_PIN  11
#define PICO_SPI_CSN_PIN 12
#define PICO_SPI_SCK_PIN 14
#define PICO_SPI_TX_PIN  15

#include <stdio.h>

int main() {
    stdio_init_all();

    // setting up GPIO
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // init logger
    log_init(LOG_DEBUG, false);

    console();
}
