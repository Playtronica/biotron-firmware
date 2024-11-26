#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <pico/time.h>
#include <pico/printf.h>
#include "leds.h"
#include "PLSDK/channel.h"
#include "raw_plant.h"
#include "global.h"
#include "buttons.h"
#include "music.h"
#include "params.h"

const uint8_t ALL_LEDS[] = {
        GROUP_BlUE_LED_CENTER, GROUP_BlUE_LED_LEFT, GROUP_BlUE_LED_RIGHT,
        FIRST_GROUP_GREEN_LED_1, FIRST_GROUP_GREEN_LED_2, FIRST_GROUP_GREEN_LED_3,
        SECOND_GROUP_GREEN_LED_1, SECOND_GROUP_GREEN_LED_2, SECOND_GROUP_GREEN_LED_3
};
const uint8_t LED_COUNT = 9;


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


void led_loop() {
    static int32_t ledsValue[ASYNC_LEDS];
    static int16_t level = (MAX_LIGHT - MIN_LIGHT) / (TIMER_MULTIPLIER * 40);

    uint8_t mute_plant = (mute_state == MutePlant || mute_state == MuteAll) ? 8 : 1;
    uint8_t mute_light = (mute_state == MuteLight || mute_state == MuteAll) ? 8 : 1;

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

        pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_1, value / mute_light);
        pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_2, value / mute_light);
        pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_3, value / mute_light);
        pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_1, value / mute_plant);
        pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_2, value / mute_plant);
        pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_3, value / mute_plant);
    }

    if (button_top_pressed || button_bottom_pressed || button_finger_pressed) {
        return;
    }

    switch (status) {
        case Sleep:
            if (TestMode) {
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

            pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_1, value / mute_light);
            pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_2, value / mute_light);
            pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_3, value / mute_light);
            pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_1, value / mute_plant);
            pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_2, value / mute_plant);
            pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_3, value / mute_plant);
            break;
        }
        case Active:
        case BPMClockActive: {
            if ((ledsValue[ASYNC_LEDS - 1] + level > MAX_LIGHT && level > 0) ||
                (ledsValue[ASYNC_LEDS - 1] + level < MIN_LIGHT && level < 0)) {
                level *= -1;
            }

            for (int i = 0; i < ASYNC_LEDS - 1; i++) {
                ledsValue[i] = ledsValue[i + 1];
            }

            uint8_t lastNotePlant = 60;
            ledsValue[ASYNC_LEDS - 1] += level;
            uint16_t first_val = MAX(MIN_LIGHT,
                                     MIN(MAX_LIGHT, ledsValue[0] + (((int) lastNotePlant - MIDDLE_NOTE) * NOTE_STRONG)));
            pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_1, first_val / mute_light);
            pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_2, first_val / mute_light);
            pwm_set_gpio_level_invert(FIRST_GROUP_GREEN_LED_3, first_val / mute_light);

            uint16_t second_val = MAX(MIN_LIGHT, MIN(MAX_LIGHT, ledsValue[ASYNC_LEDS - 1] +
                                                                (((int) lastNotePlant - MIDDLE_NOTE) * NOTE_STRONG)));
            pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_1, second_val / mute_plant);
            pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_2, second_val / mute_plant);
            pwm_set_gpio_level_invert(SECOND_GROUP_GREEN_LED_3, second_val / mute_plant);


            break;
        }
    }
}
