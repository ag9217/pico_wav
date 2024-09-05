#include "log.h"

static struct {
    uint8_t level;
    uint8_t num_logs;
    bool silent;
    log_event *logs[100];
} logger;

static const char *level_strings[] = {
  "\033[0;35mDEBUG\033[0;37m", "\033[0;32mINFO\033[0;37m", "\033[0;33mWARN\033[0;37m", "\033[0;31mERROR\033[0;37m"
};

static void log_reset_num_logs() {
    logger.num_logs = 0;
}

static void log_increment_num_logs() {
    logger.num_logs++;
}

static void log_set_level(uint8_t level) {
    logger.level = level;
}

static void log_set_silent(bool enable) {
    logger.silent = enable;
}

void log_init(uint8_t level, bool enable) {
    log_set_level(level);
    log_set_silent(enable);
    log_reset_num_logs();
}

void print_log(log_event *log) {
    if (!logger.silent && log->level >= logger.level) {
        printf("%7.0f %s %s %d\n", log->time, level_strings[log->level], log->msg, log->num);
    }
}

void log_print_all() {
    for(int i = 0; i < logger.num_logs; i++)
    {
        print_log(logger.logs[i]);
    }
}

void Log(enum log_level ll, char *msg, int num) {
    // grab time of log
    time_t log_time = clock();
    double seconds_since_log = difftime(log_time, time(0));

    // creating new log event
    log_event *log = malloc(sizeof(log_event));
    log->time = seconds_since_log;
    log->level = ll;
    log->msg = msg;
    log->num = num;

    print_log(log);

    logger.logs[logger.num_logs] = log;
    log_increment_num_logs();
}
