#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hardware/pwm.h"
#include "buttons.h"
#include "global.h"
#include "leds.h"
#include "music.h"
#include "params.h"

Settings_t settings;
enum Status status = Active;
enum Status active_status = Active;
bool isMutedByButton = false;
bool TestMode = false;
bool isTestModeGreen = false;
bool button_finger_pressed = false;
bool button_bottom_pressed = false;
bool button_top_pressed = false;
bool LOGGER_FLAG = false;
uint8_t last_note_plant = MIDDLE_NOTE;

static uint32_t now_us = 0;
static uint16_t pwm_levels[32];

uint64_t time_us_64(void) { return now_us; }
uint32_t time_us_32(void) { return now_us; }
void sleep_ms(uint32_t milliseconds) { now_us += milliseconds * 1000u; }
void remind_midi(void) {}
void note_on(uint8_t channel, uint8_t note, uint8_t velocity) {
    (void)channel;
    (void)note;
    (void)velocity;
}
void note_off(uint8_t channel, uint8_t note) {
    (void)channel;
    (void)note;
}

void gpio_init_mask(uint32_t mask) { (void)mask; }
void gpio_set_function(uint gpio, uint function) {
    (void)gpio;
    (void)function;
}
uint pwm_gpio_to_slice_num(uint gpio) { return gpio / 2u; }
pwm_config pwm_get_default_config(void) {
    const pwm_config config = {0};
    return config;
}
void pwm_init(uint slice, const pwm_config *config, bool start) {
    (void)slice;
    (void)config;
    (void)start;
}
void pwm_set_gpio_level(uint gpio, uint16_t level) {
    assert(gpio < sizeof(pwm_levels) / sizeof(pwm_levels[0]));
    pwm_levels[gpio] = level;
}

static uint16_t brightness(uint gpio) {
    return (uint16_t)(MAX_LIGHT - pwm_levels[gpio]);
}

static void reset_fixture(void) {
    memset(&settings, 0, sizeof(settings));
    settings.middle_plant_note = MIDDLE_NOTE;
    settings.light_note_range = 12;
    status = Active;
    isMutedByButton = false;
    TestMode = false;
    isTestModeGreen = false;
    button_finger_pressed = false;
    button_bottom_pressed = false;
    button_top_pressed = false;
    now_us = 0;
    memset(pwm_levels, 0, sizeof(pwm_levels));
    init_leds();
    led_loop();
}

static void test_spatial_pin_map_and_polarity(void) {
    reset_fixture();
    led_music_note_on(LED_SOURCE_LIGHT, 24, 127);
    led_loop();
    assert(brightness(3) == LED_ENGINE_MAX_LEVEL);
    assert(brightness(4) == 0);
    assert(brightness(9) == 0);

    reset_fixture();
    led_music_note_on(LED_SOURCE_PLANT, 97, 127);
    led_loop();
    assert(brightness(14) == LED_ENGINE_MAX_LEVEL);
    assert(brightness(10) == 0);
    assert(brightness(11) == 0);
}

static void test_beat_and_mute_domains(void) {
    reset_fixture();
    led_music_note_on(LED_SOURCE_PLANT, 60, 127);
    led_music_beat();
    led_loop();
    assert(brightness(0) == LED_ENGINE_MAX_LEVEL);
    assert(brightness(1) == LED_ENGINE_MAX_LEVEL);
    assert(brightness(2) == LED_ENGINE_MAX_LEVEL);
    assert(brightness(11) == LED_ENGINE_MAX_LEVEL);

    isMutedByButton = true;
    led_loop();
    assert(brightness(11) == 0);
    assert(brightness(0) == LED_ENGINE_MAX_LEVEL);
}

int main(void) {
    test_spatial_pin_map_and_polarity();
    test_beat_and_mute_domains();
    puts("led_adapter: A06-A08 pins, polarity, beat and mute passed");
    return 0;
}
