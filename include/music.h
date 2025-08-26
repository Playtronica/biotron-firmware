#ifndef BIOTRON_MUSIC_H
#define BIOTRON_MUSIC_H


#define LOWEST_NOTE_RANGE 24
#define HIGHEST_NOTE_RANGE 37
#define MIDDLE_NOTE 60
#define LIGHT_DIFFERENCE 24
extern uint8_t last_note_plant;


#define MAX_OF_LIGHT 3200
extern uint8_t last_note_light;

void reset_plant_note_off();
void midi_plant(int64_t to_the_next_beat_us);

void midi_light();
void midi_light_pitch();

void stop_midi();

#endif //BIOTRON_MUSIC_H
