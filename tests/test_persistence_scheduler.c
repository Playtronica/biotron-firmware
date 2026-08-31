#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "persistence_scheduler.h"

int main(void) {
    persistence_scheduler_t scheduler = {
        .pending = false,
        .last_change_us = 0,
        .debounce_us = UINT64_C(1000000),
    };
    persistence_note_change(&scheduler, 100);
    assert(!persistence_is_due(&scheduler, 1000099));
    assert(persistence_is_due(&scheduler, 1000100));
    persistence_note_saved(&scheduler);
    assert(!persistence_is_due(&scheduler, 2000000));

    persistence_note_change(&scheduler, 3000000);
    persistence_note_change(&scheduler, 3500000);
    assert(!persistence_is_due(&scheduler, 4499999));
    assert(persistence_is_due(&scheduler, 4500000));

    scheduler.debounce_us = 20;
    persistence_note_change(&scheduler, UINT64_MAX - 9);
    assert(!persistence_is_due(&scheduler, 9));
    assert(persistence_is_due(&scheduler, 10));
    puts("persistence_scheduler: debounce, coalescing and wrap passed");
    return 0;
}
