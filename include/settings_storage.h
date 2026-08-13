#ifndef BIOTRON_SETTINGS_STORAGE_H
#define BIOTRON_SETTINGS_STORAGE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define STORAGE_ROUND_UP(value, alignment) \
    ((((value) + (alignment) - 1) / (alignment)) * (alignment))

static inline bool settings_storage_pack(uint8_t *destination,
                                         size_t destination_size,
                                         const void *settings,
                                         size_t settings_size) {
    if (destination == NULL || settings == NULL ||
        destination_size < settings_size) return false;
    memset(destination, 0xff, destination_size);
    memcpy(destination, settings, settings_size);
    return true;
}

#endif
