#include <stddef.h>
#include <string.h>

#include "led_engine.h"

#define NOTE_MIN_ATTACK UINT16_C(9000)
#define NOTE_DECAY_FLOOR UINT16_C(96)
#define BEAT_LEVEL LED_ENGINE_MAX_LEVEL
#define BEAT_DECAY_FLOOR UINT16_C(256)
#define MAX_DECAY_STEPS UINT32_C(256)

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

static uint16_t decay_many(uint16_t value, uint32_t steps, uint8_t shift,
                           uint16_t minimum) {
    if (steps > MAX_DECAY_STEPS) return 0;
    while (steps-- > 0 && value > 0) {
        value = decay_once(value, shift, minimum);
    }
    return value;
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
    uint16_t *energy = &engine->green_energy[source_index][lane];
    *energy = saturating_add(*energy, note_attack(velocity));
    engine->dirty = true;
}

void led_engine_clear_notes(led_engine_t *engine) {
    if (engine == NULL) return;
    memset(engine->green_energy, 0, sizeof(engine->green_energy));
    engine->dirty = true;
}

void led_engine_beat(led_engine_t *engine) {
    if (engine == NULL) return;
    engine->beat_energy = saturating_add(engine->beat_energy, BEAT_LEVEL);
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
                uint16_t *energy = &engine->green_energy[source][lane];
                const uint16_t decayed = decay_many(
                        *energy, steps, 4, NOTE_DECAY_FLOOR);
                if (decayed != *energy) engine->dirty = true;
                *energy = decayed;
            }
        }
        const uint16_t beat = decay_many(
                engine->beat_energy, steps, 3, BEAT_DECAY_FLOOR);
        if (beat != engine->beat_energy) engine->dirty = true;
        engine->beat_energy = beat;
    }

    if (!engine->dirty) return false;
    for (uint8_t lane = 0; lane < LED_ENGINE_LANES; ++lane) {
        frame->blue[lane] = engine->beat_energy;
        for (uint8_t source = 0; source < LED_ENGINE_SOURCE_COUNT; ++source) {
            frame->green[source][lane] = engine->green_energy[source][lane];
        }
    }
    engine->dirty = false;
    return true;
}
