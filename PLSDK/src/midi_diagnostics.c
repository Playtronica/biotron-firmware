#include <stdbool.h>
#include <string.h>

#include "PLSDK/midi_diagnostics.h"

static midi_diagnostics_snapshot_t counters;
static uint64_t last_service_us;
static bool service_seen;

static void increment(uint32_t *value) {
    if (*value != UINT32_MAX) ++*value;
}

void midi_diagnostics_reset(void) {
    memset(&counters, 0, sizeof counters);
    last_service_us = 0;
    service_seen = false;
}

void midi_diagnostics_snapshot(midi_diagnostics_snapshot_t *snapshot) {
    if (snapshot != NULL) *snapshot = counters;
}

void midi_diagnostics_service(uint64_t now_us, uint32_t rx_backlog_bytes) {
    counters.uptime_us = now_us;
    if (service_seen) {
        const uint64_t gap64 = now_us - last_service_us;
        const uint32_t gap = gap64 > UINT32_MAX ? UINT32_MAX :
                (uint32_t)gap64;
        if (gap > counters.midi_service_gap_max_us) {
            counters.midi_service_gap_max_us = gap;
        }
    }
    service_seen = true;
    last_service_us = now_us;
    if (rx_backlog_bytes > counters.rx_backlog_high_water_bytes) {
        counters.rx_backlog_high_water_bytes = rx_backlog_bytes;
    }
}

void midi_diagnostics_set_reset_reason(
        midi_diagnostics_reset_reason_t reset_reason) {
    counters.last_reset_reason = reset_reason;
}

void midi_diagnostics_usb_event(midi_diagnostics_usb_event_t event,
                                bool remote_wakeup_enabled) {
    switch (event) {
        case MIDI_DIAGNOSTICS_USB_MOUNT:
            increment(&counters.usb_mount_count);
            counters.usb_mounted = true;
            counters.usb_suspended = false;
            counters.usb_remote_wakeup_enabled = false;
            break;
        case MIDI_DIAGNOSTICS_USB_UNMOUNT:
            increment(&counters.usb_unmount_count);
            counters.usb_mounted = false;
            counters.usb_suspended = false;
            counters.usb_remote_wakeup_enabled = false;
            break;
        case MIDI_DIAGNOSTICS_USB_SUSPEND:
            increment(&counters.usb_suspend_count);
            counters.usb_suspended = true;
            counters.usb_remote_wakeup_enabled = remote_wakeup_enabled;
            break;
        case MIDI_DIAGNOSTICS_USB_RESUME:
            increment(&counters.usb_resume_count);
            counters.usb_suspended = false;
            counters.usb_remote_wakeup_enabled = false;
            break;
    }
}

void midi_diagnostics_rx_packet(uint8_t cable) {
    if (cable >= MIDI_DIAGNOSTICS_CABLES) {
        increment(&counters.ignored_cable_packets);
        return;
    }
    increment(&counters.usb_packets_rx[cable]);
}

void midi_diagnostics_rx_event(uint8_t cable,
                               midi_diagnostics_rx_kind_t kind) {
    if (cable >= MIDI_DIAGNOSTICS_CABLES) return;
    switch (kind) {
        case MIDI_DIAGNOSTICS_CHANNEL:
            increment(&counters.parsed_channel[cable]);
            break;
        case MIDI_DIAGNOSTICS_SYSTEM_COMMON:
            increment(&counters.parsed_system_common[cable]);
            break;
        case MIDI_DIAGNOSTICS_REALTIME:
            increment(&counters.parsed_realtime[cable]);
            break;
        case MIDI_DIAGNOSTICS_SYSEX:
            increment(&counters.parsed_sysex[cable]);
            break;
        case MIDI_DIAGNOSTICS_MALFORMED:
            increment(&counters.malformed[cable]);
            break;
        case MIDI_DIAGNOSTICS_SYSEX_OVERFLOW:
            increment(&counters.malformed[cable]);
            increment(&counters.sysex_overflow[cable]);
            break;
        case MIDI_DIAGNOSTICS_SYSEX_ABORTED:
            increment(&counters.sysex_aborted[cable]);
            break;
    }
}

void midi_diagnostics_tx_enqueued(size_t queue_depth) {
    increment(&counters.tx_enqueued);
    if (queue_depth > UINT16_MAX) queue_depth = UINT16_MAX;
    if (queue_depth > counters.tx_high_water) {
        counters.tx_high_water = (uint16_t)queue_depth;
    }
}

void midi_diagnostics_tx_enqueue_attempt(void) {
    increment(&counters.tx_enqueue_attempts);
}

void midi_diagnostics_tx_handed_to_tinyusb(void) {
    increment(&counters.tx_handed_to_tinyusb);
}

void midi_diagnostics_tx_coalesced(void) {
    increment(&counters.tx_coalesced);
}

void midi_diagnostics_tx_rejected(void) {
    increment(&counters.tx_rejected);
}

void midi_diagnostics_tx_evicted(void) {
    increment(&counters.tx_evicted);
}

void midi_diagnostics_tx_partial_write(void) {
    increment(&counters.tx_partial_write);
}

void midi_diagnostics_tx_recovery_panic(void) {
    increment(&counters.tx_recovery_panic);
}

void midi_diagnostics_settings_changed(bool dirty) {
    increment(&counters.settings_dirty_generation);
    if (!dirty) {
        counters.settings_persisted_generation =
                counters.settings_dirty_generation;
    }
}

void midi_diagnostics_settings_saved(uint32_t critical_duration_us) {
    counters.settings_persisted_generation =
            counters.settings_dirty_generation;
    increment(&counters.flash_save_count);
    counters.flash_critical_last_us = critical_duration_us;
    if (critical_duration_us > counters.flash_critical_max_us) {
        counters.flash_critical_max_us = critical_duration_us;
    }
}
