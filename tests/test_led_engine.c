#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#include "led_engine.h"

static led_frame_t render(led_engine_t *engine, uint32_t now_ms) {
    led_frame_t frame = {0};
    assert(led_engine_service(engine, now_ms, &frame));
    return frame;
}

static void assert_bounded(const led_frame_t *frame) {
    for (uint8_t lane = 0; lane < LED_ENGINE_LANES; ++lane) {
        assert(frame->blue[lane] <= LED_ENGINE_MAX_LEVEL);
        for (uint8_t source = 0; source < LED_ENGINE_SOURCE_COUNT; ++source) {
            assert(frame->green[source][lane] <= LED_ENGINE_MAX_LEVEL);
        }
    }
}

static void test_source_pitch_and_velocity(void) {
    led_engine_t engine;
    led_engine_init(&engine, 100);
    (void)render(&engine, 100);

    led_engine_note_on(&engine, LED_SOURCE_LIGHT, 36, 16, 36, 60);
    led_frame_t low = render(&engine, 100);
    assert(low.green[LED_SOURCE_LIGHT][0] > 0);
    assert(low.green[LED_SOURCE_LIGHT][1] == 0);
    assert(low.green[LED_SOURCE_PLANT][0] == 0);

    led_engine_reset(&engine, 100);
    (void)render(&engine, 100);
    led_engine_note_on(&engine, LED_SOURCE_PLANT, 60, 127, 36, 60);
    led_frame_t high = render(&engine, 100);
    assert(high.green[LED_SOURCE_PLANT][2] > 0);
    assert(high.green[LED_SOURCE_PLANT][2] < LED_ENGINE_MAX_LEVEL);
    assert(high.green[LED_SOURCE_LIGHT][2] == 0);

    uint16_t previous = 0;
    for (uint8_t velocity = 1; velocity < 128; ++velocity) {
        led_engine_reset(&engine, 100);
        (void)render(&engine, 100);
        led_engine_note_on(&engine, LED_SOURCE_PLANT, 48, velocity, 36, 60);
        const led_frame_t frame = render(&engine, 100);
        assert(frame.green[LED_SOURCE_PLANT][1] >= previous);
        previous = frame.green[LED_SOURCE_PLANT][1];
    }
}

static void test_retrigger_decay_and_beat(void) {
    led_engine_t engine;
    led_engine_init(&engine, 0);
    (void)render(&engine, 0);

    led_engine_note_on(&engine, LED_SOURCE_PLANT, 48, 96, 36, 60);
    const led_frame_t inhale = render(&engine, 0);
    const led_frame_t peak = render(&engine, 32);
    const led_frame_t faded = render(&engine, 320);
    assert(peak.green[LED_SOURCE_PLANT][1] >
           inhale.green[LED_SOURCE_PLANT][1]);
    assert(faded.green[LED_SOURCE_PLANT][1] <
           peak.green[LED_SOURCE_PLANT][1]);
    led_engine_note_on(&engine, LED_SOURCE_PLANT, 48, 96, 36, 60);
    const led_frame_t retriggered = render(&engine, 320);
    assert(retriggered.green[LED_SOURCE_PLANT][1] >
           faded.green[LED_SOURCE_PLANT][1]);
    assert(retriggered.green[LED_SOURCE_PLANT][1] <= LED_ENGINE_MAX_LEVEL);

    led_engine_beat(&engine);
    const led_frame_t beat = render(&engine, 320);
    assert(beat.blue[0] > 0);
    assert(beat.blue[0] < LED_ENGINE_MAX_LEVEL);
    assert(beat.blue[0] == beat.blue[1]);
    assert(beat.blue[1] == beat.blue[2]);

    led_engine_clear_notes(&engine);
    const led_frame_t muted = render(&engine, 320);
    assert(muted.green[LED_SOURCE_PLANT][1] == 0);
    assert(muted.blue[0] == beat.blue[0]);

    const led_frame_t idle = render(&engine, 2320);
    for (uint8_t lane = 0; lane < LED_ENGINE_LANES; ++lane) {
        assert(idle.blue[lane] < LED_ENGINE_MAX_LEVEL / 100u);
        assert(idle.green[LED_SOURCE_PLANT][lane] <
               LED_ENGINE_MAX_LEVEL / 100u);
    }
}

static uint32_t repeated_note_brightness(uint32_t interval_ms) {
    led_engine_t engine;
    led_engine_init(&engine, 0);
    (void)render(&engine, 0);
    led_frame_t frame = {0};
    uint32_t total = 0;
    uint32_t next_note = 0;
    for (uint32_t now = 0; now <= 2000; now += LED_ENGINE_TICK_MS) {
        if (now >= next_note) {
            led_engine_note_on(&engine, LED_SOURCE_PLANT, 48, 72, 36, 60);
            next_note += interval_ms;
        }
        if (led_engine_service(&engine, now, &frame)) {
            total += frame.green[LED_SOURCE_PLANT][1];
        }
    }
    return total;
}

static void test_note_rate_shapes_brightness(void) {
    const uint32_t slow = repeated_note_brightness(500);
    const uint32_t medium = repeated_note_brightness(250);
    const uint32_t fast = repeated_note_brightness(125);
    assert(medium > slow);
    assert(fast > medium);
}

