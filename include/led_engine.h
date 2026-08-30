#ifndef BIOTRON_LED_ENGINE_H
#define BIOTRON_LED_ENGINE_H

#include <stdbool.h>
#include <stdint.h>

#define LED_ENGINE_LANES 3u
#define LED_ENGINE_SOURCE_COUNT 2u
#define LED_ENGINE_MAX_LEVEL UINT16_C(60000)
#define LED_ENGINE_TICK_MS UINT32_C(4)

typedef enum {
    LED_SOURCE_LIGHT = 0,
    LED_SOURCE_PLANT = 1,
} led_source_t;

typedef struct {
    uint16_t blue[LED_ENGINE_LANES];
    uint16_t green[LED_ENGINE_SOURCE_COUNT][LED_ENGINE_LANES];
} led_frame_t;

typedef struct {
    uint16_t green_energy[LED_ENGINE_SOURCE_COUNT][LED_ENGINE_LANES];
    uint16_t beat_energy;
    uint32_t last_tick_ms;
    bool dirty;
} led_engine_t;

void led_engine_init(led_engine_t *engine, uint32_t now_ms);
void led_engine_reset(led_engine_t *engine, uint32_t now_ms);
void led_engine_note_on(led_engine_t *engine, led_source_t source,
                        uint8_t note, uint8_t velocity,
                        uint8_t minimum_note, uint8_t maximum_note);
void led_engine_clear_notes(led_engine_t *engine);
void led_engine_beat(led_engine_t *engine);
bool led_engine_service(led_engine_t *engine, uint32_t now_ms,
                        led_frame_t *frame);

#endif
