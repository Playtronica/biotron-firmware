#include <class/midi/midi_device.h>
#include <hardware/adc.h>
#include <stdlib.h>
#include <hardware/sync.h>
#include "../include/notes.h"
#include "global.h"
#include "frequency_counter.h"
#include "settings.h"
#include "tusb.h"

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


int sameNotePlay = 0;

uint8_t get_control_same_note() {
    return sameNotePlay;
}

void control_same_note(uint8_t value) {
    sameNotePlay = value;
//    SaveSettings();
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
//    SaveSettings();
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
        if (abs((int)currentNote - (int)lastNotePlant) < sameNotePlay) {
            return;
        }

        if (get_status() == BPMClockActive) {
            uint8_t note_off[3] = {0x80 | 0, lastNotePlant, 0};
            tud_midi_stream_write(0, note_off, 3);
        }

        uint8_t note_on[3] = {0x90 | channel, currentNote, getMaxPlantVelocity()};
        if (getRandomPlantVelocity()) {
            note_on[2] = rand() % (getMaxPlantVelocity() + 1 - getMinPlantVelocity()) + getMinPlantVelocity();
        }

        tud_midi_stream_write(cable_num, note_on, 3);


        if (get_status() == Active) {
            add_repeating_timer_us(getBPM() / 100 * get_note_off_speed_percent(), MidiPlantNoteOff,
                                   NULL, &plantNoteOffTimer);
        }
    }
//    // printf("\nNote perc %d %d\n", last_diff, get_note_off_speed_percent());

    uint8_t note_cc[3] = {0xB0 | channel, 90, getCC(currentNote)};
    tud_midi_stream_write(cable_num, note_cc, 3);

    lastNotePlant = currentNote;
}

uint8_t light_notes_min = LIGHT_NOTE_MIN_DEFAULT;
uint8_t light_notes_max = LIGHT_NOTE_MAX_DEFAULT;
void set_light_min_notes(uint8_t count) {
    light_notes_min = count;
    // printf("[!] Set light notes min: %d\n", light_notes_min);
//    SaveSettings();
}

uint8_t get_light_min_notes() {
    return light_notes_min;
}

void set_light_max_notes(uint8_t count) {
    light_notes_max = count;
    // printf("[!] Set light notes max: %d\n", light_notes_max);
//    SaveSettings();
}

uint8_t get_light_max_notes() {
    return light_notes_max;
}

void set_default() {
    setBPM(TIMER_MIDI_US);
    setFreqPower(DEF_FIB_POW, DEF_FIB_FIRST);
    setFilterPercent(DEF_FILTER_PERCENT);
    setScale(SCALE);
    setPlantVelocity(0, 75, false);
    setLightVelocity(0, 75, false);
    setLightBPM(LIGHT_BPM_DEF);
    enable_random_note(true);
    control_same_note(0);
    set_note_off_speed_percent(100);
    set_light_min_notes(LIGHT_NOTE_MIN_DEFAULT);
    set_light_max_notes(LIGHT_NOTE_MAX_DEFAULT);
    // printf("[!] Return default settings\n");
}

