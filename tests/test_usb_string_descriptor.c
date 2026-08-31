#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pico/unique_id.h"
#include "tusb.h"

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid);

void pico_get_unique_board_id_string(char *destination, size_t length) {
    static const char serial[] = "0123456789ABCDEF";
    assert(length >= sizeof(serial));
    memcpy(destination, serial, sizeof(serial));
}

static void expect_ascii(uint8_t index, const char *expected) {
    const uint16_t *descriptor = tud_descriptor_string_cb(index, 0x0409);
    assert(descriptor != NULL);
    const size_t length = descriptor[0] & 0xffu;
    assert(length == 2u + strlen(expected) * 2u);
    for (size_t i = 0; expected[i] != '\0'; ++i) {
        assert(descriptor[i + 1] == (uint8_t)expected[i]);
    }
}

int main(void) {
    const uint16_t *language = tud_descriptor_string_cb(0, 0);
    assert(language != NULL && (language[0] & 0xffu) == 4u);
    assert(language[1] == 0x0409);
    expect_ascii(1, "Playtronica");
    expect_ascii(2, "Biotron");
    expect_ascii(3, "0123456789ABCDEF");
    expect_ascii(4, "TinyUSB CDC");
    assert(tud_descriptor_string_cb(5, 0x0409) == NULL);
    for (int iteration = 0; iteration < 10000; ++iteration) {
        expect_ascii(3, "0123456789ABCDEF");
    }
    puts("usb_string_descriptor: repeated production callback passed");
    return 0;
}
