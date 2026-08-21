#ifndef BIOTRON_PERSISTENCE_SCHEDULER_H
#define BIOTRON_PERSISTENCE_SCHEDULER_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool pending;
    uint64_t last_change_us;
    uint64_t debounce_us;
} persistence_scheduler_t;

static inline void persistence_note_change(persistence_scheduler_t *scheduler,
                                           uint64_t now_us) {
    scheduler->pending = true;
    scheduler->last_change_us = now_us;
}

static inline bool persistence_is_due(const persistence_scheduler_t *scheduler,
                                      uint64_t now_us) {
    return scheduler->pending &&
           now_us - scheduler->last_change_us >= scheduler->debounce_us;
}

static inline void persistence_note_saved(persistence_scheduler_t *scheduler) {
    scheduler->pending = false;
}

#endif
