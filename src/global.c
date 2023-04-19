#include <hardware/adc.h>
#include <pico/printf.h>
#include <stdlib.h>
#include "global.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"
#include "notes.h"
#include "frequency_counter.h"

enum Status status;


double filterPercent = 0;
void setFilterPercent(double newFilterPercent) {
    filterPercent = newFilterPercent;
    if (filterPercent >= 1) filterPercent = 0.99;
    filterPercent = 1 - filterPercent;
}

uint32_t freq = 0;
uint32_t getFreq() {
    return freq;
}

uint32_t lastFreq = 0;
uint32_t getLastFreq() {
    return lastFreq;
}

uint32_t averageFreq = 0;
uint32_t getAvgFreq() {
    return averageFreq;
}

uint32_t averageFreqChanges = 0;
uint32_t getAvgFreqChanges() {
    return averageFreqChanges;
}


struct repeating_timer midiTimer;
bool _repeating_timer_callback(struct repeating_timer *t) {
    static uint8_t counter = 0;
    midi_plant();
    if (counter++ == 3) {
        midi_light();
        counter = 0;
    }
    printf("{\"AverageFreq\": %d, \"Freq\": %d, \"PlantNote\": %d, \"LightNote\": %d }\n",
           averageFreq, freq, getLastNotePlant(), getLastNoteLight());

    return true;
}


int time = TIMER_MIDI_US;
void setBPM(int newTime) {
    time = newTime;
    if (status == Active) {
        cancel_repeating_timer(&midiTimer);
        add_repeating_timer_us(time, _repeating_timer_callback, NULL, &midiTimer);
    }
}

pwm_config config;

void Setup() {
    adc_init();

    adc_gpio_init(LIGHT_PIN);
    adc_select_input(0);

    gpio_init(PLANT_PIN);
    gpio_init(GROUP_BlUE_LED_CENTER);
    gpio_init(GROUP_BlUE_LED_LEFT);
    gpio_init(GROUP_BlUE_LED_RIGHT);
    gpio_init(FIRST_GROUP_GREEN_LED_1);
    gpio_init(FIRST_GROUP_GREEN_LED_2);
    gpio_init(FIRST_GROUP_GREEN_LED_3);
    gpio_init(SECOND_GROUP_GREEN_LED_1);
    gpio_init(SECOND_GROUP_GREEN_LED_2);
    gpio_init(SECOND_GROUP_GREEN_LED_3);
    gpio_init(TEST_LED);

    gpio_set_dir(PLANT_PIN, GPIO_IN);
    gpio_set_dir(LIGHT_PIN, GPIO_IN);
    gpio_set_function(GROUP_BlUE_LED_CENTER, GPIO_FUNC_PWM);
    gpio_set_function(GROUP_BlUE_LED_LEFT, GPIO_FUNC_PWM);
    gpio_set_function(GROUP_BlUE_LED_RIGHT, GPIO_FUNC_PWM);
    gpio_set_function(FIRST_GROUP_GREEN_LED_1, GPIO_FUNC_PWM);
    gpio_set_function(FIRST_GROUP_GREEN_LED_2, GPIO_FUNC_PWM);
    gpio_set_function(FIRST_GROUP_GREEN_LED_3, GPIO_FUNC_PWM);
    gpio_set_function(SECOND_GROUP_GREEN_LED_1, GPIO_FUNC_PWM);
    gpio_set_function(SECOND_GROUP_GREEN_LED_2, GPIO_FUNC_PWM);
    gpio_set_function(SECOND_GROUP_GREEN_LED_3, GPIO_FUNC_PWM);

    config = pwm_get_default_config();
    pwm_init(pwm_gpio_to_slice_num(GROUP_BlUE_LED_CENTER), &config, true);
    pwm_init(pwm_gpio_to_slice_num(GROUP_BlUE_LED_LEFT), &config, true);
    pwm_init(pwm_gpio_to_slice_num(GROUP_BlUE_LED_RIGHT), &config, true);
    pwm_init(pwm_gpio_to_slice_num(FIRST_GROUP_GREEN_LED_1), &config, true);
    pwm_init(pwm_gpio_to_slice_num(FIRST_GROUP_GREEN_LED_2), &config, true);
    pwm_init(pwm_gpio_to_slice_num(FIRST_GROUP_GREEN_LED_3), &config, true);
    pwm_init(pwm_gpio_to_slice_num(SECOND_GROUP_GREEN_LED_1), &config, true);
    pwm_init(pwm_gpio_to_slice_num(SECOND_GROUP_GREEN_LED_2), &config, true);
    pwm_init(pwm_gpio_to_slice_num(SECOND_GROUP_GREEN_LED_3), &config, true);


    status = Sleep;
    initFrequencyTimer();
}


