#include "tusb.h"
#include "frequency_counter.h"
#include "notes.h"
#include "global.h"
#include "pico/stdlib.h"
#include "pico.h"
#include "cap_buttons.h"


int main(void)
{
    stdio_init_all();
    tusb_init();

    Setup();
    Intro();


    while (true)
    {
        tud_task();
        gpio_put(TEST_LED, 1);
        if (isReady()) {
            MainStage();
        }

        MidiSettings();
        LedStage();
        buttons_task();
        sleep_ms(1);
    }

}