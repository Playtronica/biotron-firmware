#ifndef BIOTRON_SETTINGS_H
#define BIOTRON_SETTINGS_H

#define FLASH_TARGET_OFFSET (512 * 1024)

typedef struct {
    bool settingsIsNull;
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
    int light_note_min;
    int light_note_max;
} Settings_t;

void SaveSettings();
void ReadSettings();
void getPublicData();
#endif //BIOTRON_SETTINGS_H
