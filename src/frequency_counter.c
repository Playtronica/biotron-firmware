#include <stdio.h>
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "../include/frequency_counter.h"
#include "pico/time.h"

uint32_t realFrequency = 0;
uint8_t slice_num = 0;
uint32_t count = 0;
uint32_t LastCount = 0;

static void _on_pwm_wrap() {
    pwm_clear_irq(slice_num);
    count += (1L << 16);
}


static uint32_t _pwm_read(uint sliceNum) {
    uint32_t this_count = count;
    uint16_t part_count = pwm_get_counter(sliceNum);

    if (part_count < 100) {
        // Maybe it just rolled over?  Re-check base_count.
        this_count = count;
    }
    this_count += part_count;
    uint32_t advance = (this_count - LastCount);  // Will handle wraparound correctly.
    LastCount = this_count;
    return advance;
}
static bool _repeating_timer_callback_t(repeating_timer_t *rt) {

    freq_ready = true;
    realFrequency = _pwm_read(slice_num);

    return true;
}



bool isReady() {

    return freq_ready;
}

uint32_t getFreq() {
    if (freq_ready) {
        freq_ready = false;
        return realFrequency;
    }
    return 0;
}

void setupPwm(uint8_t freq_pin) {
    assert(pwm_gpio_to_channel(freq_pin) == PWM_CHAN_B);
    pin = freq_pin;
    slice_num = pwm_gpio_to_slice_num(freq_pin);

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv_mode(&cfg, PWM_DIV_B_RISING);
    pwm_config_set_clkdiv(&cfg, 1);
    pwm_init(slice_num, &cfg, false);
    gpio_set_function(freq_pin, GPIO_FUNC_PWM);
    pwm_set_enabled(slice_num, true); //TODO Check
// Новый код из другого файла
    pwm_clear_irq(slice_num);
    pwm_set_irq_enabled(slice_num, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, _on_pwm_wrap);
    irq_set_enabled(PWM_IRQ_WRAP, true);
}


void beginTimer(uint8_t freq_pin, uint16_t period_ms) {
    setupPwm(freq_pin);
    add_repeating_timer_ms(period_ms, _repeating_timer_callback_t,NULL, &timer);
}





