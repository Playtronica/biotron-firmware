#ifndef TEST_HARDWARE_GPIO_H
#define TEST_HARDWARE_GPIO_H

#include <stdbool.h>
#include <stdint.h>
#include "pico/stdlib.h"

#define GPIO_FUNC_PWM 1u
#define GPIO_IN false

void gpio_set_function(uint gpio, uint function);
void gpio_init(uint gpio);
void gpio_init_mask(uint32_t mask);
void gpio_set_dir(uint gpio, bool output);

#endif
