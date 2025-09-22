#include <string.h>
#include <pico/stdlib.h>
#include "params.h"
#include "PLSDK/constants.h"
#include "global.h"
#include "PLSDK/commands.h"
#include "PLSDK/music.h"
#include "music.h"
#include "PLSDK.h"
#include <hardware/flash.h>
#include <hardware/sync.h>
#include <pico/bootrom.h>
#include <pico/printf.h>

Settings_t settings;
enum MuteState mute_state = MuteNone;
bool TestMode = false;
bool isTestModeGreen = true;

// region presets
const Settings_t fast_role_preset = {
        .BPM = BPM_TO_US(404),
        .lightBPM = 8,
        .fraction_note_off = 1,
        .fibPower = 50,
        .firstValue = 10,
        .filterPercent = 0,
        .scale = 3,
        .minPlantVelocity = 0,
        .maxPlantVelocity = 97,
        .minLightVelocity = 0,
        .maxLightVelocity = 68,
        .random_note = true,
        .same_note_plant = 0,
        .same_note_light = 0,
        .light_note_range = 12,
        .light_pitch_mode = 0,
        .isMutePlantVelocity = 0,
        .isMuteLightVelocity = 0,
        .isRandomPlantVelocity = true,
        .isRandomLightVelocity = true,
        .performance_mode = 0,
        .middle_plant_note = 60,
        .plant_channel = 1,
        .light_channel = 2,
        .swing_first_note_percent = 100,
};

const Settings_t the_performer_mode = {
        .BPM = BPM_TO_US(404),
        .lightBPM = 2,
        .fraction_note_off = 2,
        .fibPower = 50,
        .firstValue = 10,
        .filterPercent = 0,
        .scale = 6,
        .minPlantVelocity = 8,
        .maxPlantVelocity = 97,
        .minLightVelocity = 0,
        .maxLightVelocity = 54,
        .random_note = false,
        .same_note_plant = 1,
        .same_note_light = 0,
        .light_note_range = 18,
        .light_pitch_mode = false,
        .isMutePlantVelocity = 0,
        .isMuteLightVelocity = true,
        .isRandomPlantVelocity = true,
        .isRandomLightVelocity = true,
        .performance_mode = true,
        .middle_plant_note = 60,
        .plant_channel = 1,
        .light_channel = 2,
        .swing_first_note_percent = 100,
};

const Settings_t in_discussion = {
        .BPM = BPM_TO_US(404),
        .lightBPM = 2,
        .fraction_note_off = 1,
        .fibPower = 50,
        .firstValue = 10,
        .filterPercent = 0,
        .scale = 5,
        .minPlantVelocity = 44,
        .maxPlantVelocity = 97,
        .minLightVelocity = 0,
        .maxLightVelocity = 54,
        .random_note = false,
        .same_note_plant = 0,
        .same_note_light = 0,
        .light_note_range = 18,
        .light_pitch_mode = false,
        .isMutePlantVelocity = 0,
        .isMuteLightVelocity = 0,
        .isRandomPlantVelocity = true,
        .isRandomLightVelocity = true,
        .performance_mode = true,
        .middle_plant_note = 60,
        .plant_channel = 1,
        .light_channel = 2,
        .swing_first_note_percent = 100,
};

const Settings_t mixolyd = {
        .BPM = BPM_TO_US(462),
        .lightBPM = 4,
        .fraction_note_off = 4,
        .fibPower = 50,
        .firstValue = 10,
        .filterPercent = 0,
        .scale = 4,
        .minPlantVelocity = 8,
        .maxPlantVelocity = 98,
        .minLightVelocity = 74,
        .maxLightVelocity = 75,
        .random_note = 0,
        .same_note_plant = 1,
        .same_note_light = 0,
        .light_note_range = 12,
        .light_pitch_mode = 0,
        .isMutePlantVelocity = 0,
        .isMuteLightVelocity = true,
        .isRandomPlantVelocity = true,
        .isRandomLightVelocity = 0,
        .performance_mode = true,
        .middle_plant_note = 60,
        .plant_channel = 1,
        .light_channel = 2,
        .swing_first_note_percent = 100,
};

#define COUNT_OF_PRESETS 4
const Settings_t * order_of_presets[COUNT_OF_PRESETS] = {
        &mixolyd,
        &fast_role_preset,
        &the_performer_mode,
        &in_discussion
};
// endregion

void default_settings() {
    settings = *order_of_presets[0];
    reset_bpm();
}


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

    if (settings.id != ID_FLASH) {
        clear_flash();
        default_settings();
        save_settings();
        return;
    }
}

