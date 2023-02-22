//
// Created by user on 17.02.23.
//

#include "pico/stdlib.h"
#include "global.h"
#include "../include/chord.h"
#include <class/midi/midi_device.h>
#include <malloc.h>

typedef struct Note {
    uint8_t note;
    uint8_t velocity;
} Note;


struct Vector {
    Note *notes;
    int size;
    int capacity;
};

typedef struct MidiFilter {
    void (*noteOn)(struct Vector* notes);
    void (*noteOff)(struct Vector* notes);
    bool active;
} MidiFilter;



struct Vector create_vector(int n) {
    struct Vector vec;

    vec.size = 0;
    vec.capacity = n;
    vec.notes = malloc(sizeof(int)*n);

    return vec;
}

void insert_shift_right(struct Vector* vec, struct Note note) {
    for (int i = vec->size; i > 0; i--) {
        if (vec->notes[i].note == note.note) {
            vec->notes[i].velocity = note.velocity;
            return;
        }
    }
    vec->notes[vec->size++] = note;
}


MidiFilter plantFilters[] = {
        {NULL, NULL, false},
        {NULL, NULL, false},
        {NULL, NULL, false},
        {NULL, NULL, false},
        {NULL, NULL, false},
        {NULL, NULL, false},
        {NULL, NULL, false},
        {NULL, NULL, false},
        {NULL, NULL, false},
        {NULL, NULL, false},
};

MidiFilter lightFilters[] = {
        {NULL, NULL, false},
        {NULL, NULL, false},
        {NULL, NULL, false},
        {NULL, NULL, false},
        {NULL, NULL, false},
        {NULL, NULL, false},
        {NULL, NULL, false},
        {NULL, NULL, false},
        {NULL, NULL, false},
        {NULL, NULL, false},
};


MidiFilter filters[] = {

};


void Play(uint8_t note, uint8_t channel, uint8_t cable_num) {
    struct Vector vec = create_vector(100);
    struct Note firstNote = {note, 127};
    insert_shift_right(&vec, firstNote);

    for (int i = 0; i < 10; ++i) {
        if (channel == 0) {
            if (plantFilters[i].active) {
                plantFilters->noteOn(&vec);
            }
        }
        else if (channel == 1) {
            if (lightFilters[i].active) {
                lightFilters->noteOn(&vec);
            }
        }
    }

    for (int i = 0; i < vec.size; i++) {
        uint8_t note_off[3] = {0x90 | channel, vec.notes[i].note, vec.notes[i].velocity};
        tud_midi_stream_write(cable_num, note_off, 3);
    }
}

void Stop(uint8_t note, uint8_t channel, uint8_t cable_num) {
    struct Vector vec = create_vector(100);
    struct Note firstNote = {note, 0};
    insert_shift_right(&vec, firstNote);

    for (int i = 0; i < 10; ++i) {
        if (channel == 0) {
            if (plantFilters[i].active) {
                plantFilters->noteOff(&vec);
            }
        }
        else if (channel == 1) {
            if (lightFilters[i].active) {
                lightFilters->noteOff(&vec);
            }
        }
    }

    for (int i = 0; i < vec.size; i++) {
        uint8_t note_off[3] = {0x80 | channel, vec.notes[i].note, 0};
        tud_midi_stream_write(cable_num, note_off, 3);
    }


}
