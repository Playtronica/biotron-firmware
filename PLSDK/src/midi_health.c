#include <stdbool.h>

#include "PLSDK/midi_health.h"

enum {
    FIELD_UPTIME_SECONDS = 1,
    FIELD_USB_FLAGS = 2,
    FIELD_USB_MOUNTS = 3,
    FIELD_USB_UNMOUNTS = 4,
    FIELD_USB_SUSPENDS = 5,
    FIELD_USB_RESUMES = 6,
    FIELD_SERVICE_GAP_MAX_US = 7,
    FIELD_RX_BACKLOG_HIGH_WATER = 8,
    FIELD_FLASH_SAVE_COUNT = 9,
    FIELD_FLASH_CRITICAL_MAX_US = 10,
    FIELD_TX_REJECTED = 11,
    FIELD_TX_RECOVERY_PANIC = 12,
    FIELD_LAST_RESET_REASON = 13,

    FIELD_RX_CABLE0_BASE = 20,
    FIELD_RX_CABLE1_BASE = 40,

    FIELD_TX_ATTEMPTS = 60,
    FIELD_TX_ENQUEUED = 61,
    FIELD_TX_HANDED = 62,
    FIELD_TX_COALESCED = 63,
    FIELD_TX_EVICTED = 64,
    FIELD_TX_PARTIAL = 65,
    FIELD_TX_HIGH_WATER = 66,
    FIELD_DIRTY_GENERATION = 67,
    FIELD_PERSISTED_GENERATION = 68,
    FIELD_FLASH_CRITICAL_LAST_US = 69,
};

typedef struct {
    uint8_t id;
    uint32_t value;
} health_field_t;

static void encode_u32(uint32_t value, uint8_t output[5]) {
    for (size_t byte = 0; byte < 5; ++byte) {
        output[byte] = (uint8_t)(value & 0x7fu);
        value >>= 7u;
    }
}

static uint32_t uptime_seconds(const midi_diagnostics_snapshot_t *snapshot) {
    const uint64_t seconds = snapshot->uptime_us / UINT64_C(1000000);
    return seconds > UINT32_MAX ? UINT32_MAX : (uint32_t)seconds;
}

static uint32_t usb_flags(const midi_diagnostics_snapshot_t *snapshot) {
    return (snapshot->usb_mounted ? 1u : 0u) |
           (snapshot->usb_suspended ? 2u : 0u) |
           (snapshot->usb_remote_wakeup_enabled ? 4u : 0u);
}

static size_t rx_fields(const midi_diagnostics_snapshot_t *snapshot,
                        uint8_t cable, health_field_t fields[8]) {
    const uint8_t base = cable == 0 ? FIELD_RX_CABLE0_BASE :
                                     FIELD_RX_CABLE1_BASE;
    fields[0] = (health_field_t){base + 0u, snapshot->usb_packets_rx[cable]};
    fields[1] = (health_field_t){base + 1u, snapshot->parsed_channel[cable]};
    fields[2] = (health_field_t){base + 2u, snapshot->parsed_system_common[cable]};
    fields[3] = (health_field_t){base + 3u, snapshot->parsed_realtime[cable]};
    fields[4] = (health_field_t){base + 4u, snapshot->parsed_sysex[cable]};
    fields[5] = (health_field_t){base + 5u, snapshot->malformed[cable]};
    fields[6] = (health_field_t){base + 6u, snapshot->sysex_overflow[cable]};
    fields[7] = (health_field_t){base + 7u, snapshot->sysex_aborted[cable]};
    return 8;
}

