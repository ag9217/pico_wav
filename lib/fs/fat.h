#ifndef FAT_H_
#define FAT_H_

#include "pico/stdlib.h"
#include "pico/audio_i2s.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sd.h"
#include "file.h"
#include "log.h"

#define MAX_NUM_FILES 100

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
#define FAT32_ENTRIES_PER_FAT 128

struct fat_block_device {
    //TODO: Make sd card struct generic block device struct
    struct sd* blk_dev;
    uint8_t fat_sector_offset;
    uint8_t fat[512];
    // allow only MAX_NUM_FILES of files for now
    struct file files[MAX_NUM_FILES];

    uint8_t sectors_per_cluster;
    uint16_t num_reserved_sectors;
    uint32_t sectors_per_fat;
    uint32_t root_dir_first_cluster;
    uint32_t fat_begin_lba;
    uint32_t cluster_begin_lba;
    uint32_t partition_LBA;
    uint32_t num_sectors;

    int (*init)(void);
    int (*open)(char file_name[]);
};

static int block_fs_init();
static void get_files();
static int search_for_file(char filename[]);
static int open_file(char file_name[]);
uint32_t cluster_to_lba(uint32_t cluster);
uint32_t big_to_small_endian32(uint8_t num[]);
uint16_t big_to_small_endian16(uint8_t num[]);

extern struct fat_block_device fs;
#endif
