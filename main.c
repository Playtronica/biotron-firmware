#include <hardware/adc.h>
#include <stdio.h>
#include <hardware/pwm.h>
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

    gpio_put(TEST_LED, 1);
    tusb_init();
    PrintLogo();

    beginTimer(PLANT_PIN, TIMER_MS);
    uint32_t step = 0;
    while (true)
    {
        tud_task();
        gpio_put(TEST_LED, 1);
        if (isReady()) {
            realFrequency = getFreq() * TIMER_MULTIPLIER;
            FrequencyStage();
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
            } else {
                pwm_set_gpio_level(FIRST_GREEN_LED, 0);
                pwm_set_gpio_level(SECOND_GREEN_LED, 0);
            }
        }
        LedStage();
        sleep_ms(10);
    }

}