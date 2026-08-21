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

static void expect_bytes(const midi_event_t *event, const uint8_t *expected,
                         size_t length) {
    assert(event->len == length);
    for (size_t i = 0; i < length; ++i) assert(event->data[i] == expected[i]);
}

static void test_channel_and_system_message_lengths(void) {
    static const struct {
        uint8_t cin;
        uint8_t status;
        size_t length;
    } cases[] = {
        {0x02, 0xf1, 2}, {0x03, 0xf2, 3}, {0x08, 0x80, 3},
        {0x09, 0x90, 3}, {0x0a, 0xa0, 3}, {0x0b, 0xb0, 3},
        {0x0c, 0xc0, 2}, {0x0d, 0xd0, 2}, {0x0e, 0xe0, 3},
        {0x0f, 0xf6, 1},
    };
    midi_parser_t parser;
    midi_event_t event;
    midi_parser_init(&parser);

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        assert(feed(&parser, &event, cases[i].cin, cases[i].status, 7, 9) ==
               MIDI_EVENT_CHANNEL);
        assert(event.len == cases[i].length);
        assert(event.data[0] == cases[i].status);
    }
}

static void test_cable_number_does_not_change_payload(void) {
    midi_parser_t parser;
    midi_event_t event;
    const uint8_t expected[] = {0xb1, 31, 127};
    midi_parser_init(&parser);

    for (uint8_t cable = 0; cable < 16; ++cable) {
        assert(feed(&parser, &event, (uint8_t)((cable << 4) | 0x0b),
                    0xb1, 31, 127) == MIDI_EVENT_CHANNEL);
        expect_bytes(&event, expected, sizeof(expected));
    }
}

static void test_consecutive_cc_is_not_coalesced_by_parser(void) {
    midi_parser_t parser;
    midi_event_t event;
    midi_parser_init(&parser);

    for (uint16_t value = 0; value < 128; ++value) {
        assert(feed(&parser, &event, 0x0b, 0xb0, 7, (uint8_t)value) ==
               MIDI_EVENT_CHANNEL);
        assert(event.len == 3 && event.data[1] == 7 &&
               event.data[2] == (uint8_t)value);
    }
}

static void test_all_sysex_end_packet_shapes(void) {
    midi_parser_t parser;
    midi_event_t event;

    midi_parser_init(&parser);
    assert(feed(&parser, &event, 0x06, 0xf0, 0xf7, 0) == MIDI_EVENT_SYSEX);
    {
        const uint8_t expected[] = {0xf0, 0xf7};
        expect_bytes(&event, expected, sizeof(expected));
    }

    midi_parser_init(&parser);
    assert(feed(&parser, &event, 0x07, 0xf0, 1, 0xf7) == MIDI_EVENT_SYSEX);
    {
        const uint8_t expected[] = {0xf0, 1, 0xf7};
        expect_bytes(&event, expected, sizeof(expected));
    }

    midi_parser_init(&parser);
    assert(feed(&parser, &event, 0x04, 0xf0, 0x14, 0x0d) == MIDI_EVENT_NONE);
    assert(feed(&parser, &event, 0x05, 0xf7, 0, 0) == MIDI_EVENT_SYSEX);
    {
        const uint8_t expected[] = {0xf0, 0x14, 0x0d, 0xf7};
        expect_bytes(&event, expected, sizeof(expected));
    }

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
        assert(event.len == 1 && event.data[0] == (uint8_t)status);
        assert(parser.in_sysex && parser.sysex_len == 3);
    }
    assert(feed(&parser, &event, 0x07, 1, 2, 0xf7) == MIDI_EVENT_SYSEX);
    assert(event.len == 6 && event.data[0] == 0xf0 && event.data[5] == 0xf7);
}

