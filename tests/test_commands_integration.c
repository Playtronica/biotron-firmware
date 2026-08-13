#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "PLSDK/commands.h"
#include "PLSDK/constants.h"

#define QUEUE_CAPACITY 2048

extern uint8_t length_cc;
extern uint8_t length_sys;

bool LOGGER_FLAG = false;

static uint8_t packet_queue[QUEUE_CAPACITY][4];
static size_t queue_read = 0;
static size_t queue_write = 0;
static size_t remind_calls = 0;
static size_t cc_calls = 0;
static size_t cc_mismatches = 0;
static size_t expected_cc_index = 0;
static size_t sysex_calls = 0;
static uint8_t sysex_value = 0;

static void enqueue(uint8_t header, uint8_t a, uint8_t b, uint8_t c) {
    assert(queue_write < QUEUE_CAPACITY);
    packet_queue[queue_write][0] = header;
    packet_queue[queue_write][1] = a;
    packet_queue[queue_write][2] = b;
    packet_queue[queue_write][3] = c;
    ++queue_write;
}

bool tud_midi_packet_read(uint8_t packet[4]) {
    if (queue_read == queue_write) return false;
    memcpy(packet, packet_queue[queue_read], 4);
    ++queue_read;
    return true;
}

uint32_t tud_midi_stream_write(uint8_t cable, const uint8_t *data,
                               uint32_t length) {
    (void)cable;
    (void)data;
    return length;
}

void remind_midi(void) { ++remind_calls; }

void plsdk_printf(const char *format, ...) { (void)format; }

static void capture_cc(uint8_t channel, uint8_t value) {
    const uint8_t expected_channel = (uint8_t)(expected_cc_index & 1u);
    const uint8_t expected_value = (uint8_t)(expected_cc_index & 0x7fu);
    if (channel != expected_channel || value != expected_value) ++cc_mismatches;
    ++expected_cc_index;
    ++cc_calls;
}

static void capture_sysex(const uint8_t data[], uint8_t length) {
    assert(length == 1);
    sysex_value = data[0];
    ++sysex_calls;
}

static void ignored_cc(uint8_t channel, uint8_t value) {
    (void)channel;
    (void)value;
}

static void ignored_sysex(const uint8_t data[], uint8_t length) {
    (void)data;
    (void)length;
}

static void test_queued_cc_order_and_count(void) {
    add_CC(capture_cc, 7);
    for (size_t i = 0; i < 1000; ++i) {
        const uint8_t channel = (uint8_t)(i & 1u);
        enqueue(0x0b, (uint8_t)(CC_START + channel), 7,
                (uint8_t)(i & 0x7fu));
    }
    for (size_t i = 0; i < 1000; ++i) {
        assert(read_sys_ex() == CUSTOM_CC_COMMAND);
    }
    assert(queue_read == queue_write);
    assert(cc_calls == 1000 && expected_cc_index == 1000);
    assert(cc_mismatches == 0 && remind_calls == 1000);
    assert(read_sys_ex() == UNKNOWN);
    assert(remind_calls == 1000);
}

static void test_sysex_with_interleaved_realtime(void) {
    add_sys_ex_com(capture_sysex, 42);
    enqueue(0x04, 0xf0, PLAYTRONICA_KEY_FIRST, PLAYTRONICA_KEY_SECOND);
    enqueue(0x0f, BPM_CLOCK_BYTE, 0, 0);
    enqueue(0x07, 42, 99, 0xf7);

    assert(read_sys_ex() == UNKNOWN);
    assert(read_sys_ex() == UNKNOWN);
    assert(read_sys_ex() == CUSTOM_COMMAND);
    assert(sysex_calls == 1 && sysex_value == 99);
}

static void test_malformed_packet_does_not_poison_next_cc(void) {
    const size_t calls_before = cc_calls;
    expected_cc_index = 0;
    enqueue(0x00, 0, 0, 0);
    enqueue(0x0b, 0xb0, 7, 0);
    assert(read_sys_ex() == UNKNOWN);
    assert(read_sys_ex() == CUSTOM_CC_COMMAND);
    assert(cc_calls == calls_before + 1 && cc_mismatches == 0);
}

static void test_command_registries_fail_closed_at_capacity(void) {
    add_CC(NULL, 99);
    add_sys_ex_com(NULL, 99);
    while (length_cc < MAX_COUNT_COMMANDS) add_CC(ignored_cc, length_cc);
    while (length_sys < MAX_COUNT_COMMANDS) add_sys_ex_com(ignored_sysex, length_sys);
    assert(length_cc == MAX_COUNT_COMMANDS && length_sys == MAX_COUNT_COMMANDS);
    for (int i = 0; i < 10; ++i) {
        add_CC(ignored_cc, (uint8_t)i);
        add_sys_ex_com(ignored_sysex, (uint8_t)i);
    }
    assert(length_cc == MAX_COUNT_COMMANDS && length_sys == MAX_COUNT_COMMANDS);
}

int main(void) {
    test_queued_cc_order_and_count();
    test_sysex_with_interleaved_realtime();
    test_malformed_packet_does_not_poison_next_cc();
    test_command_registries_fail_closed_at_capacity();
    puts("commands: all deterministic integration tests passed");
    return 0;
}
