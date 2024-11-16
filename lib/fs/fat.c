#include "fat.h"

struct fat_block_device fs = {
    .init = block_fs_init,
    .open_wav = open_wav_file
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
            memcpy(temp_file.filename, &(fs.blk_dev->in_buf[0x20 + 0x40 * i + FAT32_DIR_NAME_OFFSET]), 12);

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

            first_cluster_high = big_to_small_endian16(&(fs.blk_dev->in_buf[0x20 + 0x40 * i + FAT32_FIRST_CLUSTER_HIGH_OFFSET]));
            first_cluster_low = big_to_small_endian16(&(fs.blk_dev->in_buf[0x20 + 0x40 * i + FAT32_FIRST_CLUSTER_LOW_OFFSET]));
            temp_file.first_cluster = (first_cluster_high << 16) | first_cluster_low;
            temp_file.filesize = big_to_small_endian32(&(fs.blk_dev->in_buf[0x20 + 0x40 * i + FAT32_DIR_FILE_SIZE_OFFSET]));
            temp_file.file_fat_cluster_offset = 0;

            fs.files[i] = temp_file;
        }
    }
}

static int search_for_file(char filename[]) {
    // searching for file
    for (int i = 0; i < MAX_NUM_FILES; i++) {
        if(strcmp(filename, (const char *)fs.files[i].filename) == 0)
            return i;
    }
    return -1;
}

static int open_wav_file(char filename[]) {
    int ret = 0;
    uint32_t bytes_read = 0;
    bool done_reading = false;

    // audio
    struct audio_buffer_pool *ap = init_audio();
    struct audio_buffer *buffer = take_audio_buffer(ap, true);
    int16_t *samples = (int16_t *) buffer->buffer->bytes;

    ret = search_for_file(filename);
    if (ret < 0) {
        Log(LOG_ERROR, "Could not find file", -1);
        return -1;
    }
    Log(LOG_DEBUG, "Found file", 0);

    printf("Filename: %s\n", fs.files[ret].filename);
    printf("Attribute: %d\n", fs.files[ret].attribute);
    printf("First cluster: %x\n", fs.files[ret].first_cluster);
    printf("File size: %d\n", fs.files[ret].filesize);

    // reading file header
    uint32_t cluster = fs.files[ret].first_cluster;
    fs.blk_dev->read_block(cluster_to_lba(cluster));
    print_wav_header(fs.blk_dev->in_buf);

    // start reading data
    fs.fat_sector_offset = cluster / FAT32_ENTRIES_PER_FAT;
    fs.blk_dev->read_block(fs.fat_begin_lba + fs.fat_sector_offset);
    do {
        for (int sector_offset = 0; sector_offset < fs.sectors_per_cluster; sector_offset++) {
            fs.blk_dev->read_block(cluster_to_lba(cluster) + sector_offset);
            bytes_read += 512;

            if (bytes_read >= fs.files[ret].filesize) {
                done_reading = true;
                break;
            }

            for (uint i = 0; i < 255; i++) {
                samples[i] = ((fs.blk_dev->in_buf[(i*2)]) | (fs.blk_dev->in_buf[(i*2) + 1] << 8)) >> 8u;
            }

            // play audio buffer
            buffer->sample_count = buffer->max_sample_count;
            give_audio_buffer(ap, buffer);
        }

        // check fat
        fs.blk_dev->read_block(fs.fat_begin_lba + fs.fat_sector_offset);
        cluster = big_to_small_endian32(&(fs.blk_dev->in_buf[(cluster * 4) % FAT32_ENTRIES_PER_FAT]));
        fs.fat_sector_offset = cluster / FAT32_ENTRIES_PER_FAT;
    } while(cluster < 0x0ffffff8 || !done_reading);

    Log(LOG_DEBUG, "Done reading file", 0);

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

