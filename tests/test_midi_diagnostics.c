#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "PLSDK/midi_diagnostics.h"

int main(void) {
    midi_diagnostics_reset();
    midi_diagnostics_service((uint64_t)UINT32_MAX - 5u, 12);
    midi_diagnostics_service((uint64_t)UINT32_MAX + 10u, 48);
    midi_diagnostics_usb_event(MIDI_DIAGNOSTICS_USB_MOUNT, false);
    midi_diagnostics_usb_event(MIDI_DIAGNOSTICS_USB_SUSPEND, true);
    midi_diagnostics_usb_event(MIDI_DIAGNOSTICS_USB_RESUME, false);
    midi_diagnostics_usb_event(MIDI_DIAGNOSTICS_USB_UNMOUNT, false);
    midi_diagnostics_rx_packet(0);
    midi_diagnostics_rx_packet(1);
    midi_diagnostics_rx_packet(2);
    midi_diagnostics_rx_event(0, MIDI_DIAGNOSTICS_CHANNEL);
    midi_diagnostics_rx_event(0, MIDI_DIAGNOSTICS_SYSTEM_COMMON);
    midi_diagnostics_rx_event(0, MIDI_DIAGNOSTICS_REALTIME);
    midi_diagnostics_rx_event(1, MIDI_DIAGNOSTICS_SYSEX);
    midi_diagnostics_rx_event(1, MIDI_DIAGNOSTICS_SYSEX_OVERFLOW);
    midi_diagnostics_rx_event(1, MIDI_DIAGNOSTICS_SYSEX_ABORTED);
    midi_diagnostics_tx_enqueue_attempt();
    midi_diagnostics_tx_enqueue_attempt();
    midi_diagnostics_tx_enqueued(3);
    midi_diagnostics_tx_enqueued(19);
    midi_diagnostics_tx_handed_to_tinyusb();
    midi_diagnostics_tx_coalesced();
    midi_diagnostics_tx_rejected();
    midi_diagnostics_tx_evicted();
    midi_diagnostics_tx_partial_write();
    midi_diagnostics_tx_recovery_panic();
    midi_diagnostics_settings_changed(true);
    midi_diagnostics_settings_changed(true);
    midi_diagnostics_settings_saved(41);
    midi_diagnostics_settings_saved(17);

    midi_diagnostics_snapshot_t snapshot;
    midi_diagnostics_snapshot(&snapshot);
    assert(snapshot.midi_service_gap_max_us == 15);
    assert(snapshot.rx_backlog_high_water_bytes == 48);
    assert(snapshot.uptime_us == (uint64_t)UINT32_MAX + 10u);
    assert(snapshot.usb_mount_count == 1);
    assert(snapshot.usb_unmount_count == 1);
    assert(snapshot.usb_suspend_count == 1);
    assert(snapshot.usb_resume_count == 1);
    assert(!snapshot.usb_mounted);
    assert(!snapshot.usb_suspended);
    assert(!snapshot.usb_remote_wakeup_enabled);
    assert(snapshot.usb_packets_rx[0] == 1);
    assert(snapshot.usb_packets_rx[1] == 1);
    assert(snapshot.ignored_cable_packets == 1);
    assert(snapshot.parsed_channel[0] == 1);
    assert(snapshot.parsed_system_common[0] == 1);
    assert(snapshot.parsed_realtime[0] == 1);
    assert(snapshot.parsed_sysex[1] == 1);
    assert(snapshot.malformed[1] == 1);
    assert(snapshot.sysex_overflow[1] == 1);
    assert(snapshot.sysex_aborted[1] == 1);
    assert(snapshot.tx_enqueue_attempts == 2);
    assert(snapshot.tx_enqueued == 2);
    assert(snapshot.tx_handed_to_tinyusb == 1);
    assert(snapshot.tx_coalesced == 1);
    assert(snapshot.tx_rejected == 1);
    assert(snapshot.tx_evicted == 1);
    assert(snapshot.tx_partial_write == 1);
    assert(snapshot.tx_recovery_panic == 1);
    assert(snapshot.tx_high_water == 19);
    assert(snapshot.settings_dirty_generation == 2);
    assert(snapshot.settings_persisted_generation == 2);
    assert(snapshot.flash_save_count == 2);
    assert(snapshot.flash_critical_last_us == 17);
    assert(snapshot.flash_critical_max_us == 41);

    midi_diagnostics_settings_changed(true);
    midi_diagnostics_snapshot(&snapshot);
    assert(snapshot.settings_dirty_generation == 3);
    assert(snapshot.settings_persisted_generation == 2);
    midi_diagnostics_settings_changed(false);
    midi_diagnostics_snapshot(&snapshot);
    assert(snapshot.settings_dirty_generation == 4);
    assert(snapshot.settings_persisted_generation == 4);

    midi_diagnostics_reset();
    midi_diagnostics_snapshot(&snapshot);
    assert(snapshot.uptime_us == 0);
    assert(snapshot.usb_mount_count == 0);
    assert(snapshot.usb_packets_rx[0] == 0);
    assert(snapshot.tx_enqueued == 0);
    assert(snapshot.flash_save_count == 0);
    puts("midi_diagnostics: counters, wrap and reset passed");
    return 0;
}
