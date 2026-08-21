#ifndef BIOTRON_MIDI_VALUE_SAFETY_H
#define BIOTRON_MIDI_VALUE_SAFETY_H

#include <stdint.h>

static inline uint8_t midi_clamp_7bit(int value) {
    if (value < 0) return 0;
    if (value > 127) return 127;
    return (uint8_t)value;
}

static inline uint8_t midi_nonzero_range(int value) {
    const uint8_t clamped = midi_clamp_7bit(value);
    return clamped == 0 ? 1 : clamped;
}

static inline uint8_t midi_random_velocity(uint32_t random_value,
                                           int minimum, int maximum) {
    uint8_t low = midi_clamp_7bit(minimum);
    uint8_t high = midi_clamp_7bit(maximum);
    if (low > high) {
        const uint8_t swap = low;
        low = high;
        high = swap;
    }
    return (uint8_t)(low + random_value % (uint32_t)(high - low + 1));
}

#endif
