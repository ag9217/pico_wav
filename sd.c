#include "sd.h"
#include "log.h"

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
    /* Procedure is as follows:
     *      1. wait for 1 millisecond
     *      2. set CS and MOSI high
     *      3. apply 74 or more clock pulses to SCLK
     *      4. send CMD0 with CS low to reset card
     */
    // setting CS pin high to initialise SPI mode
    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_put(CS_PIN, 1);

    int ret = -1;

    // Sending at least 74 clocks pulses
    sleep_ms(1);
    uint8_t ones[10];
    memset(ones, 0xff, sizeof(ones));
    uint32_t start = get_absolute_time();
    do {
        spi_write_blocking(spi_default, ones, sizeof ones);
    } while (get_absolute_time() - start < 1);

    // CMD0
    sd_card.write(CMD0, 0x00);

    // NCR time
    gpio_put(CS_PIN, 0);
    spi_write_read_blocking(spi_default, ones, NULL, 1);

    // looking for R1 response
    for (int i = 0; i < R1_TIMEOUT; i++) {
        spi_write_read_blocking(spi_default, ones, sd_card.in_buf, 1);

        if (sd_card.in_buf[i] == R1_IDLE_STATE) {
            ret = 0;
            printbuf(sd_card.in_buf, i+1);
            Log(LOG_DEBUG, "Got 0x01 R1 response", ret);
            break;
        }
    }
    sleep_us(10);
    gpio_put(CS_PIN, 1);

    // if no 0x01 R1 response found
    if (ret) {
        Log(LOG_DEBUG, "No R1 response found", ret);
        printbuf(sd_card.in_buf, R1_TIMEOUT);
    }

    return ret;
}

static int sd_init() {
    int ret = 0;

    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);
    // 100 kHz SPI0 bus
    spi_init(spi0, 1000 * 100);

    // Initialize output buffer
    for (size_t i = 0; i < sd_card.buf_len; ++i) {
        sd_card.out_buf[i] = 0;
    }

    // set sd card into spi mode
    if (sd_spi_mode_init() < 0) {
        Log(LOG_ERROR, "Initialization of SD card into SPI mode failed", -1);
        return -1;
    }

    //CMD8
    sd_card.write(CMD8, 0x1AA);

    return ret;
}

static int sd_close() {
    return 0;
}

static int sd_read(uint8_t len) {
    return 0;
}

static int sd_write(uint8_t CMD, uint32_t arg) {
    uint8_t ret = 0;
    uint8_t CRC = 0x01;

    // clear input buffer
    for (int i = 0; i < BUF_LEN; i++)
        sd_card.in_buf[i] = 0;

    const uint32_t a0 = (arg & 0xff000000) >> 24;
    const uint32_t a1 = (arg & 0x00ff0000) >> 16;
    const uint32_t a2 = (arg & 0x0000ff00) >> 8;
    const uint32_t a3 = (arg & 0x000000ff) >> 0;

    if (CMD == CMD0) CRC = 0x95;
    if (CMD == CMD8) CRC = 0x87;

    uint8_t msg[6] = { 0x40 | CMD0 , a0, a1, a2, a3, CRC };

    gpio_put(CS_PIN, 0);
    ret = spi_write_read_blocking(spi_default, msg, sd_card.in_buf, 6);
    sleep_us(10);
    gpio_put(CS_PIN, 1);
    return ret;
}

