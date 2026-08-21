#ifndef TEST_TUSB_H
#define TEST_TUSB_H

#include <stdbool.h>
#include <stdint.h>

bool tud_midi_packet_read(uint8_t packet[4]);
uint32_t tud_midi_stream_write(uint8_t cable, const uint8_t *data,
                               uint32_t length);

#endif