void MidiLight(void) {
    uint8_t const cable_num = 0;
    uint8_t channel = 1;

    if (light_notes_min > light_notes_max) {
        uint8_t buff = light_notes_min;
        set_light_max_notes(light_notes_min);
        set_light_min_notes(buff);
    }

    uint16_t buff = MIN(adc_read(), MAX_OF_LIGHT);
    uint8_t currentNote = (MAX_OF_LIGHT - buff) / (MAX_OF_LIGHT / (light_notes_max - light_notes_min)) + light_notes_min;

    if (!get_mute_mode()) {
        uint8_t note_off[3] = {0x80 | channel, lastNoteLight, 0};
        tud_midi_stream_write(cable_num, note_off, 3);


        uint8_t note_on[3] = {0x90 | channel, currentNote, getMaxLightVelocity()};
        if (getRandomLightVelocity()) {
            note_on[2] = rand() % (getMaxLightVelocity() + 1 - getMinLightVelocity()) + getMinLightVelocity();
        }
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
    uint8_t res[1000];
    uint8_t len = 0;
    static uint8_t last_byte = 0;
    /** @brief Read bytes
     *
     *
     * @paragraph 240 - it is F0 (Start byte),
     * @paragraph 247 - it is F7 (End byte)
     */

     if ((len = tud_midi_available())) {
         tud_midi_stream_read(res, len);
     }
    else return;

    if (res[0] == 250) {
        last_byte = 250;
        return;
    }
    if (res[0] == 252) {
        BPM_clock_disable();
        cancel_repeating_timer(&plantNoteOffTimer);
        last_byte = 252;
        return;
    }
    if (res[0] == 248) {
        if (last_byte == 250) {
            BPM_clock_active();
            last_timer = time_us_64();
            cancel_repeating_timer(&plantNoteOffTimer);
            clk = 0;
        }
        MidiClock();
        last_byte = 248;
        return;
    }

    if (getStatus() == Stabilization) return;


    /** @brief Byte Commands
     *
     * res[0] == 11 is the key to remove trash commands (11 = B) */
    if (res[1] == 11 && res[0] == 240) {
        uint16_t su = 0;
        switch (res[2]) {
            /** @brief F0 B 0 x y ... F7 - BPM command
             *
             * @param x,y,.. - Bpm (Every value can contain maximum FF, so takes sum of all variables)
             * */
            case (0):
                for (int i = 3; res[i] != 247; i++) {
                    su += res[i];
                    tud_task();
                }
                setBPM((int) (1000000.0 / (su / 60.0)));
//                 printf("[!] BPM HAS CHANGED. BPM: %d, TIME: %d.\n", (int)su, (int)(1000000.0 / (su / 60.0)));
                tud_task();
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
                setFreqPower((double) res[3] / 100, getFirstValue());
                // printf("[!] FIBONACCI ALGORITHM HAS CHANGED. NOTE DISTANCE: %.2f, FIRST_VALUE: %.2f.\n",
//                       getFibPower(), getFirstValue());
                tud_task();
                break;
            case (2):
                if (len != 4) break;
                setFreqPower(getFibPower(), (double) res[3] / 100);
                // printf("[!] FIBONACCI ALGORITHM HAS CHANGED. NOTE DISTANCE: %.2f, FIRST_VALUE: %.2f.\n",
//                       getFibPower(), getFirstValue());
                tud_task();
                break;
                /** @brief F0 B 2 x F7 - Filter in active state command
                 *
                 * Makes playing notes more smooth
                 * (x divide by 100)
                 *
                 * @param x - filter power (Max = 0,99)
                 * */
            case (3):
                setFilterPercent((double) (res[3]) / 100);
                // printf("[!] FILTER VALUE HAS CHANGED. FILTER VALUE: %.2f.\n", getFilterPercent());
                tud_task();
                break;
                /** @brief F0 B 2 x F7 - Choose scale command
                 *
                 * @param x - num of scale
                 */
            case (4):
                if (res[3] >= 0 && res[3] < 12) {
                    setScale(res[3]);
                }
                tud_task();
                break;
            case(5):
                setPlantVelocity(getMinPlantVelocity(), res[3], getRandomPlantVelocity());
                // printf("[!] Plant Velocity has been changed. Velocity: %d\n", getPlantVelocity());
                tud_task();
                break;
            case(6):
                setLightVelocity(getMinLightVelocity(), res[3], getRandomLightVelocity());
                // printf("[!] Light Velocity has been changed. Velocity: %d\n", getLightVelocity());
                tud_task();
                break;
            case (7):
                set_default();
                tud_task();
                break;
            case (8):
                PrintInfo();
                tud_task();
                break;
            case(9):
                if (res[3] > 0) {
                    setLightBPM(res[3]);
                    // printf("[!] Light BPM has been changed. Every %d Plant Note\n", res[3]);
                }
                tud_task();
                break;
            case(10):
                if (res[3] != 0) {
                    enable_random_note(true);
                    // printf("[!] Random Notes Active\n");
                } else {
                    enable_random_note(false);
                    // printf("[!] Random Notes Inactive\n");
                }
                tud_task();
                break;
            case(11):
                if (res[3] == 0) {
                    control_same_note(res[3]);
                    // printf("[!] Same Notes Active\n");
                } else {
                    control_same_note(res[3]);
                    // printf("[!] Same Notes Inactive, %d\n", res[3]);
                }
                tud_task();
                break;
            case(12):
                set_note_off_speed_percent(res[3]);
                // printf("[!] Note Off time has changed. %d\n", get_note_off_speed_percent());
                tud_task();
                break;
            case (13):
                set_light_min_notes(res[3]);
                tud_task();
                break;
            case (14):
                set_light_max_notes(res[3]);
                tud_task();
                break;
            case(15):
                setPlantVelocity(res[3], getMaxPlantVelocity(), getRandomPlantVelocity());
                break;
            case(16):
                setPlantVelocity(getMinPlantVelocity(), getMaxPlantVelocity(), res[3] > 0);
                break;
            case(17):
                setLightVelocity(res[3], getMaxLightVelocity(), getRandomLightVelocity());
                break;
            case(18):
                setLightVelocity(getMinLightVelocity(), getMaxLightVelocity(), res[3] > 0);
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
                // printf("[!] FILTER VALUE HAS CHANGED. FILTER VALUE: %.2f.\n", (double)(res[2]) / 127);
                break;
                /** @brief CC9 x - Change velocity command
                *
                * @param x - velocity (Max = 127)
                 */
            case(9):
                setPlantVelocity(getMinPlantVelocity(), res[2], getRandomPlantVelocity());
                // printf("[!] Plant Velocity has been changed. Velocity: %d\n", getPlantVelocity());
                break;
            case(14):
                setBPM((int) (1000000.0 / ((res[2] * 5) / 60.0)));
                // printf("[!] BPM HAS CHANGED. BPM: %d, TIME: %d.\n", (int)res[2] * 5,
//                       (int)(1000000.0 / ((res[2] * 10) / 60.0)));
                break;
            case(15):
                if (res[2] >= 63) {
                    enable_random_note(true);
                    // printf("[!] Random Notes Active\n");
                } else {
                    enable_random_note(false);
                    // printf("[!] Random Notes Inactive\n");
                }
                break;
            case(20):
                if (res[2] * 127 / 1000 <= 0) {
                    control_same_note(0);
                    // printf("[!] Same Notes Active\n");
                } else {
                    control_same_note(res[2] * 127 / 1000 <= 0);
                    // printf("[!] Same Notes Inactive, %d\n", res[2] * 127 / 1000);
                }
                break;
            case(21):
                set_note_off_speed_percent(res[2] / 127 * 100);
                // printf("[!] Note Off time has changed. %d\n", get_note_off_speed_percent());
                break;
                /** @brief CC120 - Stop all notes */
            case(22):
                setFreqPower((double )res[2] / 127, getFirstValue());
                // printf("[!] FIBONACCI ALGORITHM HAS CHANGED. NOTE DISTANCE: %.2f, FIRST_VALUE: %.2f.\n",
//                       getFibPower(), getFirstValue());
                break;
            case(23):
                setFreqPower(getFibPower(), (double) res[2] / 127);
                // printf("[!] FIBONACCI ALGORITHM HAS CHANGED. NOTE DISTANCE: %.2f, FIRST_VALUE: %.2f.\n",
//                       getFibPower(), getFirstValue());
                break;
            case(24):
                setScale(res[2] / 10.5);
                // printf("[!] SCALE HAS CHANGED. CURRENT SCALE IS %s.\n", octaveName);
                break;
            case(25):
                setPlantVelocity(res[2], getMaxPlantVelocity(), getRandomPlantVelocity());
                break;
            case(26):
                setPlantVelocity(getMinPlantVelocity(), getMaxPlantVelocity(), res[2] > 63);
                break;
            case(120):
                MidiStop();
                // printf("[!] All NOTES OFF\n");
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
                setLightVelocity(getMinLightVelocity(), res[2], getRandomLightVelocity());
                // printf("[!] Light Velocity has been changed. Velocity: %d\n", getLightVelocity());
                break;
            case(14):
                if (res[2] > 0) {
                    setLightBPM(res[2]);
                    // printf("[!] Light BPM has been changed. Every %d Plant Note\n", res[2]);
                }
                break;
            case(15):
                if (res[2] >= 63) {
                    enable_random_note(true);
                    // printf("[!] Random Notes Active\n");
                } else {
                    enable_random_note(false);
                    // printf("[!] Random Notes Inactive\n");
                }
                break;
            case(20):
                if (res[2] * 127 / 1000 <= 0) {
                    control_same_note(0);
                    // printf("[!] Same Notes Active\n");
                } else {
                    control_same_note(res[2] * 127 / 1000 <= 0);
                    // printf("[!] Same Notes Inactive, %d\n", res[2] * 127 / 1000);
                }
                break;
            case(21):
                set_note_off_speed_percent(res[2] / 127 * 100);
                // printf("[!] Note Off time has changed. %d\n", get_note_off_speed_percent());
                break;
                /** @brief CC120 - Stop all notes */
            case(22):
                setFreqPower((double )res[2] / 127, getFirstValue());
                // printf("[!] FIBONACCI ALGORITHM HAS CHANGED. NOTE DISTANCE: %.2f, FIRST_VALUE: %.2f.\n",
//                       getFibPower(), getFirstValue());
                break;
            case(23):
                setFreqPower(getFibPower(), (double) res[2] / 127);
                // printf("[!] FIBONACCI ALGORITHM HAS CHANGED. NOTE DISTANCE: %.2f, FIRST_VALUE: %.2f.\n",
//                       getFibPower(), getFirstValue());
                break;
            case(24):
                setScale(res[2] / 10.5);
                break;
            case(25):
                setLightVelocity(res[2], getMaxLightVelocity(), getRandomLightVelocity());
                break;
            case(26):
                setLightVelocity(getMinLightVelocity(), getMaxLightVelocity(), res[2] > 63);
                break;
            case(120):
                MidiStop();
                // printf("[!] All NOTES OFF\n");
                break;
        }
    }
    SaveSettings();
}




