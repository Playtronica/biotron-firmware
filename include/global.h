#ifndef BIOTRON_GLOBAL_H
#define BIOTRON_GLOBAL_H

/** @brief Led Pins */
#include "notes.h"

#define DEBUG_LED_BUILD

#ifdef DEBUG_LED_BUILD
    #define PLANT_PIN  13

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
    #define MIN_LIGHT 7000
    #define NOTE_STRONG 414
#else
    #define PLANT_PIN  1
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
    #define MIN_LIGHT 10000
    #define NOTE_STRONG 414
#endif
#define TEST_LED 25

/** @brief Pins for getting data for music */

#define LIGHT_PIN 26

/** @brief Time (in counters) for change status
 *
 * STABILIZATION_TIME change time between SLEEP and STABILIZATION statuses,
 * AVERAGE_TIME change time between STABILIZATION and ACTIVE statuses,
 * SLEEP_TIME change time between ACTIVE and STABILIZATION statuses,
 * Change status from STABILIZATION to SLEEP happens instantly
 */
#define STABILIZATION_TIME (5 * TIMER_MULTIPLIER)
#define AVERAGE_TIME (5 * TIMER_MULTIPLIER)
#define SLEEP_TIME (3 * TIMER_MULTIPLIER)

/** @brief Constants for describing getting values from PLANT PIN
 *
 * TIMER_PLANT_MS responsible for the speed of data receiving (the higher the speed, the greater the error),
 * TIMER_MULTIPLIER coefficient controls scale of horizontal sweep,
 * MIN_FREQ is minimum perceived frequency
 */
#define TIMER_PLANT_MS 100
#define TIMER_MULTIPLIER (1000 / TIMER_PLANT_MS)
#define MIN_FREQ 60

/** @brief Start BPM of device in us
 *
 *  This variable works with repeating times, that play notes.
 *  Converting formula => BPM = (1 sec * 60) / TIME
 */
#define TIMER_MIDI_US 500000
#define LIGHT_BPM_DEF 4

/** @brief Notes settings */
#define LOWEST_NOTE 36
#define HIGHEST_NOTE 107
#define MIDDLE_NOTE 60

/** @brief (Maximum possible value from photoresistor - 10%) for LIGHT NOTES*/
#define MAX_OF_LIGHT 4000

/** @brief Asynchronous LEDs for wave effect */
#define ASYNC 200

#define DEF_FIB_POW 0.5
#define DEF_FIB_FIRST 0.1

#define DEF_FILTER_PERCENT 0

#ifndef SCALE
#define SCALE 0
#endif

#define LIGHT_NOTE_MIN_DEFAULT 24
#define LIGHT_NOTE_MAX_DEFAULT 48

/** @brief Enum of possible device states */
enum Status {
    Sleep,
    Stabilization,
    Active,
    BPMClockActive
};

void play_music();

enum Status get_status();
void BPM_clock_active();
void BPM_clock_disable();

void setScale(int id);
int getScale();

void setPlantVelocity(uint8_t velocity);
uint8_t getPlantVelocity();

void setLightVelocity(uint8_t velocity);
uint8_t getLightVelocity();

/** @brief change BPM in us (if device is Active reload timer) */
void setBPM(int newTime);
int getBPM();
void setLightBPM(uint32_t newBPM);
int getLightBPM();

bool get_mute_mode();

void setFreqPower(double power, double value);
double getFibPower();
double getFirstValue();

/** @brief Change filter values in active mode */
void setFilterPercent(double newFilterPercent);
double getFilterPercent();

/** @brief Getters for public variables */
uint32_t getFreq();
uint32_t getLastFreq();
uint32_t getAvgFreq();
uint32_t getAvgFreqChanges();

/** @brief Init pins */
void Setup();
/** @brief Animation before start
 *
 *  Smooth switching on of blue LEDs
 * */
void Intro();
void PrintInfo();

/** @brief Work with leds
 *
 * Smooth switching on of green LEDs on stabilization
 * Controls green LEDs on Active mode
 *
 */
void LedStage();

/** @brief Main part of code
 *
 * Collects frequency from PLANT_PIN and filter it (if it require),
 * Changes statuses of device,
 * Init MIDI repeating timer
 */
void MainStage();



#endif //BIOTRON_GLOBAL_H
