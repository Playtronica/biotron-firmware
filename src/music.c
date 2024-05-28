#include <stdlib.h>
#include <pico/time.h>
#include <hardware/adc.h>
#include "music.h"
#include "pico/stdio.h"
#include "params.h"
#include "global.h"
#include "PLSDK/music.h"
#include "PLSDK/commands.h"

uint8_t last_note_plant = MIDDLE_NOTE;
uint8_t last_note_light = 0;


struct repeating_timer plantNoteOffTimer;
bool MidiPlantNoteOff(struct repeating_timer *t) {
    note_off(0, last_note_plant);
    cancel_repeating_timer(&plantNoteOffTimer);
    return true;
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

    uint32_t oldVal = average_freq;
    uint32_t newVal = last_freq;

    int diff = (int)newVal - (int)oldVal;
    bool minus = diff < 0;
    diff = abs(diff);

    if (abs((int)last_val - (int)newVal) >= average_delta_freq * 3) {
        last_change_time = time_us_64();
        last_val = newVal;
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

    uint32_t noteChangeValue = average_delta_freq;
    if (diff - noteChangeValue >= 0) {
        i = 1;
        diff -= noteChangeValue;
    } else return 0;

    double extra;
    while (diff - (noteChangeValue + settings.fibPower * (first + second)) >= 0) {
        diff -= noteChangeValue + settings.fibPower * (first + second);
        extra = first + second;
        first = second;
        second = extra;
        i++;
    }

    if (minus) {
        return -i + extra_counter;
    }
    return i + extra_counter;
}



void midi_plant() {
    uint8_t currentNote = calculate_note_by_scale(MIDDLE_NOTE, get_plant_counter(), settings.scale);

    if (!is_mute) {
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
            add_repeating_timer_us(settings.BPM / 100 * settings.percent_note_off, MidiPlantNoteOff,
                                   NULL, &plantNoteOffTimer);
        }
    }

    uint8_t note_cc[3] = {0xB0, 90, get_CC(currentNote)};
    print_pure(0, note_cc, 3);

    last_note_plant = currentNote;
}


void midi_light() {
    if (settings.light_note_min > settings.light_note_max) {
        uint8_t buff = settings.light_note_min;
        settings.light_note_max = settings.light_note_min;
        settings.light_note_min = buff;
    }

    uint16_t buff = MIN(adc_read(), MAX_OF_LIGHT);
    uint8_t current_note = (MAX_OF_LIGHT - buff) / (MAX_OF_LIGHT / (settings.light_note_max - settings.light_note_min))
                           + settings.light_note_min;

    if (!is_mute) {
        note_off(1, last_note_light);
        uint8_t vel = settings.isRandomLightVelocity ?
                rand() % (settings.light_note_max + 1 - settings.light_note_min) + settings.light_note_min :
                      settings.light_note_max;
        note_on(1, current_note, vel);

    }
    last_note_light = current_note;
}

void midi_light_pitch() {
    uint16_t buff = (uint16_t)(((double)MIN(adc_read(), MAX_OF_LIGHT) / (double)MAX_OF_LIGHT) * 4096.0);
    change_pitch(0, buff % 127, buff / 12);
}
