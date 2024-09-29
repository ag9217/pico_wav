#include <boards/pico.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "sd.h"
#include <string.h>
#include "pico/time.h"
#include "hardware/spi.h"

#define CS_PIN 22

struct sd sd_card = {
    .buf_len = BUF_LEN,
    .init = sd_init,
    .close = sd_close,
    .read = sd_read,
    .write = sd_write
};

static void printbuf(uint8_t buf[], size_t len) {
    size_t i;
    for (i = 0; i < len; ++i) {
        if (i % 16 == 15)
            printf("%02x\n", buf[i]);
        else
            printf("%02x ", buf[i]);
    }

    // append trailing newline if there isn't one
    if (i % 16) {
        putchar('\n');
    }
}

static int sd_spi_mode_init() {
    // setting CSI pin high to initialise SPI mode
    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_put(CS_PIN, 1);

    // Sending at least 74 clocks pulses
    sleep_ms(1);
    uint8_t ones[10];
    memset(ones, 0xff, sizeof(ones));
    uint32_t start = get_absolute_time();
    do {
        spi_write_blocking(spi_default, ones, sizeof ones);
    } while (get_absolute_time() - start < 1);

    // CMD 0
    uint8_t soft_reset[6] = { 0 };
    soft_reset[0] = 0x40 | CMD0;
    // 0x95 CRC always the case for CMD0
    soft_reset[5] = 0x95;

    gpio_put(CS_PIN, 0);
    spi_write_read_blocking(spi_default, soft_reset, NULL, 6);
    spi_write_read_blocking(spi_default, ones, sd_card.out_buf, 2);
    sleep_us(10);
    gpio_put(CS_PIN, 1);

    printbuf(sd_card.out_buf, 2);

    return 0;
}

static int sd_init() {
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);

    spi_init(spi0, 1000 * 100);

    // Initialize output buffer
    for (size_t i = 0; i < sd_card.buf_len; ++i) {
        sd_card.out_buf[i] = 0;
    }

    // set sd card into spi mode
    sd_spi_mode_init();

    //for (size_t i = 0; i < 5; ++i) {
    //    // Write the output buffer to MOSI, and at the same time read from MISO.
    //    spi_write_read_blocking(spi_default, sd_card.out_buf, sd_card.in_buf, sd_card.buf_len);

    //    // Write to stdio whatever came in on the MISO line.
    //    printf("SPI master says: read page %d from the MISO line:\n", i);
    //    printbuf(sd_card.in_buf, sd_card.buf_len);

    //    // Sleep for ten seconds so you get a chance to read the output.
    //    sleep_ms(1 * 1000);
    //}

    return 0;
}

static int sd_close() {
    return 0;
}

static int sd_read() {
    return 0;
}

static int sd_write() {
    return 0;
}