size_t midi_health_encode_page(const midi_diagnostics_snapshot_t *snapshot,
                               uint8_t page, uint8_t output[],
                               size_t output_capacity) {
    if (snapshot == NULL || output == NULL || page >= MIDI_HEALTH_PAGE_COUNT) {
        return 0;
    }

    health_field_t fields[13];
    size_t field_count = 0;
    switch (page) {
        case 0:
            fields[field_count++] = (health_field_t){FIELD_UPTIME_SECONDS,
                                                     uptime_seconds(snapshot)};
            fields[field_count++] = (health_field_t){FIELD_USB_FLAGS,
                                                     usb_flags(snapshot)};
            fields[field_count++] = (health_field_t){FIELD_USB_MOUNTS,
                                                     snapshot->usb_mount_count};
            fields[field_count++] = (health_field_t){FIELD_USB_UNMOUNTS,
                                                     snapshot->usb_unmount_count};
            fields[field_count++] = (health_field_t){FIELD_USB_SUSPENDS,
                                                     snapshot->usb_suspend_count};
            fields[field_count++] = (health_field_t){FIELD_USB_RESUMES,
                                                     snapshot->usb_resume_count};
            fields[field_count++] = (health_field_t){FIELD_SERVICE_GAP_MAX_US,
                                                     snapshot->midi_service_gap_max_us};
            fields[field_count++] = (health_field_t){FIELD_RX_BACKLOG_HIGH_WATER,
                                                     snapshot->rx_backlog_high_water_bytes};
            fields[field_count++] = (health_field_t){FIELD_FLASH_SAVE_COUNT,
                                                     snapshot->flash_save_count};
            fields[field_count++] = (health_field_t){FIELD_FLASH_CRITICAL_MAX_US,
                                                     snapshot->flash_critical_max_us};
            fields[field_count++] = (health_field_t){FIELD_TX_REJECTED,
                                                     snapshot->tx_rejected};
            fields[field_count++] = (health_field_t){FIELD_TX_RECOVERY_PANIC,
                                                     snapshot->tx_recovery_panic};
            fields[field_count++] = (health_field_t){FIELD_LAST_RESET_REASON,
                                                     snapshot->last_reset_reason};
            break;
        case 1:
            field_count = rx_fields(snapshot, 0, fields);
            break;
        case 2:
            field_count = rx_fields(snapshot, 1, fields);
            break;
        case 3:
            fields[field_count++] = (health_field_t){FIELD_TX_ATTEMPTS,
                                                     snapshot->tx_enqueue_attempts};
            fields[field_count++] = (health_field_t){FIELD_TX_ENQUEUED,
                                                     snapshot->tx_enqueued};
            fields[field_count++] = (health_field_t){FIELD_TX_HANDED,
                                                     snapshot->tx_handed_to_tinyusb};
            fields[field_count++] = (health_field_t){FIELD_TX_COALESCED,
                                                     snapshot->tx_coalesced};
            fields[field_count++] = (health_field_t){FIELD_TX_EVICTED,
                                                     snapshot->tx_evicted};
            fields[field_count++] = (health_field_t){FIELD_TX_PARTIAL,
                                                     snapshot->tx_partial_write};
            fields[field_count++] = (health_field_t){FIELD_TX_HIGH_WATER,
                                                     snapshot->tx_high_water};
            fields[field_count++] = (health_field_t){FIELD_DIRTY_GENERATION,
                                                     snapshot->settings_dirty_generation};
            fields[field_count++] = (health_field_t){FIELD_PERSISTED_GENERATION,
                                                     snapshot->settings_persisted_generation};
            fields[field_count++] = (health_field_t){FIELD_FLASH_CRITICAL_LAST_US,
                                                     snapshot->flash_critical_last_us};
            break;
    }

    const size_t required = 5u + field_count * 6u;
    if (required > output_capacity || required > MIDI_HEALTH_MAX_PAYLOAD_BYTES) {
        return 0;
    }
    output[0] = MIDI_HEALTH_QUERY_ID;
    output[1] = MIDI_HEALTH_PROTOCOL_VERSION;
    output[2] = page;
    output[3] = MIDI_HEALTH_PAGE_COUNT;
    output[4] = (uint8_t)field_count;
    size_t offset = 5;
    for (size_t field = 0; field < field_count; ++field) {
        output[offset++] = fields[field].id;
        encode_u32(fields[field].value, &output[offset]);
        offset += 5;
    }
    return offset;
}
