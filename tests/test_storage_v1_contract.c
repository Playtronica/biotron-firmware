#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "params.h"

#define EXPECT_OFFSET(field, offset) assert(offsetof(Settings_t, field) == (offset))

int main(void) {
    assert(FLASH_TARGET_OFFSET == 524288);
    assert(sizeof(Settings_t) == 112);
    assert(_Alignof(Settings_t) == 8);
    EXPECT_OFFSET(id, 0);
    EXPECT_OFFSET(BPM, 4);
    EXPECT_OFFSET(lightBPM, 8);
    EXPECT_OFFSET(fibPower, 16);
    EXPECT_OFFSET(firstValue, 24);
    EXPECT_OFFSET(filterPercent, 32);
    EXPECT_OFFSET(scale, 40);
    EXPECT_OFFSET(isRandomPlantVelocity, 44);
    EXPECT_OFFSET(isMutePlantVelocity, 45);
    EXPECT_OFFSET(minPlantVelocity, 48);
    EXPECT_OFFSET(maxPlantVelocity, 52);
    EXPECT_OFFSET(isRandomLightVelocity, 56);
    EXPECT_OFFSET(isMuteLightVelocity, 57);
    EXPECT_OFFSET(minLightVelocity, 60);
    EXPECT_OFFSET(maxLightVelocity, 64);
    EXPECT_OFFSET(random_note, 68);
    EXPECT_OFFSET(same_note_plant, 72);
    EXPECT_OFFSET(same_note_light, 76);
    EXPECT_OFFSET(fraction_note_off, 80);
    EXPECT_OFFSET(light_note_range, 84);
    EXPECT_OFFSET(light_pitch_mode, 88);
    EXPECT_OFFSET(performance_mode, 89);
    EXPECT_OFFSET(middle_plant_note, 92);
    EXPECT_OFFSET(plant_channel, 96);
    EXPECT_OFFSET(light_channel, 100);
    EXPECT_OFFSET(swing_first_note_percent, 104);
    EXPECT_OFFSET(is_mute_button_active, 108);
    puts("storage_v1_contract: shipping Settings_t ABI and flash offset passed");
    return 0;
}
