#ifndef BIOTRON_SETTINGS_STORAGE_H
#define BIOTRON_SETTINGS_STORAGE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define STORAGE_ROUND_UP(value, alignment) \
    ((((value) + (alignment) - 1u) / (alignment)) * (alignment))

static inline bool settings_storage_pack(uint8_t *destination,
                                         size_t destination_size,
                                         const void *source,
                                         size_t source_size) {
    if (destination == NULL || source == NULL || destination_size < source_size) {
        return false;
    }
    memset(destination, 0xff, destination_size);
    memcpy(destination, source, source_size);
    return true;
}

#endif
