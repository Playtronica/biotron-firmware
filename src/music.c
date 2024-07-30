#include <stdlib.h>
#include <pico/time.h>
#include <hardware/adc.h>
#include <pico/printf.h>
#include "music.h"
#include "pico/stdio.h"
#include "params.h"
#include "global.h"
#include "PLSDK/music.h"
#include "PLSDK/commands.h"

uint8_t last_note_plant = MIDDLE_NOTE;
uint8_t last_note_light = 0;


struct repeating_timer plantNoteOffTimer;
bool plant_note_off(struct repeating_timer *t) {
    note_off(0, last_note_plant);
    cancel_repeating_timer(&plantNoteOffTimer);
    return true;
}

void reset_plant_note_off() {
    note_off(0, last_note_plant);
    cancel_repeating_timer(&plantNoteOffTimer);
}

uint8_t get_CC(uint8_t currentNote) {
    static int lastCC = 127;
    uint8_t buff = abs(MIDDLE_NOTE - currentNote);
    uint8_t target_CC;
    if (currentNote > MIDDLE_NOTE) {
        target_CC = (1 - ((double )buff / (HIGHEST_NOTE - MIDDLE_NOTE))) * 127;
    }
    else {
        target_CC = (1 - ((double )buff / (MIDDLE_NOTE - LOWEST_NOTE))) * 127;
    }

    if (target_CC > lastCC) {
        lastCC += (target_CC - lastCC) / 2;
    }
    else {
        lastCC -= (lastCC - target_CC) / 2;
    }

    return lastCC;
}


int get_plant_counter() {
    static uint64_t last_change_time = 0;
    static uint32_t last_val = 0;
    static int8_t extra_counter = 0;
    const uint8_t a = 10, b = 30;


    int diff = (int)average_freq - (int)last_freq;
    bool minus = diff < 0;
    diff = abs(diff);

    if ((abs((int)last_val - (int)last_freq) >= average_delta_freq * 3) || settings.performance_mode) {
        last_change_time = time_us_64();
        last_val = last_freq;
        extra_counter = 0;
    }

    if (time_us_64() - last_change_time > 90000000) {
        int chance = rand() % 101;
        if (chance <= a * ((extra_counter + 10.0) / 10.0)) {
            extra_counter = MAX(-10, extra_counter - 2);
        } else if (chance <= a * ((extra_counter + 10.0) / 10.0) + b * ((extra_counter + 10.0) / 10.0)) {
            extra_counter = MAX(-10, extra_counter - 1);
        } else if (chance <= a * ((extra_counter + 10.0) / 10.0) + b * ((extra_counter + 10.0) / 10.0) + 20) {

        } else if (chance <= a * ((extra_counter + 10.0) / 10.0) + b * 2 + 20) {
            extra_counter = MIN(10, extra_counter + 1);
        } else {
            extra_counter = MIN(10, extra_counter + 2);
        }
    }

    double first = 0;
    double second = settings.firstValue;

    int i;

    if (diff - average_delta_freq >= 0) {
        i = 1;
        diff -= (int)average_delta_freq;
    } else return extra_counter;

    double extra;
    while (diff - (average_delta_freq + settings.fibPower * (first + second)) > 0) {
        diff -= (int)average_delta_freq + (int)(settings.fibPower * (first + second));
        extra = first + second;
        first = second;
        second = extra;
        i++;
    }
//    printf("%d %f\n", diff, settings.fibPower * (first + second));

    if (minus) {
        return -i + extra_counter;
    }
    return i + extra_counter;
}


void midi_plant() {
    uint8_t currentNote = MAX(LOWEST_NOTE,
                              MIN(HIGHEST_NOTE,
                                  calculate_note_by_scale(MIDDLE_NOTE, get_plant_counter(), settings.scale)));

    if (!is_mute && !settings.isMutePlantVelocity) {
        if (abs((int)currentNote - (int)last_note_plant) < settings.same_note) {
            return;
        }

        if (active_status == BPMClockActive) {
            note_off(0, last_note_plant);
        }

        uint8_t velocity = settings.isRandomPlantVelocity ?
                rand() % (settings.maxPlantVelocity + 1 - settings.minPlantVelocity) + settings.minPlantVelocity :
                settings.maxPlantVelocity;

        note_on(0, currentNote, velocity);

        if (active_status == Active) {
            add_repeating_timer_us(settings.BPM / 100 * settings.percent_note_off, plant_note_off,
                                   NULL, &plantNoteOffTimer);
        }
    }

    uint8_t note_cc[3] = {0xB0, 90, get_CC(currentNote)};
    print_pure(0, note_cc, 3);

    last_note_plant = currentNote;
}


void midi_light() {
    uint16_t adc = MIN(adc_read(), MAX_OF_LIGHT);
    uint16_t step = MAX_OF_LIGHT / (settings.light_note_range * 2);

    int counter = abs(MAX_OF_LIGHT / 2 - (int)adc) / step;
    if (adc > MAX_OF_LIGHT / 2) {
        counter = -counter;
    }


    uint8_t current_note = MAX(MIDDLE_NOTE - 24 - settings.light_note_range,
                               MIN(MIDDLE_NOTE - 24 + settings.light_note_range,
                                   calculate_note_by_scale(MIDDLE_NOTE - 24, counter, settings.scale)));


    note_off(1, last_note_light);
    if (!is_mute && !settings.isMuteLightVelocity) {
        uint8_t vel = settings.isRandomLightVelocity ?
                rand() % (settings.maxLightVelocity + 1 - settings.minLightVelocity) + settings.minLightVelocity :
                      settings.maxLightVelocity;
        note_on(1, current_note, vel);

    }
    last_note_light = current_note;
}

void midi_light_pitch() {
    uint16_t buff = (uint16_t)(((double)MIN(adc_read(), MAX_OF_LIGHT) / (double)MAX_OF_LIGHT) * 4096.0);
    change_pitch(0, buff % 127, buff / 12);
}

void stop_midi() {
    note_off(0, last_note_plant);
    note_off(1, last_note_light);
    change_pitch(0, 63, 63);
}