#include <malloc.h>
#include <class/midi/midi_device.h>
#include <hardware/adc.h>
#include <stdlib.h>
#include "../include/notes.h"
#include "global.h"


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

typedef struct Note {
    uint8_t note;
    uint8_t velocity;
} Note;

typedef struct Vector {
    Note *notes;
    int size;
    int capacity;
} Vector;

struct Vector vecPlant;
struct Vector vecLight;


typedef struct MidiFilter {
    void (*noteOn)(struct Vector* notes, struct MidiFilter filterInfo);
    bool active;
    int * values;
    uint8_t size;
} MidiFilter;

struct Vector create_vector(int n) {
    struct Vector vec;

    vec.size = 0;
    vec.capacity = n;
    vec.notes = malloc(sizeof(Note) * n);

    return vec;
}

void free_vector(struct Vector* vec) {
    vec->size = 0;
    vec->capacity = 0;
    free(vec->notes);
}

void insert_note(struct Vector* vec, struct Note note) {
    for (int i = 0; i < vec->size; i++) {
        if (vec->notes[i].note == note.note) {
            vec->notes[i].velocity = note.velocity;
            return;
        }
    }
    vec->notes[vec->size++] = note;
}

void ChordFilter(struct Vector* notes, struct MidiFilter filterInfo) {
    int newNotes[10];
    uint8_t countNotes;

    if (filterInfo.size == 0) {
        countNotes = 2;
        newNotes[0] = 3;
        newNotes[1] = 7;
    } else {
        countNotes = filterInfo.size;
        for (int i = 0; i < countNotes; i++) {
            newNotes[i] = filterInfo.values[i];
        }
    }
    
    int len = notes->size;
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < countNotes; j++) {
            struct Note newNote = {
                    notes->notes[i].note + newNotes[j],
                    127
            };
            insert_note(notes, newNote);
        }
    }
}

void VelocityRandomizerFilter(struct Vector* notes, struct MidiFilter filterInfo) {
    int len = notes->size;
    for (int i = 0; i < len; i++) {
        notes->notes[i].velocity = rand() % 127;
    }
}

MidiFilter plantFilters[10];

MidiFilter lightFilters[10];

void init_filters() {
    struct MidiFilter filter;
    filter.noteOn = &ChordFilter;
    filter.active = true;
    filter.values = malloc(sizeof(Note) * 10);
    filter.size = 0;
    plantFilters[0] = filter;

    filter.noteOn = &VelocityRandomizerFilter;
    plantFilters[1] = filter;
}

uint32_t getNote(int percent, int note) {
    NotesScale_t _scale = scales[scale];
    bool minus = percent < 0;
    if (minus) percent *= -1;


    if (!minus) {
        int step = _scale.steps - 1;

        for (int i = 0; i < percent; i++) {
            if (step < 0) {
                step = _scale.steps - 1;
            }

            note -= _scale.scale[step];
            step--;

            if (note <= LOWEST_NOTE) {
//                note = -1;  TOUCH MODE
                note = LOWEST_NOTE;
                break;
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
//                note = -1;  TOUCH MODE
                note = HIGHEST_NOTE;
                break;
            }
        }
    }

    return note;
}


void midi_plant(void) {
    uint8_t const cable_num = 0;
    uint8_t channel = 0;

    // TODO Maybe replace printf
    printf("{\"AverageFreq\": %d, \"Freq\": %d, \"PlantNote\": %d, \"LightNote\": %d }\n",
           averageFreq, realFrequency, lastNotePlant, lastNoteLight);

    if (vecPlant.size != 0) {
        for (int i = 0; i < vecPlant.size; i++) {
            uint8_t note_off[3] = {0x80 | channel, vecPlant.notes[i].note,
                                   0};
            tud_midi_stream_write(cable_num, note_off, 3);
        }
        free_vector(&vecPlant);
    }

    uint8_t packet[4];
    while (tud_midi_available()) tud_midi_packet_read(packet);

    uint8_t note =  getNote(GetNoteDiff(averageFreq, realFrequency), MIDDLE_NOTE);
    lastNotePlant = note;
    vecPlant = create_vector(100);

    struct Note firstNote = {note, 127};
    insert_note(&vecPlant, firstNote);

    for (int i = 0; i < 10; ++i) {
        if (plantFilters[i].active) {
            plantFilters[i].noteOn(&vecPlant, plantFilters[i]);
        }
    }

    for (int i = 0; i < vecPlant.size; i++) {
        uint8_t note_on[3] = {0x90 | channel, vecPlant.notes[i].note, vecPlant.notes[i].velocity};
        tud_midi_stream_write(cable_num, note_on, 3);
    }
}


