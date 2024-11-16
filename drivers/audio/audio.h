#ifndef AUDIO_H_
#define AUDIO_H_

#include "pico/audio_i2s.h"
#include "fat.h"

#define SAMPLES_PER_BUFFER 256

struct audio_buffer_pool *init_audio();
void print_wav_header(uint8_t header[512]);

#endif

