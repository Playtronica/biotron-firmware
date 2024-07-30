#include <pico/printf.h>
#include "PLSDK/channel.h"
#include "tusb.h"
#include "PLSDK/commands.h"
#include "PLSDK/constants.h"

Channel channels[MAX_COUNT_CHANNELS];


void init_midi() {
    tusb_init();
    for (int i = 0; i < MAX_COUNT_CHANNELS; i++) {
        channels[i].BPM = DEFAULT_BPM;
        channels[i].velocity = DEFAULT_VELOCITY;

        channels[i].scale = SCALE_MAJOR;

        channels[i].min_note = 48;
        channels[i].start_note = 60;
        channels[i].max_note = 80;
    }
    init_commands();
}

void remind_midi() {
    tud_task();
}

void set_BPM(uint8_t channel_id, uint16_t newBPM) {
    channels[channel_id].BPM = newBPM;
}

uint16_t get_BPM(uint8_t channel_id) {
    return channels[channel_id].BPM;
}

void set_velocity(uint8_t channel_id, uint8_t velocity) {
    channels[channel_id].velocity = velocity;
}

uint8_t get_velocity(uint8_t channel_id) {
    return channels[channel_id].velocity;
}


void set_scale(uint8_t channel_id, ScaleNums_t scale) {
    channels[channel_id].scale = scale;
}

ScaleNums_t get_scale(uint8_t channel_id) {
    return channels[channel_id].scale;
}

void set_min_note(uint8_t channel_id, uint8_t note) {
    channels[channel_id].min_note = note;
}

uint8_t get_min_note(uint8_t channel_id) {
    return channels[channel_id].min_note;
}

void set_start_note(uint8_t channel_id, uint8_t note) {
    channels[channel_id].start_note = note;
}

uint8_t get_start_note(uint8_t channel_id) {
    return channels[channel_id].start_note;
}

void set_max_note(uint8_t channel_id, uint8_t note) {
    channels[channel_id].max_note = note;
}

uint8_t get_max_note(uint8_t channel_id) {
    return channels[channel_id].max_note;
}
