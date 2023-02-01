#include <hardware/adc.h>
#include <stdio.h>
#include "tusb.h"
#include "frequency_counter.h"
#include "notes.h"
#include "global.h"
#include "pico/stdlib.h"
#include "pico.h"

int main(void)
{
    stdio_init_all();
    Setup();

    tusb_init();
    Intro();


    while (true)
    {
        tud_task();
        gpio_put(TEST_LED, 1);
        if (isReady()) {
            realFrequency = getFreq() * TIMER_MULTIPLIER;
            FrequencyStage();
            if (status == Active) {
                if (step % bps == 0) {
                    midi_plant();
                    if (step == bps * 8) {
                        midi_light();
                        step = 0;
                    }
                    printf("{\"AverageFreq\": %d, \"Freq\": %d, \"PlantNote\": %d, \"LightNote\": %d }\n",
                           averageFreq, realFrequency, lastNotePlant, lastNoteLight);
                }
            }
            step++;
        }
        midi_settings();
        LedStage();
        sleep_ms(10);
    }

}