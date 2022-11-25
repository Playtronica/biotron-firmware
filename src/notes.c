#include "../include/notes.h"

#define LOWEST_NOTE 36
#define HIGHEST_NOTE 119
#define MIDDLE_NOTE 79


typedef struct {
    int steps;
    const uint8_t * scale;
} NotesScale_t;


const uint8_t major[] = { 2, 2, 1, 2, 2, 2, 1 };    
const uint8_t minor[] = { 2, 1, 2, 2, 1, 2, 2 };
const uint8_t chrom[] = { 1 };
const uint8_t dorian[] = { 2, 1, 2, 2, 2, 1, 2 };
const uint8_t mixolydian[] = { 2, 2, 1, 2, 2, 1, 2 };
const uint8_t lydian[] = { 2, 2, 1, 1, 1, 2, 2, 1 };
const uint8_t wholetone[] = { 2, 2, 2, 2, 2, 2 };
const uint8_t minblues[] = { 3, 2, 1, 1, 3, 2 };
const uint8_t majblues[] = { 2, 1, 1, 3, 2, 3 };
const uint8_t minpen[] = { 3, 2, 2, 3, 2 };
const uint8_t majpen[] = { 2, 2, 3, 2, 3 }; 
const uint8_t diminished[] = { 2, 1, 2, 1, 2, 1, 2, 1 }; 

const NotesScale_t scales[] = {
    {7, major},
    {7, minor},
    {1, chrom},
    {7, dorian},
    {7, mixolydian},
    {8, lydian},
    {6, wholetone},
    {6, minblues},
    {6, majblues},
    {5, minpen},
    {5, majpen},
    {8, diminished},
};

ScaleNums_t scale = SCALE_CHROM;


uint32_t getNote(uint32_t percent) {
    NotesScale_t * _scale = &scales[scale];
    bool minus = percent < 0;

    if (minus) percent *= -1;

    uint32_t note = MIDDLE_NOTE;
    int step = 0;
    for (int i = 0; i < percent; i++) {
        if (step >= _scale[scale].steps) {
            step = 0;
        }
        if (minus) {
            note += _scale[scale].scale[step] * -1;
        } else {
            note += _scale[scale].scale[step];
        }

        step++;
    }

    if (note > HIGHEST_NOTE) {
        note = HIGHEST_NOTE;
    }
    if (note < LOWEST_NOTE) {
        note = LOWEST_NOTE;
    }

    return note;
}





