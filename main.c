#include "tusb.h"
#include "frequency_counter.h"
#include "notes.h"
#include "global.h"
#include "pico/stdlib.h"
#include "pico.h"
#include "cap_buttons.h"
#include "pico/multicore.h"

uint32_t interrupts;

int main(void)
{
    stdio_init_all();
    tusb_init();

    Setup();
    Intro();

    uint16_t i = 0;
    while (true)
    {
        tud_task();
        if (isReady()) {
            MainStage();
        }


        if (i % 100 == 0) {
            buttons_task();
        }
        else {
            MidiSettings();
        }
        LedStage();
        i++;
        sleep_ms(1);
    }
}