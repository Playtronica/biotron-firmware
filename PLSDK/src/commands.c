#include "pico/stdlib.h"
#include "PLSDK/commands.h"
#include "PLSDK/constants.h"
#include "PLSDK/midi_parser.h"
#include "PLSDK.h"
#include "tusb.h"


CC_command_s CC[MAX_COUNT_COMMANDS];
uint8_t length_cc = 0;
sys_ex_command_s sys_com[MAX_COUNT_COMMANDS];
uint8_t length_sys = 0;


void add_CC(void action(uint8_t channel, uint8_t value), uint8_t num) {
    if (length_cc >= MAX_COUNT_COMMANDS || action == NULL) return;
    CC_command_s new_CC;
    new_CC.num = num;
    new_CC.action = action;
    CC[length_cc++] = new_CC;
}

void add_sys_ex_com(void action(const uint8_t data[], uint8_t len), uint8_t num) {
    if (length_sys >= MAX_COUNT_COMMANDS || action == NULL) return;
    sys_ex_command_s new_sys_ex_com;
    new_sys_ex_com.num = num;
    new_sys_ex_com.action = action;
    sys_com[length_sys++] = new_sys_ex_com;
}

void print_sys_ex(const uint8_t data[], uint8_t len) {
    uint8_t message[4 + len];
    message[0] = SYS_EX_START;
    message[1] = PLAYTRONICA_KEY_FIRST;
    message[2] = PLAYTRONICA_KEY_SECOND;
    for (int i = 0; i < len; ++i) {
        message[3 + i] = data[i];
    }
    message[3 + len] = SYS_EX_END;
    tud_midi_stream_write(CABLE_NUM_EXTRA, message, 4 + len);
}

void print_pure(uint8_t cable, const uint8_t data[], uint8_t len) {
    tud_midi_stream_write(cable, data, len);
}

static uint8_t clocks_since_beat = 0;
static bool midi_clock_running = false;


int read_sys_ex(void) {
    static midi_parser_t parser;
    static bool parser_initialized = false;
    uint8_t buff[4];
    midi_event_t event;

    if (!parser_initialized) {
        midi_parser_init(&parser);
        parser_initialized = true;
    }
    if (!tud_midi_packet_read(buff)) return UNKNOWN;
    remind_midi();
    midi_event_kind_t kind = midi_parser_feed_usb_packet(&parser, buff, &event);
    if (kind == MIDI_EVENT_NONE || kind == MIDI_EVENT_MALFORMED) return MIDI_PACKET_IGNORED;
    const uint8_t *res = event.data;
    const size_t len = event.len;

    if (len >= 1 && res[0] == BPM_CLOCK_START_BYTE) {
        printf("START\n");
        clocks_since_beat = 0;
        midi_clock_running = true;
        return BPM_CLOCK_ACTIVATE;
    }

    if (len >= 1 && res[0] == BPM_CLOCK_CONTINUE_BYTE) {
        printf("CONTINUE\n");
        clocks_since_beat = 0;
        midi_clock_running = true;
        return BPM_CLOCK_ACTIVATE;
    }

    if (len >= 1 && res[0] == BPM_CLOCK_STOP_BYTE) {
        printf("END\n");
        clocks_since_beat = 0;
        midi_clock_running = false;
        return BPM_CLOCK_DEACTIVATE;
    }

    if (len >= 1 && res[0] == BPM_CLOCK_BYTE) {
        if (!midi_clock_running) return MIDI_PACKET_IGNORED;
        if (++clocks_since_beat < 24) return BPM_CLOCK_INACTIVE;
        clocks_since_beat = 0;
        return BPM_CLOCK_PLAY;
    }


    if (len >= CC_LENGTH && res[0] >= CC_START && res[0] <= CC_END) {
        for (int i = 0; i < length_cc; i++) {
            if (CC[i].num == res[1]) {
                CC[i].action(res[0] - CC_START, res[2]);
                return CUSTOM_CC_COMMAND;
            }
        }
        return MIDI_PACKET_IGNORED;
    }


    if (kind == MIDI_EVENT_SYSEX && len >= 2 && res[0] == SYS_EX_START && res[len - 1] == SYS_EX_END) {
        if (len >= 5 && len - 5 <= UINT8_MAX &&
            res[1] == PLAYTRONICA_KEY_FIRST && res[2] == PLAYTRONICA_KEY_SECOND) {
            for (int i = 0; i < length_sys; i++) {
                if (sys_com[i].num == res[3]) {
                    sys_com[i].action(&res[4], (uint8_t)(len - 5));
                    return CUSTOM_COMMAND;
                }
            }
        }
        if (len >= 6 && res[1] == PLAYTRONICA_SYS_KEY && res[2] == PLAYTRONICA_KEY_FIRST && res[3] == PLAYTRONICA_KEY_SECOND) {
            switch (res[4]) {
                case 0:
                    plsdk_printf("Device is in TEST GREEN mode\n");
                    return TEST_MODE_GREEN_ACTIVATE;
                case 1:
                    plsdk_printf("Device is in TEST BLUE mode\n");
                    return TEST_MODE_BLUE_ACTIVATE;
                case 2:
                    plsdk_printf("Device is in PLAY mode\n");
                    return TEST_MODE_DEACTIVATE;
                case 3:
                    LOGGER_FLAG = true;
                    plsdk_printf("Log is working\n");
                    return LOGGER_ACTIVATE;
                case 4:
                    plsdk_printf("Log won't work\n");
                    LOGGER_FLAG = false;
                    return LOGGER_DEACTIVATE;
                case 125:
                    // TODO Scala reader
                    break;
                case 126: {
                    return LIST_OF_COMMANDS_ACTION;
                }
                case 127:
                    return RESET_DEVICE;
            }
        }
        return MIDI_PACKET_IGNORED;
    }
    return MIDI_PACKET_IGNORED;
}
