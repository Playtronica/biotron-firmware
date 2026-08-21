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
#include "leds.h"
#include "PLSDK.h"
#include "midi_note_lifecycle.h"
#include "midi_value_safety.h"

uint8_t last_note_plant = MIDDLE_NOTE;
uint8_t last_note_light = 0;


alarm_id_t note_off_alarm_id = -1;


int64_t plant_note_off(alarm_id_t id, void *user_data) {
    const uintptr_t identity = (uintptr_t)user_data;
    note_off(midi_note_identity_channel(identity),
             midi_note_identity_note(identity));
    if (note_off_alarm_id == id) note_off_alarm_id = -1;
    return 0;
}

void reset_plant_note_off() {
    note_off(settings.plant_channel, last_note_plant);
    stop_all_notes(settings.plant_channel);
    if (note_off_alarm_id >= 0) {
        cancel_alarm(note_off_alarm_id);
        note_off_alarm_id = -1;
    }
}

uint8_t get_CC(int counter) {
//    static int lastCC = 127;
//    uint8_t buff = abs(MIDDLE_NOTE - counter);
//    uint8_t target_CC;
//    if (counter > MIDDLE_NOTE) {
//        target_CC = (1 - ((double )buff / HIGHEST_NOTE_RANGE)) * 127;
//    }
//    else {
//        target_CC = (1 - ((double )buff / LOWEST_NOTE_RANGE)) * 127;
//    }
//
//    if (target_CC > lastCC) {
//        lastCC += (target_CC - lastCC) / 2;
//    }
//    else {
//        lastCC -= (lastCC - target_CC) / 2;
//    }

    return 63 + counter;
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

    if (minus) {
        return -i + extra_counter;
    }
    return i + extra_counter;
}


void midi_plant(int64_t to_the_next_beat_us) {
    int plant_counter = get_plant_counter();

    uint8_t currentNote = MAX(settings.middle_plant_note - LOWEST_NOTE_RANGE,
                              MIN(settings.middle_plant_note + HIGHEST_NOTE_RANGE,
                                  calculate_note_by_scale(settings.middle_plant_note,
                                                          plant_counter, settings.scale)));

    if (!isMutedByButton && !settings.isMutePlantVelocity) {
        if (abs((int)currentNote - (int)last_note_plant) < settings.same_note_plant) {
            return;
        }

        if (active_status == BPMClockActive) {
            note_off(settings.plant_channel, last_note_plant);
        }

        uint8_t velocity = settings.isRandomPlantVelocity ?
                midi_random_velocity((uint32_t)rand(), settings.minPlantVelocity,
                                     settings.maxPlantVelocity) :
                midi_clamp_7bit(settings.maxPlantVelocity);

        note_on(settings.plant_channel, currentNote, velocity);

        if (active_status == Active) {
            const uintptr_t identity = midi_note_identity_pack(
                    settings.plant_channel, currentNote);
            note_off_alarm_id = add_alarm_in_us(
                    MAX(1, to_the_next_beat_us / MAX(1, settings.fraction_note_off)),
                    plant_note_off, (void *)identity, false);
            if (note_off_alarm_id < 0) {
                // Failing silent here leaves a synth note held forever. A
                // short note is safer than a missing Note Off.
                note_off(settings.plant_channel, currentNote);
            }
        }
    }

    uint8_t note_cc[3] = {0xB0 | settings.plant_channel, 90, get_CC(plant_counter)};
    print_pure(0, note_cc, 3);

    last_note_plant = currentNote;
}


void midi_light() {
    uint16_t adc = MIN(adc_read(), MAX_OF_LIGHT);
    const uint8_t light_note_range = midi_nonzero_range(settings.light_note_range);
    uint16_t step = MAX_OF_LIGHT / (light_note_range * 2);

    int counter = abs(MAX_OF_LIGHT / 2 - (int)adc) / step;
    if (adc > MAX_OF_LIGHT / 2) {
        counter = -counter;
    }


    uint8_t current_note = MAX(settings.middle_plant_note - LIGHT_DIFFERENCE - light_note_range,
                               MIN(settings.middle_plant_note - LIGHT_DIFFERENCE + light_note_range,
                                   calculate_note_by_scale(settings.middle_plant_note - LIGHT_DIFFERENCE, counter,
                                                           settings.scale)));

    note_off(settings.light_channel, last_note_light);

    if (abs((int)current_note - (int)last_note_light) < settings.same_note_light) {
        return;
    }

    if (!isMutedByButton && !settings.isMuteLightVelocity) {
        uint8_t vel = settings.isRandomLightVelocity ?
                midi_random_velocity((uint32_t)rand(), settings.minLightVelocity,
                                     settings.maxLightVelocity) :
                midi_clamp_7bit(settings.maxLightVelocity);
        note_on(settings.light_channel, current_note, vel);

    }
    last_note_light = current_note;
}

void midi_light_pitch() {
    uint16_t buff = (uint16_t)(((double)MIN(adc_read(), MAX_OF_LIGHT) / (double)MAX_OF_LIGHT) * 4096.0);
    change_pitch(0, buff % 127, buff / 12);
}

void stop_midi() {
    note_off(settings.plant_channel, last_note_plant);
    note_off(settings.light_channel, last_note_light);
    stop_all_notes(settings.plant_channel);
    stop_all_notes(settings.light_channel);
    change_pitch(0, 63, 63);
}

void play_music(int64_t to_the_next_beat) {
    static uint64_t time_log = 0;
    static uint8_t counter = 1;

    midi_plant(to_the_next_beat);

    if (settings.light_pitch_mode) midi_light_pitch();
    else if (counter++ >= settings.lightBPM) {
        midi_light();
        light_note_observer();
        counter = 1;
    }

    if (time_log == 0) {
        time_log = time_us_64();
    }
    if (time_us_64() - time_log > 100000) {
        plsdk_printf("{\"AverageFreq\": %d, \"Freq\": %d, \"PlantNote\": %d, \"LightNote\": %d }\n",
                     average_freq, last_freq, last_note_plant, last_note_light);
        time_log = time_us_64();
    }
}

void play_music_bpm_clock() {
    if (status != BPMClockActive) return;
    play_music(0);
}
