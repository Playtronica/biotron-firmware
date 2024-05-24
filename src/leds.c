#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include "leds.h"

void init_leds() {
    gpio_init_mask(LED_MASK);

    gpio_set_function(GROUP_BlUE_LED_CENTER, GPIO_FUNC_PWM);
    gpio_set_function(GROUP_BlUE_LED_LEFT, GPIO_FUNC_PWM);
    gpio_set_function(GROUP_BlUE_LED_RIGHT, GPIO_FUNC_PWM);
    gpio_set_function(FIRST_GROUP_GREEN_LED_1, GPIO_FUNC_PWM);
    gpio_set_function(FIRST_GROUP_GREEN_LED_2, GPIO_FUNC_PWM);
    gpio_set_function(FIRST_GROUP_GREEN_LED_3, GPIO_FUNC_PWM);
    gpio_set_function(SECOND_GROUP_GREEN_LED_1, GPIO_FUNC_PWM);
    gpio_set_function(SECOND_GROUP_GREEN_LED_2, GPIO_FUNC_PWM);
    gpio_set_function(SECOND_GROUP_GREEN_LED_3, GPIO_FUNC_PWM);

    pwm_config config = pwm_get_default_config();
    pwm_init(pwm_gpio_to_slice_num(GROUP_BlUE_LED_CENTER), &config, true);
    pwm_init(pwm_gpio_to_slice_num(GROUP_BlUE_LED_LEFT), &config, true);
    pwm_init(pwm_gpio_to_slice_num(GROUP_BlUE_LED_RIGHT), &config, true);
    pwm_init(pwm_gpio_to_slice_num(FIRST_GROUP_GREEN_LED_1), &config, true);
    pwm_init(pwm_gpio_to_slice_num(FIRST_GROUP_GREEN_LED_2), &config, true);
    pwm_init(pwm_gpio_to_slice_num(FIRST_GROUP_GREEN_LED_3), &config, true);
    pwm_init(pwm_gpio_to_slice_num(SECOND_GROUP_GREEN_LED_1), &config, true);
    pwm_init(pwm_gpio_to_slice_num(SECOND_GROUP_GREEN_LED_2), &config, true);
    pwm_init(pwm_gpio_to_slice_num(SECOND_GROUP_GREEN_LED_3), &config, true);
}