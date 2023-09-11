#include <class/midi/midi_device.h>
#include <hardware/adc.h>
#include <stdlib.h>
#include <hardware/sync.h>
#include "../include/notes.h"
#include "global.h"
#include "frequency_counter.h"
#include "settings.h"


uint32_t lastNotePlant = MIDDLE_NOTE;
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
 * Every array describe how to play notes in current scale.
 * For example: If scale is major, the third note it is MIDDLE_NOTE + (2 + 2 + 1)
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

/** @brief Length of every scale description */
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

int GetFrequencyDiff() {
    uint32_t oldVal = getAvgFreq();
    uint32_t newVal = getFreq();
    int diff = (int)newVal - (int)oldVal;
    bool minus = diff < 0;
    diff = abs(diff);

    double first = 0;
    double second = getFirstValue();

    int i;
    uint32_t noteChangeValue = getAvgFreqChanges();
    if (diff - noteChangeValue >= 0) {
        i = 1;
        diff -= noteChangeValue;
    } else return 0;

    double extra;
    while (diff - (noteChangeValue + getFibPower() * (first + second)) >= 0) {
        diff -= noteChangeValue + getFibPower() * (first + second);
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
    NotesScale_t _scale = scales[getScale()];
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


bool sameNotePlay = true;

bool get_control_same_note() {
    return sameNotePlay;
}

void control_same_note(bool flag) {
    sameNotePlay = flag;
    SaveSettings();
}

struct repeating_timer plantNoteOffTimer;
bool MidiPlantNoteOff(struct repeating_timer *t) {
    uint8_t note_off[3] = {0x80 | 0, lastNotePlant, 0};
    tud_midi_stream_write(0, note_off, 3);
    cancel_repeating_timer(&plantNoteOffTimer);
    return true;
}


void resetPlantNoteOff() {
    cancel_repeating_timer(&plantNoteOffTimer);
}

uint8_t note_off_speed_percent = 100;
void set_note_off_speed_percent(uint8_t val) {
    val = val < 100 ? val : 100;
    note_off_speed_percent = val;
    SaveSettings();
}

uint8_t get_note_off_speed_percent() {
    return note_off_speed_percent;
}

static uint16_t clk = 0;
static uint64_t last_timer = 0;
static uint64_t last_diff = 0;
void MidiClock() {
    clk++;
    if (clk != 25) return;
    clk = 1;
    if (get_status() != BPMClockActive) return;
    last_diff = time_us_64() - last_timer;
    last_timer = time_us_64();
    play_music();
}


uint8_t getCC(uint8_t currentNote) {
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

void MidiPlant(void) {

    uint8_t const cable_num = 0;
    uint8_t channel = 0;

    uint8_t currentNote = GetNote();
    if (!get_mute_mode()) {
        if (currentNote == lastNotePlant && !sameNotePlay) {
            return;
        }

        if (get_status() == BPMClockActive) {
            uint8_t note_off[3] = {0x80 | 0, lastNotePlant, 0};
            tud_midi_stream_write(0, note_off, 3);
        }

        uint8_t note_on[3] = {0x90 | channel, currentNote, getPlantVelocity()};
        tud_midi_stream_write(cable_num, note_on, 3);


        if (get_status() == Active) {
            add_repeating_timer_us(getBPM() / 100 * get_note_off_speed_percent(), MidiPlantNoteOff,
                                   NULL, &plantNoteOffTimer);
        }
    }
//    printf("\nNote perc %d %d\n", last_diff, get_note_off_speed_percent());

    uint8_t note_cc[3] = {0xB0 | channel, 90, getCC(currentNote)};
    tud_midi_stream_write(cable_num, note_cc, 3);

    lastNotePlant = currentNote;
}


void MidiLight(void) {
    uint8_t const cable_num = 0;
    uint8_t channel = 1;

    uint8_t currentNote = (MAX_OF_LIGHT - adc_read()) / (MAX_OF_LIGHT / 24) + 24;

    if (!get_mute_mode()) {
        uint8_t note_off[3] = {0x80 | channel, lastNoteLight, 0};
        tud_midi_stream_write(cable_num, note_off, 3);


        uint8_t note_on[3] = {0x90 | channel, currentNote, getLightVelocity()};
        tud_midi_stream_write(cable_num, note_on, 3);
    }
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
    static uint8_t last_byte = 0;
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
            if (buff[i] == 250) {
                last_byte = 250;
                break;
            }
            if (buff[i] == 252) {
                BPM_clock_disable();
                cancel_repeating_timer(&plantNoteOffTimer);
                last_byte = 252;
                break;
            }
            if (buff[i] == 248) {
                if (last_byte == 250) {
                    BPM_clock_active();
                    last_timer = time_us_64();
                    cancel_repeating_timer(&plantNoteOffTimer);
                    clk = 0;
                }
                MidiClock();
                last_byte = 248;
                break;
            }
            res[len++] = buff[i];

//            printf("%d ", buff[i]);
        }
//        printf("\n");
    }


    if (len == 0) {
        return;
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
                setFreqPower((double) res[2] / 100, (double) res[3] / 100);
                printf("[!] FIBONACCI ALGORITHM HAS CHANGED. NOTE DISTANCE: %.2f, FIRST_VALUE: %.2f.\n",
                       getFibPower(), getFirstValue());
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
                printf("[!] FILTER VALUE HAS CHANGED. FILTER VALUE: %.2f.\n", getFilterPercent());
                break;
                /** @brief F0 B 2 x F7 - Choose scale command
                 *
                 * @param x - num of scale
                 */
            case (3):
                if (res[2] >= 0 && res[2] < 12) {
                    setScale(res[2]);
                    char *octaveName = "";
                    switch (getScale()) {
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
                    printf("[!] SCALE HAS CHANGED. CURRENT SCALE IS %s.\n", octaveName);
                }
                break;
                /** @brief F0 B 1 x y F7 - Change velocity command
                 *
                 * @param x - if equals 1, change plant midi velocity, else if equals = 2 change light midi velocity
                 * @param y - velocity (Max = 127)
                 */
            case(4):
                if (res[2] == 1) {
                    setPlantVelocity(res[3]);
                    printf("[!] Plant Velocity has been changed. Velocity: %d\n", getPlantVelocity());
                } else if (res[2] == 2) {
                    setLightVelocity(res[3]);
                    printf("[!] Light Velocity has been changed. Velocity: %d\n", getLightVelocity());
                }
                break;
            case (5):
                setBPM(TIMER_MIDI_US);
                setFreqPower(DEF_FIB_POW, DEF_FIB_FIRST);
                setFilterPercent(DEF_FILTER_PERCENT);
                setScale(SCALE);
                setPlantVelocity(127);
                setLightVelocity(127);
                setLightBPM(LIGHT_BPM_DEF);
                enable_random_note(true);
                control_same_note(true);
                set_note_off_speed_percent(100);
                printf("[!] Return default settings\n");
                break;
            case (6):
                PrintInfo();
                break;
            case(7):
                if (res[2] > 0) {
                    setLightBPM(res[2]);
                    printf("[!] Light BPM has been changed. Every %d Plant Note\n", res[2]);
                }
                break;
            case(8):
                if (res[2] != 0) {
                    enable_random_note(true);
                    printf("[!] Random Notes Active\n");
                } else {
                    enable_random_note(false);
                    printf("[!] Random Notes Inactive\n");
                }
                break;
            case(9):
                if (res[2] != 0) {
                    control_same_note(true);
                    printf("[!] Same Notes Active\n");
                } else {
                    control_same_note(false);
                    printf("[!] Same Notes Inactive\n");
                }
                break;
            case(10):
                set_note_off_speed_percent(res[2]);
                printf("[!] Note Off time has changed. %d\n", get_note_off_speed_percent());
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
                setPlantVelocity(res[2]);
                printf("[!] Plant Velocity has been changed. Velocity: %d\n", getPlantVelocity());
                break;
            case(14):
                setBPM((int) (1000000.0 / ((res[2] * 5) / 60.0)));
                printf("[!] BPM HAS CHANGED. BPM: %d, TIME: %d.\n", (int)res[2] * 5,
                       (int)(1000000.0 / ((res[2] * 10) / 60.0)));
                break;
            case(15):
                if (res[2] >= 63) {
                    enable_random_note(true);
                    printf("[!] Random Notes Active\n");
                } else {
                    enable_random_note(false);
                    printf("[!] Random Notes Inactive\n");
                }
                break;
            case(20):
                if (res[2] >= 63) {
                    control_same_note(true);
                    printf("[!] Same Notes Active\n");
                } else {
                    control_same_note(false);
                    printf("[!] Same Notes Inactive\n");
                }
                break;
            case(21):
                set_note_off_speed_percent(res[2] / 127 * 100);
                printf("[!] Note Off time has changed. %d\n", get_note_off_speed_percent());
                break;
                /** @brief CC120 - Stop all notes */
            case(22):
                setFreqPower((double )res[2] / 127, getFirstValue());
                printf("[!] FIBONACCI ALGORITHM HAS CHANGED. NOTE DISTANCE: %.2f, FIRST_VALUE: %.2f.\n",
                       getFibPower(), getFirstValue());
                break;
            case(23):
                setFreqPower(getFibPower(), (double) res[2] / 127);
                printf("[!] FIBONACCI ALGORITHM HAS CHANGED. NOTE DISTANCE: %.2f, FIRST_VALUE: %.2f.\n",
                       getFibPower(), getFirstValue());
                break;
            case(24):
                setScale(res[2] / 10.5);
                char *octaveName = "";
                switch (getScale()) {
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
                printf("[!] SCALE HAS CHANGED. CURRENT SCALE IS %s.\n", octaveName);
                break;
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
                setLightVelocity(res[2]);
                printf("[!] Light Velocity has been changed. Velocity: %d\n", getLightVelocity());
                break;
            case(14):
                if (res[2] > 0) {
                    setLightBPM(res[2]);
                    printf("[!] Light BPM has been changed. Every %d Plant Note\n", res[2]);
                }
                break;
            case(15):
                if (res[2] >= 63) {
                    enable_random_note(true);
                    printf("[!] Random Notes Active\n");
                } else {
                    enable_random_note(false);
                    printf("[!] Random Notes Inactive\n");
                }
                break;
            case(20):
                if (res[2] >= 63) {
                    control_same_note(true);
                    printf("[!] Same Notes Active\n");
                } else {
                    control_same_note(false);
                    printf("[!] Same Notes Inactive\n");
                }
                break;
            case(21):
                set_note_off_speed_percent(res[2] / 127 * 100);
                printf("[!] Note Off time has changed. %d\n", get_note_off_speed_percent());
                break;
                /** @brief CC120 - Stop all notes */
            case(22):
                setFreqPower((double )res[2] / 127, getFirstValue());
                printf("[!] FIBONACCI ALGORITHM HAS CHANGED. NOTE DISTANCE: %.2f, FIRST_VALUE: %.2f.\n",
                       getFibPower(), getFirstValue());
                break;
            case(23):
                setFreqPower(getFibPower(), (double) res[2] / 127);
                printf("[!] FIBONACCI ALGORITHM HAS CHANGED. NOTE DISTANCE: %.2f, FIRST_VALUE: %.2f.\n",
                       getFibPower(), getFirstValue());
                break;
            case(24):
                setScale(res[2] / 10.5);
                char *octaveName = "";
                switch (getScale()) {
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
                printf("[!] SCALE HAS CHANGED. CURRENT SCALE IS %s.\n", octaveName);
                break;
            case(120):
                MidiStop();
                printf("[!] All NOTES OFF\n");
                break;
        }

    }
}




