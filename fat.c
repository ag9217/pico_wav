#include "fat.h"

struct fat_block_device fs = {
    .init = block_fs_init
};

static int block_fs_init() {
    uint8_t filename[11];
    uint8_t attribute;
    uint16_t first_cluster_high;
    uint16_t first_cluster_low;
    uint32_t first_cluster;
    uint32_t file_size;

    // reading master boot record
    fs.blk_dev->read_block(0);

    fs.partition_LBA = big_to_small_endian32(&(fs.blk_dev->in_buf[MBR_PARTITION_OFFSET + MBR_LBS_ABS_FIRST_SECTOR_OFFSET]));
    fs.num_sectors = big_to_small_endian32(&(fs.blk_dev->in_buf[MBR_PARTITION_OFFSET + MBR_NUM_SECTORS_IN_PARTITION_OFFSET]));

    fs.blk_dev->read_block(fs.partition_LBA);

    fs.num_reserved_sectors = big_to_small_endian16(&(fs.blk_dev->in_buf[FAT32_NUM_RESERVED_SECTORS_OFFSET]));
    fs.sectors_per_fat = big_to_small_endian32(&(fs.blk_dev->in_buf[FAT32_SECTORS_PER_FAT]));
    fs.sectors_per_cluster = fs.blk_dev->in_buf[FAT32_SECTORS_PER_CLUSTER_OFFSET];
    fs.root_dir_first_cluster = big_to_small_endian32(&(fs.blk_dev->in_buf[FAT32_ROOT_DIR_FIRST_CLUSTER]));

    fs.fat_begin_lba = fs.partition_LBA + fs.num_reserved_sectors;
    fs.cluster_begin_lba = fs.partition_LBA + fs.num_reserved_sectors + (FAT32_NUM_FATS * fs.sectors_per_fat);

    fs.blk_dev->read_block(fs.cluster_begin_lba + (fs.root_dir_first_cluster - 2) * fs.sectors_per_cluster);

    // for some reason my file is shifted by 0x40
    memcpy(filename, &(fs.blk_dev->in_buf[0x40 + FAT32_DIR_NAME_OFFSET]), 11);
    attribute = fs.blk_dev->in_buf[0x40 + FAT32_DIR_ATTR_OFFSET];
    first_cluster_high = big_to_small_endian16(&(fs.blk_dev->in_buf[0x40 + FAT32_FIRST_CLUSTER_HIGH_OFFSET]));
    first_cluster_low = big_to_small_endian16(&(fs.blk_dev->in_buf[0x40 + FAT32_FIRST_CLUSTER_LOW_OFFSET]));
    first_cluster = (first_cluster_high << 16) | first_cluster_low;
    file_size = big_to_small_endian32(&(fs.blk_dev->in_buf[0x40 + FAT32_DIR_FILE_SIZE_OFFSET]));

    for(int i = 0; i < 11; i++) {
        printf("%c", filename[i]);
    }
    printf("\n");

    printf("%d\n", attribute);
    printf("%d\n", first_cluster_high);
    printf("%d\n", first_cluster_low);
    printf("%d\n", first_cluster);
    printf("%d\n", file_size);

    fs.blk_dev->read_block(fs.cluster_begin_lba + (first_cluster - 2) * fs.sectors_per_cluster);

    return 0;
}

uint32_t big_to_small_endian32(uint8_t num[]) {
    return num[3] << 24 | num[2] << 16 | num[1] << 8 | num[0];
}

uint16_t big_to_small_endian16(uint8_t num[]) {
    return num[1] << 8 | num[0];
}
