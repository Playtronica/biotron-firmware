#ifndef BIOTRON_MIDI_NOTE_LIFECYCLE_H
#define BIOTRON_MIDI_NOTE_LIFECYCLE_H

#include <stdint.h>

// Pico alarms carry a void* user_data value. Encode the exact channel/note in
// that value so a delayed callback never reads a newer global last-note value.
// The marker bit guarantees non-NULL user_data for channel 0, note 0.
#define MIDI_NOTE_IDENTITY_MARKER ((uintptr_t)1u << 16)

static inline uintptr_t midi_note_identity_pack(uint8_t channel, uint8_t note) {
    return MIDI_NOTE_IDENTITY_MARKER |
           ((uintptr_t)(channel & 0x0fu) << 8) |
           (uintptr_t)(note & 0x7fu);
}

static inline uint8_t midi_note_identity_channel(uintptr_t identity) {
    return (uint8_t)((identity >> 8) & 0x0fu);
}

static inline uint8_t midi_note_identity_note(uintptr_t identity) {
    return (uint8_t)(identity & 0x7fu);
}

#endif
