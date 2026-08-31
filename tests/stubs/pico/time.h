#ifndef TEST_PICO_TIME_H
#define TEST_PICO_TIME_H

#include <stdbool.h>
#include <stdint.h>

typedef int32_t alarm_id_t;
typedef struct repeating_timer { int placeholder; } repeating_timer_t;
typedef int64_t (*alarm_callback_t)(alarm_id_t id, void *user_data);
typedef bool (*repeating_timer_callback_t)(repeating_timer_t *timer);

uint64_t time_us_64(void);
uint32_t time_us_32(void);
alarm_id_t add_alarm_in_us(int64_t delay_us, alarm_callback_t callback,
                           void *user_data, bool fire_if_past);
bool cancel_alarm(alarm_id_t alarm_id);
bool add_repeating_timer_ms(int32_t delay_ms,
                            repeating_timer_callback_t callback,
                            void *user_data, repeating_timer_t *out);

#endif
