//
// Created by user on 06.12.22.
//

#ifndef BIOTRON_GLOBAL_H
#define BIOTRON_GLOBAL_H

// PINS
#define PLANT_PIN  1
#define BlUE_LED 0
#define FIRST_GREEN_LED 2
#define SECOND_GREEN_LED 3
#define PHOTORESISTORS_PIN 4
#define TEST_LED 25

// Time Status (by counter)
#define STAB_TIME 40
#define AVERAGE_TIME 40
#define SLEEP_TIME 24

// Plant MIDI
#define TIMER_MS 125 // Timer for receiving plant frequency
#define TIMER_MULTIPLIER (1000 / TIMER_MS)
#define MIN_FREQ 60 // Frequency that is ignored

//Light MIDI
#define MAX_OF_PHOTO 4200

//Fibonacci
#define NOTE_DISTANCE 0
#define FIRST_VALUE 0.1


uint32_t realFrequency;
uint32_t averageFreq;



// Device Status
enum Status {
    Sleep,
    Avg,
    Active
};

enum Status status;

void Setup();

// Debug LEDS
void LedStage();

void FrequencyStage();

int GetNoteDiff(int oldVal, int newVal);

void PrintLogo();

#endif //BIOTRON_GLOBAL_H
