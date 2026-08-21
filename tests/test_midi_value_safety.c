#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "midi_value_safety.h"

int main(void) {
    assert(midi_clamp_7bit(-1) == 0);
    assert(midi_clamp_7bit(128) == 127);
    assert(midi_nonzero_range(0) == 1);
    assert(midi_nonzero_range(127) == 127);

    for (int minimum = -10; minimum <= 140; ++minimum) {
        for (int maximum = -10; maximum <= 140; ++maximum) {
            uint8_t expected_low = midi_clamp_7bit(minimum);
            uint8_t expected_high = midi_clamp_7bit(maximum);
            if (expected_low > expected_high) {
                const uint8_t swap = expected_low;
                expected_low = expected_high;
                expected_high = swap;
            }
            for (uint32_t random_value = 0; random_value < 256; ++random_value) {
                const uint8_t velocity = midi_random_velocity(
                        random_value, minimum, maximum);
                assert(velocity >= expected_low && velocity <= expected_high);
            }
        }
    }

    assert(midi_random_velocity(999, 64, 64) == 64);
    assert(midi_random_velocity(0, 127, 0) == 0);
    assert(midi_random_velocity(127, 127, 0) == 127);
    puts("midi_value_safety: clamping, reversed ranges and zero span verified");
    return 0;
}
