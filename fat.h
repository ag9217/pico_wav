#include "pico/stdlib.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define MBR_PARTITION_OFFSET 0x01be
#define MBR_LBS_ABS_FIRST_SECTOR_OFFSET 0x08
#define MBR_NUM_SECTORS_IN_PARTITION_OFFSET 0x0c

#define FAT32_SECTORS_PER_CLUSTER_OFFSET 0x0d
#define FAT32_NUM_RESERVED_SECTORS_OFFSET 0x0e
#define FAT32_SECTORS_PER_FAT 0x24
#define FAT32_ROOT_DIR_FIRST_CLUSTER 0x2c

#define FAT32_DIR_NAME_OFFSET 0x00
#define FAT32_DIR_ATTR_OFFSET 0x0b
#define FAT32_FIRST_CLUSTER_HIGH_OFFSET 0x14
#define FAT32_FIRST_CLUSTER_LOW_OFFSET 0x1a
#define FAT32_DIR_FILE_SIZE_OFFSET 0x1c

#define FAT32_NUM_FATS 2

uint32_t big_to_small_endian32(uint8_t num[]);
uint16_t big_to_small_endian16(uint8_t num[]);
