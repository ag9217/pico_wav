#include "fat.h"

uint32_t big_to_small_endian32(uint8_t num[]) {
    return num[3] << 24 | num[2] << 16 | num[1] << 8 | num[0];
}

uint16_t big_to_small_endian16(uint8_t num[]) {
    return num[1] << 8 | num[0];
}
