#include "pico/stdlib.h"

#ifndef BIOTRON_FREQUENCY_COUNTER_H
#define BIOTRON_FREQUENCY_COUNTER_H

/** @brief Initialization of receiving the frequency of the plant */
void initFrequencyTimer();

/** @brief Checking the new frequency */
bool isReady();
/** @brief Get pure frequency from the plant */
uint32_t getRealFreq();


#endif //BIOTRON_FREQUENCY_COUNTER_H
