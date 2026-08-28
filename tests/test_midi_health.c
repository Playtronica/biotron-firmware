#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "PLSDK/midi_health.h"

static uint32_t decode_u32(const uint8_t encoded[5]) {
    uint32_t value = 0;
    for (int byte = 4; byte >= 0; --byte) {
        assert(encoded[byte] < 0x80);
        value = (value << 7u) | encoded[byte];
    }
    return value;
}

static uint32_t field_value(const uint8_t payload[], size_t length,
                            uint8_t id) {
    assert(length >= 5);
    for (size_t field = 0; field < payload[4]; ++field) {
        const size_t offset = 5u + field * 6u;
        assert(offset + 6u <= length);
        if (payload[offset] == id) return decode_u32(&payload[offset + 1]);
    }
    assert(!"missing field");
    return 0;
}

int main(void) {
    midi_diagnostics_snapshot_t snapshot = {0};
    snapshot.uptime_us = UINT64_C(123456789000000);
    snapshot.last_reset_reason = MIDI_DIAGNOSTICS_RESET_FORCED_OR_BOOTROM;
    snapshot.usb_mount_count = 3;
    snapshot.usb_unmount_count = 2;
    snapshot.usb_suspend_count = 7;
    snapshot.usb_resume_count = 6;
    snapshot.usb_mounted = true;
    snapshot.usb_suspended = true;
    snapshot.usb_remote_wakeup_enabled = true;
    snapshot.midi_service_gap_max_us = UINT32_MAX;
    snapshot.rx_backlog_high_water_bytes = 64;
    snapshot.usb_packets_rx[0] = 1000;
    snapshot.parsed_channel[1] = 500;
    snapshot.tx_enqueue_attempts = 900;
    snapshot.tx_high_water = 16;
    snapshot.settings_dirty_generation = 9;
    snapshot.settings_persisted_generation = 8;

    uint8_t payload[MIDI_HEALTH_MAX_PAYLOAD_BYTES];
    for (uint8_t page = 0; page < MIDI_HEALTH_PAGE_COUNT; ++page) {
        const size_t length = midi_health_encode_page(
                &snapshot, page, payload, sizeof payload);
        assert(length >= 5 && length <= sizeof payload);
        assert(payload[0] == MIDI_HEALTH_QUERY_ID);
        assert(payload[1] == MIDI_HEALTH_PROTOCOL_VERSION);
        assert(payload[2] == page);
        assert(payload[3] == MIDI_HEALTH_PAGE_COUNT);
        for (size_t byte = 0; byte < length; ++byte) assert(payload[byte] < 0x80);
    }

    size_t length = midi_health_encode_page(&snapshot, 0, payload,
                                             sizeof payload);
    assert(field_value(payload, length, 1) == 123456789u);
    assert(field_value(payload, length, 2) == 7u);
    assert(field_value(payload, length, 3) == 3u);
    assert(field_value(payload, length, 7) == UINT32_MAX);
    assert(field_value(payload, length, 13) ==
           MIDI_DIAGNOSTICS_RESET_FORCED_OR_BOOTROM);

    length = midi_health_encode_page(&snapshot, 1, payload, sizeof payload);
    assert(field_value(payload, length, 20) == 1000u);
    length = midi_health_encode_page(&snapshot, 2, payload, sizeof payload);
    assert(field_value(payload, length, 41) == 500u);
    length = midi_health_encode_page(&snapshot, 3, payload, sizeof payload);
    assert(field_value(payload, length, 60) == 900u);
    assert(field_value(payload, length, 66) == 16u);
    assert(field_value(payload, length, 67) == 9u);
    assert(field_value(payload, length, 68) == 8u);

    assert(midi_health_encode_page(&snapshot, MIDI_HEALTH_PAGE_COUNT,
                                   payload, sizeof payload) == 0);
    assert(midi_health_encode_page(&snapshot, 0, payload, 4) == 0);
    assert(midi_health_encode_page(NULL, 0, payload, sizeof payload) == 0);
    puts("midi_health: four 7-bit read-only pages passed");
    return 0;
}
