#ifndef FILE_H_
#define FILE_H_

#include <pico/stdio.h>
#include <stdint.h>

struct file {
    // extra byte for char null terminator
    uint8_t filename[12];
    // TODO: would like this to just be a is_directory boolean instead
    uint8_t attribute;
    uint32_t first_cluster;
    uint32_t filesize;
};
#endif
