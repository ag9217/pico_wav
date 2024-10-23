#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/malloc.h"
#include "stdlib.h"
#include "time.h"

typedef struct {
    double time;
    uint8_t level;
    const char *msg;
    int num;
} log_event;

enum log_level { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR };

void Log(enum log_level, char *msg, int num);
void log_init(uint8_t level, bool enable);
void log_print_all();
#endif
