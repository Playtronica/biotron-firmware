#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "PLSDK/constants.h"
#include "PLSDK/music.h"

static uint8_t captured_cable;
static uint8_t captured[3];
static uint8_t captured_length;

bool print_pure(uint8_t cable, const uint8_t data[], uint8_t length) {
    assert(length <= sizeof(captured));
    captured_cable = cable;
    captured_length = length;
    memcpy(captured, data, length);
    return true;
}

static void expect(uint8_t status, uint8_t data1, uint8_t data2) {
    assert(captured_cable == CABLE_NUM_MAIN);
    assert(captured_length == 3);
    assert(captured[0] == status);
    assert(captured[1] == data1);
    assert(captured[2] == data2);
}

int main(void) {
    note_on(1, 60, 100);
    expect(0x91, 60, 100); /* shipping human MIDI channel 2 */

    note_off(2, 61);
    expect(0x82, 61, 0); /* shipping human MIDI channel 3 */

    stop_all_notes(1);
    expect(0xb1, CC_STOP_ALL_NOTES, 0);

    change_pitch(1, 0, 64);
    expect(0xe1, 0, 64);

    change_pitch(31, 255, 255);
    expect(0xef, 127, 127);

    note_on(31, 255, 255);
    expect(0x9f, 127, 127);

    assert(calculate_note_by_scale(60, 0, SCALE_MIXOLYDIAN) == 60);
    assert(calculate_note_by_scale(60, 7, SCALE_MIXOLYDIAN) == 72);
    assert(calculate_note_by_scale(60, 1, (ScaleNums_t)255) == 62);

    puts("music_v1_contract: shipping MIDI bytes and scale vectors passed");
    return 0;
}
