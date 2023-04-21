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

/** @brief Octave description
 *
 * Every array describe how to play notes in current octave.
 * For example: If octave is major, the third note it is MIDDLE_NOTE + (2 + 2 + 1)
 *
 */
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

/** @brief Length of every octave description */
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

/** @brief Function to find distance from average and current + (Fibonacci algorithm)
 *
 * Find difference from frequencies, and divide it on Average Frequency Changes to find how to need to change note.
 * If Fibonacci algorithm is on, Average Frequency Changes will be multiplied on gradually increasing coefficient
 *
 * @var
 * fibPower - show how powerful would be a algorithm algorithm (must be lower then 1)
 * firstValue - first value for fibonacci sequence
 *
 * @return Distance to change note
 */
double fibPower = 0.5;
double firstValue = 0.1;
int GetFrequencyDiff() {
    uint32_t oldVal = getAvgFreq();
    uint32_t newVal = getFreq();
    int diff = (int)newVal - (int)oldVal;
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
    while (diff - (noteChangeValue + fibPower * (first + second)) >= 0) {
        diff -= noteChangeValue + fibPower * (first + second);
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



/** @brief Function to calculate note
 *
 * Get frequency difference and from Octave description calculate note
 *
 * @return Current note
 */
uint32_t GetNote() {
    int counter = GetFrequencyDiff();
    NotesScale_t _scale = scales[scale];
    bool minus = counter < 0;
    if (minus) counter *= -1;

    uint32_t note = MIDDLE_NOTE;

    if (!minus) {
        int step = _scale.steps - 1;
        for (int i = 0; i < counter; i++) {
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
        for (int i = 0; i < counter; i++) {
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


uint8_t plantVelocity = 127;
void MidiPlant(void) {
    uint8_t const cable_num = 0;
    uint8_t channel = 0;

    uint8_t packet[4];
    while (tud_midi_available()) tud_midi_packet_read(packet);

    uint8_t note_off[3] = {0x80 | channel, lastNotePlant, 0};
    tud_midi_stream_write(cable_num, note_off, 3);

    uint8_t currentNote = GetNote();
    uint8_t note_on[3] = {0x90 | channel, currentNote, plantVelocity};
    tud_midi_stream_write(cable_num, note_on, 3);

    lastNotePlant = currentNote;
}


uint8_t lightVelocity = 127;
void MidiLight(void) {
    uint8_t const cable_num = 0;
    uint8_t channel = 1;

    uint8_t packet[4];
    while (tud_midi_available()) tud_midi_packet_read(packet);

    uint8_t note_off[3] = {0x80 | channel, lastNoteLight, 0};
    tud_midi_stream_write(cable_num, note_off, 3);

    uint8_t currentNote = (MAX_OF_LIGHT - adc_read()) / (MAX_OF_LIGHT / 24) + 24;

    uint8_t note_on[3] = {0x90 | channel, currentNote, lightVelocity};
    tud_midi_stream_write(cable_num, note_on, 3);

    lastNoteLight = currentNote;
}


void MidiStop() {
    uint8_t const cable_num = 0;

    uint8_t packet[4];
    while (tud_midi_available()) tud_midi_packet_read(packet);

    uint8_t plantNote[3] = {0x80 | 0, lastNotePlant, 0};
    tud_midi_stream_write(cable_num, plantNote, 3);

    uint8_t lightNote[3] = {0x80 | 1, lastNoteLight, 0};
    tud_midi_stream_write(cable_num, lightNote, 3);
}


void MidiSettings() {
    uint8_t buff[4];
    uint8_t res[100];
    uint8_t len = 0;
    /** @brief Read bytes
     *
     *
     * @paragraph 240 - it is F0 (Start byte),
     * @paragraph 247 - it is F7 (End byte)
     */
    while (tud_midi_packet_read(buff)) {
        for (int i = 1; i < 4; ++i) {
            if (buff[i] == 240) continue;
            if (buff[i] == 247) break;
            res[len++] = buff[i];
            printf("%d ", buff[i]);
        }
        printf("\n");
    }


    /** @brief Byte Commands
     *
     * res[0] == 11 is the key to remove trash commands (11 = B) */
    if (res[0] == 11) {
        double su = 0;
        switch (res[1]) {
            /** @brief F0 B 0 x y ... F7 - BPM command
             *
             * @param x,y,.. - Bpm (Every value can contain maximum FF, so takes sum of all variables)
             * */
            case (0):
                for (int i = 2; i < len; i++) {
                    su += res[i];;
                }
                setBPM((int) (1000000.0 / (su / 60.0)));
                printf("[!] BPM HAS CHANGED. BPM: %d, TIME: %d.\n", (int)su, (int)(1000000.0 / (su / 60.0)));
                break;
            /** @brief
             * F0 B 1 x y F7 - Fibonacci command
             *
             * (x and y values divide by 100)
             *
             * @param x - fibPower (Max = 1)
             * @param y - firstValue (Max = 1)
             *
             *
             */
            case (1):
                if (len != 4) break;
                fibPower = (double) res[2] / 100;
                firstValue = (double) res[3] / 100;
                if (fibPower > 1) fibPower = 1;
                if (firstValue > 1) firstValue = 1;
                printf("[!] FIBONACCI ALGORITHM HAS CHANGED. NOTE DISTANCE: %.2f, FIRST_VALUE: %.2f.\n",
                       fibPower, firstValue);
                break;
            /** @brief F0 B 2 x F7 - Filter in active state command
             *
             * Makes playing notes more smooth
             * (x divide by 100)
             *
             * @param x - filter power (Max = 0,99)
             * */
            case (2):
                setFilterPercent((double) (res[2]) / 100);
                printf("[!] FILTER VALUE HAS CHANGED. FILTER VALUE: %.2f.\n", (double) (res[2]) / 100);
                break;
            /** @brief F0 B 2 x F7 - Choose octave command
             *
             * @param x - num of octave
             */
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
            /** @brief F0 B 1 x y F7 - Change velocity command
             *
             * @param x - if equals 1, change plant midi velocity, else if equals = 2 change light midi velocity
             * @param y - velocity (Max = 127)
             */
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
    /** @brief CC commands (plant midi) */
    else if (res[0] == 176) {
        switch (res[1]) {
            /** @brief CC3 x - Filter in active state command
             *
             * Makes playing notes more smooth
             * (x divide by 100)
             *
             * @param x - filter power
             */
            case (3):
                setFilterPercent((double)(res[2]) / 127);
                printf("[!] FILTER VALUE HAS CHANGED. FILTER VALUE: %.2f.\n", (double)(res[2]) / 127);
                break;
            /** @brief CC9 x - Change velocity command
            *
            * @param x - velocity (Max = 127)
             */
            case(9):
                plantVelocity = res[2];
                printf("[!] Plant Velocity has been changed. Velocity: %d\n", plantVelocity);
                break;
            /** @brief CC120 - Stop all notes */
            case(120):
                MidiStop();
                printf("[!] All NOTES OFF\n");
                break;
        }
    }
    /** @brief CC commands (light midi) */
    else if (res[0] == 177) {
        switch (res[1]) {
            /** @brief CC9 x - Change velocity command
            *
            * @param x - velocity (Max = 127)
             */
            case(9):
                lightVelocity = res[2];
                printf("[!] Light Velocity has been changed. Velocity: %d\n", lightVelocity);
                break;
            /** @brief CC120 - Stop all notes */
            case(120):
                MidiStop();
                printf("[!] All NOTES OFF\n");
                break;
        }
    }
}




