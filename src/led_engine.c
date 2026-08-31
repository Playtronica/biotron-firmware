#include <stddef.h>
#include <string.h>

#include "led_engine.h"

#define NOTE_MIN_ATTACK UINT16_C(9000)
#define NOTE_TARGET_DECAY_FLOOR UINT16_C(96)
#define NOTE_LEVEL_DECAY_FLOOR UINT16_C(48)
#define BEAT_LEVEL LED_ENGINE_MAX_LEVEL
#define BEAT_TARGET_DECAY_FLOOR UINT16_C(256)
#define BEAT_LEVEL_DECAY_FLOOR UINT16_C(96)
#define MAX_DECAY_STEPS UINT32_C(256)

_Static_assert(sizeof(led_engine_t) <= 128,
               "LED engine must remain a small static main-loop state");

static uint16_t saturating_add(uint16_t current, uint16_t addition) {
    const uint32_t sum = (uint32_t)current + addition;
    return sum > LED_ENGINE_MAX_LEVEL ? LED_ENGINE_MAX_LEVEL :
            (uint16_t)sum;
}

static uint16_t note_attack(uint8_t velocity) {
    const uint32_t value = velocity == 0 ? 0 : velocity;
    const uint32_t dynamic_range = LED_ENGINE_MAX_LEVEL - NOTE_MIN_ATTACK;
    return velocity == 0 ? 0 : (uint16_t)(NOTE_MIN_ATTACK +
            (value * value * dynamic_range) / UINT32_C(16129));
}

static uint8_t pitch_lane(uint8_t note, uint8_t minimum,
                          uint8_t maximum) {
    if (minimum >= maximum) return 1;
    if (note <= minimum) return 0;
    if (note >= maximum) return 2;
    const uint16_t span = (uint16_t)maximum - minimum + 1u;
    const uint16_t position = (uint16_t)note - minimum;
    const uint8_t lane = (uint8_t)((position * LED_ENGINE_LANES) / span);
    return lane > 2 ? 2 : lane;
}

static uint16_t decay_once(uint16_t value, uint8_t shift,
                           uint16_t minimum) {
    if (value == 0) return 0;
    uint16_t decrement = value >> shift;
    if (decrement < minimum) decrement = minimum;
    return decrement >= value ? 0 : (uint16_t)(value - decrement);
}

static uint16_t approach_once(uint16_t level, uint16_t target,
                              uint8_t rise_shift, uint8_t fall_shift,
                              uint16_t fall_floor) {
    if (level == target) return level;
    if (level < target) {
        uint16_t increment = (uint16_t)((target - level) >> rise_shift);
        if (increment == 0) increment = 1;
        return increment >= target - level ? target :
                (uint16_t)(level + increment);
    }
    return decay_once((uint16_t)(level - target), fall_shift, fall_floor) +
            target;
}

static void advance_envelope(uint16_t *target, uint16_t *level,
                             uint32_t steps, uint8_t target_decay_shift,
                             uint16_t target_decay_floor,
                             uint8_t rise_shift, uint8_t fall_shift,
                             uint16_t fall_floor) {
    if (steps > MAX_DECAY_STEPS) {
        *target = 0;
        *level = 0;
        return;
    }
    while (steps-- > 0) {
        *target = decay_once(*target, target_decay_shift,
                             target_decay_floor);
        *level = approach_once(*level, *target, rise_shift, fall_shift,
                               fall_floor);
    }
}

void led_engine_reset(led_engine_t *engine, uint32_t now_ms) {
    if (engine == NULL) return;
    memset(engine, 0, sizeof(*engine));
    engine->last_tick_ms = now_ms;
    engine->dirty = true;
}

void led_engine_init(led_engine_t *engine, uint32_t now_ms) {
    led_engine_reset(engine, now_ms);
}

void led_engine_note_on(led_engine_t *engine, led_source_t source,
                        uint8_t note, uint8_t velocity,
                        uint8_t minimum_note, uint8_t maximum_note) {
    const unsigned source_index = (unsigned)source;
    if (engine == NULL || velocity == 0 ||
        source_index >= LED_ENGINE_SOURCE_COUNT) {
        return;
    }
    const uint8_t lane = pitch_lane(note, minimum_note, maximum_note);
    uint16_t *target = &engine->green_target[source_index][lane];
    uint16_t *level = &engine->green_level[source_index][lane];
    *target = saturating_add(*target, note_attack(velocity));
    /* A quick, bounded inhale avoids a hard LED edge. Repeated notes add to
       the target, so faster playing naturally becomes brighter and fuller. */
    *level = approach_once(*level, *target, 2, 6, NOTE_LEVEL_DECAY_FLOOR);
    engine->dirty = true;
}

void led_engine_clear_notes(led_engine_t *engine) {
    if (engine == NULL) return;
    memset(engine->green_target, 0, sizeof(engine->green_target));
    memset(engine->green_level, 0, sizeof(engine->green_level));
    engine->dirty = true;
}

void led_engine_beat(led_engine_t *engine) {
    if (engine == NULL) return;
    engine->beat_target = saturating_add(engine->beat_target, BEAT_LEVEL);
    engine->beat_level = approach_once(engine->beat_level,
            engine->beat_target, 3, 5, BEAT_LEVEL_DECAY_FLOOR);
    engine->dirty = true;
}

bool led_engine_service(led_engine_t *engine, uint32_t now_ms,
                        led_frame_t *frame) {
    if (engine == NULL || frame == NULL) return false;

    const uint32_t elapsed_ms = now_ms - engine->last_tick_ms;
    const uint32_t steps = elapsed_ms / LED_ENGINE_TICK_MS;
    if (steps > 0) {
        engine->last_tick_ms += steps * LED_ENGINE_TICK_MS;
        for (uint8_t source = 0; source < LED_ENGINE_SOURCE_COUNT; ++source) {
            for (uint8_t lane = 0; lane < LED_ENGINE_LANES; ++lane) {
                uint16_t *target = &engine->green_target[source][lane];
                uint16_t *level = &engine->green_level[source][lane];
                const uint16_t previous_target = *target;
                const uint16_t previous_level = *level;
                advance_envelope(target, level, steps, 5,
                        NOTE_TARGET_DECAY_FLOOR, 1, 6,
                        NOTE_LEVEL_DECAY_FLOOR);
                if (*target != previous_target || *level != previous_level) {
                    engine->dirty = true;
                }
            }
        }
        const uint16_t previous_beat_target = engine->beat_target;
        const uint16_t previous_beat_level = engine->beat_level;
        advance_envelope(&engine->beat_target, &engine->beat_level, steps,
                4, BEAT_TARGET_DECAY_FLOOR, 3, 5,
                BEAT_LEVEL_DECAY_FLOOR);
        if (engine->beat_target != previous_beat_target ||
            engine->beat_level != previous_beat_level) {
            engine->dirty = true;
        }
    }

    if (!engine->dirty) return false;
    for (uint8_t lane = 0; lane < LED_ENGINE_LANES; ++lane) {
        frame->blue[lane] = engine->beat_level;
        for (uint8_t source = 0; source < LED_ENGINE_SOURCE_COUNT; ++source) {
            frame->green[source][lane] = engine->green_level[source][lane];
        }
    }
    engine->dirty = false;
    return true;
}
