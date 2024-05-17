#ifndef TOUCHME_NOTES_H
#define TOUCHME_NOTES_H


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

/** @brief Getters for public variables */
uint32_t getLastNotePlant();
uint32_t getLastNoteLight();


void set_note_off_speed_percent(uint8_t val);
uint8_t get_note_off_speed_percent();

/** @brief MIDI from plant (Channel 1) */
void MidiPlant(void);
void resetPlantNoteOff();
/** @brief MIDI from light (Channel 2) */
void set_light_notes_count(uint8_t count);
void set_light_min_notes(uint8_t count);
uint8_t get_light_min_notes();
void set_light_max_notes(uint8_t count);
uint8_t get_light_max_notes();
void MidiLight(void);
void MidiLightPitch(void);

/** @brief Stop all notes */
void MidiStop();

/** @brief Settings for device (CC commands and Byte commands (F0 .. F7)) */
void MidiSettings();

void control_same_note(uint8_t value);

uint8_t get_control_same_note();

void set_default();

#endif