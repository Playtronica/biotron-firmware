#include <string.h>

#include "PLSDK/midi_parser.h"

static uint8_t cin_payload_length(uint8_t cin) {
    static const uint8_t lengths[16] = {
        0, 0, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1
    };
    return lengths[cin & 0x0f];
}

void midi_parser_init(midi_parser_t *parser) {
    memset(parser, 0, sizeof(*parser));
}

static midi_event_kind_t malformed(midi_parser_t *parser, midi_event_t *event) {
    parser->in_sysex = false;
    parser->sysex_len = 0;
    event->kind = MIDI_EVENT_MALFORMED;
    event->len = 0;
    return event->kind;
}

midi_event_kind_t midi_parser_feed_usb_packet(midi_parser_t *parser,
                                               const uint8_t packet[4],
                                               midi_event_t *event) {
    const uint8_t cin = packet[0] & 0x0f;
    const uint8_t payload_len = cin_payload_length(cin);
    event->kind = MIDI_EVENT_NONE;
    event->len = 0;

    if (payload_len == 0) return malformed(parser, event);

    if (cin >= 0x4 && cin <= 0x7) {
        if (!parser->in_sysex) {
            if (packet[1] != 0xf0) return malformed(parser, event);
            parser->in_sysex = true;
            parser->sysex_len = 0;
        }
        if (parser->sysex_len + payload_len > MIDI_PARSER_SYSEX_CAPACITY) {
            return malformed(parser, event);
        }
        memcpy(&parser->sysex[parser->sysex_len], &packet[1], payload_len);
        parser->sysex_len += payload_len;

        if (cin == 0x4) return MIDI_EVENT_NONE;
        if (parser->sysex[parser->sysex_len - 1] != 0xf7) {
            return malformed(parser, event);
        }
        event->kind = MIDI_EVENT_SYSEX;
        event->len = parser->sysex_len;
        memcpy(event->data, parser->sysex, event->len);
        parser->in_sysex = false;
        parser->sysex_len = 0;
        return event->kind;
    }

    /* MIDI realtime may legally be interleaved with a SysEx stream. */
    if (cin == 0x0f && packet[1] >= 0xf8) {
        event->kind = MIDI_EVENT_REALTIME;
        event->len = 1;
        event->data[0] = packet[1];
        return event->kind;
    }

    if (parser->in_sysex) {
        parser->in_sysex = false;
        parser->sysex_len = 0;
    }
    event->kind = MIDI_EVENT_CHANNEL;
    event->len = payload_len;
    memcpy(event->data, &packet[1], payload_len);
    return event->kind;
}
