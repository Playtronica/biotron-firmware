#include <pico/printf.h>
#include <malloc.h>
#include "PLSDK/commands.h"
#include "PLSDK/constants.h"
#include "PLSDK/music.h"


const uint8_t MAJOR[] = {2, 4, 5, 7, 9, 11, 12 };
const uint8_t MINOR[] = {2, 3, 5, 7, 8, 10, 12 };
const uint8_t CHROM[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
const uint8_t dorian[] = { 2, 3, 5, 7, 9, 10, 12 };
const uint8_t mixolydian[] = { 2, 4, 5, 7, 9, 10, 12 };
const uint8_t lydian[] = { 2, 4, 5, 6, 7, 9, 11, 12 };
const uint8_t wholetone[] = { 2, 4, 6, 8, 10, 12 };
const uint8_t minblues[] = { 3, 5, 6, 7, 10, 12 };
const uint8_t majblues[] = { 2, 3, 4, 7, 9, 12 };
const uint8_t minpen[] = { 3, 5, 7, 10, 12 };
const uint8_t majpen[] = { 2, 4, 7, 9, 12 };
const uint8_t diminished[] = { 2, 3, 5, 6, 8, 9, 11, 12 };

uint8_t len_of_custom = 7;
uint16_t custom[71] = { 195, 289, 513, 686, 796, 1008, 1200};

const NotesScale_t scales[] = {
        {7,  MAJOR},
        {7,  MINOR},
        {12, CHROM},
        {7,  dorian},
        {7,  mixolydian},
        {8,  lydian},
        {6,  wholetone},
        {6,  minblues},
        {6,  majblues},
        {5,  minpen},
        {5,  majpen},
        {8,  diminished}
};


int calculate_note_by_scale(uint8_t start_note, int counter, ScaleNums_t scale) {
    NotesScale_t _scale = scales[scale];

    if (counter < 0) {
        counter *= -1;
        start_note -= MIN(start_note, _scale.scale[_scale.steps - 1] * (counter / _scale.steps + 1));
        counter -= _scale.steps * (counter / _scale.steps + 1);
        counter *= -1;
    }

    int note = MIN(127, _scale.scale[_scale.steps - 1] * (counter / _scale.steps));
    if (counter % _scale.steps > 0) {
        note += MIN(127 - note, _scale.scale[counter % _scale.steps - 1]);
    }

    return start_note + note;
}


void change_volume(uint8_t channel_id, uint8_t volume) {
    uint8_t change_volume[3] = {CC_START | channel_id, CC_VOLUME, volume};
    print_pure(CABLE_NUM_MAIN, change_volume, 3);
}

void note_on(uint8_t channel_id, uint8_t note, uint8_t velocity) {
    uint8_t note_on[3] = {NOTE_ON | channel_id, note, velocity};
    print_pure(CABLE_NUM_MAIN, note_on, 3);
}

void note_off(uint8_t channel_id, uint8_t note) {
    uint8_t note_off[3] = {NOTE_OFF | channel_id, note, 0};
    print_pure(CABLE_NUM_MAIN, note_off, 3);
}

void change_pitch(uint8_t channel_id, uint8_t lsb, uint8_t msb) {
    uint8_t change_pitch[3] = {CHANGE_PITCH | channel_id, lsb, msb};
    print_pure(CABLE_NUM_MAIN, change_pitch, 3);
}

void stop_all_notes(uint8_t channel_id) {
    uint8_t stop_all_notes[3] = {CC_START | channel_id, CC_STOP_ALL_NOTES};
    print_pure(CABLE_NUM_MAIN, stop_all_notes, 3);
}



