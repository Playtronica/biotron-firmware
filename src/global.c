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
#include "pico/multicore.h"
#include "tusb.h"

enum Status status;

enum Status getStatus() {
    return status;
}

double filterPercent = DEF_FILTER_PERCENT;
void setFilterPercent(double newFilterPercent) {
    filterPercent = newFilterPercent;
    if (filterPercent >= 1) filterPercent = 0.99;
    filterPercent = 1 - filterPercent;
//    SaveSettings();
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
//    SaveSettings();
}

int getLightBPM() {
    return lightBPM;
}

void play_music() {
    static uint64_t time_log = 0;
    static uint8_t counter = 1;
    MidiPlant();
    if (counter++ >= lightBPM) {
        MidiLight();
        counter = 1;
    }
    if (time_log == 0) {
        time_log = time_us_64();
    }
    if (time_us_64() - time_log > 100000) {
        printf("{\"AverageFreq\": %d, \"Freq\": %d, \"PlantNote\": %d, \"LightNote\": %d }\n",
               averageFreq, freq, getLastNotePlant(), getLastNoteLight());
        time_log = time_us_64();
    }
}


/** @brief Repeating timer and function for midi
 *
 *  Everytime calls midi_plant and every 4th note calls midi_light
 *
 */
struct repeating_timer midiTimer;
bool _repeating_timer_callback(struct repeating_timer *t) {

    play_music();

    return true;
}


int time = TIMER_MIDI_US;
void setBPM(int newTime) {
    time = newTime;
    /** if status is active, restart timer */
    if (status == Active) {
        resetPlantNoteOff();
        cancel_repeating_timer(&midiTimer);
        add_repeating_timer_us(time, _repeating_timer_callback, NULL, &midiTimer);
    }
    // printf("Current bpm: %d\n", (int)((double)1000000 / (double)getBPM() * (double)60));
//    SaveSettings();
}

int getBPM() {
    return time;
}

double fibPower = DEF_FIB_POW;
double firstValue = DEF_FIB_FIRST;
void setFreqPower(double power, double value) {
    fibPower = power < 1 ? power : 1;
    firstValue = value < 1 ? value : 1;
//    SaveSettings();
}

double getFibPower() {
    return fibPower;
}
double getFirstValue() {
    return firstValue;
}


ScaleNums_t scale = SCALE;
void setScale(int id) {
    scale = id % 12;
//    SaveSettings();
}
int getScale() {
    return scale;
}

bool plantVelocityRandom = false;
uint8_t minPlantVelocity = 0;
uint8_t maxPlantVelocity = 127;
void setPlantVelocity(uint8_t min_velocity, uint8_t max_velocity, bool isRandom) {
    if (max_velocity > 127) return;
    maxPlantVelocity = max_velocity;
    minPlantVelocity = min_velocity;
    if (minPlantVelocity > maxPlantVelocity && isRandom) {
        minPlantVelocity = maxPlantVelocity;
    }
    plantVelocityRandom = isRandom;
//    SaveSettings();
}

uint8_t getMaxPlantVelocity() {
    return maxPlantVelocity;
}

uint8_t getMinPlantVelocity() {
    return minPlantVelocity;
}

bool getRandomPlantVelocity() {
    return plantVelocityRandom;
}

bool lightVelocityRandom = false;
uint8_t maxLightVelocity = 0;
uint8_t minLightVelocity = 127;
void setLightVelocity(uint8_t min_velocity, uint8_t max_velocity, bool isRandom) {
    if (max_velocity > 127) return;
    maxLightVelocity = max_velocity;
    minLightVelocity = min_velocity;
    if (minLightVelocity > maxLightVelocity && isRandom) {
        minLightVelocity = maxLightVelocity;
    }
    lightVelocityRandom = isRandom;
//    SaveSettings();
}

uint8_t getMaxLightVelocity() {
    return maxLightVelocity;
}

uint8_t getMinLightVelocity() {
    return minLightVelocity;
}

bool getRandomLightVelocity() {
    return lightVelocityRandom;
}

struct repeating_timer tap_tempo_timer;
static uint64_t middle_tap_tempo = 0;
static int counter_tap_tempo = -1;

