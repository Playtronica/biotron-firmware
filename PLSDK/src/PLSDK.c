#include "tusb.h"
#include <stdarg.h>
#include "PLSDK.h"

bool LOGGER_FLAG = true;


void init_midi() {
    tusb_init();
}


void remind_midi() {
    tud_task();
}

void plsdk_printf(const char *__restrict format, ...) {
    if (!LOGGER_FLAG) return;
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}