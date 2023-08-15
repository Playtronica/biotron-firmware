#ifndef BIOTRON_SETTINGS_H
#define BIOTRON_SETTINGS_H

#define FLASH_TARGET_OFFSET (512 * 1024)

typedef struct {
    int BPM;
    int lightBPM;
    double fibPower;
    double firstValue;
    double filterPercent;
    int scale;
    int plantVelocity;
    int lightVelocity;
    bool random_note;
    bool same_note;
} Settings_t;

void SaveSettings();
void ReadSettings();
void getPublicData();
#endif //BIOTRON_SETTINGS_H