bool calculate_bpm_by_tap_tempo(struct repeating_timer *t) {
    if (counter_tap_tempo >= 3) {
        setBPM(middle_tap_tempo / counter_tap_tempo);
        // printf("Success\n");
    }
    counter_tap_tempo = -1;
    cancel_repeating_timer(&tap_tempo_timer);
    return true;
}


void tap_tempo() {
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


bool mute_mode = false;
void change_mute_mode() {
    mute_mode = !mute_mode;
    MidiStop();
    printf("Mute mode: %d\n", mute_mode);
}

void change_same_mode() {
    bool res = get_control_same_note() > 0 ? 0 : 1;
    control_same_note(res);
    SaveSettings();
    printf("Same mode %d\n", res);
}

void change_scale() {
    uint8_t res = (getScale() + 1) % 12;
    setScale(res);
    SaveSettings();
    printf("Change Scale %d\n", res);
}

bool get_mute_mode() {
    return mute_mode;
}


uint16_t second_val;
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

    buttons_add_button(BUTTON_FINGER, change_mute_mode, NULL, NULL);
    buttons_add_button(BUTTON_BOTTOM, change_same_mode, NULL, NULL);
    buttons_add_button(BUTTON_TOP, change_scale, NULL, NULL);

    buttons_init(5);

    ReadSettings();
    SaveSettings();
    status = Sleep;
    initFrequencyTimer();
}

#ifdef INVERT_LED
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
    PrintInfo();
}
#else
void Intro() {
    uint32_t startTime = to_ms_since_boot(get_absolute_time());
    while (to_ms_since_boot(get_absolute_time()) - startTime <= 1000) {
        pwm_set_gpio_level(GROUP_BlUE_LED_LEFT,
                           (uint16_t)((float)MAX_LIGHT - ((float)(to_ms_since_boot(get_absolute_time()) - startTime) / 1000 * MAX_LIGHT)));
        pwm_set_gpio_level(GROUP_BlUE_LED_CENTER,
                           (uint16_t)((float)MAX_LIGHT - ((float)(to_ms_since_boot(get_absolute_time()) - startTime) / 1000 * MAX_LIGHT)));
        pwm_set_gpio_level(GROUP_BlUE_LED_RIGHT,
                           (uint16_t)((float)MAX_LIGHT - ((float)(to_ms_since_boot(get_absolute_time()) - startTime) / 1000 * MAX_LIGHT)));
        pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_1,
                           (uint16_t)(((float)(to_ms_since_boot(get_absolute_time()) - startTime) / 1000 * MAX_LIGHT)));
        pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_2,
                           (uint16_t)(((float)(to_ms_since_boot(get_absolute_time()) - startTime) / 1000 * MAX_LIGHT)));
        pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_3,
                           (uint16_t)(((float)(to_ms_since_boot(get_absolute_time()) - startTime) / 1000 * MAX_LIGHT)));
        pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_1,
                          (uint16_t)(((float)(to_ms_since_boot(get_absolute_time()) - startTime) / 1000 * MAX_LIGHT)));
        pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_2,
                           (uint16_t)(((float)(to_ms_since_boot(get_absolute_time()) - startTime) / 1000 * MAX_LIGHT)));
        pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_3,
                           (uint16_t)(((float)(to_ms_since_boot(get_absolute_time()) - startTime) / 1000 * MAX_LIGHT)));
    }
    PrintInfo();
}
#endif

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
     printf("MinPlantVelocity: %d, MaxPlantVelocity: %d, isRandom: %b\n", getMinPlantVelocity(), getMaxPlantVelocity(), getRandomPlantVelocity());
     printf("MinLightVelocity: %d, MaxLightVelocity: %d, isRandom: %b\n", getMinLightVelocity(), getMaxLightVelocity(), getRandomLightVelocity());
     printf("Random note is %s", get_random_note_state() ? "enabled\n" : "disabled\n");
     printf("Same note: %d\n", get_control_same_note());
     printf("Note off percent: %d\n", get_note_off_speed_percent());
     printf("Min light note: %d, Max light note: %d\n", get_light_min_notes(), get_light_max_notes());



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

