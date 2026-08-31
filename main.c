#include <hardware/adc.h>
#include <hardware/watchdog.h>
#include <pico/printf.h>
#include "pico/stdlib.h"
#include "PLSDK.h"
#include "global.h"
#include "buttons.h"
#include "leds.h"
#include "raw_plant.h"
#include "params.h"
#include "music.h"
#include "PLSDK/midi_tx.h"
#include "PLSDK/midi_diagnostics.h"



void setup() {
    stdio_init_all();
    midi_diagnostics_set_reset_reason(
            watchdog_enable_caused_reboot() ?
            MIDI_DIAGNOSTICS_RESET_WATCHDOG_TIMEOUT :
            watchdog_caused_reboot() ?
            MIDI_DIAGNOSTICS_RESET_FORCED_OR_BOOTROM :
            MIDI_DIAGNOSTICS_RESET_POWER_OR_RUN);
    // Load settings before USB callbacks, button IRQs or plant timers observe
    // the zero-initialized global settings object.
    read_settings();
    init_midi();

    adc_init();
    adc_gpio_init(LIGHT_PIN);
    adc_select_input(0);

    init_leds();
    init_buttons();
    init_plant();

    intro_leds();
    setup_commands();
}


int main(void)
{
    setup();
    while (true)
    {
        service_midi_note_lifecycle();
        service_music_alarm();
        status_loop();
        led_loop();
        check_buttons();
        remind_midi();
        service_midi_tx();
        get_sys_ex_and_behave();
        service_settings_persistence();
        sleep_ms(1);
    }
}
