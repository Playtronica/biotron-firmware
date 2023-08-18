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
    settings.BPM = getBPM();
    settings.lightBPM = getLightBPM();
    settings.fibPower = getFibPower();
    settings.firstValue = getFirstValue();
    settings.filterPercent = getFilterPercent();
    settings.scale = getScale();
    settings.plantVelocity = getPlantVelocity();
    settings.lightVelocity = getLightVelocity();
    settings.random_note = get_random_note_state();
    settings.same_note = get_control_same_note();
    settings.percent_note_off = get_note_off_speed_percent();
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
    //int settingsSize = sizeof(settings);
    memcpy(&settings, flash_target_contents, sizeof(settings));

    for (uint8_t i = 0; i < 8; i++)
    {
        unsigned char c = ((char*)flash_target_contents)[i] ;
        printf ("%02x ", c) ;
    }
    printf("\n");
    uint8_t* settingsAsBytes = (uint8_t*) &settings;
    for (uint8_t i = 0; i < 8; i++)
    {
        unsigned char c = ((char*)settingsAsBytes)[i] ;
        printf ("%02x ", c) ;
    }
    printf("\n");

    init = true;
    settings.BPM = settings.BPM <= 0 ? TIMER_MIDI_US : settings.BPM;
    settings.lightBPM = settings.lightBPM <= 0 ? LIGHT_BPM_DEF : settings.lightBPM;
    settings.fibPower = settings.fibPower == 0 ? DEF_FIB_POW : settings.fibPower;
    settings.firstValue = settings.firstValue == 0 ? DEF_FIB_FIRST : settings.firstValue;
    settings.filterPercent = settings.filterPercent == 0 ? DEF_FILTER_PERCENT : 1 - settings.filterPercent;
    settings.scale = settings.scale < 0 || settings.scale > 11 ? SCALE : settings.scale;
    settings.plantVelocity = settings.plantVelocity == 0 ? 127 : settings.plantVelocity;
    settings.lightVelocity = settings.lightVelocity == 0 ? 127 : settings.lightVelocity;


    setBPM(settings.BPM);
    setLightBPM(settings.lightBPM);
    setFreqPower(settings.fibPower, settings.firstValue);
    setFilterPercent(settings.filterPercent);
    setScale(settings.scale);
    setPlantVelocity(settings.plantVelocity);
    setLightVelocity(settings.lightVelocity);
    enable_random_note(settings.random_note);
    control_same_note(settings.same_note);
    set_note_off_speed_percent(settings.percent_note_off);
    init = false;
}