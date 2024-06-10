#include <string.h>
#include "params.h"
#include "pico/stdlib.h"
#include "PLSDK/constants.h"
#include "global.h"
#include "PLSDK/commands.h"
#include "PLSDK/music.h"
#include <hardware/flash.h>
#include <hardware/sync.h>
#include <pico/printf.h>

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


//region MIDI commands

void change_plant_bpm(uint16_t bpm) {
    if (bpm == 0) return;
    settings.BPM = BPM_TO_US(bpm);

    if (status == Active) {
        reset_bpm();
    }

}

void change_plant_bpm_sys_ex(const uint8_t data[], uint8_t len) {
    uint16_t bpm = 0;
    for (int i = 0; i < len; i++) {
        bpm += data[i];
    }
    change_plant_bpm(bpm);
}


void change_light_bpm_sys_ex(const uint8_t data[], uint8_t len) {
    settings.lightBPM = data[0];
}

void change_bpm_cc(uint8_t channel, uint8_t value) {
    switch (channel) {
        case 0:
            change_plant_bpm(value * 5);
            break;
        case 1:
            settings.lightBPM = value;
            break;
        default:
            break;
    }
}

void set_fib_power_sys_ex(const uint8_t data[], uint8_t len) {
    settings.fibPower = (double )data[0] / 100;
}

void set_fib_power_cc(uint8_t channel, uint8_t value) {
    settings.fibPower = (double )value / 127;
}

void set_fib_first_sys_ex(const uint8_t data[], uint8_t len) {
    settings.firstValue = (double )data[0] / 100;
}

void set_fib_first_cc(uint8_t channel, uint8_t value) {
    settings.firstValue = (double )value / 127;
}

void set_filter_sys_ex(const uint8_t data[], uint8_t len) {
    settings.filterPercent = (double )data[0] / 100;
}

void set_filter_cc(uint8_t channel, uint8_t value) {
    settings.filterPercent = (double )value / 127;
}

void set_scale_sys_ex(const uint8_t data[], uint8_t len) {
    printf("Change scale %d\n", data[0] % 12);
    settings.scale = data[0] % 12;
}

void set_scale_cc(uint8_t channel, uint8_t value) {
    settings.scale = (int)(value / (127.0 / 12));
}

void set_max_plant_vel_sys_ex(const uint8_t data[], uint8_t len) {
    settings.maxPlantVelocity = data[0];
}

void set_min_plant_vel_sys_ex(const uint8_t data[], uint8_t len) {
    settings.minPlantVelocity = data[0];
}

void set_random_plant_vel_sys_ex(const uint8_t data[], uint8_t len) {
    settings.isRandomPlantVelocity = data[0] > 0;
}

void set_max_light_vel_sys_ex(const uint8_t data[], uint8_t len) {
    settings.maxLightVelocity = data[0];
}

void set_min_light_vel_sys_ex(const uint8_t data[], uint8_t len) {
    settings.minLightVelocity = data[0];
}

void set_random_light_vel_sys_ex(const uint8_t data[], uint8_t len) {
    settings.isRandomLightVelocity = data[0] > 0;
}

void set_max_vel_cc(uint8_t channel, uint8_t value) {
    switch (channel) {
        case 0:
            settings.maxPlantVelocity = value;
            break;
        case 1:
            settings.maxLightVelocity = value;
            break;
        default:
            break;
    }
}

void set_min_vel_cc(uint8_t channel, uint8_t value) {
    switch (channel) {
        case 0:
            settings.minPlantVelocity = value;
            break;
        case 1:
            settings.minLightVelocity = value;
            break;
        default:
            break;
    }
}

void set_random_vel_cc(uint8_t channel, uint8_t value) {
    switch (channel) {
        case 0:
            settings.isRandomPlantVelocity = value;
            break;
        case 1:
            settings.isRandomLightVelocity = value;
            break;
        default:
            break;
    }
}

void set_default_sys_ex(const uint8_t data[], uint8_t len) {
    default_settings();
    reset_bpm();
}

void set_random_note_sys_ex(const uint8_t data[], uint8_t len) {
    settings.random_note = data[0] > 0;
}

void set_random_note_cc(uint8_t channel, uint8_t value) {
    settings.random_note = value > 63;
}

void set_same_note_sys_ex(const uint8_t data[], uint8_t len) {
    settings.same_note = data[0];
}

void set_same_note_cc(uint8_t channel, uint8_t value) {
    settings.same_note = value;
}

void set_note_off_percent_sys_ex(const uint8_t data[], uint8_t len) {
    settings.percent_note_off = data[0];
}

void set_note_off_percent_cc(uint8_t channel, uint8_t value) {
    settings.percent_note_off = (int)((double)value / 127.0 * 100.0);
}

void set_light_min_note_sys_ex(const uint8_t data[], uint8_t len) {
    settings.light_note_min = data[0];
}

void set_light_max_note_sys_ex(const uint8_t data[], uint8_t len) {
    settings.light_note_max = data[0];
}

void set_light_pitch_mode_sys_ex(const uint8_t data[], uint8_t len) {
    settings.light_pitch_mode = data[0] > 0;
    change_pitch(0, 63, 63);
}

void set_light_pitch_mode_cc(uint8_t channel, uint8_t value) {
    settings.light_pitch_mode = value > 63;
    change_pitch(0, 63, 63);
}
//endregion


void setup_commands() {
    add_sys_ex_com(change_plant_bpm_sys_ex, 0);
    add_sys_ex_com(change_light_bpm_sys_ex, 9);
    add_CC(change_bpm_cc, 14);

    add_sys_ex_com(set_fib_power_sys_ex, 1);
    add_CC(set_fib_power_cc, 22);

    add_sys_ex_com(set_fib_first_sys_ex, 2);
    add_CC(set_fib_first_cc, 23);

    add_sys_ex_com(set_filter_sys_ex, 3);
    add_CC(set_filter_cc, 3);

    add_sys_ex_com(set_scale_sys_ex, 4);
    add_CC(set_scale_cc, 3);

    add_sys_ex_com(set_max_plant_vel_sys_ex, 5);
    add_sys_ex_com(set_max_light_vel_sys_ex, 6);
    add_sys_ex_com(set_min_plant_vel_sys_ex, 15);
    add_sys_ex_com(set_min_light_vel_sys_ex, 17);
    add_sys_ex_com(set_random_plant_vel_sys_ex, 16);
    add_sys_ex_com(set_random_light_vel_sys_ex, 18);
    add_CC(set_max_vel_cc, 9);
    add_CC(set_min_vel_cc, 25);
    add_CC(set_random_vel_cc, 26);

    add_sys_ex_com(set_default_sys_ex, 7);

    add_sys_ex_com(set_random_note_sys_ex, 10);
    add_CC(set_random_note_cc, 15);

    add_sys_ex_com(set_same_note_sys_ex, 11);
    add_CC(set_same_note_cc, 20);

    add_sys_ex_com(set_note_off_percent_sys_ex, 12);
    add_CC(set_note_off_percent_cc, 21);

    add_sys_ex_com(set_light_min_note_sys_ex, 13);
    add_sys_ex_com(set_light_max_note_sys_ex, 14);

    add_sys_ex_com(set_light_pitch_mode_sys_ex, 19);
    add_CC(set_light_pitch_mode_cc, 27);

}

