#include <hardware/adc.h>
#include <pico/printf.h>
#include <stdlib.h>
#include "global.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"
#include "notes.h"
#include "frequency_counter.h"
#include "settings.h"
#include "cap_buttons.h"


enum Status status;


double filterPercent = DEF_FILTER_PERCENT;
void setFilterPercent(double newFilterPercent) {
    filterPercent = newFilterPercent;
    if (filterPercent >= 1) filterPercent = 0.99;
    filterPercent = 1 - filterPercent;
    SaveSettings();
}
double getFilterPercent() {
    return filterPercent;
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

uint32_t lightBPM = 3;
void setLightBPM(uint32_t newBPM) {
    lightBPM = newBPM;
    SaveSettings();
}
int getLightBPM() {
    return lightBPM;
}

/** @brief Repeating timer and function for midi
 *
 *  Everytime calls midi_plant and every 4th note calls midi_light
 *
 */
struct repeating_timer midiTimer;
bool _repeating_timer_callback(struct repeating_timer *t) {
    static uint8_t counter = 1;

    MidiPlant();
    if (counter++ >= lightBPM) {
        MidiLight();
        counter = 1;
    }
    printf("{\"AverageFreq\": %d, \"Freq\": %d, \"PlantNote\": %d, \"LightNote\": %d }\n",
           averageFreq, freq, getLastNotePlant(), getLastNoteLight());

    return true;
}


int time = TIMER_MIDI_US;
void setBPM(int newTime) {
    time = newTime;
    /** if status is active, restart timer */
    if (status == Active) {
        cancel_repeating_timer(&midiTimer);
        add_repeating_timer_us(time, _repeating_timer_callback, NULL, &midiTimer);
    }
    printf("Current bpm: %d\n", (int)((double)1000000 / (double)getBPM() * (double)60));
    SaveSettings();
}

int getBPM() {
    return time;
}

double fibPower = DEF_FIB_POW;
double firstValue = DEF_FIB_FIRST;
void setFreqPower(double power, double value) {
    fibPower = power < 1 ? power : 1;
    firstValue = value < 1 ? value : 1;
    SaveSettings();
}

double getFibPower() {
    return fibPower;
}
double getFirstValue() {
    return firstValue;
}


ScaleNums_t scale = SCALE;
void setScale(int id) {
    scale = id;
    SaveSettings();
}
int getScale() {
    return scale;
}

uint8_t plantVelocity = 127;
void setPlantVelocity(uint8_t velocity) {
    if (velocity > 127) return;
    plantVelocity = velocity;
    SaveSettings();
}

uint8_t getPlantVelocity() {
    return plantVelocity;
}

uint8_t lightVelocity = 127;
void setLightVelocity(uint8_t velocity) {
    if (velocity > 127) return;
    lightVelocity = velocity;
    SaveSettings();
}

uint8_t getLightVelocity() {
    return lightVelocity;
}

struct repeating_timer tap_tempo_timer;
static uint64_t middle_tap_tempo = 0;
static int counter_tap_tempo = -1;

bool calculate_bpm_by_tap_tempo(struct repeating_timer *t) {
    if (counter_tap_tempo >= 3) {
        setBPM(middle_tap_tempo / counter_tap_tempo);
        printf("Success\n");
    }
    counter_tap_tempo = -1;
    cancel_repeating_timer(&tap_tempo_timer);
    return true;
}


void tap_tempo() {
    printf("3\n");
    static uint64_t last = 0;
    if (counter_tap_tempo == -1) {
        middle_tap_tempo = 0;
        last = time_us_64();
        counter_tap_tempo++;
        add_repeating_timer_ms(2000, calculate_bpm_by_tap_tempo, NULL, &tap_tempo_timer);
        return;
    }
    middle_tap_tempo += time_us_64() - last;
    last = time_us_64();
    counter_tap_tempo++;
    cancel_repeating_timer(&tap_tempo_timer);
    add_repeating_timer_ms(2000, calculate_bpm_by_tap_tempo, NULL, &tap_tempo_timer);
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

    buttons_add_button(8, tap_tempo, NULL, NULL);
    buttons_init(5);

    ReadSettings();
    SaveSettings();
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
////    pwm_set_gpio_level(GROUP_BlUE_LED_LEFT,
////                       MIN_LIGHT);
//    pwm_set_gpio_level(GROUP_BlUE_LED_CENTER,
//                       MIN_LIGHT);
//    pwm_set_gpio_level(GROUP_BlUE_LED_RIGHT,
//                       MIN_LIGHT);
    PrintInfo();
}

void PrintInfo() {
    printf("\n\n"
           " _______ .-./`)     ,-----.  ,---------. .-------.        ,-----.    ,---.   .--. \n"
           "\\  ____  \\ .-.')  .'  .-,  '.\\          \\|  _ _   \\     .'  .-,  '.  |    \\  |  | \n"
           "| |    \\ / `-' \\ / ,-.|  \\ _ \\`--.  ,---'| ( ' )  |    / ,-.|  \\ _ \\ |  ,  \\ |  | \n"
           "| |____/ /`-'`\"`;  \\  '_ /  | :  |   \\   |(_ o _) /   ;  \\  '_ /  | :|  |\\_ \\|  | \n"
           "|   _ _ '..---. |  _`,/ \\ _/  |  :_ _:   | (_,_).' __ |  _`,/ \\ _/  ||  _( )_\\  | \n"
           "|  ( ' )  \\   | : (  '\\_/ \\   ;  (_I_)   |  |\\ \\  |  |: (  '\\_/ \\   ;| (_ o _)  | \n"
           "| (_(;)_) |   |  \\ `\"/  \\  ) /  (_(=)_)  |  | \\ `'   / \\ `\"/  \\  ) / |  (_,_)\\  | \n"
           "|  (_,_)  /   |   '. \\_/``\".'    (_I_)   |  |  \\    /   '. \\_/``\".'  |  |    |  | \n"
           "/_______.''---'     '-----'      '---'   ''-'   `'-'      '-----'    '--'    '--' \n"
           "                                                                                  \n");
    printf("BPM: %d Fib. NOTE DISTANCE: %.2f, FIRST_VALUE: %.2f\n",
           (int)((double)1000000 / (double)getBPM() * (double)60),
           getFibPower(), getFirstValue());
    printf("LIGHT NOTE EVERY %d PLANT NOTE\n", getLightBPM());
    printf("FILTER VALUE: %.2f\n", getFilterPercent());
    printf("SCALE: %d\n", getScale());
    printf("PlantVelocity: %d, LightVelocity: %d\n", getPlantVelocity(), getLightVelocity());
}

/** @brief Filter for frequency
 *
 * In Average device state smoothes trash values,
 * In Active device state (if enabled) makes smooth transition between notes
 * Working by Running Average
 *
 * @var
 *  filterValue - last saved value of frequency, if it equals zero takes first value without filtration.
 *  If both params are equal zero, filterValue resets to zero
 *
 * @param
 *  newVal - current frequency
 * @param
 *  k - coefficient of filter power (must be lower then 1)
 */
uint32_t filterValue;
uint32_t FilterFrequency(double newVal, double k) {
    if (filterValue == 0) {
        filterValue = newVal;
        return filterValue;
    }
    if (newVal == 0 && k == 0) {
        filterValue = 0;
    }
    filterValue +=  (newVal - filterValue) * k;
    if (!get_random_note_state()) (filterValue = filterValue / 10 * 10);
    return filterValue;
}


uint8_t counterValues = 0;
void MainStage() {
    freq = getRealFreq();
    switch (status) {
        /** @brief Sleep mode
         *
         * Does nothing, until doesn't have frequencies above MIN_FREQ STABILIZATION_TIME in a row.
         * After that change status to Stabilization
         *
         */
        case Sleep:
            if (freq > MIN_FREQ) {
                counterValues++;
            } else {
                counterValues = 0;
            }

            if (counterValues >= STABILIZATION_TIME) {
                counterValues = 0;
                status = Stabilization;
                printf("[+] Change status: Sleep -> Stab\n");
            }
            break;
         /** @brief Stabilization mode
         *
         * Collects frequency for average frequency and average frequency changes.
         * If one of the frequency become below MIN_FREQ, state of device changes to SLEEP
         * If gets AVERAGE_TIME frequencies, state of device changes to ACTIVE and repeating midi timer activates
         *
         */
        case Stabilization:
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
                FilterFrequency(0, 0);
                add_repeating_timer_us(time, _repeating_timer_callback, NULL, &midiTimer);
                status = Active;
                printf("[+] Change status: Stab -> Active\n");
            }
            break;
         /** @brief Active mode
         *
         * Catch new Frequency for midi_plant.
         * If gets frequencies below MIN_FREQ SLEEP_TIME in a row change device state to sleep
         *
         */
        case Active:
            if (freq < MIN_FREQ) {
                counterValues++;
            } else {
                counterValues = 0;
            }

            if (filterPercent != 0)
                freq = FilterFrequency(freq, filterPercent);


            if (counterValues > SLEEP_TIME) {
                counterValues = 0;
                lastFreq = 0;
                averageFreq = 0;
                averageFreqChanges = 0;
                status = Sleep;
                cancel_repeating_timer(&midiTimer);
                FilterFrequency(0, 0);
                MidiStop();
                printf("[+] Change status: Active -> Sleep\n");
            }


            break;
    }
}


uint32_t ledsValue[ASYNC];
void LedStage() {
    static int level = (MAX_LIGHT - MIN_LIGHT) / (TIMER_MULTIPLIER * 40);
    switch (status) {
        /** @brief Sleep mode
         *
         * All green LEDs are disabled
         *
         */
        case Sleep:
            if (level < 0) level *= -1;
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
         /** @brief Stabilization mode
         *
         * Smooth switching on green LEDs
         *
         */
        case Stabilization:
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
         /** @brief Active mode
         *
         * Leds work in pulse mode,
         * also LEDs change brightness in dependence of last played midi_plant note (higher note, higher brightness)
         * Leds work in ASYNC mode, so one of them work with a delay
         */
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