void midi_light(void) {
    uint8_t const cable_num = 0;
    uint8_t channel = 1;

    if (vecLight.size != 0) {
        for (int i = 0; i < vecLight.size; i++) {
            uint8_t note_off[3] = {0x80 | channel, vecLight.notes[i].note, 0};
            tud_midi_stream_write(cable_num, note_off, 3);
        }
        free_vector(&vecLight);
    }

    uint8_t packet[4];
    while (tud_midi_available()) tud_midi_packet_read(packet);

    int a = MAX_OF_PHOTO / 24;
    uint8_t note = (MAX_OF_PHOTO - adc_read()) / a + 24;
    lastNoteLight = note;

    vecLight = create_vector(100);
    struct Note firstNote = {note, 127};
    insert_note(&vecLight, firstNote);

    for (int i = 0; i < 10; ++i) {
        if (lightFilters[i].active) {
            lightFilters[i].noteOn(&vecLight, lightFilters[i]);
        }
    }

    for (int i = 0; i < vecLight.size; i++) {
        uint8_t note_on[3] = {0x90 | channel, vecLight.notes[i].note, vecLight.notes[i].velocity};
        tud_midi_stream_write(cable_num, note_on, 3);
    }
}


void midi_stop() {
    uint8_t const cable_num = 0;

    uint8_t packet[4];
    while (tud_midi_available()) tud_midi_packet_read(packet);

    for (int i = 0; i < vecPlant.size; i++) {
        uint8_t note_off[3] = {0x80 | 0, vecPlant.notes[i].note, 0};
        tud_midi_stream_write(cable_num, note_off, 3);
    }
    free_vector(&vecPlant);

    for (int i = 0; i < vecLight.size; i++) {
        uint8_t note_off[3] = {0x80 | 1, vecLight.notes[i].note, 0};
        tud_midi_stream_write(cable_num, note_off, 3);
    }
    free_vector(&vecLight);
}


