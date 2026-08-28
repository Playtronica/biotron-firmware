#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "PLSDK/commands.h"
#include "PLSDK/constants.h"

#define QUEUE_CAPACITY 4096

extern uint8_t length_cc;
extern uint8_t length_sys;

bool LOGGER_FLAG = false;
static uint8_t queue[QUEUE_CAPACITY][4];
static size_t queue_read;
static size_t queue_write;
static size_t remind_calls;
static size_t cc_calls;
static size_t expected_cc;
static uint8_t sysex_values[8];
static size_t sysex_calls;
static size_t query_calls;

static void enqueue(uint8_t header, uint8_t a, uint8_t b, uint8_t c) {
    assert(queue_write < QUEUE_CAPACITY);
    queue[queue_write][0] = header;
    queue[queue_write][1] = a;
    queue[queue_write][2] = b;
    queue[queue_write][3] = c;
    ++queue_write;
}

bool tud_midi_packet_read(uint8_t packet[4]) {
    if (queue_read == queue_write) return false;
    memcpy(packet, queue[queue_read++], 4);
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
    assert(channel == (expected_cc & 1u));
    assert(value == (expected_cc & 0x7fu));
    ++expected_cc;
    ++cc_calls;
}

static void capture_sysex(const uint8_t data[], uint8_t length) {
    assert(length == 1 && sysex_calls < sizeof(sysex_values));
    sysex_values[sysex_calls++] = data[0];
}

static void capture_query(const uint8_t data[], uint8_t length) {
    assert(length == 1 && data[0] == 77);
    ++query_calls;
}

static void ignored_cc(uint8_t channel, uint8_t value) {
    (void)channel;
    (void)value;
}

static void ignored_sysex(const uint8_t data[], uint8_t length) {
    (void)data;
    (void)length;
}

static void test_1000_cc_are_not_dropped(void) {
    add_CC(capture_cc, 7);
    for (size_t index = 0; index < 1000; ++index) {
        const uint8_t channel = (uint8_t)(index & 1u);
        enqueue(0x0b, (uint8_t)(CC_START + channel), 7,
                (uint8_t)(index & 0x7fu));
    }
    for (size_t index = 0; index < 1000; ++index) {
        assert(read_sys_ex() == CUSTOM_CC_COMMAND);
    }
    assert(cc_calls == 1000 && expected_cc == 1000);
    assert(queue_read == queue_write && remind_calls == 1000);
}

static void test_two_cable_sysex_isolation_and_realtime(void) {
    add_sys_ex_com(capture_sysex, 42);
    enqueue(0x04, 0xf0, PLAYTRONICA_KEY_FIRST, PLAYTRONICA_KEY_SECOND);
    enqueue(0x14, 0xf0, PLAYTRONICA_KEY_FIRST, PLAYTRONICA_KEY_SECOND);
    enqueue(0x0f, BPM_CLOCK_BYTE, 0, 0);
    enqueue(0x17, 42, 22, 0xf7);
    enqueue(0x07, 42, 11, 0xf7);
    assert(read_sys_ex() == MIDI_PACKET_IGNORED);
    assert(read_sys_ex() == MIDI_PACKET_IGNORED);
    assert(read_sys_ex() == MIDI_PACKET_IGNORED);
    assert(read_sys_ex() == CUSTOM_COMMAND);
    assert(read_sys_ex() == CUSTOM_COMMAND);
    assert(sysex_calls == 2 && sysex_values[0] == 22 && sysex_values[1] == 11);
}

static void test_query_status_and_malformed_recovery(void) {
    add_sys_ex_query(capture_query, 43);
    enqueue(0x04, 0xf0, PLAYTRONICA_KEY_FIRST, PLAYTRONICA_KEY_SECOND);
    enqueue(0x07, 43, 77, 0xf7);
    assert(read_sys_ex() == MIDI_PACKET_IGNORED);
    assert(read_sys_ex() == CUSTOM_QUERY_COMMAND);
    assert(query_calls == 1);

    enqueue(0x00, 0, 0, 0);
    enqueue(0x2b, 0xb0, 7, 104);
    enqueue(0x0b, 0xb0, 7, 104);
    assert(read_sys_ex() == MIDI_PACKET_IGNORED);
    assert(read_sys_ex() == MIDI_PACKET_IGNORED);
    assert(read_sys_ex() == CUSTOM_CC_COMMAND);
    assert(cc_calls == 1001);
}

static void test_clock_is_exactly_24_ppqn(void) {
    enqueue(0x0f, BPM_CLOCK_START_BYTE, 0, 0);
    assert(read_sys_ex() == BPM_CLOCK_ACTIVATE);
    for (int pulse = 1; pulse < 24; ++pulse) {
        enqueue(0x0f, BPM_CLOCK_BYTE, 0, 0);
        assert(read_sys_ex() == BPM_CLOCK_INACTIVE);
    }
    enqueue(0x0f, BPM_CLOCK_BYTE, 0, 0);
    assert(read_sys_ex() == BPM_CLOCK_PLAY);
    enqueue(0x0f, BPM_CLOCK_STOP_BYTE, 0, 0);
    assert(read_sys_ex() == BPM_CLOCK_DEACTIVATE);
    enqueue(0x0f, BPM_CLOCK_BYTE, 0, 0);
    assert(read_sys_ex() == MIDI_PACKET_IGNORED);
}

static void test_registries_fail_closed_at_capacity(void) {
    while (length_cc < MAX_COUNT_COMMANDS) add_CC(ignored_cc, length_cc);
    while (length_sys < MAX_COUNT_COMMANDS) {
        add_sys_ex_com(ignored_sysex, length_sys);
    }
    for (int attempt = 0; attempt < 10; ++attempt) {
        add_CC(ignored_cc, (uint8_t)attempt);
        add_sys_ex_com(ignored_sysex, (uint8_t)attempt);
    }
    assert(length_cc == MAX_COUNT_COMMANDS && length_sys == MAX_COUNT_COMMANDS);
}

int main(void) {
    test_1000_cc_are_not_dropped();
    test_two_cable_sysex_isolation_and_realtime();
    test_query_status_and_malformed_recovery();
    test_clock_is_exactly_24_ppqn();
    test_registries_fail_closed_at_capacity();
    assert(read_sys_ex() == UNKNOWN);
    puts("commands_integration: burst, two-cable SysEx, query and Clock passed");
    return 0;
}
