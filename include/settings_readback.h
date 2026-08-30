#ifndef BIOTRON_SETTINGS_READBACK_H
#define BIOTRON_SETTINGS_READBACK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "params.h"

#define BIOTRON_SETTINGS_QUERY_ID 123u
#define BIOTRON_SETTINGS_PROTOCOL_VERSION 1u
#define BIOTRON_SETTINGS_SCHEMA_VERSION 1u
#define BIOTRON_SETTINGS_SOURCE_RAM 0u
#define BIOTRON_SETTINGS_SOURCE_PERSISTED 1u
#define BIOTRON_SETTINGS_HEADER_BYTES 16u
#define BIOTRON_SETTINGS_VECTOR_BYTES 27u
#define BIOTRON_SETTINGS_PAYLOAD_BYTES \
    (BIOTRON_SETTINGS_HEADER_BYTES + BIOTRON_SETTINGS_VECTOR_BYTES)

size_t biotron_settings_encode(const Settings_t *source, bool source_valid,
                               bool dirty, uint8_t source_id,
                               uint8_t request_id,
                               uint32_t dirty_generation,
                               uint32_t persisted_generation,
                               uint8_t output[], size_t output_capacity);

#endif