enum Status currentActiveStatus = Active;
enum Status get_status() {
    return status;
}


void BPM_clock_active() {
    if (status == Active) {
        cancel_repeating_timer(&midiTimer);
        status = BPMClockActive;
    }
     printf("BPM Clock Active\n");
    currentActiveStatus = BPMClockActive;
}

void BPM_clock_disable() {
    if (status == BPMClockActive) {
        add_repeating_timer_us(time, _repeating_timer_callback, NULL, &midiTimer);
        status = Active;
    }
    printf("BPM Clock Disable\n");
    currentActiveStatus = Active;
}


void MainStage() {
    static uint8_t counterValues = 0;
    static uint8_t stab_song[] = {36, 40, 43, 48};
    static uint8_t stab_song_step = 0;
    freq = getRealFreq();

//    freq = rand() % 32000 + 60;

    switch (status) {
        /** @brief Sleep mode
         *
         * Does nothing, until doesn't have frequencies above MIN_FREQ STABILIZATION_TIME in a row.
         * After that change status to Stabilization
         *
         */
        case Sleep:
//            // printf("%d\n", status);
            if (freq > MIN_FREQ) {
                counterValues++;
            } else {
                counterValues = 0;
            }

            if (counterValues >= STABILIZATION_TIME) {
                counterValues = 0;
                status = Stabilization;
                uint8_t note_on[3] = {0x90 | 0, stab_song[stab_song_step], 90};
                tud_midi_stream_write(0, note_on, 3);
                stab_song_step++;
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
                uint8_t note_off[3] = {0x80 | 0, stab_song[stab_song_step - 1], 0};
                tud_midi_stream_write(0, note_off, 3);
                stab_song_step = 0;
                printf("[+] Change status: Stab -> Sleep\n");
                return;
            }

            if (counterValues > AVERAGE_TIME) {
                averageFreq /= counterValues;
                averageFreqChanges /= counterValues;
                counterValues = 0;
                FilterFrequency(0, 0);
                if (currentActiveStatus == Active) {
                    add_repeating_timer_us(time, _repeating_timer_callback, NULL, &midiTimer);
                }
                status = currentActiveStatus;
                uint8_t note_off[3] = {0x80 | 0, stab_song[stab_song_step - 1], 0};
                tud_midi_stream_write(0, note_off, 3);
                stab_song_step = 0;
                printf("[+] Change status: Stab -> Active\n");
            }
            else {
                if (counterValues >= (AVERAGE_TIME / 3) * stab_song_step && stab_song_step < 4) {
                    uint8_t note_off[3] = {0x80 | 0, stab_song[stab_song_step - 1], 0};
                    tud_midi_stream_write(0, note_off, 3);
                    uint8_t note_on[3] = {0x90 | 0, stab_song[stab_song_step], 90};
                    tud_midi_stream_write(0, note_on, 3);
                    stab_song_step++;
                }
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
                return;
            }


            break;
        case BPMClockActive:
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
                FilterFrequency(0, 0);
                MidiStop();
                printf("[+] Change status: Active -> Sleep\n");
            }
    }
}

