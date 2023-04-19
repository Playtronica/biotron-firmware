#include <class/midi/midi_device.h>
#include <hardware/adc.h>
#include <stdlib.h>
#include "../include/notes.h"
#include "global.h"


#ifndef OCTAVE
#define OCTAVE 0
#endif


uint32_t lastNotePlant;
uint32_t lastNoteLight;
uint32_t getLastNotePlant() {
    return lastNotePlant;
}
uint32_t getLastNoteLight() {
    return lastNoteLight;
}


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
uint8_t getLengthOctave() {
    return scales[scale].steps;
}


uint32_t getNote(int percent) {
    NotesScale_t _scale = scales[scale];
    bool minus = percent < 0;
    if (minus) percent *= -1;

    uint32_t note = MIDDLE_NOTE;

    if (!minus) {
        int step = _scale.steps - 1;
        for (int i = 0; i < percent; i++) {
            if (step < 0) {
                step = _scale.steps - 1;
            }
            note -= _scale.scale[step];
            step--;

            if (note <= LOWEST_NOTE) {
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
                note = HIGHEST_NOTE;
                break;
            }
        }
    }

    return note;
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


double noteDistance = 0.5;
double firstValue = 0.1;
int GetNoteDiff(int oldVal, int newVal) {
    int diff = newVal - oldVal;
    bool minus = diff < 0;
    diff = abs(diff);

    double first = 0;
    double second = firstValue;

    int i;
    uint32_t noteChangeValue = getAvgFreqChanges();
    if (diff - noteChangeValue >= 0) {
        i = 1;
        diff -= noteChangeValue;
    } else return 0;

    double extra;
    while (diff - (noteChangeValue + noteDistance * (first + second)) >= 0) {
        diff -= noteChangeValue + noteDistance * (first + second);
        extra = first + second;
        first = second;
        second = extra;
        i++;
    }

    if (minus) {
        return -i;
    }
    return i;
}


uint8_t plantVelocity = 127;
void midi_plant(void) {
    static uint32_t currentNote;
    static uint32_t previousNote;
    uint8_t const cable_num = 0;
    uint8_t channel = 0;

    uint8_t packet[4];
    while (tud_midi_available()) tud_midi_packet_read(packet);

    previousNote = currentNote;

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

    currentNote = getNote(GetNoteDiff(getAvgFreq(), getFreq()));
    lastNotePlant = currentNote;
    uint8_t note_on[3] = {0x90 | channel, currentNote, plantVelocity};
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
}


uint8_t lightVelocity = 127;
void midi_light(void) {
    static uint32_t currentNote;
    static uint32_t previousNote;

    uint8_t const cable_num = 0;
    uint8_t channel = 1;

    uint8_t packet[4];
    while (tud_midi_available()) tud_midi_packet_read(packet);

    previousNote = currentNote;
    int a = MAX_OF_PHOTO / 24;
    currentNote = (MAX_OF_PHOTO - adc_read()) / a + 24;
    lastNoteLight = previousNote;

    uint8_t note_off[3] = {0x80 | channel, previousNote, 0};
    tud_midi_stream_write(cable_num, note_off, 3);

    uint8_t note_on[3] = {0x90 | channel, currentNote, lightVelocity};
    tud_midi_stream_write(cable_num, note_on, 3);
}


void midi_stop() {
    uint8_t const cable_num = 0;

    uint8_t packet[4];
    while (tud_midi_available()) tud_midi_packet_read(packet);

    uint8_t PlantNote[3] = {0x80 | 0, lastNotePlant, 0};
    tud_midi_stream_write(cable_num, PlantNote, 3);

    uint8_t LightNote[3] = {0x80 | 1, lastNoteLight, 0};
    tud_midi_stream_write(cable_num, LightNote, 3);
}


void midi_settings() {
    uint8_t buff[4];
    uint8_t res[100];
    uint8_t len = 0;
    while (tud_midi_packet_read(buff)) {
        for (int i = 1; i < 4; ++i) {
            if (buff[i] == 240) continue;
            if (buff[i] == 247) break;
            res[len++] = buff[i];
            printf("%d ", buff[i]);
        }
        printf("\n");
    }

    if (res[0] == 11) {
        double su = 0;
        switch (res[1]) {
            case (0):
                for (int i = 2; i < len; i++) {
                    su += res[i];;
                }
                setBPM((int) (1000000.0 / (su / 60.0)));
                printf("[!] BPM HAS CHANGED. BPM: %d, TIME: %d.\n", (int)su, (int)(1000000.0 / (su / 60.0)));
                break;
            case (1):
                if (len != 4) break;
                noteDistance = (double) res[2] / 100;
                firstValue = (double) res[3] / 100;
                if (noteDistance > 1) noteDistance = 1;
                if (firstValue > 1) firstValue = 1;
                printf("[!] FIBONACCI ALGORITHM HAS CHANGED. NOTE DISTANCE: %.2f, FIRST_VALUE: %.2f.\n",
                       noteDistance, firstValue);
                break;
            case (2):
                setFilterPercent((double) (res[2]) / 100);
                printf("[!] FILTER VALUE HAS CHANGED. FILTER VALUE: %.2f.\n", (double) (res[2]) / 100);
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
            case(5):
                if (res[2] == 1) {
                    plantVelocity = res[3];
                    printf("[!] Plant Velocity has been changed. Velocity: %d\n", plantVelocity);
                } else if (res[2] == 2) {
                    lightVelocity = res[3];
                    printf("[!] Light Velocity has been changed. Velocity: %d\n", lightVelocity);
                }
                break;
        }
    }
    else if (res[0] == 176) {
        switch (res[1]) {
            case (3):
                setFilterPercent((double)(res[2]) / 127);
                printf("[!] FILTER VALUE HAS CHANGED. FILTER VALUE: %.2f.\n", (double)(res[2]) / 127);
                break;
            case(9):
                plantVelocity = res[2];
                printf("[!] Plant Velocity has been changed. Velocity: %d\n", plantVelocity);
                break;
            case(120):
                midi_stop();
                printf("[!] All NOTES OFF\n");
                break;
        }
    }
    else if (res[0] == 177) {
        switch (res[1]) {
            case(9):
                lightVelocity = res[2];
                printf("[!] Light Velocity has been changed. Velocity: %d\n", lightVelocity);
                break;
            case(120):
                midi_stop();
                printf("[!] All NOTES OFF\n");
                break;
        }
    }
}




