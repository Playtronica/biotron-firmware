/*!
   * @file notes.h
   * @brief Implementation of notes generation from ADC values
   *
   * @author Vladislav Aidarov
   * @date   2022-04-20
   */

#ifndef TOUCHME_NOTES_H
#define TOUCHME_NOTES_H

#include <stdio.h>
#include "pico/stdlib.h"


uint32_t lastNotePlant;
uint32_t lastNoteLight;

/** @brief Enumeration for scales */
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
    SCALE_DIMINISHED
} ScaleNums_t;

void midi_plant(void);
void midi_light(void);

void midi_stop();
void midi_settings();

#endif