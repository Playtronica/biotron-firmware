#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "persistence_scheduler.h"
#include "settings_storage.h"

static void test_debounce_resets_and_saves_once(void) {
    persistence_scheduler_t scheduler = {
        .pending = false, .last_change_us = 0, .debounce_us = 1000000,
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
}

static void test_immediate_save_cancels_pending_debounce(void) {
    persistence_scheduler_t scheduler = {
        .pending = false, .last_change_us = 0, .debounce_us = 1000000,
    };
    persistence_note_change(&scheduler, 10);
    persistence_note_saved(&scheduler);
    assert(!persistence_is_due(&scheduler, 1000010));
}

static void test_unsigned_time_wrap_is_well_defined(void) {
    persistence_scheduler_t scheduler = {
        .pending = false, .last_change_us = 0, .debounce_us = 20,
    };
    persistence_note_change(&scheduler, UINT64_MAX - 9);
    assert(!persistence_is_due(&scheduler, 9));
    assert(persistence_is_due(&scheduler, 10));
}

static void test_storage_pack_copies_only_source_and_pads_page(void) {
    uint8_t source[113];
    uint8_t destination[256];
    for (size_t i = 0; i < sizeof(source); ++i) source[i] = (uint8_t)i;
    assert(settings_storage_pack(destination, sizeof(destination),
                                 source, sizeof(source)));
    for (size_t i = 0; i < sizeof(source); ++i) {
        assert(destination[i] == source[i]);
    }
    for (size_t i = sizeof(source); i < sizeof(destination); ++i) {
        assert(destination[i] == 0xff);
    }
    assert(!settings_storage_pack(destination, 112, source, sizeof(source)));
    assert(!settings_storage_pack(NULL, sizeof(destination), source, sizeof(source)));
    assert(!settings_storage_pack(destination, sizeof(destination), NULL, sizeof(source)));
    assert(STORAGE_ROUND_UP(113, 256) == 256);
    assert(STORAGE_ROUND_UP(256, 256) == 256);
    assert(STORAGE_ROUND_UP(257, 256) == 512);
}

int main(void) {
    test_debounce_resets_and_saves_once();
    test_immediate_save_cancels_pending_debounce();
    test_unsigned_time_wrap_is_well_defined();
    test_storage_pack_copies_only_source_and_pads_page();
    puts("persistence: all deterministic tests passed");
    return 0;
}
