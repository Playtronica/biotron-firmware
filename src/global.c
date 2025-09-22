#include <stdlib.h>
#include <pico/stdlib.h>
#include <hardware/adc.h>

#include "PLSDK/music.h"
#include "PLSDK.h"
#include "PLSDK/cap_buttons.h"

#include "params.h"
#include "music.h"
#include "raw_plant.h"
#include "leds.h"

#include "global.h"

enum Status status = Sleep;
enum Status active_status = Active;
uint32_t last_freq = 0;
uint32_t average_freq = 0;
uint32_t average_delta_freq = 0;

uint32_t filter_freq(double val, double k) {
    static uint32_t filter_val = 0;
    if (filter_val == 0) {
        filter_val = val;
        return filter_val;
    }
    if (val == 0 && k == 0) {
        filter_val = 0;
    }
    filter_val += (int32_t)((val - filter_val) * (1 - k));
    if (!settings.random_note) filter_val = (filter_val << 1) >> 1;
    return filter_val;
}


int64_t play_music_alarm(alarm_id_t id, void *user_data) {
    static bool is_swing_note = false;

    int64_t to_the_next_beat_us = is_swing_note ? (int64_t)(settings.BPM * (settings.swing_first_note_percent / 100.0))
            : (int64_t)(settings.BPM * ((100 + (100 - settings.swing_first_note_percent)) / 100.0));
    printf("%d %d %d\n", is_swing_note, to_the_next_beat_us, settings.swing_first_note_percent);
    is_swing_note = !is_swing_note;

    play_music(to_the_next_beat_us);

    return to_the_next_beat_us;
}

alarm_id_t play_music_alarm_id;


void start_music_alarm() {
    play_music_alarm_id = add_alarm_in_us(settings.BPM, play_music_alarm, NULL, false);
}

void stop_music_alarm() {
    cancel_alarm(play_music_alarm_id);
}


void bpm_clock_control(bool enabled) {
    if (enabled && status == Active) {
        stop_music_alarm();
        status = BPMClockActive;

    }
    else if (!enabled && status == BPMClockActive) {
        start_music_alarm();
        status = Active;
    }
    active_status = enabled ? BPMClockActive : Active;
    reset_plant_note_off();
}

void stop_bpm() {
    if (status == Active) {
        reset_plant_note_off();
        stop_music_alarm();
    }
}


void reset_bpm() {
    if (status == Active) {
        stop_bpm();
        start_music_alarm();
    }
}



void load_settings() {
    static bool is_stopped = false;
    if (is_stopped) {
        if (status == Active) {
            start_music_alarm();
        }

    } else {
        if (status == Active) {
            stop_music_alarm();
            reset_plant_note_off();
        }
    }
    is_stopped = !is_stopped;
}


void status_loop() {
    static uint8_t counter = 0;

    if (!plant_is_ready()) {
        return;
    }

    uint32_t raw_freq = get_real_freq();

    switch (status) {
        case Sleep:
            if (TestMode) {
                plsdk_printf("{\"generator_freq\": %d, \"photoresistor_adc\": %d,"
                       "\"buttons_state\": {"
                       "\"finger_button\": %d, \"button_bottom\": %d, \"button_top\": %d}}\n",
                       raw_freq, adc_read(), button_states[0],  button_states[1],  button_states[2]);

                break;
            }

            if (raw_freq > MIN_FREQ) {
                counter++;
            } else {
                counter = 0;
            }

            if (counter >= STABILIZATION_COUNTER) {
                status = Stabilization;
                counter = 0;
                plsdk_printf("[+] Change status: Sleep -> Stab\n");
            }
            break;
        case Stabilization: {
            if (raw_freq > MIN_FREQ && !TestMode) {
                counter++;
                uint32_t b = filter_freq(raw_freq, 0.3);
                if (average_freq == 0) {
                    last_freq = raw_freq;
                }
                else {
                    average_delta_freq += abs((int)last_freq - (int)raw_freq);
                    raw_freq = last_freq;
                }
                average_freq += b;
            } else {
                counter = 0;
                average_delta_freq = 0;
                average_freq = 0;
                last_freq = 0;
                note_off(settings.plant_channel, 92);
                note_off(settings.plant_channel, 91);
                status = Sleep;
                plsdk_printf("[+] Change status: Stab -> Sleep\n");
                break;
            }

            if (counter > AVERAGE_COUNTER) {
                average_freq /= counter;
                average_delta_freq /= counter;
                counter = 0;
                note_off(settings.plant_channel, 92);
                note_off(settings.plant_channel, 91);
                if (active_status == Active) {
                    start_music_alarm();
                }
                status = active_status;
                plsdk_printf("[+] Change status: Stab -> Active\n");
            }

            break;
        }
        case Active:
        case BPMClockActive:
            if (raw_freq < MIN_FREQ) {
                counter++;
            } else {
                counter = 0;
            }

            if (settings.filterPercent != 0) {
                last_freq = filter_freq(raw_freq,  settings.filterPercent);
            }
            else {
                last_freq = raw_freq;
            }


            if (counter > SLEEP_COUNTER || TestMode) {
                counter = 0;
                last_freq = 0;
                average_freq = 0;
                average_delta_freq = 0;
                if (status == Active) {
                    stop_music_alarm();
                }
                status = Sleep;
                filter_freq(0, 0);
                stop_midi();
                plsdk_printf("[+] Change status: Active -> Sleep\n");
                return;
            }

            break;
    }
}
