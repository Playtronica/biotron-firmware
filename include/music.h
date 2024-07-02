#ifndef BIOTRON_MUSIC_H
#define BIOTRON_MUSIC_H


#define LOWEST_NOTE 36
#define HIGHEST_NOTE 107
#define MIDDLE_NOTE 60
extern uint8_t last_note_plant;


#define MAX_OF_LIGHT 4000
#define MIN_OF_LIGHT 300
#define MAX_OF_LIGHT_BPM 240
#define MIN_OF_LIGHT_BPM 1
extern uint8_t last_note_light;

void reset_plant_note_off();
void midi_plant();

void midi_light();
void midi_light_pitch();

void stop_midi();

#endif //BIOTRON_MUSIC_H
