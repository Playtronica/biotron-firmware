#ifndef BIOTRON_RUNTIME_SAFETY_H
#define BIOTRON_RUNTIME_SAFETY_H

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

static inline int biotron_clamp_int(int value, int minimum, int maximum) {
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

static inline uint32_t biotron_abs_diff_u32(uint32_t left, uint32_t right) {
    return left >= right ? left - right : right - left;
}

static inline uint8_t biotron_midi_7bit(int value) {
    return (uint8_t)biotron_clamp_int(value, 0, 127);
}

static inline uint8_t biotron_midi_channel(int value) {
    return (uint8_t)biotron_clamp_int(value, 0, 15);
}

static inline uint32_t biotron_random_note_jitter(uint32_t random_value,
                                                   bool enabled) {
    return enabled ? random_value % 10u : 0u;
}

static inline uint8_t biotron_effective_light_bpm(int value) {
    return (uint8_t)biotron_clamp_int(value, 1, 127);
}

static inline uint8_t biotron_effective_light_range(int value) {
    return (uint8_t)biotron_clamp_int(value, 1, 127);
}

static inline uint8_t biotron_random_velocity(uint32_t random_value,
                                               int minimum, int maximum) {
    const uint8_t low = biotron_midi_7bit(minimum);
    const uint8_t high = biotron_midi_7bit(maximum);
    if (high <= low) return high;
    return (uint8_t)(low + random_value % ((uint32_t)high - low + 1u));
}

static inline uint16_t biotron_pitch_from_adc(uint32_t adc,
                                               uint32_t adc_maximum) {
    if (adc_maximum == 0) return 0;
    if (adc > adc_maximum) adc = adc_maximum;
    return (uint16_t)(((uint64_t)adc * UINT16_C(0x3fff)) / adc_maximum);
}

static inline uint8_t biotron_pitch_lsb(uint16_t bend) {
    return (uint8_t)(bend & 0x7fu);
}

static inline uint8_t biotron_pitch_msb(uint16_t bend) {
    return (uint8_t)((bend >> 7) & 0x7fu);
}

/* Continue the released Fibonacci counter after its initial subtraction. */
static inline int biotron_fibonacci_counter(int diff, uint32_t average_delta,
                                             double fib_power,
                                             double first_value) {
    double first = 0.0;
    double second = first_value;
    int steps = 1;

    if (average_delta > (uint32_t)INT_MAX) return steps;

    for (unsigned int iterations = 0; iterations < 126; ++iterations) {
        const double fib_part = fib_power * (first + second);
        if (!(fib_part >= (double)INT_MIN && fib_part <= (double)INT_MAX)) {
            break;
        }
        const double threshold = (double)average_delta + fib_part;
        if ((double)diff - threshold <= 0.0) break;

        const int64_t decrement = (int64_t)(int)average_delta +
                (int64_t)(int)fib_part;
        if (decrement <= 0 || decrement > INT_MAX) break;
        diff -= (int)decrement;

        const double next = first + second;
        first = second;
        second = next;
        ++steps;
    }
    return biotron_clamp_int(steps, 1, 127);
}

#endif
