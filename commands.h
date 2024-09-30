#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "sd.h"
#include <stdint.h>
#include <string.h>

typedef void(*function_ponter_type)(void);
struct command {
    char const *name;
    function_ponter_type execute;
    const char *help;
};

extern struct command commands[];
