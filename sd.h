#include "pico/stdlib.h"
#include <stdint.h>

#define BUF_LEN 256

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

struct sd {
    uint8_t out_buf[BUF_LEN];
    uint8_t in_buf[BUF_LEN];
    uint16_t buf_len;

    int (*init)(void);
    int (*close)(void);
    int (*read)(void);
    int (*write)(void);
};

static int sd_init();
static int sd_close();
static int sd_read();
static int sd_write();

extern struct sd sd_card;
