#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <pico/time.h>
#include <pico/printf.h>
#include "leds.h"
#include "PLSDK/channel.h"

uint8_t ALL_LEDS[] = {
        GROUP_BlUE_LED_CENTER, GROUP_BlUE_LED_LEFT, GROUP_BlUE_LED_RIGHT,
        FIRST_GROUP_GREEN_LED_1, FIRST_GROUP_GREEN_LED_2, FIRST_GROUP_GREEN_LED_3,
        SECOND_GROUP_GREEN_LED_1, SECOND_GROUP_GREEN_LED_2, SECOND_GROUP_GREEN_LED_3
};
uint8_t LED_COUNT = 9;


void pwm_set_gpio_level_invert(uint gpio, uint16_t level) {
    pwm_set_gpio_level(gpio, ((1 << 12)) - level);
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
        uint16_t val = (uint16_t)(4096.0 * ((double)(time_us_64() - startTime) / 1000000));
        for (int i = 0; i < LED_COUNT; ++i) {
            pwm_set_gpio_level_invert(ALL_LEDS[i], val);
            remind_midi();
        }
    }
}