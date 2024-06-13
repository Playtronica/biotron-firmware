#ifndef BIOTRON_PARAMS_H
#define BIOTRON_PARAMS_H

#include <stdbool.h>
#define FLASH_TARGET_OFFSET (512 * 1024)

#ifdef FLASH_ID_STARTUP
#define ID_FLASH FLASH_ID_STARTUP
#else
#define ID_FLASH 0
#endif

#define DEF_TIMER_MIDI_US 500000
#define DEF_LIGHT_BPM 4
#define DEF_FIB_POW 0.5
#define DEF_FIB_FIRST 0.1
#define DEF_FILTER_PERCENT 0
#define DEF_SCALE 0
#define DEF_RAND_VEL false
#define DEF_MIN_VEL 0
#define DEF_MAX_VEL 75
#define DEF_RANDOM_NOTE true
#define DEF_SANE_NOTE 0
#define DEF_PERCENT_NOTE_OFF 100
#define DEF_LIGHT_NOTE_RANGE 12
#define DEF_LIGHT_PITCH_MODE false

typedef struct {
    int id;
    int BPM;
    int lightBPM;
    double fibPower;
    double firstValue;
    double filterPercent;
    int scale;
    bool isRandomPlantVelocity;
    int minPlantVelocity;
    int maxPlantVelocity;
    bool isRandomLightVelocity;
    int minLightVelocity;
    int maxLightVelocity;
    bool random_note;
    int same_note;
    int percent_note_off;
    int light_note_range;
    int undef;
    bool light_pitch_mode;
} Settings_t;

extern Settings_t settings;
extern bool is_mute;

void save_settings();
void read_settings();
void clear_flash();

void setup_commands();


#endif //BIOTRON_PARAMS_H