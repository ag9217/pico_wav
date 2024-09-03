/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "console.h"

#define PICO_SPI_RX_PIN  11
#define PICO_SPI_CSN_PIN 12
#define PICO_SPI_SCK_PIN 14
#define PICO_SPI_TX_PIN  15

#include <stdio.h>

int main() {
    stdio_init_all();

    // Setting up GPIO
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    console();
}
