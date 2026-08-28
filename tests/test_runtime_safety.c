#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "runtime_safety.h"

static int legacy_progressing_counter(int diff, uint32_t average_delta,
                                      double fib_power, double first_value) {
    double first = 0.0;
    double second = first_value;
    int steps = 1;
    while ((double)diff - (average_delta + fib_power * (first + second)) > 0.0) {
        diff -= (int)average_delta + (int)(fib_power * (first + second));
        const double next = first + second;
        first = second;
        second = next;
        ++steps;
        assert(steps < 127);
    }
    return steps;
}

static void test_midi_domains(void) {
    assert(biotron_midi_7bit(-1) == 0);
    assert(biotron_midi_7bit(128) == 127);
    assert(biotron_midi_channel(-1) == 0);
    assert(biotron_midi_channel(16) == 15);
    assert(biotron_abs_diff_u32(10, 3) == 7);
    assert(biotron_abs_diff_u32(3, 10) == 7);
    assert(biotron_abs_diff_u32(0, UINT32_MAX) == UINT32_MAX);
}

static void test_random_inputs_never_divide_by_zero(void) {
    for (uint32_t value = 0; value < 256; ++value) {
        assert(biotron_random_note_jitter(value, false) == 0);
        assert(biotron_random_note_jitter(value, true) == value % 10u);
        assert(biotron_random_velocity(value, 64, 64) == 64);
        assert(biotron_random_velocity(value, 65, 64) == 64);
        assert(biotron_random_velocity(value, 127, 0) == 0);
        const uint8_t velocity = biotron_random_velocity(value, 10, 20);
        assert(velocity >= 10 && velocity <= 20);
    }
    for (int minimum = -1; minimum <= 128; ++minimum) {
        for (int maximum = -1; maximum <= 128; ++maximum) {
            const uint8_t low = biotron_midi_7bit(minimum);
            const uint8_t high = biotron_midi_7bit(maximum);
            const uint8_t velocity = biotron_random_velocity(
                    UINT32_C(0x12345678), minimum, maximum);
            if (high <= low) assert(velocity == high);
            else assert(velocity >= low && velocity <= high);
        }
    }
}

static void test_light_zero_preserves_every_beat_safely(void) {
    assert(biotron_effective_light_bpm(-1) == 1);
    assert(biotron_effective_light_bpm(0) == 1);
    assert(biotron_effective_light_bpm(1) == 1);
    assert(biotron_effective_light_bpm(2) == 2);
    assert(biotron_effective_light_range(0) == 1);
    assert(biotron_effective_light_range(127) == 127);
}

static void test_pitch_is_valid_14_bit_midi(void) {
    assert(biotron_pitch_from_adc(0, 3200) == 0);
    assert(biotron_pitch_from_adc(1600, 3200) == 8191);
    assert(biotron_pitch_from_adc(3200, 3200) == 16383);
    assert(biotron_pitch_from_adc(9999, 3200) == 16383);
    assert(biotron_pitch_from_adc(1, 0) == 0);
    for (uint32_t adc = 0; adc <= 3200; ++adc) {
        const uint16_t bend = biotron_pitch_from_adc(adc, 3200);
        assert(bend <= 16383);
        assert(biotron_pitch_lsb(bend) <= 127);
        assert(biotron_pitch_msb(bend) <= 127);
    }
    assert(biotron_pitch_lsb(8192) == 0);
    assert(biotron_pitch_msb(8192) == 64);
}

static void test_fibonacci_counter_is_bounded(void) {
    assert(biotron_fibonacci_counter(100, 0, 0.0, 0.0) == 1);
    assert(biotron_fibonacci_counter(100, 0, 0.5, 0.0) == 1);
    assert(biotron_fibonacci_counter(100, 1, 0.0, 0.0) == 100);
    assert(biotron_fibonacci_counter(1000000, 1, 0.0, 0.0) == 127);
    assert(biotron_fibonacci_counter(100, 0, -1.0, 1.0) == 1);
    assert(biotron_fibonacci_counter(100, UINT32_MAX, 1.0, 1.0) == 1);
    assert(biotron_fibonacci_counter(100, 1, INFINITY, 1.0) == 1);

    static const double powers[] = {0.1, 0.5, 1.0};
    static const double first_values[] = {0.1, 1.0, 10.0};
    for (uint32_t average = 1; average <= 20; ++average) {
        for (size_t power = 0; power < sizeof(powers) / sizeof(powers[0]); ++power) {
            for (size_t first = 0;
                 first < sizeof(first_values) / sizeof(first_values[0]); ++first) {
                for (int diff = 0; diff <= 500; diff += 25) {
                    const int legacy = legacy_progressing_counter(
                            diff, average, powers[power], first_values[first]);
                    assert(biotron_fibonacci_counter(
                            diff, average, powers[power], first_values[first]) == legacy);
                }
            }
        }
    }
}

int main(void) {
    test_midi_domains();
    test_random_inputs_never_divide_by_zero();
    test_light_zero_preserves_every_beat_safely();
    test_pitch_is_valid_14_bit_midi();
    test_fibonacci_counter_is_bounded();
    puts("runtime_safety: arithmetic, liveness and MIDI boundaries passed");
    return 0;
}
