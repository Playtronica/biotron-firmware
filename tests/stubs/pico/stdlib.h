#ifndef TEST_PICO_STDLIB_H
#define TEST_PICO_STDLIB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef unsigned int uint;

#ifndef MIN
#define MIN(left, right) ((left) < (right) ? (left) : (right))
#endif
#ifndef MAX
#define MAX(left, right) ((left) > (right) ? (left) : (right))
#endif

uint64_t time_us_64(void);
uint32_t time_us_32(void);
void sleep_ms(uint32_t milliseconds);

#endif
