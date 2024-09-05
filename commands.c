#include "commands.h"
#include "log.h"
#include "pico/bootrom.h"

static void bootrom_boot() {
    reset_usb_boot(0,0);
}

static void print_help() {
    uint8_t i = 0;

    printf("Commands:\n");
    while(commands[i].execute) {
        printf("%s - %s\n", commands[i].name, commands[i].help);
        i++;
    }
    Log(LOG_DEBUG, "Printed from help function!", 0);
}

static void led_toggle() {
    static bool led_state = 0;

    if(!led_state)
        gpio_put(PICO_DEFAULT_LED_PIN, true);
    else
        gpio_put(PICO_DEFAULT_LED_PIN, false);

    led_state = !led_state;
    Log(LOG_DEBUG, "LED toggled!", led_state);
}

static void print_logs() {
    log_print_all();
}

struct command commands[] ={
    {"help", print_help, "Display this help command"},
    {"led", led_toggle, "Toggle onboard LED"},
    {"bootrom", bootrom_boot, "Reboot to boorom"},
    {"logs", print_logs, "Print all logs"},
    {"",0,""}
};

