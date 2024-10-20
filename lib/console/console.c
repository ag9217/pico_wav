#include "console.h"
#include "commands.h"

char command_line_buffer[LINE_LEN + 1];

void console() {
    int16_t c; // 16 bit to handle potential error codes
    uint8_t cursor = 0;
    char line_buffer[LINE_LEN + 1];

    init_console(line_buffer, command_line_buffer);

    while(true) {
        c = getchar();

        // escape sequence
        if (c == 0x1b || c == 0x03) {
            flush_input();
            printf("\n");
            cursor = clear_buffer(line_buffer);
            printf("> ");
            continue;
        }

        // handle enter
        if (c == 0x0d) {
            memcpy(command_line_buffer, line_buffer, LINE_LEN);
            printf("\n");
            command_processor(command_line_buffer);
            cursor = clear_buffer(line_buffer);
            printf("> ");
            continue;
        }

        // handle backspace / del
        if (c == 0x7f || c == 0x08) {
            // if cursor is at beginning, ignore
            if (cursor == 0)
                continue;
            cursor = delete_char(line_buffer);
            // use backspace, space, backspace to erase char in terminal
            printf("%c%c%c", 0x08, 0x20, 0x08);
            continue;
        }

        // at end of buffer
        if (cursor == LINE_LEN)
            continue;

        // char input
        cursor = add_char(line_buffer, c);
        printf("%c", c);
    }
}

void init_console(char * line_buffer, char * command_line_buffer) {
    // reset both buffers
    clear_buffer(line_buffer);
    clear_buffer(command_line_buffer);

    // flushing input
    flush_input();

    // print console arrow
    printf("> ");
}

void flush_input() {
    int16_t c_flush = PICO_ERROR_NONE;

  do {
    // read characters until nothing left to read
    c_flush = getchar_timeout_us(500);
  } while (c_flush != PICO_ERROR_TIMEOUT && c_flush != PICO_ERROR_GENERIC);
}

uint8_t clear_buffer(char * buffer) {
    memset(buffer, '\0', LINE_LEN);
    return strlen(buffer);
}

uint8_t add_char(char * buffer, char c) {
  if (strlen(buffer) == LINE_LEN)
      return LINE_LEN;

  buffer[strlen(buffer)] = c;
  return strlen(buffer);
}

uint8_t delete_char(char * buffer) {
  if (strlen(buffer) == 0)
      return 0;
  buffer[strlen(buffer) - 1] = '\0';
  return strlen(buffer);
}

void command_processor(char * buffer) {
    uint8_t i = 0;

    if (strlen(buffer) == 0)
        return;

    while(strcmp(commands[i].name, "") != 0) {
        if(strcmp(buffer, commands[i].name) == 0) {
            commands[i].execute();
            return;
        }
        else
            i++;
    }

    printf("Unknown command %s, enter help command for help.\n", buffer);
    return;
}
