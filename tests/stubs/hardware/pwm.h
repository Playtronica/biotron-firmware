#ifndef TEST_HARDWARE_PWM_H
#define TEST_HARDWARE_PWM_H

#include <stdbool.h>
#include <stdint.h>
#include "pico/stdlib.h"

#define PWM_CHAN_B 1u
#define PWM_DIV_B_RISING 1u

typedef struct { uint8_t placeholder; } pwm_config;

void pwm_clear_irq(uint slice_num);
uint16_t pwm_get_counter(uint slice_num);
uint pwm_gpio_to_channel(uint gpio);
uint pwm_gpio_to_slice_num(uint gpio);
pwm_config pwm_get_default_config(void);
void pwm_config_set_clkdiv_mode(pwm_config *config, uint mode);
void pwm_config_set_clkdiv(pwm_config *config, float divider);
void pwm_init(uint slice_num, const pwm_config *config, bool start);
void pwm_set_enabled(uint slice_num, bool enabled);
void pwm_set_irq_enabled(uint slice_num, bool enabled);
void pwm_set_gpio_level(uint gpio, uint16_t level);

#endif
