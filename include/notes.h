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

uint32_t getNote(int percent);
uint32_t getCountNote();

uint8_t getLengthOctave();
uint32_t * getOctaveNotes();

#endif