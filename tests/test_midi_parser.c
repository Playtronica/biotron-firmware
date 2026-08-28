#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "PLSDK/midi_parser.h"

static midi_event_kind_t feed(midi_parser_t *parser, midi_event_t *event,
                              uint8_t header, uint8_t a, uint8_t b, uint8_t c) {
    const uint8_t packet[4] = {header, a, b, c};
    return midi_parser_feed_usb_packet(parser, packet, event);
}

static void test_channel_lengths_and_consecutive_cc(void) {
    midi_parser_t parser;
    midi_event_t event;
    midi_parser_init(&parser);
    for (uint16_t value = 0; value < 128; ++value) {
        assert(feed(&parser, &event, 0x0b, 0xb0, 7, (uint8_t)value) ==
               MIDI_EVENT_CHANNEL);
        assert(event.len == 3 && event.data[0] == 0xb0 &&
               event.data[1] == 7 && event.data[2] == (uint8_t)value);
    }
    assert(feed(&parser, &event, 0x0c, 0xc0, 9, 0) == MIDI_EVENT_CHANNEL);
    assert(event.len == 2);
    assert(feed(&parser, &event, 0x0f, 0xf6, 0, 0) == MIDI_EVENT_CHANNEL);
    assert(event.len == 1);
}

static void test_all_sysex_end_shapes(void) {
    midi_parser_t parser;
    midi_event_t event;

    midi_parser_init(&parser);
    assert(feed(&parser, &event, 0x06, 0xf0, 0xf7, 0) == MIDI_EVENT_SYSEX);
    assert(event.len == 2 && event.data[1] == 0xf7);

    midi_parser_init(&parser);
    assert(feed(&parser, &event, 0x07, 0xf0, 1, 0xf7) == MIDI_EVENT_SYSEX);
    assert(event.len == 3 && event.data[2] == 0xf7);

    midi_parser_init(&parser);
    assert(feed(&parser, &event, 0x04, 0xf0, 0x14, 0x0d) == MIDI_EVENT_NONE);
    assert(feed(&parser, &event, 0x05, 0xf7, 0, 0) == MIDI_EVENT_SYSEX);
    assert(event.len == 4 && event.data[3] == 0xf7);

    midi_parser_init(&parser);
    assert(feed(&parser, &event, 0x04, 0xf0, 0x14, 0x0d) == MIDI_EVENT_NONE);
    assert(feed(&parser, &event, 0x06, 1, 0xf7, 0) == MIDI_EVENT_SYSEX);
    assert(event.len == 5 && event.data[4] == 0xf7);

    midi_parser_init(&parser);
    assert(feed(&parser, &event, 0x04, 0xf0, 0x14, 0x0d) == MIDI_EVENT_NONE);
    assert(feed(&parser, &event, 0x07, 1, 2, 0xf7) == MIDI_EVENT_SYSEX);
    assert(event.len == 6 && event.data[5] == 0xf7);
}

static void test_realtime_interleaving_preserves_sysex(void) {
    midi_parser_t parser;
    midi_event_t event;
    midi_parser_init(&parser);
    assert(feed(&parser, &event, 0x04, 0xf0, 0x14, 0x0d) == MIDI_EVENT_NONE);
    for (int status = 0xf8; status <= 0xff; ++status) {
        assert(feed(&parser, &event, 0x0f, (uint8_t)status, 0, 0) ==
               MIDI_EVENT_REALTIME);
        assert(parser.in_sysex && parser.sysex_len == 3);
    }
    assert(feed(&parser, &event, 0x07, 1, 2, 0xf7) == MIDI_EVENT_SYSEX);
    assert(event.len == 6 && event.data[0] == 0xf0 && event.data[5] == 0xf7);
}

static void test_capacity_overflow_and_recovery(void) {
    midi_parser_t parser;
    midi_event_t event;
    midi_parser_init(&parser);
    for (int packet = 0; packet < 99; ++packet) {
        assert(feed(&parser, &event, 0x04, packet == 0 ? 0xf0 : 1, 2, 3) ==
               MIDI_EVENT_NONE);
    }
    assert(feed(&parser, &event, 0x07, 4, 5, 0xf7) == MIDI_EVENT_SYSEX);
    assert(event.len == MIDI_PARSER_SYSEX_CAPACITY);

    midi_parser_init(&parser);
    for (int packet = 0; packet < 100; ++packet) {
        assert(feed(&parser, &event, 0x04, packet == 0 ? 0xf0 : 1, 2, 3) ==
               MIDI_EVENT_NONE);
    }
    assert(feed(&parser, &event, 0x05, 0xf7, 0, 0) == MIDI_EVENT_MALFORMED);
    assert(!parser.in_sysex && parser.sysex_len == 0);
    assert(feed(&parser, &event, 0x0b, 0xb0, 7, 64) == MIDI_EVENT_CHANNEL);
}

static void test_malformed_recovery(void) {
    midi_parser_t parser;
    midi_event_t event;
    midi_parser_init(&parser);
    assert(feed(&parser, &event, 0x00, 0, 0, 0) == MIDI_EVENT_MALFORMED);
    assert(feed(&parser, &event, 0x04, 1, 2, 3) == MIDI_EVENT_MALFORMED);
    assert(feed(&parser, &event, 0x04, 0xf0, 1, 2) == MIDI_EVENT_NONE);
    assert(feed(&parser, &event, 0x0b, 0xb0, 1, 2) == MIDI_EVENT_CHANNEL);
    assert(!parser.in_sysex);
    assert(feed(&parser, &event, 0x05, 0xf7, 0, 0) == MIDI_EVENT_MALFORMED);
    assert(feed(&parser, &event, 0x0b, 0xb0, 1, 3) == MIDI_EVENT_CHANNEL);
}

static void test_deterministic_fuzz_invariants(void) {
    midi_parser_t parser;
    midi_event_t event;
    uint32_t state = UINT32_C(0x5eed1234);
    midi_parser_init(&parser);
    for (int iteration = 0; iteration < 250000; ++iteration) {
        uint8_t packet[4];
        for (int byte = 0; byte < 4; ++byte) {
            state = state * UINT32_C(1664525) + UINT32_C(1013904223);
            packet[byte] = (uint8_t)(state >> 24);
        }
        const midi_event_kind_t kind = midi_parser_feed_usb_packet(
                &parser, packet, &event);
        assert(parser.sysex_len <= MIDI_PARSER_SYSEX_CAPACITY);
        assert(event.len <= MIDI_PARSER_SYSEX_CAPACITY);
        if (kind == MIDI_EVENT_SYSEX) {
            assert(event.len >= 2 && event.data[0] == 0xf0 &&
                   event.data[event.len - 1] == 0xf7);
        } else if (kind == MIDI_EVENT_REALTIME) {
            assert(event.len == 1 && event.data[0] >= 0xf8);
        } else if (kind == MIDI_EVENT_MALFORMED) {
            assert(event.len == 0 && !parser.in_sysex);
        }
    }
}

int main(void) {
    test_channel_lengths_and_consecutive_cc();
    test_all_sysex_end_shapes();
    test_realtime_interleaving_preserves_sysex();
    test_capacity_overflow_and_recovery();
    test_malformed_recovery();
    test_deterministic_fuzz_invariants();
    puts("midi_parser: boundaries, realtime, recovery and fuzz passed");
    return 0;
}
