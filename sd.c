#include "sd.h"
#include "log.h"
#include "fat.h"
#include <pico/time.h>

struct sd sd_card = {
    .buf_len = BUF_LEN,
    .init = sd_init,
    .close = sd_close,
    .read = sd_read,
    .read_block = sd_read_block,
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

static void clear_input_buf() {
    // clear input buffer
    for (int i = 0; i < BUF_LEN; i++)
        sd_card.in_buf[i] = 0;
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

    // Sending at least 74 clocks pulses
    sleep_ms(1);
    uint8_t ones[10];
    memset(ones, 0xff, sizeof(ones));
    spi_write_blocking(spi_default, ones, sizeof ones);

    // CMD0
    sd_card.write(CMD0, 0x00);

    // NCR time
    gpio_put(CS_PIN, 0);
    spi_write_read_blocking(spi_default, ones, NULL, 1);

    // looking for R1 response
    for (int i = 0; i < R1_TIMEOUT; i++) {
        spi_write_read_blocking(spi_default, ones, sd_card.in_buf, 1);

        if (sd_card.in_buf[i] == R1_IDLE_STATE) {
            sleep_us(10);
            gpio_put(CS_PIN, 1);
            Log(LOG_DEBUG, "Got 0x01 R1 response", 0);
            return 0;
        }
    }

    Log(LOG_DEBUG, "No R1 response found", -1);
    printbuf(sd_card.in_buf, R1_TIMEOUT);

    return -1;
}

static int sd_init() {
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

    // CMD8
    sd_card.write(CMD8, 0x1aa);
    sd_card.read(5);

    // byte 1 is R1
    uint8_t R1 = sd_card.in_buf[0];
    // remaining 4 bytes are R7, first two are 0x00 because of 0x000001aa sent
    uint8_t R7_b1 = sd_card.in_buf[3];
    uint8_t R7_b2 = sd_card.in_buf[4];

    if (R1 != 0x01 || (R7_b1 != 0x01 && R7_b2 != 0xaa)) {
        Log(LOG_ERROR, "Did not receive matched R7 response", -2);
        return -2;
    }
    Log(LOG_DEBUG, "Got matched R7 response", 0);

    // ACMD41, ACMDs start with CMD55
    // 0x00 R1 response byte can take up to 1 second to arrive
    uint32_t start_ms = to_ms_since_boot(get_absolute_time());
    do {
        sd_card.write(CMD55, 0);
        sd_card.read(2);
        sd_card.write(ACMD41, 0x40000000);
        sd_card.read(2);

        if(sd_card.in_buf[0] == R1_READY_STATE) {
            Log(LOG_DEBUG, "ACMD41 0x00 R1 response", 0);
            break;
        }
    } while (to_ms_since_boot(get_absolute_time()) - start_ms < 1000);

    // TODO: try not to duplicate code here
    // if time difference is longer than a second, no 0x00 R1 response was found
    if (to_ms_since_boot(get_absolute_time()) - start_ms > 1000) {
        Log(LOG_ERROR, "Did not receive 0x01 R1 response from ACMD41", -3);
        return -3;
    }

    // CMD58
    sd_card.write(CMD58, 0);
    // R1 (1 byte) + R3 (4 bytes)
    sd_card.read(5);

    // bit 30 in OCR is set high, meaning SDHC card
    if ((sd_card.in_buf[1] & 0x40) != 0x40) {
        Log(LOG_ERROR, "Only SDHC cards are supported", -4);
        return -4;
    }

    Log(LOG_DEBUG, "SDHC card detected", 0);

    sd_fs_init();

    Log(LOG_INFO, "SD card initialised", 0);

    return 0;
}

static int sd_close() {
    return 0;
}

static int sd_read(uint32_t len) {
    // TODO: make read command ignore initial 0xff responses (e.g. for loop until valid response)
    // since Ncr is variable between 0 to 8 bytes
    uint8_t ret = 0;
    uint8_t count = 0;

    uint8_t ones[len];
    memset(ones, 0xff, sizeof(ones));

    clear_input_buf();

    // sleep before reading
    sleep_ms(10);

    gpio_put(CS_PIN, 0);
    // read until NCR time is finished
    do {
        spi_write_read_blocking(spi_default, ones, sd_card.in_buf, 1);
        count++;
    } while ((sd_card.in_buf[0] == 0xff) && (count < 10));

    spi_write_read_blocking(spi_default, ones, &sd_card.in_buf[1], sizeof(ones)-1);
    sleep_us(10);
    gpio_put(CS_PIN, 1);

    printbuf(sd_card.in_buf, len);

    return ret;
}

static int sd_write(uint8_t CMD, uint32_t arg) {
    uint8_t ret = 0;
    uint8_t CRC = 0xff;
    uint8_t ones[1];
    memset(ones, 0xff, sizeof(ones));

    clear_input_buf();

    // sleep before writing
    sleep_ms(10);

    // dummy clock cycles before writing
    spi_write_read_blocking(spi_default, ones, NULL, sizeof(ones));
    sleep_us(10);

    const uint32_t a0 = (arg & 0xff000000) >> 24;
    const uint32_t a1 = (arg & 0x00ff0000) >> 16;
    const uint32_t a2 = (arg & 0x0000ff00) >> 8;
    const uint32_t a3 = (arg & 0x000000ff) >> 0;

    if (CMD == CMD0) CRC = 0x95;
    if (CMD == CMD8) CRC = 0x87;

    uint8_t msg[SPI_CMD_LEN] = { 0x40 | CMD , a0, a1, a2, a3, CRC };

    gpio_put(CS_PIN, 0);
    ret = spi_write_read_blocking(spi_default, msg, sd_card.in_buf, SPI_CMD_LEN);
    sleep_us(10);
    gpio_put(CS_PIN, 1);
    return ret;
}

static int sd_read_block(uint32_t block_address) {
    uint8_t ones[2];
    memset(ones, 0xff, sizeof(ones));

    sd_card.write(CMD17, block_address);
    sd_card.read(1);

    // data token
    sd_card.read(1);

    // data
    sd_card.read(512);

    // crc (don't care so just doing regular SPI read)
    sleep_ms(10);
    gpio_put(CS_PIN, 0);
    spi_write_read_blocking(spi_default, ones, NULL, sizeof(ones));
    sleep_us(10);
    gpio_put(CS_PIN, 1);

    return 0;
}

static int sd_fs_init() {
    uint8_t sectors_per_cluster;
    uint16_t num_reserved_sectors;
    uint32_t sectors_per_fat;
    uint32_t root_dir_first_cluster;
    uint32_t fat_begin_lba;
    uint32_t cluster_begin_lba;

    uint8_t filename[11];
    uint8_t attribute;
    uint16_t first_cluster_high;
    uint16_t first_cluster_low;
    uint32_t first_cluster;
    uint32_t file_size;

    // reading master boot record
    sd_card.read_block(0);

    sd_card.partition_LBA = big_to_small_endian32(&sd_card.in_buf[MBR_PARTITION_OFFSET + MBR_LBS_ABS_FIRST_SECTOR_OFFSET]);
    sd_card.num_sectors = big_to_small_endian32(&sd_card.in_buf[MBR_PARTITION_OFFSET + MBR_NUM_SECTORS_IN_PARTITION_OFFSET]);

    sd_card.read_block(sd_card.partition_LBA);

    num_reserved_sectors = big_to_small_endian16(&sd_card.in_buf[FAT32_NUM_RESERVED_SECTORS_OFFSET]);
    sectors_per_fat = big_to_small_endian32(&sd_card.in_buf[FAT32_SECTORS_PER_FAT]);
    sectors_per_cluster = sd_card.in_buf[FAT32_SECTORS_PER_CLUSTER_OFFSET];
    root_dir_first_cluster = big_to_small_endian32(&sd_card.in_buf[FAT32_ROOT_DIR_FIRST_CLUSTER]);

    fat_begin_lba = sd_card.partition_LBA + num_reserved_sectors;
    cluster_begin_lba = sd_card.partition_LBA + num_reserved_sectors + (FAT32_NUM_FATS * sectors_per_fat);

    sd_card.read_block(cluster_begin_lba + (root_dir_first_cluster - 2) * sectors_per_cluster);

    // for some reason my file is shifted by 0x40
    memcpy(filename, &sd_card.in_buf[0x40 + FAT32_DIR_NAME_OFFSET], 11);
    attribute = sd_card.in_buf[0x40 + FAT32_DIR_ATTR_OFFSET];
    first_cluster_high = big_to_small_endian16(&sd_card.in_buf[0x40 + FAT32_FIRST_CLUSTER_HIGH_OFFSET]);
    first_cluster_low = big_to_small_endian16(&sd_card.in_buf[0x40 + FAT32_FIRST_CLUSTER_LOW_OFFSET]);
    first_cluster = (first_cluster_high << 16) | first_cluster_low;
    file_size = big_to_small_endian32(&sd_card.in_buf[0x40 + FAT32_DIR_FILE_SIZE_OFFSET]);

    for(int i = 0; i < 11; i++) {
        printf("%c", filename[i]);
    }
    printf("\n");

    printf("%d\n", attribute);
    printf("%d\n", first_cluster_high);
    printf("%d\n", first_cluster_low);
    printf("%d\n", first_cluster);
    printf("%d\n", file_size);

    sd_card.read_block(cluster_begin_lba + (first_cluster - 2) * sectors_per_cluster);

    return 0;
}
