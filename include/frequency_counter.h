//
// Created by user on 24.11.22.
//

#include "pico/stdlib.h"

#ifndef BIOTRON_FREQUENCY_COUNTER_H
#define BIOTRON_FREQUENCY_COUNTER_H

uint8_t pin;

bool freq_ready;
struct repeating_timer timerPlant;

void setupPwm(uint8_t freq_pin);
void beginTimer(uint8_t freq_pin, uint16_t period_ms);

bool isReady();
uint32_t getFreq();

uint32_t count;
uint32_t LastCount;
uint8_t slice_num;

#endif //BIOTRON_FREQUENCY_COUNTER_H
