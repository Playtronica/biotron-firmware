#include <malloc.h>
#include <class/midi/midi_device.h>
#include <hardware/adc.h>
#include "../include/notes.h"
#include "global.h"

#define LOWEST_NOTE 36
#define HIGHEST_NOTE 107
#define MIDDLE_NOTE 60

#ifndef OCTAVE
#define OCTAVE 0
#endif

typedef struct {
    int steps;
    const uint8_t * scale;
} NotesScale_t;


const uint8_t major[] = { 2, 2, 1, 2, 2, 2, 1 };
const uint8_t minor[] = { 2, 1, 2, 2, 1, 2, 2 };
const uint8_t chrom[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1 };
const uint8_t dorian[] = { 2, 1, 2, 2, 2, 1, 2 };
const uint8_t mixolydian[] = { 2, 2, 1, 2, 2, 1, 2 };
const uint8_t lydian[] = { 2, 2, 1, 1, 1, 2, 2, 1 };
const uint8_t wholetone[] = { 2, 2, 2, 2, 2, 2 };
const uint8_t minblues[] = { 3, 2, 1, 1, 3, 2 };
const uint8_t majblues[] = { 2, 1, 1, 3, 2, 3 };
const uint8_t minpen[] = { 3, 2, 2, 3, 2 };
const uint8_t majpen[] = { 2, 2, 3, 2, 3 };
const uint8_t diminished[] = { 2, 1, 2, 1, 2, 1, 2, 1 };

uint32_t getCountNote(){
    return HIGHEST_NOTE - LOWEST_NOTE;
}

const NotesScale_t scales[] = {
        {7, major},
        {7, minor},
        {12, chrom},
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

ScaleNums_t scale = OCTAVE;


uint32_t getNote(int percent) {
    NotesScale_t _scale = scales[scale];
    bool minus = percent < 0;
    printf("LST %hhu\n", _scale.scale[0]);
    if (minus) percent *= -1;

    uint32_t note = MIDDLE_NOTE;

    if (!minus) {
        int step = _scale.steps - 1;

        for (int i = 0; i < percent; i++) {
            if (step < 0) {
                step = _scale.steps - 1;
            }
            printf("I AM NOTE %lu\n", note);

            note -= _scale.scale[step];
            printf("I AM NOTE %lu\n", note);
            step--;

            if (note <= LOWEST_NOTE) {
                note = -1;
            }
        }
    }
    else {
        int step = 0;
        for (int i = 0; i < percent; i++) {
            if (step >= _scale.steps) {
                step = 0;
            }
            note += _scale.scale[step];

            step++;

            if (note >= HIGHEST_NOTE) {
                note = -1;
                break;
            }
        }
    }

    return note;
}


uint8_t getLengthOctave() {
    return scales[scale].steps;
}

uint32_t* getOctaveNotes() {
    NotesScale_t _scale = scales[scale];
    uint32_t* notes = malloc(_scale.steps * sizeof(uint32_t));
    uint8_t note = MIDDLE_NOTE;
    for (int i = 0; i < _scale.steps; i++) {
        notes[i] = note;
        note += _scale.scale[i];
    }
    return notes;
}


bool touch;
void midi_plant(void) {
    static uint32_t currentNote;
    static uint32_t previousNote;
    uint8_t const cable_num = 0;
    uint8_t channel = 1;

    uint8_t packet[4];
    while (tud_midi_available()) tud_midi_packet_read(packet);

    previousNote = currentNote;

    currentNote = getNote(GetNoteDiff(averageFreq, realFrequency));

    uint8_t note_on[3] = {0x90 | channel, currentNote, 127};
    if (currentNote != -1) {
        tud_midi_stream_write(cable_num, note_on, 3);
    }
    else {
        if (previousNote != -1) {
            uint32_t *notes = getOctaveNotes();
            for (int i = 0; i < getLengthOctave(); i++) {
                note_on[1] = notes[i];
                tud_midi_stream_write(cable_num, note_on, 3);
            }
        }
    }

    uint8_t note_off[3] = {0x80 | channel, previousNote, 0};
    if (previousNote != -1) {
        tud_midi_stream_write(cable_num, note_off, 3);
    }
    else {
        if (currentNote != -1) {
            uint32_t *notes = getOctaveNotes();
            for (int i = 0; i < getLengthOctave(); i++) {
                note_off[1] = notes[i];
                tud_midi_stream_write(cable_num, note_off, 3);
            }
        }
    }
}


void midi_light(void) {
    static uint32_t currentNote;
    static uint32_t previousNote;

    uint8_t const cable_num = 0;
    uint8_t channel = 2;

    uint8_t packet[4];
    while (tud_midi_available()) tud_midi_packet_read(packet);

    previousNote = currentNote;
    int a = 4000 / 24;
    currentNote = (4000 - adc_read()) / a + 24;

    uint8_t note_on[3] = {0x90 | channel, currentNote, 127};
    tud_midi_stream_write(cable_num, note_on, 3);

    uint8_t note_off[3] = {0x80 | channel, previousNote, 0};
    tud_midi_stream_write(cable_num, note_off, 3);
}






