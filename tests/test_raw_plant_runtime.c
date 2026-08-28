#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/sync.h"
#include "pico/time.h"
#include "params.h"
#include "raw_plant.h"

Settings_t settings;
bool isMutedByButton = false;
bool TestMode = false;
bool isTestModeGreen = false;

static repeating_timer_callback_t timer_callback = NULL;
static uint16_t fake_pwm_counter = 0;
static size_t random_calls = 0;
static size_t critical_entries = 0;
static size_t critical_exits = 0;

int rand(void) {
    ++random_calls;
    return 7;
}

bool add_repeating_timer_ms(int32_t delay_ms,
                            repeating_timer_callback_t callback,
                            void *user_data, repeating_timer_t *out) {
    (void)user_data;
    (void)out;
    assert(delay_ms == TIMER_PLANT_MS);
    timer_callback = callback;
    return true;
}

uint32_t save_and_disable_interrupts(void) {
    ++critical_entries;
    return 123;
}

void restore_interrupts(uint32_t status) {
    assert(status == 123);
    ++critical_exits;
}

void pwm_clear_irq(uint slice_num) { (void)slice_num; }
uint16_t pwm_get_counter(uint slice_num) {
    (void)slice_num;
    return fake_pwm_counter;
}
uint pwm_gpio_to_channel(uint gpio) { (void)gpio; return PWM_CHAN_B; }
uint pwm_gpio_to_slice_num(uint gpio) { (void)gpio; return 0; }
pwm_config pwm_get_default_config(void) { return (pwm_config){0}; }
void pwm_config_set_clkdiv_mode(pwm_config *config, uint mode) {
    (void)config; (void)mode;
}
void pwm_config_set_clkdiv(pwm_config *config, float divider) {
    (void)config; (void)divider;
}
void pwm_init(uint slice_num, const pwm_config *config, bool start) {
    (void)slice_num; (void)config; (void)start;
}
void pwm_set_enabled(uint slice_num, bool enabled) {
    (void)slice_num; (void)enabled;
}
void pwm_set_irq_enabled(uint slice_num, bool enabled) {
    (void)slice_num; (void)enabled;
}
void gpio_set_function(uint gpio, uint function) { (void)gpio; (void)function; }
void gpio_init(uint gpio) { (void)gpio; }
void gpio_set_dir(uint gpio, bool output) { (void)gpio; (void)output; }
void irq_set_exclusive_handler(uint irq, void (*handler)(void)) {
    (void)irq; (void)handler;
}
void irq_set_enabled(uint irq, bool enabled) { (void)irq; (void)enabled; }

int main(void) {
    init_plant();
    assert(timer_callback != NULL);

    settings.random_note = false;
    fake_pwm_counter = 10;
    timer_callback(NULL);
    assert(random_calls == 0);
    assert(plant_is_ready());
    assert(get_real_freq() == 100);
    assert(random_calls == 0);

    settings.random_note = true;
    fake_pwm_counter = 20;
    timer_callback(NULL);
    assert(random_calls == 0);
    assert(get_real_freq() == 107);
    assert(random_calls == 1);
    assert(critical_entries == 2);
    assert(critical_exits == 2);

    puts("raw_plant_runtime: IRQ sampling and main-loop jitter passed");
    return 0;
}
