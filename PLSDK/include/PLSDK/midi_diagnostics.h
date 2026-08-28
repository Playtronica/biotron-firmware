#ifndef PLSDK_MIDI_DIAGNOSTICS_H
#define PLSDK_MIDI_DIAGNOSTICS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MIDI_DIAGNOSTICS_CABLES 2u

typedef enum {
    MIDI_DIAGNOSTICS_CHANNEL = 0,
    MIDI_DIAGNOSTICS_SYSTEM_COMMON,
    MIDI_DIAGNOSTICS_REALTIME,
    MIDI_DIAGNOSTICS_SYSEX,
    MIDI_DIAGNOSTICS_MALFORMED,
    MIDI_DIAGNOSTICS_SYSEX_OVERFLOW,
    MIDI_DIAGNOSTICS_SYSEX_ABORTED,
} midi_diagnostics_rx_kind_t;

typedef struct {
    uint32_t usb_packets_rx[MIDI_DIAGNOSTICS_CABLES];
    uint32_t parsed_channel[MIDI_DIAGNOSTICS_CABLES];
    uint32_t parsed_system_common[MIDI_DIAGNOSTICS_CABLES];
    uint32_t parsed_realtime[MIDI_DIAGNOSTICS_CABLES];
    uint32_t parsed_sysex[MIDI_DIAGNOSTICS_CABLES];
    uint32_t malformed[MIDI_DIAGNOSTICS_CABLES];
    uint32_t sysex_overflow[MIDI_DIAGNOSTICS_CABLES];
    uint32_t sysex_aborted[MIDI_DIAGNOSTICS_CABLES];
    uint32_t ignored_cable_packets;
    uint32_t rx_backlog_high_water_bytes;
    uint32_t midi_service_gap_max_us;
    uint32_t tx_enqueue_attempts;
    uint32_t tx_enqueued;
    uint32_t tx_handed_to_tinyusb;
    uint32_t tx_coalesced;
    uint32_t tx_rejected;
    uint32_t tx_evicted;
    uint32_t tx_partial_write;
    uint32_t tx_recovery_panic;
    uint16_t tx_high_water;
    uint32_t settings_dirty_generation;
    uint32_t settings_persisted_generation;
    uint32_t flash_save_count;
    uint32_t flash_critical_last_us;
    uint32_t flash_critical_max_us;
} midi_diagnostics_snapshot_t;

/*
 * All counters are RAM-only and are updated from the main execution context.
 * settings_dirty_generation is an accepted mutation-event generation.
 * A successful save, a same-value command or a revert to the persisted value
 * copies it to settings_persisted_generation, so equality means RAM is clean.
 * TX "handed" means that TinyUSB accepted the complete message into its FIFO;
 * it is not evidence that bytes reached a host. With counters unsaturated and
 * no reset during traffic: enqueued = handed + evicted + current queue depth.
 */

void midi_diagnostics_reset(void);
void midi_diagnostics_snapshot(midi_diagnostics_snapshot_t *snapshot);

void midi_diagnostics_service(uint32_t now_us, uint32_t rx_backlog_bytes);
void midi_diagnostics_rx_packet(uint8_t cable);
void midi_diagnostics_rx_event(uint8_t cable,
                               midi_diagnostics_rx_kind_t kind);

void midi_diagnostics_tx_enqueue_attempt(void);
void midi_diagnostics_tx_enqueued(size_t queue_depth);
void midi_diagnostics_tx_handed_to_tinyusb(void);
void midi_diagnostics_tx_coalesced(void);
void midi_diagnostics_tx_rejected(void);
void midi_diagnostics_tx_evicted(void);
void midi_diagnostics_tx_partial_write(void);
void midi_diagnostics_tx_recovery_panic(void);

void midi_diagnostics_settings_changed(bool dirty);
void midi_diagnostics_settings_saved(uint32_t critical_duration_us);

#endif
