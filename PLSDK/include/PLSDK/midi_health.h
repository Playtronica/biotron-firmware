#ifndef PLSDK_MIDI_HEALTH_H
#define PLSDK_MIDI_HEALTH_H

#include <stddef.h>
#include <stdint.h>

#include "PLSDK/midi_diagnostics.h"

#define MIDI_HEALTH_QUERY_ID 124u
#define MIDI_HEALTH_PROTOCOL_VERSION 1u
#define MIDI_HEALTH_PAGE_COUNT 4u
#define MIDI_HEALTH_MAX_PAYLOAD_BYTES 96u

/*
 * Read-only diagnostic payload carried inside the existing Playtronica vendor
 * SysEx envelope. All bytes remain 7-bit clean:
 *
 *   124, version, page, page_count, field_count,
 *   repeated { field_id, uint32 little-endian base-128 (5 bytes) }
 *
 * Unknown pages and undersized output buffers return zero and emit nothing.
 */
size_t midi_health_encode_page(const midi_diagnostics_snapshot_t *snapshot,
                               uint8_t page, uint8_t output[],
                               size_t output_capacity);

#endif
