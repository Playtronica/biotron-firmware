#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "params.h"
#include "PLSDK/constants.h"
#include "PLSDK/music.h"

static uint8_t captured_cable;
static uint8_t captured[3];
static uint8_t captured_length;

void print_pure(uint8_t cable, const uint8_t data[], uint8_t length) {
    assert(length <= sizeof(captured));
    captured_cable = cable;
    captured_length = length;
    memcpy(captured, data, length);
}

static void assert_message(uint8_t status, uint8_t data1, uint8_t data2) {
    assert(captured_cable == CABLE_NUM_MAIN);
    assert(captured_length == 3);
    assert(captured[0] == status);
    assert(captured[1] == data1);
    assert(captured[2] == data2);
}

int main(void) {
    assert(calculate_note_by_scale(60, 1, (ScaleNums_t)255) ==
           calculate_note_by_scale(60, 1, SCALE_MAJOR));

    note_on(DEFAULT_PLANT_MIDI_CHANNEL, 60, 100);
    assert_message(0x90, 60, 100); // Human-readable MIDI channel 1.

    note_on(DEFAULT_LIGHT_MIDI_CHANNEL, 61, 101);
    assert_message(0x91, 61, 101); // Human-readable MIDI channel 2.

    note_off(DEFAULT_PLANT_MIDI_CHANNEL, 60);
    assert_message(0x80, 60, 0);

    stop_all_notes(DEFAULT_LIGHT_MIDI_CHANNEL);
    assert_message(0xb1, CC_STOP_ALL_NOTES, 0);

    puts("music_messages: default channel and Note Off bytes verified");
    return 0;
}
