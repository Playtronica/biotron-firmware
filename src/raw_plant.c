#include <pico/printf.h>
#include <stdlib.h>
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "raw_plant.h"
#include "pico/time.h"
#include "hardware/irq.h"
#include "params.h"


struct repeating_timer getFrequencyTimer;
uint8_t slice_num = 0;
uint32_t count = 0;
uint32_t LastCount = 0;
bool freq_ready = false;
uint32_t realFreq = 0;


static void _on_pwm_wrap() {
    pwm_clear_irq(slice_num);
    count += (1L << 16);
}


static uint16_t _pwm_read(uint sliceNum) {
    static uint64_t last_count = 0;
    uint64_t this_count = count + pwm_get_counter(sliceNum);
    uint16_t res = this_count - last_count;
    last_count = this_count;

    return res;
}


static bool _repeating_timer_callback_t(repeating_timer_t *rt) {
    freq_ready = true;
    realFreq = _pwm_read(slice_num) * TIMER_MULTIPLIER + rand() % (settings.random_note * 10);
    return true;
}


bool plant_is_ready() {
    return freq_ready;
}


uint32_t get_real_freq() {
    static uint32_t old_one = 0;
    if (freq_ready) {
        freq_ready = false;
        old_one = realFreq;
        return realFreq;
    }
    return old_one;
}


void setupPwm(uint8_t freq_pin) {
    assert(pwm_gpio_to_channel(freq_pin) == PWM_CHAN_B);
    slice_num = pwm_gpio_to_slice_num(freq_pin);
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv_mode(&cfg, PWM_DIV_B_RISING);
    pwm_config_set_clkdiv(&cfg, 1);
    pwm_init(slice_num, &cfg, false);
    gpio_set_function(freq_pin, GPIO_FUNC_PWM);
    pwm_set_enabled(slice_num, true);
    pwm_clear_irq(slice_num);
    pwm_set_irq_enabled(slice_num, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, _on_pwm_wrap);
    irq_set_enabled(PWM_IRQ_WRAP, true);
}


void initFrequencyTimer() {
    setupPwm(PLANT_PIN);
    add_repeating_timer_ms(TIMER_PLANT_MS, _repeating_timer_callback_t,
                           NULL, &getFrequencyTimer);
}

void init_plant() {
    gpio_init(PLANT_PIN);
    gpio_set_dir(PLANT_PIN, GPIO_IN);

    initFrequencyTimer();
}