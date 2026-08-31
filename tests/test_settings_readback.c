#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "settings_readback.h"

static Settings_t example(void) {
    return (Settings_t){
        .BPM = BPM_TO_US(462), .lightBPM = 4,
        .fibPower = 0.5, .firstValue = 0.1, .filterPercent = 0,
        .scale = 4, .isRandomPlantVelocity = true,
        .isMutePlantVelocity = false, .minPlantVelocity = 8,
        .maxPlantVelocity = 98, .isRandomLightVelocity = false,
        .isMuteLightVelocity = true, .minLightVelocity = 74,
        .maxLightVelocity = 75, .random_note = false,
        .same_note_plant = 1, .same_note_light = 0,
        .fraction_note_off = 4, .light_note_range = 12,
        .light_pitch_mode = false, .performance_mode = true,
        .middle_plant_note = 60, .plant_channel = 1,
        .light_channel = 2, .swing_first_note_percent = 100,
        .is_mute_button_active = false,
    };
}

static uint16_t decoded_bpm(const uint8_t payload[]) {
    return (uint16_t)payload[BIOTRON_SETTINGS_HEADER_BYTES] |
           ((uint16_t)payload[BIOTRON_SETTINGS_HEADER_BYTES + 1] << 7u);
}

static const uint8_t *encoded_values(const Settings_t *settings,
                                     uint8_t payload[]) {
    assert(biotron_settings_encode(
            settings, true, false, BIOTRON_SETTINGS_SOURCE_PERSISTED,
            0, 0, 0, payload, BIOTRON_SETTINGS_PAYLOAD_BYTES) ==
           BIOTRON_SETTINGS_PAYLOAD_BYTES);
    return &payload[BIOTRON_SETTINGS_HEADER_BYTES];
}

static void test_exact_web_vector(void) {
    Settings_t settings = example();
    uint8_t payload[BIOTRON_SETTINGS_PAYLOAD_BYTES];
    assert(biotron_settings_encode(
            &settings, true, false, BIOTRON_SETTINGS_SOURCE_PERSISTED,
            42, 7, 7, payload, sizeof payload) == sizeof payload);
    const uint8_t prefix[] = {
        BIOTRON_SETTINGS_QUERY_ID, BIOTRON_SETTINGS_PROTOCOL_VERSION,
        BIOTRON_SETTINGS_SCHEMA_VERSION, BIOTRON_SETTINGS_SOURCE_PERSISTED,
        42, 1, 7, 0, 0, 0, 0, 7, 0, 0, 0, 0,
    };
    const uint8_t values[] = {
        78, 3, 4, 4, 50, 10, 0, 4, 8, 98, 74, 75, 0, 1, 0, 12,
        0, 0, 1, 1, 0, 1, 60, 2, 3, 100, 0,
    };
    assert(memcmp(payload, prefix, sizeof prefix) == 0);
    assert(memcmp(&payload[sizeof prefix], values, sizeof values) == 0);
    for (size_t byte = 0; byte < sizeof payload; ++byte) {
        assert(payload[byte] <= 0x7f);
    }
}

static void test_invalid_source_is_explicit(void) {
    uint8_t payload[BIOTRON_SETTINGS_PAYLOAD_BYTES];
    memset(payload, 0x7f, sizeof payload);
    assert(biotron_settings_encode(
            NULL, false, true, BIOTRON_SETTINGS_SOURCE_PERSISTED,
            127, UINT32_MAX, 9, payload, sizeof payload) == sizeof payload);
    assert(payload[4] == 127 && payload[5] == 2);
    for (size_t byte = BIOTRON_SETTINGS_HEADER_BYTES;
         byte < sizeof payload; ++byte) {
        assert(payload[byte] == 0);
    }
}

static void test_boundaries_and_rejections(void) {
    Settings_t settings = example();
    settings.BPM = 1;
    settings.lightBPM = -1;
    settings.fibPower = 2.0;
    settings.firstValue = -1.0;
    settings.scale = 200;
    settings.plant_channel = -1;
    settings.light_channel = 99;
    uint8_t payload[BIOTRON_SETTINGS_PAYLOAD_BYTES];
    assert(biotron_settings_encode(
            &settings, true, true, BIOTRON_SETTINGS_SOURCE_RAM,
            255, UINT32_MAX, 0, payload, sizeof payload) == sizeof payload);
    assert(payload[4] == 127 && payload[5] == 3);
    assert(payload[16] == 127 && payload[17] == 127);
    assert(payload[18] == 0);
    assert(payload[20] == 127 && payload[21] == 0);
    assert(payload[23] == 127);
    assert(payload[39] == 1 && payload[40] == 16);
    assert(biotron_settings_encode(
            &settings, true, false, 2, 0, 0, 0, payload, sizeof payload) == 0);
    assert(biotron_settings_encode(
            &settings, true, false, 0, 0, 0, 0, payload,
            sizeof payload - 1) == 0);
    assert(biotron_settings_encode(
            NULL, true, false, 0, 0, 0, 0, payload, sizeof payload) == 0);
}

static void test_supported_web_values_round_trip(void) {
    Settings_t settings = example();
    uint8_t payload[BIOTRON_SETTINGS_PAYLOAD_BYTES];

    for (uint16_t bpm = 1; bpm <= 1000; ++bpm) {
        settings.BPM = BPM_TO_US(bpm);
        encoded_values(&settings, payload);
        assert(decoded_bpm(payload) == bpm);
    }
    for (uint8_t value = 0; value <= 100; ++value) {
        settings.fibPower = (double)value / 100.0;
        settings.firstValue = (double)value / 100.0;
        settings.filterPercent = (double)value / 100.0;
        const uint8_t *values = encoded_values(&settings, payload);
        assert(values[4] == value && values[5] == value && values[6] == value);
    }
    for (int channel = 0; channel < 16; ++channel) {
        settings.plant_channel = channel;
        settings.light_channel = 15 - channel;
        const uint8_t *values = encoded_values(&settings, payload);
        assert(values[23] == channel + 1 && values[24] == 16 - channel);
    }
    for (int value = 0; value <= 127; ++value) {
        settings.minPlantVelocity = value;
        settings.maxPlantVelocity = 127 - value;
        const uint8_t *values = encoded_values(&settings, payload);
        assert(values[8] == value && values[9] == 127 - value);
    }
}

int main(void) {
    test_exact_web_vector();
    test_invalid_source_is_explicit();
    test_boundaries_and_rejections();
    test_supported_web_values_round_trip();
    puts("settings_readback: exact 27-field vector and Web ranges passed");
    return 0;
}
