#include <pico/printf.h>
#include "global.h"
#include "pico/stdlib.h"
#include <stdlib.h>
#include "raw_plant.h"
#include "PLSDK/music.h"
#include "params.h"
#include "music.h"


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

struct repeating_timer midi_timer;
bool play_music() {
    static uint64_t time_log = 0;
    static uint8_t counter = 1;

    midi_plant();

    if (settings.light_pitch_mode) midi_light_pitch();
    else if (counter++ >= settings.lightBPM) {
        midi_light();
        counter = 1;
    }

    if (time_log == 0) {
        time_log = time_us_64();
    }
    if (time_us_64() - time_log > 100000) {
        printf("{\"AverageFreq\": %d, \"Freq\": %d, \"PlantNote\": %d, \"LightNote\": %d }\n",
               average_freq, last_freq, last_note_plant, last_note_light);
        time_log = time_us_64();
    }
    return true;
}


void bpm_clock_control(bool enabled) {
    if (enabled && status == Active) {
        cancel_repeating_timer(&midi_timer);
        status = BPMClockActive;

    }
    else if (!enabled && status == BPMClockActive) {
        add_repeating_timer_us(settings.BPM, play_music, NULL, &midi_timer);
        status = Active;
    }
    active_status = enabled ? BPMClockActive : Active;
    reset_plant_note_off();
}

void reset_bpm() {
    if (status == Active) {
        reset_plant_note_off();
        cancel_repeating_timer(&midi_timer);
        add_repeating_timer_us(settings.BPM, play_music, NULL, &midi_timer);
    }
}

void load_settings() {
    static bool is_stopped = false;
    if (is_stopped) {
        if (status == Active) {
            add_repeating_timer_us(settings.BPM, play_music, NULL, &midi_timer);
        }

    } else {
        if (status == Active) {
            cancel_repeating_timer(&midi_timer);
            reset_plant_note_off();
        }
    }
    is_stopped = !is_stopped;

}


void status_loop() {
    const uint8_t jingle_notes[] = {36, 40, 43, 48};
    static uint8_t jingle_step = 0;
    static uint8_t counter = 0;

    if (!plant_is_ready()) {
        return;
    }

    uint32_t raw_freq = get_real_freq();
//    printf("%d\n", raw_freq);

    switch (status) {
        case Sleep:
            if (raw_freq > MIN_FREQ) {
                counter++;
            } else {
                counter = 0;
            }

            if (counter >= STABILIZATION_COUNTER) {
                note_on(0, jingle_notes[jingle_step++], 90);

                status = Stabilization;
                counter = 0;
                printf("[+] Change status: Sleep -> Stab\n");
            }
            break;
        case Stabilization: {
            if (raw_freq > MIN_FREQ) {
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
                note_off(0, jingle_notes[jingle_step - 1]);
                jingle_step = 0;
                status = Sleep;
                printf("[+] Change status: Stab -> Sleep\n");
                break;
            }

            if (counter > AVERAGE_COUNTER) {
                average_freq /= counter;
                average_delta_freq /= counter;
                counter = 0;
                note_off(0, jingle_notes[jingle_step - 1]);
                jingle_step = 0;
                if (active_status == Active) {
                    add_repeating_timer_us(settings.BPM, play_music, NULL, &midi_timer);
                }
                status = active_status;
                printf("[+] Change status: Stab -> Active\n");
            }
            else {
                if (counter >= (AVERAGE_COUNTER / 3) * jingle_step && jingle_step < 4) {
                    note_off(0, jingle_notes[jingle_step - 1]);
                    note_on(0, jingle_notes[jingle_step++], 90);
                }
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


            if (counter > SLEEP_COUNTER) {
                counter = 0;
                last_freq = 0;
                average_freq = 0;
                average_delta_freq = 0;
                status = Sleep;
                if (status == Active) {
                    cancel_repeating_timer(&midi_timer);
                }

                filter_freq(0, 0);
                stop_midi();
                printf("[+] Change status: Active -> Sleep\n");
                return;
            }

            break;
    }
}
