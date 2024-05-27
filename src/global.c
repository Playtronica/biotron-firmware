#include <pico/printf.h>
#include "global.h"
#include "pico/stdlib.h"
#include <stdlib.h>
#include "raw_plant.h"
#include "PLSDK/music.h"


enum Status status = Sleep;
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
    filter_val += (int32_t)((val - filter_val) * k);
//    if (!get_random_note_state()) (filter_val = filter_val / 10 * 10);
    filter_val = (filter_val >> 1) << 1;
    return filter_val;
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
                if (average_freq != 0) {
                    average_delta_freq += abs((int)last_freq - (int)raw_freq);
                }
                average_freq += raw_freq;
                last_freq = raw_freq;
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
                status = Active;
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
            if (raw_freq < MIN_FREQ) {
                counter++;
            } else {
                counter = 0;
            }

//            if (filterPercent != 0)
            if (true) {
                last_freq = filter_freq(raw_freq, 0.3);
            }
            else {
                last_freq = raw_freq;
            }


            printf("%d %d\n", average_freq, last_freq);
            if (counter > SLEEP_COUNTER) {
                counter = 0;
                last_freq = 0;
                average_freq = 0;
                average_delta_freq = 0;
                status = Sleep;
//                cancel_repeating_timer(&midiTimer);
                filter_freq(0, 0);
//                MidiStop();
                printf("[+] Change status: Active -> Sleep\n");
                return;
            }

            break;
        case BPMClockActive:
            break;
    }
}
