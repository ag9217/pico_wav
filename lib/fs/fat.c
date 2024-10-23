#include "fat.h"

struct fat_block_device fs = {
    .init = block_fs_init,
    .open = open_file
};

static int block_fs_init() {
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

    fs.fat_sector_offset = 0;
    fs.blk_dev->read_block(fs.fat_begin_lba + fs.fat_sector_offset);

    // get files
    get_files();

    return 0;
}

static void get_files() {
    uint16_t first_cluster_high;
    uint16_t first_cluster_low;

    for (int sector_offset = 0; sector_offset < fs.sectors_per_cluster; sector_offset++) {
        for (int i = 0; i < 8; i++) { // 512 / 64 = 8
            struct file temp_file;

            fs.blk_dev->read_block(cluster_to_lba(fs.root_dir_first_cluster) + sector_offset);

            // for some reason my file is shifted by 0x40
            memcpy(temp_file.filename, &(fs.blk_dev->in_buf[0x40 * i + FAT32_DIR_NAME_OFFSET]), 12);

            // sanitise filename
            for (int i = 0; i < 11; i++) {
                // checking its not a space between words
                if (temp_file.filename[i] <= 0x20 && temp_file.filename[i+1] <= 0x20)
                    temp_file.filename[i] = '\0';
            }
            temp_file.filename[11] = '\0';

            temp_file.attribute = fs.blk_dev->in_buf[0x40 * i + FAT32_DIR_ATTR_OFFSET];
            // end of dir
            if (temp_file.attribute == 0)
                return;

            first_cluster_high = big_to_small_endian16(&(fs.blk_dev->in_buf[0x40 * i + FAT32_FIRST_CLUSTER_HIGH_OFFSET]));
            first_cluster_low = big_to_small_endian16(&(fs.blk_dev->in_buf[0x40 * i + FAT32_FIRST_CLUSTER_LOW_OFFSET]));
            temp_file.first_cluster = (first_cluster_high << 16) | first_cluster_low;
            temp_file.filesize = big_to_small_endian32(&(fs.blk_dev->in_buf[0x40 * i + FAT32_DIR_FILE_SIZE_OFFSET]));

            fs.files[i] = temp_file;
        }
    }
}

static int search_for_file(char filename[]) {
    // searching for file
    for (int i = 0; i < MAX_NUM_FILES; i++) {
        if(strcmp(filename, (const char *)fs.files[i].filename) == 0) {
            Log(LOG_DEBUG, "Found file", 0);
            return i;
        }
    }
    for (int i = 0; i < 12; i++) {
        printf("%x", filename[i]);
    }
    printf("\n");
    for (int i = 0; i < 12; i++) {
        printf("%x", fs.files[3].filename[i]);
    }
    printf("\n");
    Log(LOG_ERROR, "Could not find file", -1);
    return -1;
}

static int open_file(char filename[]) {
    int ret = 0;

    ret = search_for_file(filename);
    if (ret < 0)
        return -1;

    printf("%s\n", fs.files[ret].filename);
    printf("%d\n", fs.files[ret].attribute);
    printf("%d\n", fs.files[ret].first_cluster);
    printf("%d\n", fs.files[ret].filesize);

    return 0;
};

uint32_t cluster_to_lba(uint32_t cluster) {
    return fs.cluster_begin_lba + (cluster - 2) * fs.sectors_per_cluster;
}

uint32_t big_to_small_endian32(uint8_t num[]) {
    return num[3] << 24 | num[2] << 16 | num[1] << 8 | num[0];
}

uint16_t big_to_small_endian16(uint8_t num[]) {
    return num[1] << 8 | num[0];
}

