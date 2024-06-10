#ifndef BIOTRON_RAW_PLANT_H
#define BIOTRON_RAW_PLANT_H


#define PLANT_PIN 13

#define TIMER_PLANT_MS 100
#define TIMER_MULTIPLIER (1000 / TIMER_PLANT_MS)

#define MIN_FREQ 60

void init_plant();
bool plant_is_ready();
uint32_t get_real_freq();

#endif //BIOTRON_RAW_PLANT_H
