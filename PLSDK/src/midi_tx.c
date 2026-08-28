#include <string.h>

#include "PLSDK/constants.h"
#include "PLSDK/midi_tx.h"
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

static bool is_critical_message(const uint8_t data[], uint16_t length) {
    if (length == 0) return false;
    const uint8_t status = data[0] & 0xf0u;
    if (status == NOTE_OFF) return true;
    if (status == NOTE_ON && length >= 3 && data[2] == 0) return true;
    return status == CC_START && length >= 3 &&
           data[1] == CC_STOP_ALL_NOTES;
}

static void remove_message(size_t index) {
    if (index + 1 < queue_count) {
        memmove(&queue[index], &queue[index + 1],
                (queue_count - index - 1) * sizeof queue[0]);
    }
    --queue_count;
}

bool midi_tx_enqueue(uint8_t cable, const uint8_t data[], uint16_t length) {
    if (data == NULL || length == 0 || length > MIDI_TX_MAX_MESSAGE_BYTES) {
        ++dropped_count;
        return false;
    }

    const bool critical = is_critical_message(data, length);
    const size_t noncritical_limit =
            MIDI_TX_QUEUE_CAPACITY - MIDI_TX_RESERVED_CRITICAL;
    if (!critical && queue_count >= noncritical_limit) {
        ++dropped_count;
        return false;
    }
    if (queue_count >= MIDI_TX_QUEUE_CAPACITY) {
        size_t replace = queue_count;
        for (size_t i = 0; i < queue_count; ++i) {
            if (!queue[i].critical) {
                replace = i;
                break;
            }
        }
        if (!critical || replace == queue_count) {
            ++dropped_count;
            return false;
        }
        remove_message(replace);
        ++dropped_count;
    }

    midi_tx_message_t *message = &queue[queue_count++];
    message->length = length;
    message->offset = 0;
    message->cable = cable;
    message->critical = critical;
    memcpy(message->data, data, length);
    return true;
}

void service_midi_tx(void) {
    size_t writes = 0;
    while (queue_count > 0 && writes++ < MIDI_TX_QUEUE_CAPACITY) {
        midi_tx_message_t *message = &queue[0];
        const uint16_t remaining = message->length - message->offset;
        uint32_t written = tud_midi_stream_write(
                message->cable, &message->data[message->offset], remaining);
        if (written == 0) return;
        if (written > remaining) written = remaining;
        message->offset = (uint16_t)(message->offset + written);
        if (message->offset < message->length) return;
        remove_message(0);
    }
}

size_t midi_tx_pending(void) {
    return queue_count;
}

uint32_t midi_tx_dropped(void) {
    return dropped_count;
}
