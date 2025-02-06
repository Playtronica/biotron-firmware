#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <pico/time.h>
#include <pico/printf.h>
#include "leds.h"
#include "raw_plant.h"
#include "global.h"
#include "buttons.h"
#include "music.h"
#include "params.h"
#include "PLSDK/music.h"
#include "PLSDK.h"

const uint8_t ALL_LEDS[] = {
        GROUP_BlUE_LED_CENTER, GROUP_BlUE_LED_LEFT, GROUP_BlUE_LED_RIGHT,
        FIRST_GROUP_GREEN_LED_1, FIRST_GROUP_GREEN_LED_2, FIRST_GROUP_GREEN_LED_3,
        SECOND_GROUP_GREEN_LED_1, SECOND_GROUP_GREEN_LED_2, SECOND_GROUP_GREEN_LED_3
};
const uint8_t LED_COUNT = 9;


inline uint8_t mute_plant() {
    return (mute_state == MutePlant || mute_state == MuteAll) ? 8 : 1;
}

inline uint8_t mute_light() {
    return (mute_state == MuteLight || mute_state == MuteAll) ? 8 : 1;
}


void pwm_set_gpio_level_invert(uint gpio, uint16_t level) {
    pwm_set_gpio_level(gpio, MAX_LIGHT - level);
}



void init_leds() {
    gpio_init_mask(LED_MASK);

    pwm_config config = pwm_get_default_config();
    for (int i = 0; i < LED_COUNT; ++i) {
        gpio_set_function(ALL_LEDS[i], GPIO_FUNC_PWM);
        pwm_init(pwm_gpio_to_slice_num(ALL_LEDS[i]), &config, true);
        remind_midi();
    }

}

void intro_leds() {
    uint64_t startTime = time_us_64();
    while (time_us_64() - startTime <= 1000000) {
        uint16_t val = (uint16_t)(MAX_LIGHT * ((double)(time_us_64() - startTime) / 1000000));
        for (int i = 0; i < LED_COUNT; ++i) {
            if (i < 3) pwm_set_gpio_level_invert(ALL_LEDS[i], val);
            else pwm_set_gpio_level(ALL_LEDS[i], val);
            remind_midi();
        }
    }
}

static int32_t ledsValue[ASYNC_LEDS];
static int16_t level = (MAX_LIGHT - MIN_LIGHT) / (TIMER_MULTIPLIER * 40);

void active_led_loop_classic();
void active_led_loop_light_bpm_sync();

void led_loop() {
    if (button_top_pressed) {
        pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_1, MAX_LIGHT);
        pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_2, MAX_LIGHT);
        pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_3, MAX_LIGHT);
    }
    if (button_bottom_pressed) {
        pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_1, MAX_LIGHT);
        pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_2, MAX_LIGHT);
        pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_3, MAX_LIGHT);
    }

    if (button_finger_pressed) {
        static int16_t led_step = 2000;
        static uint16_t value = MIN_LIGHT;
        if ((value + led_step > MAX_LIGHT && led_step > 0)
            || (value + led_step < MIN_LIGHT && led_step < 0)) {
            led_step *= -1;
        }

        value += led_step;

        pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_1, value / mute_light());
        pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_2, value / mute_light());
        pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_3, value / mute_light());
        pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_1, value / mute_plant());
        pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_2, value / mute_plant());
        pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_3, value / mute_plant());
    }

    if (button_top_pressed || button_bottom_pressed || button_finger_pressed) {
        return;
    }

    switch (status) {
        case Sleep:
            if (TestMode == 1) {
                pwm_set_gpio_level_invert(GROUP_BlUE_LED_CENTER, MAX_LIGHT);
                pwm_set_gpio_level_invert(GROUP_BlUE_LED_LEFT, MAX_LIGHT);
                pwm_set_gpio_level_invert(GROUP_BlUE_LED_RIGHT, MAX_LIGHT);
                pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_1, 0);
                pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_2, 0);
                pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_3, 0);
                pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_1, 0);
                pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_2, 0);
                pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_3, 0);
                break;
            }
            else if (TestMode == 2) {
                pwm_set_gpio_level_invert(GROUP_BlUE_LED_CENTER, 0);
                pwm_set_gpio_level_invert(GROUP_BlUE_LED_LEFT, 0);
                pwm_set_gpio_level_invert(GROUP_BlUE_LED_RIGHT, 0);
                pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_1, MAX_LIGHT);
                pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_2, MAX_LIGHT);
                pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_3, MAX_LIGHT);
                pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_1, MAX_LIGHT);
                pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_2, MAX_LIGHT);
                pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_3, MAX_LIGHT);
                break;
            }


            if (level < 0) level *= -1;

            for (int i = ASYNC_LEDS - 1; i >= 0; i--) {
                ledsValue[i] = 0;
            }

            pwm_set_gpio_level_invert(GROUP_BlUE_LED_CENTER, MAX_LIGHT);
            pwm_set_gpio_level_invert(GROUP_BlUE_LED_LEFT, MAX_LIGHT);
            pwm_set_gpio_level_invert(GROUP_BlUE_LED_RIGHT, MAX_LIGHT);
            pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_1, 0);
            pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_2, 0);
            pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_3, 0);
            pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_1, 0);
            pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_2, 0);
            pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_3, 0);

            break;
        case Stabilization: {
            static int16_t led_step = 1000;
            static uint16_t value = MIN_LIGHT;
            if ((value + led_step > MAX_LIGHT && led_step > 0)
                || (value + led_step < MIN_LIGHT && led_step < 0)) {
                led_step *= -1;
                if (led_step < 0) {
                    note_off(0, 91);
                    note_on(0, 92, 90);
                }
                else {
                    note_off(0, 92);
                    note_on(0, 91, 90);
                }
            }

            value += led_step;

            pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_1, value / mute_light());
            pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_2, value / mute_light());
            pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_3, value / mute_light());
            pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_1, value / mute_plant());
            pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_2, value / mute_plant());
            pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_3, value / mute_plant());
            break;
        }
        case Active:
        case BPMClockActive: {
            pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_1, 0);
            pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_2, 0);
            pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_3, 0);
            pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_1, 0);
            pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_2, 0);
            pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_3, 0);