static void test_breath_timing_contract(void) {
    led_engine_t engine;
    led_engine_init(&engine, 0);
    (void)render(&engine, 0);
    led_engine_note_on(&engine, LED_SOURCE_PLANT, 48, 127, 36, 60);
    const uint16_t first = render(&engine, 0).green[LED_SOURCE_PLANT][1];
    const uint16_t attack = render(&engine, 16).green[LED_SOURCE_PLANT][1];
    const uint16_t release = render(&engine, 320).green[LED_SOURCE_PLANT][1];
    const uint16_t dark = render(&engine, 1200).green[LED_SOURCE_PLANT][1];
    assert(first > 0 && first < attack);
    assert(attack > LED_ENGINE_MAX_LEVEL * 4u / 5u);
    assert(release > 0 && release < attack);
    assert(dark == 0);

    led_engine_reset(&engine, 0);
    (void)render(&engine, 0);
    led_engine_beat(&engine);
    const uint16_t beat_first = render(&engine, 0).blue[0];
    const uint16_t beat_peak = render(&engine, 40).blue[0];
    const uint16_t beat_dark = render(&engine, 600).blue[0];
    assert(beat_first > 0 && beat_first < beat_peak);
    assert(beat_peak < LED_ENGINE_MAX_LEVEL);
    assert(beat_dark == 0);
}

static void test_frame_drop_does_not_change_envelope(void) {
    led_engine_t fine;
    led_engine_t coarse;
    led_engine_init(&fine, 0);
    led_engine_init(&coarse, 0);
    (void)render(&fine, 0);
    (void)render(&coarse, 0);
    led_engine_note_on(&fine, LED_SOURCE_PLANT, 48, 100, 36, 60);
    led_engine_note_on(&coarse, LED_SOURCE_PLANT, 48, 100, 36, 60);
    led_engine_beat(&fine);
    led_engine_beat(&coarse);
    (void)render(&fine, 0);
    (void)render(&coarse, 0);

    led_frame_t fine_frame = {0};
    for (uint32_t now = LED_ENGINE_TICK_MS; now <= 400;
         now += LED_ENGINE_TICK_MS) {
        assert(led_engine_service(&fine, now, &fine_frame));
    }
    const led_frame_t coarse_frame = render(&coarse, 400);
    assert(fine_frame.green[LED_SOURCE_PLANT][1] ==
           coarse_frame.green[LED_SOURCE_PLANT][1]);
    assert(fine_frame.blue[0] == coarse_frame.blue[0]);
}

static void test_invalid_events_are_ignored(void) {
    led_engine_t engine;
    led_engine_init(&engine, 0);
    (void)render(&engine, 0);

    led_engine_note_on(&engine, LED_SOURCE_PLANT, 48, 0, 36, 60);
    led_engine_note_on(&engine, (led_source_t)-1, 48, 127, 36, 60);
    led_engine_note_on(&engine, (led_source_t)LED_ENGINE_SOURCE_COUNT,
                       48, 127, 36, 60);
    led_frame_t frame = {0};
    assert(!led_engine_service(&engine, 0, &frame));
}

static void test_time_wrap_and_random_stress(void) {
    led_engine_t engine;
    led_engine_init(&engine, UINT32_MAX - 6u);
    (void)render(&engine, UINT32_MAX - 6u);
    led_engine_note_on(&engine, LED_SOURCE_LIGHT, 48, 127, 36, 60);
    const led_frame_t before = render(&engine, UINT32_MAX - 6u);
    const led_frame_t after = render(&engine, 5u);
    assert(after.green[LED_SOURCE_LIGHT][1] >
           before.green[LED_SOURCE_LIGHT][1]);
    const led_frame_t settled = render(&engine, 1500u);
    assert(settled.green[LED_SOURCE_LIGHT][1] <
           after.green[LED_SOURCE_LIGHT][1]);

    uint32_t random = UINT32_C(0x51f15e);
    uint32_t now_ms = 1500;
    led_frame_t frame = {0};
    for (uint32_t event = 0; event < UINT32_C(1000000); ++event) {
        random = random * UINT32_C(1664525) + UINT32_C(1013904223);
        const led_source_t source = (led_source_t)(random & 1u);
        led_engine_note_on(&engine, source,
                           (uint8_t)((random >> 8) & 0x7fu),
                           (uint8_t)(((random >> 16) % 127u) + 1u),
                           24, 96);
        if ((event & 7u) == 0) led_engine_beat(&engine);
        now_ms += event % 3u;
        if (led_engine_service(&engine, now_ms, &frame)) {
            assert_bounded(&frame);
        }
    }
}

int main(void) {
    test_source_pitch_and_velocity();
    test_retrigger_decay_and_beat();
    test_note_rate_shapes_brightness();
    test_breath_timing_contract();
    test_frame_drop_does_not_change_envelope();
    test_time_wrap_and_random_stress();
    test_invalid_events_are_ignored();
    puts("led_engine: source, pitch, velocity, rhythm and stress passed");
    return 0;
}
