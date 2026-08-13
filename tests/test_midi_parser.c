#include <assert.h>
#include <stdio.h>
#include "PLSDK/midi_parser.h"

static midi_event_kind_t feed(midi_parser_t *p, midi_event_t *e,
                              uint8_t cin, uint8_t a, uint8_t b, uint8_t c) {
    const uint8_t packet[4] = {cin, a, b, c};
    return midi_parser_feed_usb_packet(p, packet, e);
}

int main(void) {
    midi_parser_t parser;
    midi_event_t event;
    midi_parser_init(&parser);

    assert(feed(&parser, &event, 0x0b, 0xb0, 7, 10) == MIDI_EVENT_CHANNEL);
    assert(event.len == 3 && event.data[1] == 7 && event.data[2] == 10);
    assert(feed(&parser, &event, 0x0b, 0xb0, 7, 11) == MIDI_EVENT_CHANNEL);
    assert(event.data[2] == 11);

    assert(feed(&parser, &event, 0x04, 0xf0, 0x14, 0x0d) == MIDI_EVENT_NONE);
    assert(feed(&parser, &event, 0x0f, 0xf8, 0, 0) == MIDI_EVENT_REALTIME);
    assert(parser.in_sysex);
    assert(feed(&parser, &event, 0x07, 0x01, 0x22, 0xf7) == MIDI_EVENT_SYSEX);
    assert(event.len == 6 && event.data[0] == 0xf0 && event.data[5] == 0xf7);

    assert(feed(&parser, &event, 0x0f, 0xf8, 0, 0) == MIDI_EVENT_REALTIME);
    assert(event.len == 1 && event.data[0] == 0xf8);

    assert(feed(&parser, &event, 0x04, 0xf0, 1, 2) == MIDI_EVENT_NONE);
    assert(feed(&parser, &event, 0x0b, 0xb0, 1, 2) == MIDI_EVENT_CHANNEL);
    assert(!parser.in_sysex && event.data[0] == 0xb0);

    assert(feed(&parser, &event, 0x04, 0xf0, 1, 2) == MIDI_EVENT_NONE);
    for (int i = 0; i < 98; ++i)
        assert(feed(&parser, &event, 0x04, 1, 2, 3) == MIDI_EVENT_NONE);
    assert(feed(&parser, &event, 0x04, 1, 2, 3) == MIDI_EVENT_NONE);
    assert(feed(&parser, &event, 0x04, 1, 2, 3) == MIDI_EVENT_MALFORMED);
    assert(!parser.in_sysex && parser.sysex_len == 0);

    assert(feed(&parser, &event, 0x00, 0, 0, 0) == MIDI_EVENT_MALFORMED);
    puts("midi_parser: all tests passed");
    return 0;
}