uint16_t first_val;
int32_t ledsValue[ASYNC];
#ifdef INVERT_LED
void LedStage() {
    uint32_t lastNotePlant;

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
            second_green_group = 0;
            break;
         /** @brief Stabilization mode
         *
         * Smooth switching on green LEDs
         *
         */
        case Stabilization:
            if ((ledsValue[ASYNC - 1] + level) < MAX_LIGHT) {
                ledsValue[ASYNC - 1] += level;
            }
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_1, ledsValue[ASYNC - 1]);
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_2, ledsValue[ASYNC - 1]);
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_3, ledsValue[ASYNC - 1]);
            second_green_group = ledsValue[ASYNC - 1];
            break;
         /** @brief Active mode
         *
         * Leds work in pulse mode,
         * also LEDs change brightness in dependence of last played midi_plant note (higher note, higher brightness)
         * Leds work in ASYNC mode, so one of them work with a delay
         */
        case Active:
            if (ledsValue[ASYNC - 1] + level > MAX_LIGHT && level > 0) {
                level *= -1;
            }
            else {
                if (ledsValue[ASYNC - 1] + level < MIN_LIGHT && level < 0) {
                    level *= -1;
                }
            }

            for (int i = 0; i < ASYNC - 1; i++) {
                ledsValue[i] = ledsValue[i + 1];
            }

            ledsValue[ASYNC - 1] += level;

            lastNotePlant = getLastNotePlant();
            first_val = MAX(MIN_LIGHT, MIN(MAX_LIGHT, ledsValue[0] + (((int)lastNotePlant - MIDDLE_NOTE) * NOTE_STRONG)));
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_1, first_val);
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_2, first_val);
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_3, first_val);
            second_green_group = MAX(MIN_LIGHT, MIN(MAX_LIGHT, ledsValue[ASYNC - 1] + (((int)lastNotePlant - MIDDLE_NOTE) * NOTE_STRONG)));
            break;
        case BPMClockActive:
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

            lastNotePlant = getLastNotePlant();
            first_val = MAX(MIN_LIGHT, MIN(MAX_LIGHT, ledsValue[0] + ((MIDDLE_NOTE - (int)lastNotePlant) * NOTE_STRONG)));
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_1, first_val);
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_2, first_val);
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_3, first_val);
            second_green_group = MAX(MIN_LIGHT, MIN(MAX_LIGHT, ledsValue[ASYNC - 1] + ((MIDDLE_NOTE - (int)lastNotePlant) * NOTE_STRONG)));
            break;
    }
}
#else
void LedStage() {
    uint32_t lastNotePlant;

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
                ledsValue[i] = MAX_LIGHT;
            }
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_1, MAX_LIGHT);
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_2, MAX_LIGHT);
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_3, MAX_LIGHT);
            pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_1, MAX_LIGHT);
            pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_2, MAX_LIGHT);
            pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_3, MAX_LIGHT);
            second_val = MAX_LIGHT;
            break;
         /** @brief Stabilization mode
         *
         * Smooth switching on green LEDs
         *
         */
        case Stabilization:
            if ((ledsValue[ASYNC - 1] - level) >= MIN_LIGHT) {
                ledsValue[ASYNC - 1] -= level;
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
            if (ledsValue[ASYNC - 1] + level > MAX_LIGHT && level > 0) {
                level *= -1;
            }
            else {
                if (ledsValue[ASYNC - 1] + level < MIN_LIGHT && level < 0) {
                    level *= -1;
                }
            }

            for (int i = 0; i < ASYNC - 1; i++) {
                ledsValue[i] = ledsValue[i + 1];
            }

            ledsValue[ASYNC - 1] += level;

            lastNotePlant = getLastNotePlant();
            first_val = MAX(MIN_LIGHT, MIN(MAX_LIGHT, ledsValue[0] + ((MIDDLE_NOTE - (int)lastNotePlant) * NOTE_STRONG)));
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_1, first_val);
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_2, first_val);
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_3, first_val);
            second_val = MAX(MIN_LIGHT, MIN(MAX_LIGHT, ledsValue[ASYNC - 1] + ((MIDDLE_NOTE - (int)lastNotePlant) * NOTE_STRONG)));
            pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_1, second_val);
            pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_2, second_val);
            pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_3, second_val);
            break;
        case BPMClockActive:
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

            lastNotePlant = getLastNotePlant();
            first_val = MAX(MIN_LIGHT, MIN(MAX_LIGHT, ledsValue[0] + ((MIDDLE_NOTE - (int)lastNotePlant) * NOTE_STRONG)));
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_1, first_val);
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_2, first_val);
            pwm_set_gpio_level(FIRST_GROUP_GREEN_LED_3, first_val);
            second_val = MAX(MIN_LIGHT, MIN(MAX_LIGHT, ledsValue[ASYNC - 1] + ((MIDDLE_NOTE - (int)lastNotePlant) * NOTE_STRONG)));
            pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_1, second_val);
            pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_2, second_val);
            pwm_set_gpio_level(SECOND_GROUP_GREEN_LED_3, second_val);
            break;
    }
}
#endif



