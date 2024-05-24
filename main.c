#include <pico/printf.h>
#include "pico/stdlib.h"
#include "PLSDK/channel.h"


int main(void)
{
    stdio_init_all();
    init_midi();

    while (true)
    {
        printf("Hello World\n");
        remind_midi();
        sleep_ms(1);
    }
}