//
// Created by Марк Захарченко on 24.05.2024.
//

#ifndef BIOTRON_GLOBAL_H
#define BIOTRON_GLOBAL_H

#define LIGHT_PIN 26


#define STABILIZATION_COUNTER (5 * TIMER_MULTIPLIER)
#define AVERAGE_COUNTER (5 * TIMER_MULTIPLIER)
#define SLEEP_COUNTER (3 * TIMER_MULTIPLIER)

enum Status {
    Sleep,
    Stabilization,
    Active,
    BPMClockActive
};

extern enum Status status;
extern enum Status active_status;
void status_loop();

extern uint32_t last_freq;
extern uint32_t average_freq;
extern uint32_t average_delta_freq;

bool play_music();

void bpm_clock_control(bool enabled);
void reset_bpm();
void load_settings();

#endif //BIOTRON_GLOBAL_H
