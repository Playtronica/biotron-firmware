#include "tusb.h"
#include "frequency_counter.h"
#include "notes.h"
#include "global.h"
#include "pico/stdlib.h"
#include "pico.h"
#include "cap_buttons.h"

struct repeating_timer tap_timer;
static uint64_t middle = 0;
static int k = -1;

bool calculate_bpm_by_tap(struct repeating_timer *t) {
    if (k >= 3) {
        setBPM(middle / k);
        printf("Success\n");
    }
    k = -1;
    cancel_repeating_timer(&tap_timer);
    return true;
}

void func1() {
    printf("1\n");
}

void func2() {
    printf("2\n");
}

void func3() {
    printf("3\n");
    static uint64_t last = 0;
    if (k == -1) {
        middle = 0;
        last = time_us_64();
        k++;
        add_repeating_timer_ms(2000, calculate_bpm_by_tap, NULL, &tap_timer);
        return;
    }
    middle += time_us_64() - last;
    last = time_us_64();
    k++;
    cancel_repeating_timer(&tap_timer);
    add_repeating_timer_ms(2000, calculate_bpm_by_tap, NULL, &tap_timer);
}

int main(void)
{
    stdio_init_all();
    tusb_init();

    Setup();
    Intro();

    buttons_add_button(6, func1, func1, NULL);
    buttons_add_button(7, func2, func2, NULL);
    buttons_add_button(8, func3, NULL, NULL);

    buttons_init(5);

    while (true)
    {
        tud_task();
        gpio_put(TEST_LED, 1);
        if (isReady()) {
            MainStage();
        }

        MidiSettings();
        LedStage();
        buttons_task();
        sleep_ms(1);
    }

}