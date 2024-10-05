#include "pico/stdlib.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/time.h"
#include "hardware/spi.h"

#define BUF_LEN 256
#define SPI_CMD_LEN 6
#define CS_PIN 22
#define R1_TIMEOUT 3

// SD card commands
/** GO_IDLE_STATE - init card in spi mode if CS low */
#define CMD0 0x00
/** SEND_IF_COND - verify SD Memory Card interface operating condition.*/
#define CMD8 0x08
/** SEND_CSD - read the Card Specific Data (CSD register) */
#define CMD9 0x09
/** SEND_CID - read the card identification information (CID register) */
#define CMD10 0x0A
/** SEND_STATUS - read the card status register */
#define CMD13 0x0D
/** READ_BLOCK - read a single data block from the card */
#define CMD17 0x11
/** WRITE_BLOCK - write a single data block to the card */
#define CMD24 0x18
/** WRITE_MULTIPLE_BLOCK - write blocks of data until a STOP_TRANSMISSION */
#define CMD25 0x19
/** ERASE_WR_BLK_START - sets the address of the first block to be erased */
#define CMD32 0x20
/** ERASE_WR_BLK_END - sets the address of the last block of the continuous
    range to be erased*/
#define CMD33 0x21
/** ERASE - erase all previously selected blocks */
#define CMD38 0x26
/** APP_CMD - escape for application specific command */
#define CMD55 0x37
/** READ_OCR - read the OCR register of a card */
#define CMD58 0x3A
/** SET_WR_BLK_ERASE_COUNT - Set the number of write blocks to be
     pre-erased before writing */
#define ACMD23 0x17
/** SD_SEND_OP_COMD - Sends host capacity support information and
    activates the card's initialization process */
#define ACMD41 0x29

/** status for card in the ready state */
#define R1_READY_STATE 0x00
/** status for card in the idle state */
#define R1_IDLE_STATE 0x01
/** status bit for illegal command */
#define R1_ILLEGAL_COMMAND 0x04
/** start data token for read or write single block*/
#define DATA_START_BLOCK 0xFE
/** stop token for write multiple blocks*/
#define STOP_TRAN_TOKEN 0xFD
/** start data token for write multiple blocks*/
#define WRITE_MULTIPLE_TOKEN 0xFC
/** mask for data response tokens after a write block operation */
#define DATA_RES_MASK 0x1F
/** write data accepted token */
#define DATA_RES_ACCEPTED 0x05
//-----------------------------------------

struct sd {
    uint8_t out_buf[BUF_LEN];
    uint8_t in_buf[BUF_LEN];
    uint16_t buf_len;

    int (*init)(void);
    int (*close)(void);
    int (*read)(uint8_t len);
    int (*write)(uint8_t CMD, uint32_t arg);
    void (*clear)(void);
};

static int sd_init();
static int sd_close();
static int sd_read(uint8_t len);
static int sd_write(uint8_t CMD, uint32_t arg);
static void clear_input_buf();

extern struct sd sd_card;