void Intro() {
    uint32_t startTime = to_ms_since_boot(get_absolute_time());
    while (to_ms_since_boot(get_absolute_time()) - startTime <= 1000) {
        pwm_set_gpio_level(GROUP_BlUE_LED_LEFT,
                           (uint16_t)((float)(to_ms_since_boot(get_absolute_time()) - startTime) / 1000 * MAX_LIGHT));
        pwm_set_gpio_level(GROUP_BlUE_LED_CENTER,
                           (uint16_t)((float)(to_ms_since_boot(get_absolute_time()) - startTime) / 1000 * MAX_LIGHT));
        pwm_set_gpio_level(GROUP_BlUE_LED_RIGHT,
                           (uint16_t)((float)(to_ms_since_boot(get_absolute_time()) - startTime) / 1000 * MAX_LIGHT));
    }
}


uint32_t FilterFrequency(double newVal, double k) {
    static uint32_t filterValue;
    filterValue +=  (newVal - filterValue) * k;
    return filterValue;
}


uint8_t counterValues = 0;
void FrequencyStage() {
    freq = getRealFreq();
    switch (status) {
        case Sleep:
            if (freq > MIN_FREQ) {
                counterValues++;
            } else {
                counterValues = 0;
            }

            if (counterValues >= STABILIZATION_TIME) {
                counterValues = 0;
                status = Avg;
                printf("[+] Change status: Sleep -> Stab\n");
            }
            break;
        case Avg:
            if (freq > MIN_FREQ) {
                counterValues++;
                uint32_t b = FilterFrequency(freq, 0.3);
                if (averageFreq == 0) {
                    lastFreq = freq;
                } else {
                    averageFreqChanges += abs((int)(lastFreq - freq));
                    freq = lastFreq;
                }
                averageFreq += b;
            } else {
                counterValues = 0;
                lastFreq = 0;
                averageFreq = 0;
                averageFreqChanges = 0;

                status = Sleep;
                FilterFrequency(0, 0);
                printf("[+] Change status: Stab -> Sleep\n");
            }

            if (counterValues > AVERAGE_TIME) {
                averageFreq /= counterValues;
                averageFreqChanges /= counterValues;
                counterValues = 0;
                add_repeating_timer_us(time, _repeating_timer_callback, NULL, &midiTimer);
                status = Active;
                printf("[+] Change status: Stab -> Active\n");
            }
            break;
        case Active:
            if (freq < MIN_FREQ) {
                counterValues++;
            } else {
                counterValues = 0;
            }

            if (counterValues > SLEEP_TIME) {
                counterValues = 0;
                lastFreq = 0;
                averageFreq = 0;
                averageFreqChanges = 0;
                status = Sleep;
                cancel_repeating_timer(&midiTimer);
                FilterFrequency(0, 0);
                midi_stop();
                printf("[+] Change status: Active -> Sleep\n");
            }
            if (filterPercent != 0) freq = FilterFrequency(freq, filterPercent);

            break;
    }
}


uint32_t ledsValue[ASYNC];
void LedStage() {
    static int level = (MAX_LIGHT - MIN_LIGHT) / (TIMER_MULTIPLIER * 40);
    switch (status) {
        case Sleep:
            for (int i = ASYNC - 1; i >= 0; i--) {
                ledsValue[i] = 0;
            }
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_1, 0);
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_2, 0);
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_3, 0);
            pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_1, 0);
            pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_2, 0);
            pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_3, 0);
            break;
        case Avg:
            if (ledsValue[ASYNC - 1] < MAX_LIGHT) {
                ledsValue[ASYNC - 1] += level;
            }
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_1, ledsValue[ASYNC - 1]);
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_2, ledsValue[ASYNC - 1]);
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_3, ledsValue[ASYNC - 1]);
            pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_1, ledsValue[ASYNC - 1]);
            pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_2, ledsValue[ASYNC - 1]);
            pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_3, ledsValue[ASYNC - 1]);
            break;
        case Active:
            if (ledsValue[ASYNC - 1] >= MAX_LIGHT && level > 0) {
                level *= -1;
            }
            else {
                if (ledsValue[ASYNC - 1] <= MIN_LIGHT && level < 0) {
                    level *= -1;
                }
            }

            for (int i = 0; i < ASYNC - 1; i++) {
                ledsValue[i] = ledsValue[i + 1];
            }

            ledsValue[ASYNC - 1] += level;

            uint32_t lastNotePlant = getLastNotePlant();
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_1, ledsValue[0] + ((lastNotePlant - MIDDLE_NOTE) * NOTE_STRONG));
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_2, ledsValue[0] + ((lastNotePlant - MIDDLE_NOTE) * NOTE_STRONG));
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_3, ledsValue[0] + ((lastNotePlant - MIDDLE_NOTE) * NOTE_STRONG));
            pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_1, ledsValue[ASYNC - 1] + ((lastNotePlant - MIDDLE_NOTE) * NOTE_STRONG));
            pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_2, ledsValue[ASYNC - 1] + ((lastNotePlant - MIDDLE_NOTE) * NOTE_STRONG));
            pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_3, ledsValue[ASYNC - 1] + ((lastNotePlant - MIDDLE_NOTE) * NOTE_STRONG));

            break;
    }
}