void midi_settings() {
    uint8_t buff[4];
    uint8_t res[100];
    uint8_t len = 0;

    while (tud_midi_packet_read(buff)) {
        for (int i = 1; i < 4; ++i) {
            printf("%d ", buff[i]);
            if (buff[i] == 240) continue;
            if (buff[i] == 247) break;
            res[len++] = buff[i];
        }
    }

    if (len == 0) return;

    printf("\n");

    if (res[0] == 11) {
        double su = 0;
        switch (res[1]) {
            case (0):
                for (int i = 2; i < len; i++) {
                    su += res[i];
                }
                timer_notes = (int) (1000000.0 / (su / 60.0));
                if (status == Active) {
                    cancel_repeating_timer(&timer);
                    add_repeating_timer_us((int) (1000000.0 / (su / 60.0)),
                                           playNote, NULL, &timer);
                }
                printf("[!] BPM HAS CHANGED. BPM: %d, TIME: %d.\n", (int) su, (int) timer_notes);
                break;
            case (1):
                if (len != 4) break;
                NOTE_DISTANCE = (double) res[2] / 100;
                FIRST_VALUE = (double) res[3] / 100;
                if (NOTE_DISTANCE > 1) NOTE_DISTANCE = 1;
                if (FIRST_VALUE > 1) FIRST_VALUE = 1;
                printf("[!] FIBONACCI ALGORITHM HAS CHANGED. NOTE DISTANCE: %.2f, FIRST_VALUE: %.2f.\n",
                       NOTE_DISTANCE, FIRST_VALUE);
                break;
            case (2):
                filterPercent = (double) (res[2]) / 100;
                if (filterPercent >= 1) filterPercent = 0.99;
                printf("[!] FILTER VALUE HAS CHANGED. FILTER VALUE: %.2f.\n", filterPercent);
                filterPercent = 1 - filterPercent;

                break;
            case (3):
                if (res[2] >= 0 && res[2] < 12) {
                    scale = res[2];
                    char *octaveName = "";
                    switch (scale) {
                        case SCALE_CHROM:
                            octaveName = "CHROM";
                            break;
                        case SCALE_DIMINISHED:
                            octaveName = "DIMINISHED";
                            break;
                        case SCALE_DORIAN:
                            octaveName = "DORIAN";
                            break;
                        case SCALE_LYDIAN:
                            octaveName = "LYDIAN";
                            break;
                        case SCALE_MAJBLUES:
                            octaveName = "MAJBLUES";
                            break;
                        case SCALE_MAJOR:
                            octaveName = "MAJOR";
                            break;
                        case SCALE_MAJPEN:
                            octaveName = "MAJPEN";
                            break;
                        case SCALE_MINBLUES:
                            octaveName = "MINBLUES";
                            break;
                        case SCALE_MINOR:
                            octaveName = "MINOR";
                            break;
                        case SCALE_MINPEN:
                            octaveName = "MINPEN";
                            break;
                        case SCALE_MIXOLYDIAN:
                            octaveName = "MIXOLYDIAN";
                            break;
                        case SCALE_WHOLETONE:
                            octaveName = "WHOLETONE";
                            break;
                    }
                    printf("[!] OCTAVE HAS CHANGED. CURRENT OCTAVE IS %s.\n", octaveName);
                }
                break;
            case(4):
                if (len >= 5) {
                    uint8_t size = len - 5;
                    if (size > 10) break;

                    struct MidiFilter filter;
                    filter.noteOn = &ChordFilter;
                    filter.active = true;
                    filter.values = malloc(sizeof(Note) * 10);
                    filter.size = 0;

                    if (size != 0) {
                        printf("Params: ");
                        for (int i = 5; i < len; i++) {
                            filter.values[filter.size++] = res[i] - 60;
                            printf("%d ", res[i] - 60);
                        }
                        printf("\n");
                    }



                    switch (res[3]) {
                        case (0):
                            if (res[2] == 1) {
                                plantFilters[res[4]].active = false;
                                printf("[!] %d MIDI FILTER ON 1 CHANNEL HAS BEEN DISABLED\n", res[3]);
                            }
                            if (res[2] == 2) {
                                lightFilters[res[4]].active = false;
                                printf("[!] %d MIDI FILTER ON 2 CHANNEL HAS BEEN DISABLED\n", res[3]);
                            }
                            break;
                        case (1):
                            filter.noteOn = &ChordFilter;
                            if (res[2] == 1) {
                                plantFilters[res[4]] = filter;
                                printf("[!] %d MIDI FILTER ON 1 CHANNEL HAS BEEN CHANGED. IT IS CHORD FILTER."
                                       " PARAMS COUNT: %d\n", res[4], size);
                            }
                            if (res[2] == 2) {
                                lightFilters[res[4]] = filter;
                                printf("[!] %d MIDI FILTER ON 2 CHANNEL HAS BEEN CHANGED. IT IS CHORD FILTER."
                                       " PARAMS COUNT: %d\n", res[4], size);
                            }
                            break;
                        case (2):
                            filter.noteOn = &VelocityRandomizerFilter;
                            if (res[2] == 1) {
                                plantFilters[res[4]] = filter;
                                printf("[!] %d MIDI FILTER ON 1 CHANNEL HAS BEEN CHANGED. IT IS VELOCITY FILTER."
                                       " PARAMS COUNT: %d\n", res[4], size);
                            }
                            if (res[2] == 2) {
                                lightFilters[res[4]] = filter;
                                printf("[!] %d MIDI FILTER ON 2 CHANNEL HAS BEEN CHANGED. IT IS VELOCITY FILTER."
                                       " PARAMS COUNT: %d\n", res[4], size);
                            }
                            break;
                    }
                }
                break;
        }
    }
    else if (res[0] == 176) {
        switch (res[1]) {
            case (3):
                filterPercent = (double) (res[2]) / 127;
                if (filterPercent >= 1) filterPercent = 0.99;
                printf("[!] FILTER VALUE HAS CHANGED. FILTER VALUE: %.2f.\n", filterPercent);
                filterPercent = 1 - filterPercent;
                break;
            case(120):
                midi_stop();
                printf("[!] All NOTES OFF\n");
                break;
        }
    }
}



//uint8_t getLengthOctave() {
//    return scales[scale].steps;
//}
//
//uint32_t* getOctaveNotes() {
//    NotesScale_t _scale = scales[scale];
//    uint32_t* notes = malloc(_scale.steps * sizeof(uint32_t));
//    uint8_t note = MIDDLE_NOTE;
//    for (int i = 0; i < _scale.steps; i++) {
//        notes[i] = note;
//        note += _scale.scale[i];
//    }
//    return notes;
//}

