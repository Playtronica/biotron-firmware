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
//    sleep_ms(3000);

    tusb_init();
    Intro();
    beginTimer(PLANT_PIN, TIMER_MS);
    uint32_t step = 0;
    while (true)
    {
        tud_task();
        gpio_put(TEST_LED, 1);
        if (isReady()) {
            realFrequency = getFreq() * TIMER_MULTIPLIER;
            FrequencyStage();
            printf("{\"AverageFreq\": %d, \"Freq\": %d, \"PlantNote\": %d, \"LightNote\": %d }\n",
                   averageFreq, realFrequency, lastNotePlant, lastNoteLight);
            if (status == Active) {
                midi_plant();
                step++;
                LedStage();
                if (step == 8) {
                    midi_light();
                    step = 0;
                }
                printf("{\"AverageFreq\": %d, \"Freq\": %d, \"PlantNote\": %d, \"LightNote\": %d }\n",
                       averageFreq, realFrequency, lastNotePlant, lastNoteLight);
            }
        }
        LedStage();
        sleep_ms(10);
    }

}