#include <pico/printf.h>
#include <hardware/sync.h>
#include <string.h>
#include "settings.h"
#include "hardware/flash.h"
#include "global.h"
#include "frequency_counter.h"

Settings_t settings;
bool init = false;

void SaveSettings() {
    if (init) return;
    settings.settingsIsNull = false;
    settings.BPM = getBPM();
    settings.lightBPM = getLightBPM();
    settings.fibPower = getFibPower();
    settings.firstValue = getFirstValue();
    settings.filterPercent = getFilterPercent();
    settings.scale = getScale();
    settings.isRandomPlantVelocity = getRandomPlantVelocity();
    settings.minPlantVelocity = getMinPlantVelocity();
    settings.maxPlantVelocity = getMaxPlantVelocity();
    settings.isRandomLightVelocity = getRandomLightVelocity();
    settings.minLightVelocity = getMinLightVelocity();
    settings.maxLightVelocity = getMaxLightVelocity();
    settings.random_note = get_random_note_state();
    settings.same_note = get_control_same_note();
    settings.percent_note_off = get_note_off_speed_percent();
    settings.light_note_min = get_light_min_notes();
    settings.light_note_max = get_light_max_notes();
    uint8_t* settingsAsBytes = (uint8_t*) &settings;
    int settingsSize = sizeof(settings);


    int writeSize = (settingsSize / FLASH_PAGE_SIZE) + 1; // how many flash pages we're gonna need to write
    int sectorCount = ((writeSize * FLASH_PAGE_SIZE) / FLASH_SECTOR_SIZE) + 1; // how many flash sectors we're gonna need to erase


    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE * sectorCount);
    flash_range_program(FLASH_TARGET_OFFSET, settingsAsBytes, FLASH_PAGE_SIZE * writeSize);
    restore_interrupts(interrupts);
}

void ReadSettings() {
    const uint8_t* flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
    memcpy(&settings, flash_target_contents, sizeof(settings));

    if (settings.settingsIsNull) {
        set_default();
    }
    init = true;

    setBPM(settings.BPM);
    setLightBPM(settings.lightBPM);
    setFreqPower(settings.fibPower, settings.firstValue);
    setFilterPercent(1 - settings.filterPercent);
    setScale(settings.scale);
    setPlantVelocity(settings.minPlantVelocity, settings.maxPlantVelocity, settings.isRandomPlantVelocity);
    setLightVelocity(settings.minLightVelocity, settings.maxLightVelocity, settings.isRandomLightVelocity);
    enable_random_note(settings.random_note);
    control_same_note(settings.same_note);
    set_note_off_speed_percent(settings.percent_note_off);
    set_light_min_notes(settings.light_note_min);
    set_light_max_notes(settings.light_note_max);
    init = false;
}