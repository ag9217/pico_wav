#include <stdio.h>
#include "console.h"
#include "log.h"

int main() {
    stdio_init_all();

    // setting up GPIO
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // init logger
    log_init(LOG_DEBUG, false);

    console();
}
