#include <stdlib.h>
#include <stdio.h>
#include <hardware/adc.h>

#include "tusb.h"
#include "frequency_counter.h"
#include "notes.h"
#include "global.h"

int main(void)
{
    stdio_init_all();
    Setup();
    sleep_ms(3000);

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
            if (realFrequency > MIN_FREQ) {
                gpio_put(TEST_LED, 1);
            }
            FrequencyStage();
            LedStage();
            if (status == Active) {
                midi_plant();
                count++;
                if (count == 8) {
                    midi_light();
                    count = 0;
                }
            }
        }
        sleep_ms(10);
    }

}
