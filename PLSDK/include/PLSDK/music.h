/**
 * @file music.h
 *
 * @brief Controls basic midi events and work with scales
 *
 * @author Shadowik
 * @date 2023-07-28
 */

#ifndef PSDK_NOTES_H
#define PSDK_NOTES_H

#define SCALES_COUNT 13

typedef enum{
    SCALE_MAJOR,
    SCALE_MINOR,
    SCALE_CHROM,
    SCALE_DORIAN,
    SCALE_MIXOLYDIAN,
    SCALE_LYDIAN,
    SCALE_WHOLETONE,
    SCALE_MINBLUES,
    SCALE_MAJBLUES,
    SCALE_MINPEN,
    SCALE_MAJPEN,
    SCALE_DIMINISHED,
    SCALE_HIRAJOSHI,
    SCALE_CUSTOM
} ScaleNums_t;

typedef struct {
    int steps;
    const uint8_t * scale;
} NotesScale_t;

/**
 * @brief Calculates notes in different scales
 *
 * @param start_note - from which note you start counting
 * @param counter - how many notes do you need past
 * @param scale - in which scale you need to get note
 *
 * @return int array with 3 digits. First is note, and two last its lsb and msb for pitch.
 * If last two variables equal -1, don't have pitch change
 *
 * @note
 * If you choose start note not equal x % 12 == 0 (C notes), scale would be alternative
 * */
int calculate_note_by_scale(uint8_t start_note, int counter, ScaleNums_t scale);

void change_volume(uint8_t channel_id, uint8_t volume);

void note_on(uint8_t channel_id, uint8_t note, uint8_t velocity);
void note_off(uint8_t channel_id, uint8_t note);

void change_pitch(uint8_t channel_id, uint8_t lsb, uint8_t msb);

void stop_all_notes(uint8_t channel_id);


#endif //PSDK_NOTES_H
