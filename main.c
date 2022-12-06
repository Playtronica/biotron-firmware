#include <stdlib.h>
#include <stdio.h>
#include <hardware/adc.h>

#include "tusb.h"
#include "frequency_counter.h"
#include "notes.h"
#include "global.h"


void midi_plant(void);
void midi_light(void);

int main(void)
{
    stdio_init_all();
    Setup();
    sleep_ms(3000);

    gpio_put(TEST_LED, 1);
    tusb_init();

    beginTimer(PLANT_PIN, TIMER_MS);

    int count = 0;
    while (true)
    {
        tud_task();
        gpio_put(TEST_LED, 1);
        if (isReady()) {

            realFrequency = getFreq() * TIMER_MULTIPLIER;
            if (realFrequency > MIN_FREQ) {
                gpio_put(TEST_LED, 1);
            }
            FrequencyStage();
            LedStage(status);
            if (status == Active) {
                midi_plant();
                count++;
                if (count == TIMER_MULTIPLIER * 2) {
                    midi_light();
                }
            }
        } else {
            count = 0;
        }
        sleep_ms(10);
    }

}


uint32_t currentNotePlant;
uint32_t previousNotePlant;
bool touch;
void midi_plant(void) {
    uint8_t const cable_num = 0;
    uint8_t channel = 1;

    uint8_t packet[4];
    while (tud_midi_available()) tud_midi_packet_read(packet);

    previousNotePlant = currentNotePlant;

    currentNotePlant = getNote(GetNoteDiff(averageFreq, realFrequency));

    uint8_t note_on[3] = {0x90 | channel, currentNotePlant, 127};
    if (currentNotePlant != -1) {
        tud_midi_stream_write(cable_num, note_on, 3);
    }
    else {
        if (previousNotePlant != -1) {
            uint32_t *notes = getOctaveNotes();
            for (int i = 0; i < getLengthOctave(); i++) {
                note_on[1] = notes[i];
                tud_midi_stream_write(cable_num, note_on, 3);
            }
        }
    }

    uint8_t note_off[3] = {0x80 | channel, previousNotePlant, 0};
    if (previousNotePlant != -1) {
        tud_midi_stream_write(cable_num, note_off, 3);
    }
    else {
        if (currentNotePlant != -1) {
            uint32_t *notes = getOctaveNotes();
            for (int i = 0; i < getLengthOctave(); i++) {
                note_off[1] = notes[i];
                tud_midi_stream_write(cable_num, note_off, 3);
            }
        }
    }
}

uint32_t currentLightPlant;
uint32_t previousLightPlant;
void midi_light(void) {
    uint8_t const cable_num = 0;
    uint8_t channel = 2;

    uint8_t packet[4];
    while (tud_midi_available()) tud_midi_packet_read(packet);

    previousLightPlant = currentLightPlant;
    int a = 4000 / 24;
    currentLightPlant = (4000 - adc_read()) / a + 24;

    uint8_t note_on[3] = {0x90 | channel, currentLightPlant, 127};
    tud_midi_stream_write(cable_num, note_on, 3);

    uint8_t note_off[3] = {0x80 | channel, previousLightPlant, 0};
    tud_midi_stream_write(cable_num, note_off, 3);
}

