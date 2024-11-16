#include "audio.h"

struct audio_buffer_pool *init_audio() {

    static audio_format_t audio_format = {
            .format = AUDIO_BUFFER_FORMAT_PCM_S16,
            .sample_freq = 22050,
            .channel_count = 1,
    };

    static struct audio_buffer_format producer_format = {
            .format = &audio_format,
            .sample_stride = 2
    };

    struct audio_buffer_pool *producer_pool = audio_new_producer_pool(&producer_format, 3,
                                                                      SAMPLES_PER_BUFFER); // todo correct size
    bool __unused ok;
    const struct audio_format *output_format;
    struct audio_i2s_config config = {
            .data_pin = PICO_AUDIO_I2S_DATA_PIN,
            .clock_pin_base = PICO_AUDIO_I2S_CLOCK_PIN_BASE,
            .dma_channel = 0,
            .pio_sm = 0,
    };

    output_format = audio_i2s_setup(&audio_format, &config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }

    ok = audio_i2s_connect(producer_pool);
    assert(ok);
    audio_i2s_set_enabled(true);
    return producer_pool;
}

void print_wav_header(uint8_t header[512]) {
    printf("File size: %d\n", big_to_small_endian32(&header[4]));
    printf("Length of format data: %d\n", big_to_small_endian32(&header[16]));
    printf("Type of format: %d\n", big_to_small_endian16(&header[20]));
    printf("Channels: %d\n", big_to_small_endian16(&header[22]));
    printf("Sample rate: %d\n", big_to_small_endian32(&header[24]));
    printf("Sample rate * BitsPerSample * Channels / 8: %d\n", big_to_small_endian32(&header[28]));
    printf("BitsPerSample * Channels / 8: %d\n", big_to_small_endian16(&header[32]));
    printf("BitsPerSample: %d\n", big_to_small_endian16(&header[34]));
    printf("Data section size: %d\n", big_to_small_endian32(&header[40]));
}
