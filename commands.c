#include "commands.h"

void print_help() {
    uint8_t i = 0;

    // help message
    const char * HELP =
      "Pi Pico Basic Command Prompt - A simple 80 character command\n"
      "Commands:\n";

    printf("%s", HELP);

    while(strcmp(commands[i].name, "") != 0) {
        printf("%s - %s\n", commands[i].name, commands[i].help);
        i++;
    }
}

struct command commands[] ={
    {"help", print_help, "Display this help command"},
    {"",0,""}
};

