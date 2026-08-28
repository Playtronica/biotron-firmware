#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "settings_storage.h"

int main(void) {
    uint8_t source[112];
    uint8_t destination[256];
    for (size_t index = 0; index < sizeof(source); ++index) {
        source[index] = (uint8_t)index;
    }
    assert(settings_storage_pack(destination, sizeof(destination),
                                 source, sizeof(source)));
    for (size_t index = 0; index < sizeof(source); ++index) {
        assert(destination[index] == source[index]);
    }
    for (size_t index = sizeof(source); index < sizeof(destination); ++index) {
        assert(destination[index] == 0xff);
    }
    assert(!settings_storage_pack(destination, 111, source, sizeof(source)));
    assert(!settings_storage_pack(NULL, sizeof(destination), source, sizeof(source)));
    assert(!settings_storage_pack(destination, sizeof(destination), NULL, sizeof(source)));
    assert(STORAGE_ROUND_UP(112, 256) == 256);
    assert(STORAGE_ROUND_UP(256, 256) == 256);
    assert(STORAGE_ROUND_UP(257, 256) == 512);
    puts("settings_storage: exact copy and erased page padding passed");
    return 0;
}
