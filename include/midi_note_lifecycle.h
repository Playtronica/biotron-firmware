#ifndef BIOTRON_MIDI_NOTE_LIFECYCLE_H
#define BIOTRON_MIDI_NOTE_LIFECYCLE_H

#include <stdint.h>

static inline uintptr_t midi_note_identity_pack(uint8_t channel, uint8_t note) {
    return (uintptr_t)((((uint16_t)channel & 0x0fu) << 7) |
                       ((uint16_t)note & 0x7fu)) + 1u;
}

static inline uint8_t midi_note_identity_channel(uintptr_t identity) {
    return (uint8_t)(((identity - 1u) >> 7) & 0x0fu);
}

static inline uint8_t midi_note_identity_note(uintptr_t identity) {
    return (uint8_t)((identity - 1u) & 0x7fu);
}

#endif
