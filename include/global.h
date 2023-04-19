#ifndef BIOTRON_GLOBAL_H
#define BIOTRON_GLOBAL_H

/** @brief Led Pins */
#ifdef DEBUG_LED_BUILD
#define GROUP_BlUE_LED_CENTER 1
    #define GROUP_BlUE_LED_LEFT 0
    #define GROUP_BlUE_LED_RIGHT 16

    #define FIRST_GROUP_GREEN_LED_1 2
    #define FIRST_GROUP_GREEN_LED_2 3
    #define FIRST_GROUP_GREEN_LED_3 4

    #define SECOND_GROUP_GREEN_LED_1 9
    #define SECOND_GROUP_GREEN_LED_2 10
    #define SECOND_GROUP_GREEN_LED_3 11

    #define MAX_LIGHT 60000
    #define MIN_LIGHT 5000
    #define NOTE_STRONG 414
#else
    #define GROUP_BlUE_LED_CENTER 4
    #define GROUP_BlUE_LED_LEFT 4
    #define GROUP_BlUE_LED_RIGHT 4

    #define FIRST_GROUP_GREEN_LED_1 2
    #define FIRST_GROUP_GREEN_LED_2 2
    #define FIRST_GROUP_GREEN_LED_3 2

    #define SECOND_GROUP_GREEN_LED_1 3
    #define SECOND_GROUP_GREEN_LED_2 3
    #define SECOND_GROUP_GREEN_LED_3 3

    #define MAX_LIGHT 40000
    #define MIN_LIGHT 5000
    #define NOTE_STRONG 414
#endif
#define TEST_LED 25

#define PLANT_PIN  1
#define LIGHT_PIN 26


#define STABILIZATION_TIME (5 * TIMER_MULTIPLIER)
#define AVERAGE_TIME (5 * TIMER_MULTIPLIER)
#define SLEEP_TIME (3 * TIMER_MULTIPLIER)

// MIDI
#define TIMER_MIDI_US 1000000 // Timer for receiving plant frequency
#define MIN_FREQ 60 // Frequency that is ignored

// Plant info
#define TIMER_PLANT_MS 100
#define TIMER_MULTIPLIER (1000 / TIMER_PLANT_MS)

#define LOWEST_NOTE 36
#define HIGHEST_NOTE 107
#define MIDDLE_NOTE 60

//Light MIDI
#define MAX_OF_PHOTO 4000

// Led
#define ASYNC 20


/** @brief Enum of possible device states */
enum Status {
    Sleep,
    Stabilization,
    Active
};

/** @brief change BPM in us (if device is Active reload timer) */
void setBPM(int newTime);

/** @brief Change filter values in active mode */
void setFilterPercent(double newFilterPercent);

/** @brief Getters for public variables */
uint32_t getFreq();
uint32_t getLastFreq();
uint32_t getAvgFreq();
uint32_t getAvgFreqChanges();

/** @brief Init pins */
void Setup();

/** @brief Debug Leds */
void LedStage();

/** @brief Collects data about plant frequency and depending on it change device status. */
void FrequencyStage();

/** @brief Calculating Notes */
int GetNoteDiff(int oldVal, int newVal);

/** @brief Function before main part (Smooth change in the brightness of the blue light) */
void Intro();

#endif //BIOTRON_GLOBAL_H
