#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "PLSDK/constants.h"
#include "PLSDK/midi_tx.h"
#include "PLSDK/midi_diagnostics.h"

static uint8_t captured[4096];
static size_t captured_length = 0;
static uint32_t write_limit = UINT32_MAX;
static bool block_writes = false;

uint32_t tud_midi_stream_write(uint8_t cable, const uint8_t *data,
                               uint32_t length) {
    (void)cable;
    if (block_writes) return 0;
    const uint32_t written = length < write_limit ? length : write_limit;
    assert(captured_length + written <= sizeof captured);
    memcpy(&captured[captured_length], data, written);
    captured_length += written;
    return written;
}

static void drain(void) {
    for (size_t i = 0; i < 128 && midi_tx_pending() > 0; ++i) {
        service_midi_tx();
    }
    assert(midi_tx_pending() == 0);
}

int main(void) {
    midi_diagnostics_reset();
    const uint8_t sysex[] = {0xf0, 0x14, 0x0d, 0x7e, 0x01, 0xf7};
    write_limit = 2;
    assert(midi_tx_enqueue(1, sysex, sizeof sysex));
    drain();
    assert(captured_length == sizeof sysex);
    assert(memcmp(captured, sysex, sizeof sysex) == 0);

    const uint8_t note_on[] = {0x90, 60, 100};
    const uint8_t note_off[] = {0x80, 60, 0};
    block_writes = true;
    assert(midi_tx_enqueue(0, note_off, sizeof note_off));
    service_midi_tx();
    assert(midi_tx_pending() == 1);
    assert(midi_tx_enqueue(0, note_off, sizeof note_off));
    assert(midi_tx_pending() == 1);
    block_writes = false;
    write_limit = UINT32_MAX;
    drain();

    block_writes = true;
    assert(midi_tx_enqueue(0, note_on, sizeof note_on));
    service_midi_tx();
    assert(midi_tx_pending() == 1);
    block_writes = false;
    write_limit = UINT32_MAX;
    drain();

    captured_length = 0;
    write_limit = 1;
    assert(midi_tx_enqueue(0, note_on, sizeof note_on));
    service_midi_tx();
    assert(midi_tx_pending() == 1);
    block_writes = true;
    for (size_t i = 0;
         i < MIDI_TX_QUEUE_CAPACITY - MIDI_TX_RESERVED_CRITICAL - 1; ++i) {
        assert(midi_tx_enqueue(0, note_on, sizeof note_on));
    }
    for (size_t i = 0; i < MIDI_TX_RESERVED_CRITICAL; ++i) {
        const uint8_t distinct_off[] = {0x80, (uint8_t)(60 + i), 0};
        assert(midi_tx_enqueue(0, distinct_off, sizeof distinct_off));
    }
    const uint32_t partial_drops = midi_tx_dropped();
    const uint8_t newest_off[] = {0x80, 70, 0};
    assert(midi_tx_enqueue(0, newest_off, sizeof newest_off));
    assert(midi_tx_dropped() == partial_drops + 1);
    block_writes = false;
    write_limit = UINT32_MAX;
    drain();
    assert(captured_length >= sizeof note_on);
    assert(memcmp(captured, note_on, sizeof note_on) == 0);

    block_writes = true;
    const uint32_t drops_before = midi_tx_dropped();
    for (size_t i = 0; i < MIDI_TX_QUEUE_CAPACITY - MIDI_TX_RESERVED_CRITICAL;
         ++i) {
        assert(midi_tx_enqueue(0, note_on, sizeof note_on));
    }
    assert(!midi_tx_enqueue(0, note_on, sizeof note_on));
    for (size_t i = 0; i < MIDI_TX_RESERVED_CRITICAL; ++i) {
        const uint8_t distinct_off[] = {0x80, (uint8_t)(80 + i), 0};
        assert(midi_tx_enqueue(0, distinct_off, sizeof distinct_off));
    }
    assert(midi_tx_pending() == MIDI_TX_QUEUE_CAPACITY);
    assert(midi_tx_dropped() == drops_before + 1);
    block_writes = false;
    drain();

    block_writes = true;
    for (size_t i = 0; i < MIDI_TX_QUEUE_CAPACITY; ++i) {
        const uint8_t distinct_off[] = {0x80, (uint8_t)i, 0};
        assert(midi_tx_enqueue(0, distinct_off, sizeof distinct_off));
    }
    const uint8_t saturated_off[] = {0x81, 100, 0};
    assert(!midi_tx_enqueue(0, saturated_off, sizeof saturated_off));
    assert(midi_tx_recovery_pending() == 1);
    block_writes = false;
    drain();
    assert(midi_tx_recovery_pending() == 0);

    assert(!midi_tx_enqueue(0, NULL, 0));
    assert(midi_tx_dropped() == drops_before + 3);
    midi_diagnostics_snapshot_t diagnostics;
    midi_diagnostics_snapshot(&diagnostics);
    assert(diagnostics.tx_enqueue_attempts > diagnostics.tx_enqueued);
    assert(diagnostics.tx_enqueued > 0);
    assert(diagnostics.tx_handed_to_tinyusb > 0);
    assert(diagnostics.tx_coalesced > 0);
    assert(diagnostics.tx_rejected + diagnostics.tx_evicted ==
           midi_tx_dropped());
    assert(diagnostics.tx_enqueued == diagnostics.tx_handed_to_tinyusb +
           diagnostics.tx_evicted + midi_tx_pending());
    assert(diagnostics.tx_partial_write > 0);
    assert(diagnostics.tx_recovery_panic > 0);
    assert(diagnostics.tx_high_water == MIDI_TX_QUEUE_CAPACITY);
    puts("midi_tx: partial writes, backpressure and Note Off reserve passed");
    return 0;
}
