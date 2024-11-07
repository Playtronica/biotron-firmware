#include <stdio.h>
#include <hardware/adc.h>
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "pico/multicore.h"
#include "PLSDK/channel.h"
#include "global.h"
#include "leds.h"
#include "buttons.h"
#include "raw_plant.h"
#include "params.h"
#include "PLSDK/commands.h"

struct repeating_timer timer;

void second_core();

bool stuck_emul = false;
bool settings_load = false;
volatile bool watchdog_var = true;
volatile bool reset = false;
bool repeating_timer_callback(struct repeating_timer *t) {
    if (watchdog_var) {
        watchdog_var = false;
        return true;
    }
    printf("Stuck identified\n");
    reset = true;
    return true;
}


void second_core() {
    stuck_emul = false;
    while (true) {
        status_loop();
        led_loop();
        check_buttons();

        remind_midi();
        sleep_ms(1);
        watchdog_var = true;

        while (stuck_emul);
    }
}


int main() {
    stdio_init_all();
    init_midi();

    adc_init();
    adc_gpio_init(LIGHT_PIN);
    adc_select_input(0);

    init_leds();
    init_buttons();
    init_plant();

    intro_leds();

    read_settings();
    setup_commands();

    multicore_launch_core1(second_core);
    add_repeating_timer_ms(100, repeating_timer_callback, NULL, &timer);

    while (true) {
        if (reset) {
            cancel_repeating_timer(&timer);
            multicore_reset_core1();
            sleep_ms(100);
            multicore_launch_core1(second_core);
            reset = false;
            add_repeating_timer_ms(100, repeating_timer_callback, NULL, &timer);
            watchdog_var = true;
            printf("Core reloaded\n");
        }
        static bool settings_last = false;
        if (settings_last != settings_load) {
            if (settings_load) {
                reset = false;
                watchdog_var = true;
                cancel_repeating_timer(&timer);
                multicore_reset_core1();
            }
            else {
                multicore_launch_core1(second_core);
                add_repeating_timer_ms(100, repeating_timer_callback, NULL, &timer);
            }
            settings_last = settings_load;
        }
        remind_midi();
        read_sys_ex();
    }
}

