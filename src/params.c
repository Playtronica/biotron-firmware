#include <string.h>
#include "params.h"
#include "pico/stdlib.h"
#include <hardware/flash.h>
#include <hardware/sync.h>

Settings_t settings;
bool is_mute = false;

void default_settings() {
    settings.settingsIsNull = false;
    settings.BPM = DEF_TIMER_MIDI_US;
    settings.lightBPM = DEF_LIGHT_BPM;
    settings.fibPower = DEF_FIB_POW;
    settings.firstValue = DEF_FIB_FIRST;
    settings.filterPercent = DEF_FILTER_PERCENT;
    settings.scale = DEF_SCALE;
    settings.isRandomPlantVelocity = DEF_RAND_VEL;
    settings.minPlantVelocity = DEF_MIN_VEL;
    settings.maxPlantVelocity = DEF_MAX_VEL;
    settings.isRandomLightVelocity = DEF_RAND_VEL;
    settings.minLightVelocity = DEF_MIN_VEL;
    settings.maxLightVelocity = DEF_MAX_VEL;
    settings.random_note = DEF_RANDOM_NOTE;
    settings.same_note = DEF_SANE_NOTE;
    settings.percent_note_off = DEF_PERCENT_NOTE_OFF;
    settings.light_note_min = DEF_LIGHT_NOTE_MIN;
    settings.light_note_max = DEF_LIGHT_NOTE_MAX;
    settings.light_pitch_mode = DEF_LIGHT_PITCH_MODE;
};


void save_settings() {
    uint8_t* settingsAsBytes = (uint8_t*) &settings;
    int settingsSize = sizeof(settings);

    int writeSize = (settingsSize / FLASH_PAGE_SIZE) + 1;
    int sectorCount = ((writeSize * FLASH_PAGE_SIZE) / FLASH_SECTOR_SIZE) + 1;

    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE * sectorCount);
    flash_range_program(FLASH_TARGET_OFFSET, settingsAsBytes, FLASH_PAGE_SIZE * writeSize);
    restore_interrupts(interrupts);
}

void read_settings() {
    const uint8_t* flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
    memcpy(&settings, flash_target_contents, sizeof(settings));

    if (settings.settingsIsNull) {
        default_settings();
        save_settings();
        return;
    }
}

