#include "pico/stdlib.h"
#include "PLSDK/commands.h"
#include "PLSDK/constants.h"
#include "PLSDK/midi_parser.h"
#include "PLSDK/midi_diagnostics.h"
#include "PLSDK/midi_tx.h"
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

static void add_sys_ex_command(void action(const uint8_t data[], uint8_t len),
                               uint8_t num, bool persists,
                               uint8_t minimum_length) {
    if (length_sys >= MAX_COUNT_COMMANDS || action == NULL) return;
    sys_ex_command_s new_sys_ex_com;
    new_sys_ex_com.num = num;
    new_sys_ex_com.action = action;
    new_sys_ex_com.persists = persists;
    new_sys_ex_com.minimum_length = minimum_length;
    sys_com[length_sys++] = new_sys_ex_com;
}

void add_sys_ex_com(void action(const uint8_t data[], uint8_t len), uint8_t num) {
    add_sys_ex_command(action, num, true, 0);
}

void add_sys_ex_query(void action(const uint8_t data[], uint8_t len), uint8_t num) {
    add_sys_ex_command(action, num, false, 0);
}

void add_sys_ex_com_len(void action(const uint8_t data[], uint8_t len),
                        uint8_t num, uint8_t minimum_length) {
    add_sys_ex_command(action, num, true, minimum_length);
}

void add_sys_ex_query_len(void action(const uint8_t data[], uint8_t len),
                          uint8_t num, uint8_t minimum_length) {
    add_sys_ex_command(action, num, false, minimum_length);
}

bool print_sys_ex(const uint8_t data[], uint8_t len) {
    uint8_t message[4 + len];
    message[0] = SYS_EX_START;
    message[1] = PLAYTRONICA_KEY_FIRST;
    message[2] = PLAYTRONICA_KEY_SECOND;
    for (int i = 0; i < len; ++i) {
        message[3 + i] = data[i];
    }
    message[3 + len] = SYS_EX_END;
    service_midi_tx();
    const bool accepted = midi_tx_enqueue(
            CABLE_NUM_EXTRA, message, (uint16_t)(4u + len));
    service_midi_tx();
    return accepted;
}

bool print_pure(uint8_t cable, const uint8_t data[], uint8_t len) {
    service_midi_tx();
    const bool accepted = midi_tx_enqueue(cable, data, len);
    service_midi_tx();
    return accepted;
}

static uint8_t clocks_since_beat = 0;
static bool midi_clock_running = false;
static midi_parser_t cable_parsers[CABLE_NUM_EXTRA + 1];
static bool parsers_initialized = false;

static void init_parsers_once(void) {
    if (parsers_initialized) return;
    for (size_t cable = 0; cable <= CABLE_NUM_EXTRA; ++cable) {
        midi_parser_init(&cable_parsers[cable]);
    }
    parsers_initialized = true;
}

int read_sys_ex(void) {
    uint8_t packet[4];
    midi_event_t event;
    init_parsers_once();
    if (!tud_midi_packet_read(packet)) return UNKNOWN;
    remind_midi();

    const uint8_t cable = (packet[0] >> 4) & 0x0f;
    midi_diagnostics_rx_packet(cable);
    if (cable > CABLE_NUM_EXTRA) return MIDI_PACKET_IGNORED;
    const midi_event_kind_t kind = midi_parser_feed_usb_packet(
            &cable_parsers[cable], packet, &event);
    if (kind == MIDI_EVENT_MALFORMED) {
        midi_diagnostics_rx_event(
                cable,
                event.error == MIDI_PARSER_ERROR_SYSEX_OVERFLOW ?
                MIDI_DIAGNOSTICS_SYSEX_OVERFLOW :
                MIDI_DIAGNOSTICS_MALFORMED);
        return MIDI_PACKET_IGNORED;
    }
    if (kind == MIDI_EVENT_NONE) {
        return MIDI_PACKET_IGNORED;
    }
    const uint8_t *res = event.data;
    const size_t len = event.len;

    if (event.sysex_aborted) {
        midi_diagnostics_rx_event(cable, MIDI_DIAGNOSTICS_SYSEX_ABORTED);
    }

    if (kind == MIDI_EVENT_REALTIME) {
        midi_diagnostics_rx_event(cable, MIDI_DIAGNOSTICS_REALTIME);
    } else if (kind == MIDI_EVENT_SYSEX) {
        midi_diagnostics_rx_event(cable, MIDI_DIAGNOSTICS_SYSEX);
    } else if (len >= 1 && res[0] >= 0xf0) {
        midi_diagnostics_rx_event(cable, MIDI_DIAGNOSTICS_SYSTEM_COMMON);
    } else {
        midi_diagnostics_rx_event(cable, MIDI_DIAGNOSTICS_CHANNEL);
    }

    if (len >= 1 && res[0] == BPM_CLOCK_START_BYTE) {
        clocks_since_beat = 0;
        midi_clock_running = true;
        return BPM_CLOCK_ACTIVATE;
    }
    if (len >= 1 && res[0] == BPM_CLOCK_CONTINUE_BYTE) {
        clocks_since_beat = 0;
        midi_clock_running = true;
        return BPM_CLOCK_ACTIVATE;
    }
    if (len >= 1 && res[0] == BPM_CLOCK_STOP_BYTE) {
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
        for (int i = 0; i < length_cc; ++i) {
            if (CC[i].num == res[1]) {
                CC[i].action(res[0] - CC_START, res[2]);
                return CUSTOM_CC_COMMAND;
            }
        }
        return MIDI_PACKET_IGNORED;
    }

    if (kind == MIDI_EVENT_SYSEX && len >= 2 && res[0] == SYS_EX_START &&
        res[len - 1] == SYS_EX_END) {
        if (len >= 5 && len - 5 <= UINT8_MAX &&
            res[1] == PLAYTRONICA_KEY_FIRST &&
            res[2] == PLAYTRONICA_KEY_SECOND) {
            for (int i = 0; i < length_sys; ++i) {
                if (sys_com[i].num == res[3]) {
                    const uint8_t payload_length = (uint8_t)(len - 5);
                    if (payload_length < sys_com[i].minimum_length) {
                        return MIDI_PACKET_IGNORED;
                    }
                    sys_com[i].action(&res[4], payload_length);
                    return sys_com[i].persists ? CUSTOM_COMMAND :
                           CUSTOM_QUERY_COMMAND;
                }
            }
        }
        if (len >= 6 && res[1] == PLAYTRONICA_SYS_KEY &&
            res[2] == PLAYTRONICA_KEY_FIRST &&
            res[3] == PLAYTRONICA_KEY_SECOND) {
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
                    LOGGER_FLAG = false;
                    plsdk_printf("Log won't work\n");
                    return LOGGER_DEACTIVATE;
                case 125:
                    break;
                case 126:
                    return LIST_OF_COMMANDS_ACTION;
                case 127:
                    return RESET_DEVICE;
            }
        }
    }
    return MIDI_PACKET_IGNORED;
}
