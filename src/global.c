#include <hardware/adc.h>
#include <pico/printf.h>
#include <stdlib.h>
#include "global.h"

int noteChangeValue;
uint32_t lastFrequency;

int counterValues = 0;

uint32_t filterValue = 0;
uint32_t Filter(float newVal, double k) {
    filterValue +=  (newVal - filterValue) * k;
    return filterValue;
}

int GetNoteDiff(int oldVal, int newVal) {
    int diff = newVal - oldVal;
    bool minus = diff < 0;
    diff = abs(diff);

    double first = 0;
    double second = FIRST_VALUE;

    int i;
    if (diff - noteChangeValue > 0) {
        i = 1;
        diff -= noteChangeValue;
    } else return 0;

    double extra;
    while (diff - noteChangeValue + NOTE_DISTANCE * (first + second) >= 0) {
        diff -= noteChangeValue + NOTE_DISTANCE * (first + second);
        extra = first + second;
        first = second;
        second = extra;
        i++;
    }

    if (minus) {
        return -i;
    }
    return i;
}

void Setup() {
    adc_init();

    adc_gpio_init(PHOTORESISTORS_PIN);
    adc_select_input(0);

    gpio_init(PLANT_PIN);
    gpio_init(BlUE_LED);
    gpio_init(FIRST_GREEN_LED);
    gpio_init(SECOND_GREEN_LED);
    gpio_init(TEST_LED);

    gpio_set_dir(PLANT_PIN, GPIO_IN);
    gpio_set_dir(PHOTORESISTORS_PIN, GPIO_IN);
    gpio_set_dir(BlUE_LED, GPIO_OUT);
    gpio_set_dir(FIRST_GREEN_LED, GPIO_OUT);
    gpio_set_dir(SECOND_GREEN_LED, GPIO_OUT);
    gpio_set_dir(TEST_LED, GPIO_OUT);

    realFrequency = 0;
    counterValues = 0;
    averageFreq = 0;

    status = Sleep;
}

void LedStage() {
    gpio_put(BlUE_LED, 0);
    gpio_put(FIRST_GREEN_LED, 0);
    gpio_put(SECOND_GREEN_LED, 0);
    switch (status) {
        case Sleep:
            gpio_put(BlUE_LED, 1);
            break;
        case Avg:
            gpio_put(FIRST_GREEN_LED, 1);
            break;
        case Active:
            gpio_put(FIRST_GREEN_LED, 1);
            gpio_put(SECOND_GREEN_LED, 1);
            break;
    }
}

void FrequencyStage() {
    switch (status) {
        case Sleep:
            filterValue = 0;
            noteChangeValue = 0;
            lastFrequency = 0;

            if (realFrequency > MIN_FREQ) {
                counterValues++;
            } else {
                counterValues = 0;
            }

            if (counterValues >= STAB_TIME) {
                counterValues = 0;
                averageFreq = 0;
                status = Avg;
                filterValue = realFrequency;
                printf("[+] Change status: Sleep -> Stab\n");
            }
            break;
        case Avg:
            if (realFrequency > MIN_FREQ) {
                counterValues++;
                uint32_t b = Filter(realFrequency, 0.3);
                printf("[TEST] %d\n", b);
                if (averageFreq == 0) {
                    lastFrequency = realFrequency;
                } else {
                    noteChangeValue += abs(lastFrequency - realFrequency);
                    realFrequency = lastFrequency;
                }
                averageFreq += b;
            } else {
                averageFreq = 0;
                counterValues = 0;
                status = Sleep;
                printf("[+] Change status: Stab -> Sleep\n");
            }

            if (counterValues > AVERAGE_TIME) {
                averageFreq /= counterValues;
                noteChangeValue /= counterValues;
                counterValues = 0;
                status = Active;
                printf("[+] Change status: Stab -> Active\n");
            }
            break;
        case Active:
            if (realFrequency < MIN_FREQ) {
                counterValues++;
            } else {
                counterValues = 0;
            }

            if (counterValues > SLEEP_TIME) {
                counterValues = 0;
                averageFreq = 0;
                status = Sleep;
                printf("[+] Change status: Active -> Sleep\n");
            }
            break;
    }
}