//            active_led_loop_classic();
            active_led_loop_light_bpm_sync();
            break;
        }
    }
}


void active_led_loop_classic() {
    if ((ledsValue[ASYNC_LEDS - 1] + level > MAX_LIGHT && level > 0) ||
        (ledsValue[ASYNC_LEDS - 1] + level < MIN_LIGHT && level < 0)) {
        level *= -1;
    }

    for (int i = 0; i < ASYNC_LEDS - 1; i++) {
        ledsValue[i] = ledsValue[i + 1];
    }

    ledsValue[ASYNC_LEDS - 1] += level;
    uint16_t first_val = MAX(MIN_LIGHT,
                             MIN(MAX_LIGHT, ledsValue[0] + (((int) last_note_plant - MIDDLE_NOTE) * NOTE_STRONG)));

    pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_1, first_val / mute_light());
    pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_2, first_val / mute_light());
    pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_3, first_val / mute_light());

    uint16_t second_val = MAX(MIN_LIGHT, MIN(MAX_LIGHT, ledsValue[ASYNC_LEDS - 1] +
                                                        (((int) last_note_plant - MIDDLE_NOTE) * NOTE_STRONG)));
    pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_1, second_val / mute_plant());
    pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_2, second_val / mute_plant());
    pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_3, second_val / mute_plant());
}



enum PulseSide {
    FirstGroupPulse,
    SecondGroupPulse
} last_pulse_side = FirstGroupPulse;

static uint32_t last_pulse_timestamp = 0;

void light_note_observer() {
//    uint8_t gpio_led_1 = last_pulse_side == FirstGroupPulse ? FIRST_GROUP_GREEN_LED_1 : SECOND_GROUP_GREEN_LED_1;
//    uint8_t gpio_led_2 = last_pulse_side == FirstGroupPulse ? FIRST_GROUP_GREEN_LED_2 : SECOND_GROUP_GREEN_LED_2;
//    uint8_t gpio_led_3 = last_pulse_side == FirstGroupPulse ? FIRST_GROUP_GREEN_LED_3 : SECOND_GROUP_GREEN_LED_3;
//
//    pwm_set_gpio_level_invert(gpio_led_1, 0);
//    pwm_set_gpio_level_invert(gpio_led_2, 0);
//    pwm_set_gpio_level_invert(gpio_led_3, 0);

    last_pulse_side = last_pulse_side == FirstGroupPulse ? SecondGroupPulse : FirstGroupPulse;
    last_pulse_timestamp = time_us_32();
}

void active_led_loop_light_bpm_sync() {
    uint64_t time_light_bpm = settings.BPM * settings.lightBPM;

    if (time_us_32() - last_pulse_timestamp > time_light_bpm) {
        return;
    }

    uint8_t k = last_pulse_side == FirstGroupPulse ? mute_light() : mute_plant();
    uint16_t value = (uint16_t)(MAX_LIGHT * ((double)(time_light_bpm - (time_us_32() - last_pulse_timestamp)) / time_light_bpm)) / k;

    pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_1, value);
    pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_2, value);
    pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_3, value);

    pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_1, value);
    pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_2, value);
    pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_3, value);
}