#ifndef PLSDK_MIDI_PARSER_H
#define PLSDK_MIDI_PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MIDI_PARSER_SYSEX_CAPACITY 300

typedef enum {
    MIDI_EVENT_NONE = 0,
    MIDI_EVENT_CHANNEL,
    MIDI_EVENT_REALTIME,
    MIDI_EVENT_SYSEX,
    MIDI_EVENT_MALFORMED,
} midi_event_kind_t;

typedef struct {
    bool in_sysex;
    size_t sysex_len;
    uint8_t sysex[MIDI_PARSER_SYSEX_CAPACITY];
} midi_parser_t;

typedef struct {
    midi_event_kind_t kind;
    size_t len;
    uint8_t data[MIDI_PARSER_SYSEX_CAPACITY];
} midi_event_t;

void midi_parser_init(midi_parser_t *parser);
midi_event_kind_t midi_parser_feed_usb_packet(midi_parser_t *parser,
                                               const uint8_t packet[4],
                                               midi_event_t *event);

#endif
