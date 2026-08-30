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
    assert(high.green[LED_SOURCE_PLANT][2] == LED_ENGINE_MAX_LEVEL);
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
    const led_frame_t attack = render(&engine, 0);
    const led_frame_t faded = render(&engine, 120);
    assert(faded.green[LED_SOURCE_PLANT][1] <
           attack.green[LED_SOURCE_PLANT][1]);
    led_engine_note_on(&engine, LED_SOURCE_PLANT, 48, 96, 36, 60);
    const led_frame_t retriggered = render(&engine, 120);
    assert(retriggered.green[LED_SOURCE_PLANT][1] >
           faded.green[LED_SOURCE_PLANT][1]);
    assert(retriggered.green[LED_SOURCE_PLANT][1] <= LED_ENGINE_MAX_LEVEL);

    led_engine_beat(&engine);
    const led_frame_t beat = render(&engine, 120);
    assert(beat.blue[0] == LED_ENGINE_MAX_LEVEL);
    assert(beat.blue[0] == beat.blue[1]);
    assert(beat.blue[1] == beat.blue[2]);

    led_engine_clear_notes(&engine);
    const led_frame_t muted = render(&engine, 120);
    assert(muted.green[LED_SOURCE_PLANT][1] == 0);
    assert(muted.blue[0] == LED_ENGINE_MAX_LEVEL);

    const led_frame_t idle = render(&engine, 1120);
    for (uint8_t lane = 0; lane < LED_ENGINE_LANES; ++lane) {
        assert(idle.blue[lane] < LED_ENGINE_MAX_LEVEL / 100u);
        assert(idle.green[LED_SOURCE_PLANT][lane] <
               LED_ENGINE_MAX_LEVEL / 100u);
    }
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
    assert(after.green[LED_SOURCE_LIGHT][1] <
           before.green[LED_SOURCE_LIGHT][1]);

    uint32_t random = UINT32_C(0x51f15e);
    uint32_t now_ms = 5;
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
    test_time_wrap_and_random_stress();
    test_invalid_events_are_ignored();
    puts("led_engine: source, pitch, velocity, rhythm and stress passed");
    return 0;
}
