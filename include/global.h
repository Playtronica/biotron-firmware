//
// Created by user on 06.12.22.
//

#ifndef BIOTRON_GLOBAL_H
#define BIOTRON_GLOBAL_H

#include <pico/time.h>

// PINS
#define PLANT_PIN  1
#define BlUE_LED 4
#define FIRST_GREEN_LED 2
#define SECOND_GREEN_LED 3
#define PHOTORESISTORS_PIN 26
#define TEST_LED 25

// Time Status (by counter)
#define STAB_TIME (5 * TIMER_MULTIPLIER)
#define AVERAGE_TIME (5 * TIMER_MULTIPLIER)
#define SLEEP_TIME (3 * TIMER_MULTIPLIER)

// MIDI
#define TIMER_MIDI_US 1000000// Timer for receiving plant frequency
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
#define MAX_LIGHT 40000
#define MIN_LIGHT 5000
#define NOTE_STRONG 414

// Fibonacci
double NOTE_DISTANCE;
double FIRST_VALUE;

// Filter
double filterPercent;

uint32_t realFrequency;
uint32_t averageFreq;

uint8_t plantVelocity;
uint8_t lightVelocity;

int time;

// Device Status
enum Status {
    Sleep,
    Avg,
    Active
};

// Plays Notes
struct repeating_timer timer;
bool repeating_timer_callback(struct repeating_timer *t);

// Device Status
enum Status status;

// Init pins and global variables
void Setup();

// Debug Leds
void LedStage();

// Collects data about plant frequency and depending on it change device status.
void FrequencyStage();

// Calculating Notes
int GetNoteDiff(int oldVal, int newVal);

void Intro();

#endif //BIOTRON_GLOBAL_H
