#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "midi_note_lifecycle.h"

int main(void) {
    for (uint8_t channel = 0; channel < 16; ++channel) {
        for (uint8_t note = 0; note < 128; ++note) {
            const uintptr_t identity = midi_note_identity_pack(channel, note);
            assert(identity != 0);
            assert(midi_note_identity_channel(identity) == channel);
            assert(midi_note_identity_note(identity) == note);
        }
    }

    // Values outside MIDI domains fail closed to the valid low bits.
    const uintptr_t identity = midi_note_identity_pack(0xff, 0xff);
    assert(midi_note_identity_channel(identity) == 15);
    assert(midi_note_identity_note(identity) == 127);
    puts("note_lifecycle: exact delayed Note Off identity verified");
    return 0;
}
