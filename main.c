#include <pico/printf.h>
#include <hardware/adc.h>
#include "pico/stdlib.h"
#include "PLSDK/channel.h"
#include "global.h"
#include "buttons.h"
#include "leds.h"
#include "raw_plant.h"


void setup() {
    stdio_init_all();
    init_midi();

    adc_init();
    adc_gpio_init(LIGHT_PIN);
    adc_select_input(0);

    init_leds();
    init_buttons();
    init_plant();
}


int main(void)
{
    setup();

    while (true)
    {
        printf("Hello World\n");
        remind_midi();
        sleep_ms(1);
    }
}