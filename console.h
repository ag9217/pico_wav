#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/mutex.h"

// console line settings
#define LINE_LEN 80

void console();
void init_console(char * line_buffer, char * command_line_buffer);
uint8_t clear_buffer(char * buffer);
uint8_t add_char(char * buffer, char c);
uint8_t delete_char(char * buffer);
void command_processor(char * buffer);
void flush_input();
