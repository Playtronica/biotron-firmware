//
// Created by user on 24.11.22.
//

#include "pico/stdlib.h"

#ifndef BIOTRON_FREQUENCY_COUNTER_H
#define BIOTRON_FREQUENCY_COUNTER_H


void initFrequencyTimer();

bool isReady();
uint32_t getRealFreq();


#endif //BIOTRON_FREQUENCY_COUNTER_H
