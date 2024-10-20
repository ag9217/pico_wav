#ifndef FAT_H_
#define FAT_H_

#include "pico/stdlib.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sd.h"

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

struct fat_block_device {
    //TODO: Make sd card struct generic block device struct
    struct sd* blk_dev;

    uint8_t sectors_per_cluster;
    uint16_t num_reserved_sectors;
    uint32_t sectors_per_fat;
    uint32_t root_dir_first_cluster;
    uint32_t fat_begin_lba;
    uint32_t cluster_begin_lba;
    uint32_t partition_LBA;
    uint32_t num_sectors;

    int (*init)(void);
};

static int block_fs_init();
uint32_t big_to_small_endian32(uint8_t num[]);
uint16_t big_to_small_endian16(uint8_t num[]);

extern struct fat_block_device fs;
#endif
