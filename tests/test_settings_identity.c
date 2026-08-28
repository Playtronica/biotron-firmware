#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "params.h"

int main(void) {
    Settings_t settings = {0};
    settings.id = ID_FLASH;

    assert(ID_FLASH == UINT32_C(3085757033));
    assert(settings.id == ID_FLASH);
    assert(sizeof(settings.id) == sizeof(uint32_t));
    puts("settings_identity: large build IDs normalize to stable uint32 values");
    return 0;
}
