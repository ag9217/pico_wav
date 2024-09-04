#include <stdio.h>
#include "pico/stdlib.h"
#include "time.h"

enum log_level { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR };

void Log(enum log_level, char *msg, int num);
void log_init(uint8_t level, bool enable);
