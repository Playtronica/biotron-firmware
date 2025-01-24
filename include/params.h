#ifndef BIOTRON_PARAMS_H
#define BIOTRON_PARAMS_H

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
#define DEF_SCALE 8
#define DEF_RAND_VEL false
#define DEF_MUTE false
#define DEF_MIN_VEL 0
#define DEF_MAX_VEL 75
#define DEF_RANDOM_NOTE true
#define DEF_SANE_NOTE 1
#define DEF_PERCENT_NOTE_OFF 1
#define DEF_LIGHT_NOTE_RANGE 12
#define DEF_LIGHT_PITCH_MODE false
#define DEF_STUCK_MODE false

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
} Settings_t;

extern Settings_t settings;
extern enum MuteState {
    MuteNone,
    MutePlant,
    MuteLight,
    MuteAll
} mute_state;
extern bool TestMode;


void save_settings();
void read_settings();
void clear_flash();

void setup_commands();
void get_sys_ex_and_behave();


#endif //BIOTRON_PARAMS_H