void clear_flash() {
    int settingsSize = sizeof(settings);

    int writeSize = (settingsSize / FLASH_PAGE_SIZE) + 1;
    int sectorCount = ((writeSize * FLASH_PAGE_SIZE) / FLASH_SECTOR_SIZE) + 1;

    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE * sectorCount);
    restore_interrupts(interrupts);
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
    settings.scale = data[0] % SCALES_COUNT;
}

void set_scale_cc(uint8_t channel, uint8_t value) {
    settings.scale = (int)(value / (127.0 / SCALES_COUNT));
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

void set_mute_plant_vel_sys_ex(const uint8_t data[], uint8_t len) {
    settings.isMutePlantVelocity = data[0] > 0;
}

void set_mute_light_vel_sys_ex(const uint8_t data[], uint8_t len) {
    settings.isMuteLightVelocity = data[0] > 0;
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
            settings.isRandomPlantVelocity = value >= 64;
            break;
        case 1:
            settings.isRandomLightVelocity = value >= 64;
            break;
        default:
            break;
    }
}


void set_mute_cc(uint8_t channel, uint8_t value) {
    switch (channel) {
        case 0:
            settings.isMutePlantVelocity = value >= 64;
            break;
        case 1:
            settings.isMuteLightVelocity = value >= 64;
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

void set_same_note_plant_sys_ex(const uint8_t data[], uint8_t len) {
    settings.same_note_plant = data[0];
}

void set_same_note_light_sys_ex(const uint8_t data[], uint8_t len) {
    settings.same_note_light = data[0];
}

void set_same_note_cc(uint8_t channel, uint8_t value) {
    switch (channel) {
        case 0:
            settings.same_note_plant = value;
            break;
        case 1:
            settings.same_note_light = value;
            break;
        default:
            break;
    }

}

#define LENGTH_POSSIBLE_NOTE_FRACTION 11
static const int POSSIBLE_NOTE_FRACTION[LENGTH_POSSIBLE_NOTE_FRACTION] = {
        64, 48, 32, 24, 16, 12, 8, 6, 4, 2, 1
};

void set_note_off_percent_sys_ex(const uint8_t data[], uint8_t len) {
    for (int i = 0; i < LENGTH_POSSIBLE_NOTE_FRACTION; i++) {
        if (data[0] == POSSIBLE_NOTE_FRACTION[i]) {
            settings.fraction_note_off = data[0];
            return;
        }
    }
}

void set_note_off_percent_cc(uint8_t channel, uint8_t value) {
    uint8_t id = MIN(LENGTH_POSSIBLE_NOTE_FRACTION - 1, value / (127 / LENGTH_POSSIBLE_NOTE_FRACTION));
    settings.fraction_note_off = POSSIBLE_NOTE_FRACTION[id];
}

void set_light_range_sys_ex(const uint8_t data[], uint8_t len) {
    settings.light_note_range = data[0];
}

void set_light_range_cc(const uint8_t channel, uint8_t value) {
    settings.light_note_range = value;
}

void set_light_pitch_mode_sys_ex(const uint8_t data[], uint8_t len) {
    settings.light_pitch_mode = data[0] > 0;
    change_pitch(0, 63, 63);
}

void set_light_pitch_mode_cc(uint8_t channel, uint8_t value) {
    settings.light_pitch_mode = value > 63;
    change_pitch(0, 63, 63);
}

void set_stuck_mode_sys_ex(const uint8_t data[], uint8_t len) {
    settings.performance_mode = data[0] > 0;
}

void set_stuck_mode_cc(uint8_t channel, uint8_t value) {
    settings.performance_mode = value > 63;
}

void set_middle_plant_note_sys_ex(const uint8_t data[], uint8_t len) {
    settings.middle_plant_note = data[0];
}

void set_middle_plant_note_cc(uint8_t channel, uint8_t value) {
    settings.middle_plant_note = value;
}

void set_swing_first_note_percent_sys_ex(const uint8_t data[], uint8_t len) {
    if (len < 1 || data[0] > 100) return;
    settings.swing_first_note_percent = MAX(1, data[0]);
}

void set_swing_first_note_percent_cc(uint8_t channel, uint8_t value) {
    settings.swing_first_note_percent = MAX(1, value / 127.0 * 100);
}

void set_channel_sys_ex(const uint8_t data[], uint8_t len) {
    printf("%d %d %d\n", len, data[0], data[1]);
    if (len != 2)
        return;

    if (data[0] >= 2 || data[1] >= 16) {
        return;
    }

    if (data[0] == 0) {
        settings.plant_channel = data[1];
    }
    else {
        settings.light_channel = data[1];
    }
}

void get_info_sys_ex(const uint8_t data[], uint8_t len) {
    if (len != 1) return;

    uint8_t sys_ex_info[] = {SYS_EX_START, PLAYTRONICA_SYS_KEY, 126, data[0],
                             MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION, SYS_EX_END};
    print_pure(0, sys_ex_info, 8);
    print_pure(1, sys_ex_info, 8);
}
//endregion


void setup_commands() {
    add_sys_ex_com(change_plant_bpm_sys_ex, 0);
    add_sys_ex_com(change_light_bpm_sys_ex, 9);
//    add_CC(change_bpm_cc, 14);

    add_sys_ex_com(set_fib_power_sys_ex, 1);
    add_CC(set_fib_power_cc, 22);

    add_sys_ex_com(set_fib_first_sys_ex, 2);
    add_CC(set_fib_first_cc, 23);

    add_sys_ex_com(set_filter_sys_ex, 3);
    add_CC(set_filter_cc, 3);

    add_sys_ex_com(set_scale_sys_ex, 4);
    add_CC(set_scale_cc, 24);

    add_sys_ex_com(set_max_plant_vel_sys_ex, 5);
    add_sys_ex_com(set_max_light_vel_sys_ex, 6);
    add_sys_ex_com(set_min_plant_vel_sys_ex, 15);
    add_sys_ex_com(set_min_light_vel_sys_ex, 17);
    add_sys_ex_com(set_random_plant_vel_sys_ex, 16);
    add_sys_ex_com(set_random_light_vel_sys_ex, 18);
    add_sys_ex_com(set_mute_plant_vel_sys_ex, 22);
    add_sys_ex_com(set_mute_light_vel_sys_ex, 23);

    add_CC(set_max_vel_cc, 9);
    add_CC(set_min_vel_cc, 25);
    add_CC(set_random_vel_cc, 26);
    add_CC(set_mute_cc, 31);

    add_sys_ex_com(set_default_sys_ex, 7);

    add_sys_ex_com(set_random_note_sys_ex, 10);
    add_CC(set_random_note_cc, 15);

    add_sys_ex_com(set_same_note_plant_sys_ex, 11);
    add_sys_ex_com(set_same_note_light_sys_ex, 24);
    add_CC(set_same_note_cc, 20);

    add_sys_ex_com(set_note_off_percent_sys_ex, 12);
    add_CC(set_note_off_percent_cc, 21);

    add_sys_ex_com(set_light_range_sys_ex, 13);
    add_CC(set_light_range_cc, 28);

    add_sys_ex_com(set_light_pitch_mode_sys_ex, 19);
    add_CC(set_light_pitch_mode_cc, 27);

    add_sys_ex_com(set_stuck_mode_sys_ex, 21);
    add_CC(set_stuck_mode_cc, 30);

    add_sys_ex_com(set_middle_plant_note_sys_ex, 25);
    add_CC(set_middle_plant_note_cc, 85);

    add_sys_ex_com(set_swing_first_note_percent_sys_ex, 26);
    add_CC(set_swing_first_note_percent_cc, 86);

    add_sys_ex_com(set_channel_sys_ex, 127);
    add_sys_ex_com(get_info_sys_ex, 126);
}


void get_sys_ex_and_behave() {
    int sys_ex_status = read_sys_ex();

    switch (sys_ex_status) {
        case RESET_DEVICE:
            clear_flash();
            reset_usb_boot(0, 0);
        case TEST_MODE_BLUE_ACTIVATE:
            TestMode = true;
            isTestModeGreen = false;
            break;
        case TEST_MODE_GREEN_ACTIVATE:
            TestMode = true;
            isTestModeGreen = true;
            break;
        case TEST_MODE_DEACTIVATE:
            TestMode = false;
            break;
        case LIST_OF_COMMANDS_ACTION:
            load_settings();
            break;
        case CUSTOM_COMMAND:
            save_settings();
            break;
        case BPM_CLOCK_PLAY:
            play_music_bpm_clock();
            break;
        case BPM_CLOCK_DEACTIVATE:
            bpm_clock_control(false);
            break;
        case BPM_CLOCK_ACTIVATE:
            bpm_clock_control(true);
            break;
    }
}

void set_next_preset() {
    static uint counter = 0;
    settings = *order_of_presets[counter];
    stop_bpm();
    counter = (counter + 1) % COUNT_OF_PRESETS;
    save_settings();

    for (int counter = 0; counter < 4; counter++) {
        uint note = calculate_note_by_scale(settings.middle_plant_note, counter, settings.scale);
        note_on(settings.plant_channel, note, 127);
        uint32_t time = time_us_32();
        while (time_us_32() - time < 500000) remind_midi();
        note_off(settings.plant_channel, note);
    }

    reset_bpm();
}