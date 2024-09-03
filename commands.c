#include "commands.h"

void print_help() {
    // help message
    const char * HELP =
      "Pi Pico Basic Command Prompt - A simple 80 character command\n"
      "line buffer used to control the Pi Pico\n"
      "Commands:\n"
      "help - Display this help message\n";

    printf("%s", HELP);
}

struct command commands[] ={
    {"help", print_help, "Display help command"},
    {"",0,""}
};

