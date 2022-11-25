#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "include/frequency_counter.h"

#define STAB_TIME 40
#define AVERAGE_TIME 40
#define SLEEP_TIME 24

#define PLANT_PIN  1
#define BlUE_LED 0
#define FIRST_GREEN_LED 2
#define SECOND_GREEN_LED 3
#define PHOTORESISTORS_PIN 4
#define TEST_LED 25

#define TIMER_MS 125
#define TIMER_MULTIPLIER (1000 / TIMER_MS)
#define MIN_FREQ 60

uint32_t freq;
const float sensitivity = 0.9;
int countActiveValues = 0;
bool sleepMode = true;

int frequency;

int averageFreq = 0;


enum Status {
    Sleep,
    Avg,
    Active
};

enum Status status = Sleep;

void setup() {
    gpio_init(PLANT_PIN);
    gpio_init(BlUE_LED);
    gpio_init(FIRST_GREEN_LED);
    gpio_init(SECOND_GREEN_LED);
    gpio_init(TEST_LED);

    gpio_set_dir(PLANT_PIN, GPIO_IN);
    gpio_set_dir(BlUE_LED, GPIO_OUT);
    gpio_set_dir(FIRST_GREEN_LED, GPIO_OUT);
    gpio_set_dir(SECOND_GREEN_LED, GPIO_OUT);
    gpio_set_dir(TEST_LED, GPIO_OUT);
}

int filVal = 0;
float k = 0.3;
int expRunningAverage(float newVal) {
    filVal += (newVal - filVal) * k;
    return filVal;
}

float percentChange(int oldVal, int newVal) {
    return ((newVal - oldVal) / (float) oldVal) * 100;
}

void frequencyWork() {
    switch (status) {
        case Sleep:
            filVal = 0;
            if (freq > MIN_FREQ) {
                countActiveValues++;
            } else {
                countActiveValues = 0;
            }

            if (countActiveValues >= STAB_TIME) {
                countActiveValues = 0;
                averageFreq = 0;
                status = Avg;
                printf("[+] Change status: Sleep -> Stab\n");
            }
            break;
        case Avg:
            if (freq > MIN_FREQ) {
                if (filVal == 0) {
                    filVal = freq;
                }
                countActiveValues++;
                int b = expRunningAverage(freq);
                printf("[TEST] %d\n", b);
                averageFreq += b;
            } else {
                averageFreq = 0;
                countActiveValues = 0;
                status = Sleep;
                printf("[+] Change status: Stab -> Sleep\n");
            }

            if (countActiveValues > AVERAGE_TIME) {
                averageFreq /= countActiveValues;
                countActiveValues = 0;
                status = Active;
                printf("[+] Change status: Stab -> Active\n");
            }
            break;
        case Active:
            if (freq < MIN_FREQ) {
                countActiveValues++;
            } else {
                countActiveValues = 0;
            }

            if (countActiveValues > SLEEP_TIME) {
                countActiveValues = 0;
                averageFreq = 0;
                status = Sleep;
                printf("[+] Change status: Active -> Sleep\n");
            }
            break;
    }
}

void ledWork() {
    gpio_put(BlUE_LED, 0);
    gpio_put(FIRST_GREEN_LED, 0);
    gpio_put(SECOND_GREEN_LED, 0);
    switch (status) {
        case Sleep:
            gpio_put(BlUE_LED, 1);
            break;
        case Avg:
            gpio_put(FIRST_GREEN_LED, 1);
            break;
        case Active:
            gpio_put(FIRST_GREEN_LED, 1);
            gpio_put(SECOND_GREEN_LED, 1);
            break;
    }
}

void midi_task(void);


int main(void)
{
    stdio_init_all();
    setup();
    sleep_ms(3000);

    gpio_put(TEST_LED, 1);
    tusb_init();

    beginTimer(PLANT_PIN, TIMER_MS);
    
    while (true)
    {
        tud_task();
        gpio_put(TEST_LED, 1);
        if (isReady()) {

            freq = getFreq() * TIMER_MULTIPLIER;
            if (freq > MIN_FREQ) {
                gpio_put(TEST_LED, 1);
            }
            frequencyWork();
            ledWork();
            if (status == Active) {
                midi_task();
            }
        }
        sleep_ms(10);
    }

}

uint32_t note_pos = 0;

// Store example melody as an array of note values
uint8_t note_sequence[] = {
        74,78,81,86,90,93,98,102,57,61,66,69,73,78,81,
        85,88,92,97,100,97,92,88,85,81,78,
        74,69,66,62,57,62,66,69,74,78,81,86,90,93,
        97,102,97,93,90,85,81,78,73,68,64,61,
        56,61,64,68,74,78,81,86,90,93,98,102
};

void midi_task(void)
{
    static uint32_t start_ms = 0;

    uint8_t const cable_num = 0; // MIDI jack associated with USB endpoint
    uint8_t const channel   = 0; // 0 for channel 1

    // The MIDI interface always creates input and output port/jack descriptors
    // regardless of these being used or not. Therefore incoming traffic should be read
    // (possibly just discarded) to avoid the sender blocking in IO
    uint8_t packet[4];
    while ( tud_midi_available() ) tud_midi_packet_read(packet);

    // send note periodically
    if (board_millis() - start_ms < 286) return; // not enough time
    start_ms += 286;

    // Previous positions in the note sequence.
    int previous = (int) (note_pos - 1);

    // If we currently are at position 0, set the
    // previous position to the last note in the sequence.
    if (previous < 0) previous = sizeof(note_sequence) - 1;

    // Send Note On for current position at full velocity (127) on channel 1.
    uint8_t note_on[3] = { 0x90 | channel, note_sequence[0], 127 };
    tud_midi_stream_write(cable_num, note_on, 3);

    // Send Note Off for previous note.
    uint8_t note_off[3] = { 0x80 | channel, note_sequence[previous], 0};
    tud_midi_stream_write(cable_num, note_off, 3);

    // Increment position
    note_pos++;

    // If we are at the end of the sequence, start over.
    if (note_pos >= sizeof(note_sequence)) note_pos = 0;
}
