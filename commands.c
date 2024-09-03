#include "commands.h"

void print_help() {
    uint8_t i = 0;

    printf("Commands:\n");
    while(strcmp(commands[i].name, "") != 0) {
        printf("%s - %s\n", commands[i].name, commands[i].help);
        i++;
    }
}

void led_toggle() {
    static bool led_state = 0;

    if(!led_state)
        gpio_put(PICO_DEFAULT_LED_PIN, true);
    else
        gpio_put(PICO_DEFAULT_LED_PIN, false);

    led_state = !led_state;
}

struct command commands[] ={
    {"help", print_help, "Display this help command"},
    {"led", led_toggle, "Toggle onboard LED"},
    {"",0,""}
};

