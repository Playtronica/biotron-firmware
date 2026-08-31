#ifndef PLSDK_MIDI_TX_H
#define PLSDK_MIDI_TX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MIDI_TX_MAX_MESSAGE_BYTES 259u
#define MIDI_TX_QUEUE_CAPACITY 16u
#define MIDI_TX_RESERVED_CRITICAL 4u

bool midi_tx_enqueue(uint8_t cable, const uint8_t data[], uint16_t length);
void service_midi_tx(void);
size_t midi_tx_pending(void);
uint32_t midi_tx_dropped(void);
size_t midi_tx_recovery_pending(void);

#endif
