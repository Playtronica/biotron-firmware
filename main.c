#include <hardware/adc.h>
#include <stdio.h>
#include "tusb.h"
#include "frequency_counter.h"
#include "notes.h"
#include "global.h"
#include "pico/stdlib.h"


int main(void)
{
    stdio_init_all();
    Setup();
    sleep_ms(3000);

    PrintLogo();

    gpio_put(TEST_LED, 1);
    tusb_init();

    beginTimer(PLANT_PIN, TIMER_MS);

    int count = 0;

    while (true)
    {
        tud_task();
        gpio_put(TEST_LED, 1);
        if (isReady()) {

            realFrequency = getFreq() * TIMER_MULTIPLIER;
            FrequencyStage();
            LedStage();
            if (status == Active) {
                midi_plant();
                count++;
                if (count == 8) {
                    midi_light();
                    count = 0;
                }
                printf("{\"AverageFreq\": %d, \"Freq\": %d, \"PlantNote\": %d, \"PhotoNote\": %d }\n",
                       averageFreq, realFrequency, lastNotePlant, lastNoteLight);
            }
        }

        sleep_ms(10);

    }

}