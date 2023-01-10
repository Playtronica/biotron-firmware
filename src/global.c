#include <hardware/adc.h>
#include <pico/printf.h>
#include <stdlib.h>
#include "global.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"
#include "notes.h"

int noteChangeValue;
uint32_t lastFrequency;

int counterValues = 0;
pwm_config config;

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
    if (diff - noteChangeValue >= 0) {
        i = 1;
        diff -= noteChangeValue;
    } else return 0;

    double extra;
    while (diff - (noteChangeValue + NOTE_DISTANCE * (first + second)) >= 0) {
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

    gpio_set_function(BlUE_LED, GPIO_FUNC_PWM);
    gpio_set_function(FIRST_GREEN_LED, GPIO_FUNC_PWM);
    gpio_set_function(SECOND_GREEN_LED, GPIO_FUNC_PWM);

    realFrequency = 0;
    counterValues = 0;
    averageFreq = 0;

    uint sliceNum = pwm_gpio_to_slice_num(BlUE_LED);
    config = pwm_get_default_config();
    pwm_init(sliceNum, &config, true);

    status = Sleep;
}

uint32_t freq[ASYNC];
void LedStage() {

    static int level = (MAX_LIGHT - MIN_LIGHT) / (TIMER_MULTIPLIER * 4);
    switch (status) {
        case Sleep:
            for (int i = ASYNC - 1; i >= 0; i--) {
                freq[i] = 0;
            }
            pwm_set_gpio_level(FIRST_GREEN_LED, 0);
            pwm_set_gpio_level(SECOND_GREEN_LED, 0);
            break;
        case Avg:
            if (freq[ASYNC - 1] < 50000) {
                freq[ASYNC - 1] += level;
            }
            pwm_set_gpio_level(FIRST_GREEN_LED, freq[ASYNC - 1]);
            pwm_set_gpio_level(SECOND_GREEN_LED, freq[ASYNC - 1]);
            break;
        case Active:
            if (freq[ASYNC - 1] >= MAX_LIGHT && level > 0) {
                level *= -1;
            }
            else {
                if (freq[ASYNC - 1] <= MIN_LIGHT && level < 0) {
                    level *= -1;
                }
            }

            for (int i = 0; i < ASYNC - 1; i++) {
                freq[i] = freq[i + 1];
            }

            freq[ASYNC - 1] += level;


            pwm_set_gpio_level(FIRST_GREEN_LED, freq[0] + ((lastNotePlant - MIDDLE_NOTE) * NOTE_STRONG));
            pwm_set_gpio_level(SECOND_GREEN_LED, freq[ASYNC - 1] + ((lastNotePlant - MIDDLE_NOTE) * NOTE_STRONG));

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
                midi_stop();
            }
            break;
    }
}

void Intro() {
    uint32_t startTime = to_ms_since_boot(get_absolute_time());
    while (to_ms_since_boot(get_absolute_time()) - startTime <= 1000) {
        pwm_set_gpio_level(BlUE_LED,
                           (uint16_t)((float)(to_ms_since_boot(get_absolute_time()) - startTime) / 1000 * MAX_LIGHT));
    }
    uint sliceNum = pwm_gpio_to_slice_num(FIRST_GREEN_LED);
    config = pwm_get_default_config();
    pwm_init(sliceNum, &config, true);
//    printf("\n\n"
//           " _______ .-./`)     ,-----.  ,---------. .-------.        ,-----.    ,---.   .--. \n"
//           "\\  ____  \\ .-.')  .'  .-,  '.\\          \\|  _ _   \\     .'  .-,  '.  |    \\  |  | \n"
//           "| |    \\ / `-' \\ / ,-.|  \\ _ \\`--.  ,---'| ( ' )  |    / ,-.|  \\ _ \\ |  ,  \\ |  | \n"
//           "| |____/ /`-'`\"`;  \\  '_ /  | :  |   \\   |(_ o _) /   ;  \\  '_ /  | :|  |\\_ \\|  | \n"
//           "|   _ _ '..---. |  _`,/ \\ _/  |  :_ _:   | (_,_).' __ |  _`,/ \\ _/  ||  _( )_\\  | \n"
//           "|  ( ' )  \\   | : (  '\\_/ \\   ;  (_I_)   |  |\\ \\  |  |: (  '\\_/ \\   ;| (_ o _)  | \n"
//           "| (_(;)_) |   |  \\ `\"/  \\  ) /  (_(=)_)  |  | \\ `'   / \\ `\"/  \\  ) / |  (_,_)\\  | \n"
//           "|  (_,_)  /   |   '. \\_/``\".'    (_I_)   |  |  \\    /   '. \\_/``\".'  |  |    |  | \n"
//           "/_______.''---'     '-----'      '---'   ''-'   `'-'      '-----'    '--'    '--' \n"
//           "                                                                                  \n");
}



