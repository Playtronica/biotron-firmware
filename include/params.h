#ifndef BIOTRON_PARAMS_H
#define BIOTRON_PARAMS_H

#define FLASH_TARGET_OFFSET (512 * 1024)

#ifdef FLASH_ID_STARTUP
#define ID_FLASH FLASH_ID_STARTUP
#else
#define ID_FLASH 0
#endif

#ifndef MAJOR_VERSION
#define MAJOR_VERSION 0
#endif

#ifndef MINOR_VERSION
#define MINOR_VERSION 0
#endif

#ifndef PATCH_VERSION
#define PATCH_VERSION 0
#endif

#include "PLSDK/constants.h"

#define DEF_TIMER_MIDI_US BPM_TO_US(462)
#define DEF_LIGHT_BPM 4
#define DEF_FIB_POW 0.5
#define DEF_FIB_FIRST 0.1
#define DEF_FILTER_PERCENT 0
#define DEF_SCALE 4
#define DEF_RAND_PLANT_VEL true
#define DEF_RAND_LIGHT_VEL false
#define DEF_MUTE_PLANT false
#define DEF_MUTE_LIGHT true
#define DEF_MIN_PLANT_VEL 8
#define DEF_MAX_PLANT_VEL 98
#define DEF_MIN_LIGHT_VEL 74
#define DEF_MAX_LIGHT_VEL 75
#define DEF_RANDOM_NOTE false
#define DEF_SANE_NOTE_PLANT 1
#define DEF_SANE_NOTE_LIGHT 0
#define DEF_PERCENT_NOTE_OFF 4
#define DEF_LIGHT_NOTE_RANGE 12
#define DEF_LIGHT_PITCH_MODE false
#define DEF_STUCK_MODE true

typedef struct {
    int id;
    int BPM;
    int lightBPM;
    double fibPower;
    double firstValue;
    double filterPercent;
    int scale;
    bool isRandomPlantVelocity;
    bool isMutePlantVelocity;
    int minPlantVelocity;
    int maxPlantVelocity;
    bool isRandomLightVelocity;
    bool isMuteLightVelocity;
    int minLightVelocity;
    int maxLightVelocity;
    bool random_note;
    int same_note_plant;
    int same_note_light;
    int fraction_note_off;
    int light_note_range;
    bool light_pitch_mode;
    bool performance_mode;
    int middle_plant_note;
    int plant_channel;
    int light_channel;
} Settings_t;

extern Settings_t settings;
extern enum MuteState {
    MuteNone,
    MutePlant,
    MuteLight,
    MuteAll
} mute_state;
extern bool TestMode;
extern bool isTestModeGreen;


void save_settings();
void read_settings();
void clear_flash();

void setup_commands();
void get_sys_ex_and_behave();
void set_next_preset();

#endif //BIOTRON_PARAMS_H
