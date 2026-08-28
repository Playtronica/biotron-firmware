#ifndef TEST_HARDWARE_IRQ_H
#define TEST_HARDWARE_IRQ_H

#include <stdbool.h>
#include "pico/stdlib.h"

#define PWM_IRQ_WRAP 4u

void irq_set_exclusive_handler(uint irq, void (*handler)(void));
void irq_set_enabled(uint irq, bool enabled);

#endif
