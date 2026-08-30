#include <string.h>

#include "settings_readback.h"

static uint8_t u7(int value) {
    if (value <= 0) return 0;
    return value >= 127 ? 127 : (uint8_t)value;
}

static uint8_t percent(double value) {
    if (value <= 0.0) return 0;
    if (value >= 1.27) return 127;
    return u7((int)(value * 100.0 + 0.5));
}

static uint8_t human_channel(int stored_channel) {
    if (stored_channel < 0) return 1;
    if (stored_channel >= 15) return 16;
    return (uint8_t)(stored_channel + 1);
}

static void encode_u32(uint32_t value, uint8_t output[5]) {
    for (size_t byte = 0; byte < 5; ++byte) {
        output[byte] = (uint8_t)(value & 0x7fu);
        value >>= 7u;
    }
}

static void encode_values(const Settings_t *settings,
                          uint8_t output[BIOTRON_SETTINGS_VECTOR_BYTES]) {
    uint32_t bpm = settings->BPM > 0 ?
                   UINT32_C(60000000) / (uint32_t)settings->BPM : 0;
    if (bpm > 16383u) bpm = 16383u;

    size_t at = 0;
    output[at++] = (uint8_t)(bpm & 0x7fu);
    output[at++] = (uint8_t)(bpm >> 7u);
    output[at++] = u7(settings->lightBPM);
    output[at++] = u7(settings->fraction_note_off);
    output[at++] = percent(settings->fibPower);
    output[at++] = percent(settings->firstValue);
    output[at++] = percent(settings->filterPercent);
    output[at++] = u7(settings->scale);
    output[at++] = u7(settings->minPlantVelocity);
    output[at++] = u7(settings->maxPlantVelocity);
    output[at++] = u7(settings->minLightVelocity);
    output[at++] = u7(settings->maxLightVelocity);
    output[at++] = settings->random_note ? 1u : 0u;
    output[at++] = u7(settings->same_note_plant);
    output[at++] = u7(settings->same_note_light);
    output[at++] = u7(settings->light_note_range);
    output[at++] = settings->light_pitch_mode ? 1u : 0u;
    output[at++] = settings->isMutePlantVelocity ? 1u : 0u;
    output[at++] = settings->isMuteLightVelocity ? 1u : 0u;
    output[at++] = settings->isRandomPlantVelocity ? 1u : 0u;
    output[at++] = settings->isRandomLightVelocity ? 1u : 0u;
    output[at++] = settings->performance_mode ? 1u : 0u;
    output[at++] = u7(settings->middle_plant_note);
    output[at++] = human_channel(settings->plant_channel);
    output[at++] = human_channel(settings->light_channel);
    output[at++] = u7(settings->swing_first_note_percent);
    output[at] = settings->is_mute_button_active ? 1u : 0u;
}

size_t biotron_settings_encode(const Settings_t *source, bool source_valid,
                               bool dirty, uint8_t source_id,
                               uint8_t request_id,
                               uint32_t dirty_generation,
                               uint32_t persisted_generation,
                               uint8_t output[], size_t output_capacity) {
    if (output == NULL || output_capacity < BIOTRON_SETTINGS_PAYLOAD_BYTES ||
        source_id > BIOTRON_SETTINGS_SOURCE_PERSISTED ||
        (source_valid && source == NULL)) return 0;

    memset(output, 0, BIOTRON_SETTINGS_PAYLOAD_BYTES);
    output[0] = BIOTRON_SETTINGS_QUERY_ID;
    output[1] = BIOTRON_SETTINGS_PROTOCOL_VERSION;
    output[2] = BIOTRON_SETTINGS_SCHEMA_VERSION;
    output[3] = source_id;
    output[4] = request_id & 0x7fu;
    output[5] = (source_valid ? 1u : 0u) | (dirty ? 2u : 0u);
    encode_u32(dirty_generation, &output[6]);
    encode_u32(persisted_generation, &output[11]);
    if (source_valid) {
        encode_values(source, &output[BIOTRON_SETTINGS_HEADER_BYTES]);
    }
    return BIOTRON_SETTINGS_PAYLOAD_BYTES;
}
