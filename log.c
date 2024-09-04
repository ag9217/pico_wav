#include "log.h"

static struct {
    uint8_t level;
    bool silent;
} logger;

static const char *level_strings[] = {
  "\033[0;35mDEBUG\033[0;37m", "\033[0;32mINFO\033[0;37m", "\033[0;33mWARN\033[0;37m", "\033[0;31mERROR\033[0;37m"
};

static void log_set_level(uint8_t level) {
    logger.level = level;
}

static void log_set_silent(bool enable) {
    logger.silent = enable;
}

void log_init(uint8_t level, bool enable) {
    log_set_level(level);
    log_set_silent(enable);
}

void Log(enum log_level ll, char *msg, int num) {
    if (!logger.silent && ll >= logger.level) {
        printf("%s - %s - %d\n", level_strings[ll], msg, num);
    }
}