static void test_exact_capacity_and_overflow_recovery(void) {
    midi_parser_t parser;
    midi_event_t event;
    midi_parser_init(&parser);

    for (int packet = 0; packet < 99; ++packet) {
        const uint8_t first = packet == 0 ? 0xf0 : 1;
        assert(feed(&parser, &event, 0x04, first, 2, 3) == MIDI_EVENT_NONE);
    }
    assert(parser.sysex_len == 297);
    assert(feed(&parser, &event, 0x07, 4, 5, 0xf7) == MIDI_EVENT_SYSEX);
    assert(event.len == MIDI_PARSER_SYSEX_CAPACITY);
    assert(event.data[0] == 0xf0 &&
           event.data[MIDI_PARSER_SYSEX_CAPACITY - 1] == 0xf7);

    midi_parser_init(&parser);
    for (int packet = 0; packet < 100; ++packet) {
        const uint8_t first = packet == 0 ? 0xf0 : 1;
        assert(feed(&parser, &event, 0x04, first, 2, 3) == MIDI_EVENT_NONE);
    }
    assert(parser.sysex_len == MIDI_PARSER_SYSEX_CAPACITY);
    assert(feed(&parser, &event, 0x05, 0xf7, 0, 0) == MIDI_EVENT_MALFORMED);
    assert(!parser.in_sysex && parser.sysex_len == 0 && event.len == 0);

    assert(feed(&parser, &event, 0x0b, 0xb0, 7, 64) == MIDI_EVENT_CHANNEL);
    assert(event.len == 3 && event.data[2] == 64);
}

static void test_malformed_streams_fail_closed_and_recover(void) {
    midi_parser_t parser;
    midi_event_t event;
    midi_parser_init(&parser);

    assert(feed(&parser, &event, 0x00, 0, 0, 0) == MIDI_EVENT_MALFORMED);
    assert(feed(&parser, &event, 0x01, 0, 0, 0) == MIDI_EVENT_MALFORMED);
    assert(feed(&parser, &event, 0x04, 1, 2, 3) == MIDI_EVENT_MALFORMED);
    assert(feed(&parser, &event, 0x07, 0xf0, 1, 2) == MIDI_EVENT_MALFORMED);

    assert(feed(&parser, &event, 0x04, 0xf0, 1, 2) == MIDI_EVENT_NONE);
    assert(feed(&parser, &event, 0x0b, 0xb0, 1, 2) == MIDI_EVENT_CHANNEL);
    assert(!parser.in_sysex && event.data[0] == 0xb0);

    assert(feed(&parser, &event, 0x05, 0xf7, 0, 0) == MIDI_EVENT_MALFORMED);
    assert(feed(&parser, &event, 0x0b, 0xb0, 1, 3) == MIDI_EVENT_CHANNEL);
    assert(event.data[2] == 3);
}

static void test_deterministic_packet_fuzz_invariants(void) {
    midi_parser_t parser;
    midi_event_t event;
    uint32_t state = 0x5eed1234u;
    midi_parser_init(&parser);

    for (int i = 0; i < 250000; ++i) {
        uint8_t bytes[4];
        for (int j = 0; j < 4; ++j) {
            state = state * 1664525u + 1013904223u;
            bytes[j] = (uint8_t)(state >> 24);
        }
        const midi_event_kind_t kind = midi_parser_feed_usb_packet(
            &parser, bytes, &event);
        assert(parser.sysex_len <= MIDI_PARSER_SYSEX_CAPACITY);
        assert(event.len <= MIDI_PARSER_SYSEX_CAPACITY);
        if (parser.in_sysex) {
            assert(parser.sysex_len > 0 && parser.sysex[0] == 0xf0);
        }
        if (kind == MIDI_EVENT_SYSEX) {
            assert(event.len >= 2 && event.data[0] == 0xf0 &&
                   event.data[event.len - 1] == 0xf7);
        } else if (kind == MIDI_EVENT_REALTIME) {
            assert(event.len == 1 && event.data[0] >= 0xf8);
        } else if (kind == MIDI_EVENT_CHANNEL) {
            assert(event.len >= 1 && event.len <= 3);
        } else if (kind == MIDI_EVENT_MALFORMED) {
            assert(event.len == 0 && !parser.in_sysex);
        } else {
            assert(kind == MIDI_EVENT_NONE && event.len == 0);
        }
    }
}

int main(void) {
    test_channel_and_system_message_lengths();
    test_cable_number_does_not_change_payload();
    test_consecutive_cc_is_not_coalesced_by_parser();
    test_all_sysex_end_packet_shapes();
    test_realtime_interleaving_preserves_sysex();
    test_exact_capacity_and_overflow_recovery();
    test_malformed_streams_fail_closed_and_recover();
    test_deterministic_packet_fuzz_invariants();
    puts("midi_parser: all deterministic tests passed");
    return 0;
}
