#include <string.h>

#include "PLSDK/constants.h"
#include "PLSDK/midi_tx.h"
#include "PLSDK/midi_diagnostics.h"
#include "tusb.h"

typedef struct {
    uint16_t length;
    uint16_t offset;
    uint8_t cable;
    bool critical;
    uint8_t data[MIDI_TX_MAX_MESSAGE_BYTES];
} midi_tx_message_t;

static midi_tx_message_t queue[MIDI_TX_QUEUE_CAPACITY];
static size_t queue_count = 0;
static uint32_t dropped_count = 0;
static bool recovery_panic_pending[16][16];
static size_t recovery_panic_count = 0;

static bool is_critical_message(const uint8_t data[], uint16_t length) {
    if (length == 0) return false;
    const uint8_t status = data[0] & 0xf0u;
    if (status == NOTE_OFF) return true;
    if (status == NOTE_ON && length >= 3 && data[2] == 0) return true;
    return status == CC_START && length >= 3 &&
           data[1] == CC_STOP_ALL_NOTES;
}

static bool is_all_notes_off(const midi_tx_message_t *message) {
    return message->length >= 3 && message->offset == 0 &&
           (message->data[0] & 0xf0u) == CC_START &&
           message->data[1] == CC_STOP_ALL_NOTES;
}

static bool is_note_off_message(const midi_tx_message_t *message) {
    return message->length >= 3 && message->offset == 0 &&
           (message->data[0] & 0xf0u) == NOTE_OFF;
}

static void remove_message(size_t index) {
    if (index + 1 < queue_count) {
        memmove(&queue[index], &queue[index + 1],
                (queue_count - index - 1) * sizeof queue[0]);
    }
    --queue_count;
}

static void request_recovery_panic(uint8_t cable, uint8_t status) {
    if (cable >= 16) return;
    const uint8_t channel = status & 0x0fu;
    if (!recovery_panic_pending[cable][channel]) {
        recovery_panic_pending[cable][channel] = true;
        ++recovery_panic_count;
        midi_diagnostics_tx_recovery_panic();
    }
}

static void append_one_recovery_panic(void) {
    if (queue_count >= MIDI_TX_QUEUE_CAPACITY || recovery_panic_count == 0) {
        return;
    }
    for (uint8_t cable = 0; cable < 16; ++cable) {
        for (uint8_t channel = 0; channel < 16; ++channel) {
            if (!recovery_panic_pending[cable][channel]) continue;
            midi_tx_message_t *message = &queue[queue_count++];
            message->length = 3;
            message->offset = 0;
            message->cable = cable;
            message->critical = true;
            message->data[0] = (uint8_t)(CC_START | channel);
            message->data[1] = CC_STOP_ALL_NOTES;
            message->data[2] = 0;
            recovery_panic_pending[cable][channel] = false;
            --recovery_panic_count;
            midi_diagnostics_tx_enqueued(queue_count);
            return;
        }
    }
}

bool midi_tx_enqueue(uint8_t cable, const uint8_t data[], uint16_t length) {
    midi_diagnostics_tx_enqueue_attempt();
    if (data == NULL || length == 0 || length > MIDI_TX_MAX_MESSAGE_BYTES) {
        ++dropped_count;
        midi_diagnostics_tx_rejected();
        return false;
    }

    const bool critical = is_critical_message(data, length);
    if (critical) {
        for (size_t i = 0; i < queue_count; ++i) {
            const midi_tx_message_t *queued = &queue[i];
            if (queued->offset == 0 && queued->cable == cable &&
                queued->length == length &&
                memcmp(queued->data, data, length) == 0) {
                midi_diagnostics_tx_coalesced();
                return true;
            }
            if ((data[0] & 0xf0u) == NOTE_OFF && is_all_notes_off(queued) &&
                queued->cable == cable &&
                (queued->data[0] & 0x0fu) == (data[0] & 0x0fu)) {
                midi_diagnostics_tx_coalesced();
                return true;
            }
        }
        if ((data[0] & 0xf0u) == CC_START && length >= 3 &&
            data[1] == CC_STOP_ALL_NOTES) {
            for (size_t i = 0; i < queue_count;) {
                const midi_tx_message_t *queued = &queue[i];
                if (is_note_off_message(queued) && queued->cable == cable &&
                    (queued->data[0] & 0x0fu) == (data[0] & 0x0fu)) {
                    remove_message(i);
                    midi_diagnostics_tx_evicted();
                } else {
                    ++i;
                }
            }
        }
    }
    const size_t noncritical_limit =
            MIDI_TX_QUEUE_CAPACITY - MIDI_TX_RESERVED_CRITICAL;
    if (!critical && queue_count >= noncritical_limit) {
        ++dropped_count;
        midi_diagnostics_tx_rejected();
        return false;
    }
    if (queue_count >= MIDI_TX_QUEUE_CAPACITY) {
        size_t replace = queue_count;
        for (size_t i = 0; i < queue_count; ++i) {
            if (!queue[i].critical && queue[i].offset == 0) {
                replace = i;
                break;
            }
        }
        if (!critical || replace == queue_count) {
            ++dropped_count;
            midi_diagnostics_tx_rejected();
            if (critical) request_recovery_panic(cable, data[0]);
            return false;
        }
        remove_message(replace);
        ++dropped_count;
        midi_diagnostics_tx_evicted();
    }

    midi_tx_message_t *message = &queue[queue_count++];
    message->length = length;
    message->offset = 0;
    message->cable = cable;
    message->critical = critical;
    memcpy(message->data, data, length);
    midi_diagnostics_tx_enqueued(queue_count);
    return true;
}

void service_midi_tx(void) {
    append_one_recovery_panic();
    size_t writes = 0;
    while (queue_count > 0 && writes++ < MIDI_TX_QUEUE_CAPACITY) {
        midi_tx_message_t *message = &queue[0];
        const uint16_t remaining = message->length - message->offset;
        uint32_t written = tud_midi_stream_write(
                message->cable, &message->data[message->offset], remaining);
        if (written == 0) return;
        if (written > remaining) written = remaining;
        if (written < remaining) midi_diagnostics_tx_partial_write();
        message->offset = (uint16_t)(message->offset + written);
        if (message->offset < message->length) return;
        remove_message(0);
        midi_diagnostics_tx_handed_to_tinyusb();
        append_one_recovery_panic();
    }
}

size_t midi_tx_pending(void) {
    return queue_count;
}

uint32_t midi_tx_dropped(void) {
    return dropped_count;
}

size_t midi_tx_recovery_pending(void) {
    return recovery_panic_count;
}
