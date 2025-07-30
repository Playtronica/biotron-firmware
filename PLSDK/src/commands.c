#include "pico/stdlib.h"
#include "PLSDK/commands.h"
#include "PLSDK/constants.h"
#include "PLSDK.h"
#include "tusb.h"


CC_command_s CC[MAX_COUNT_COMMANDS];
uint8_t length_cc = 0;
sys_ex_command_s sys_com[MAX_COUNT_COMMANDS];
uint8_t length_sys = 0;


void add_CC(void action(uint8_t channel, uint8_t value), uint8_t num) {
    CC_command_s new_CC;
    new_CC.num = num;
    new_CC.action = action;
    CC[length_cc++] = new_CC;
}

void add_sys_ex_com(void action(const uint8_t data[], uint8_t len), uint8_t num) {
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

static uint16_t clk = 0;
int midi_clock() {
    if (clk++ < 25) return BPM_CLOCK_INACTIVE;
    clk = 1;
    return BPM_CLOCK_PLAY;
}


int read_sys_ex() {
    uint8_t res[300];
    uint8_t buff[4];
    uint32_t len = 0;
    static uint8_t bpm_clock_prepare = BPM_CLOCK_STOP_BYTE;


    while (tud_midi_packet_read(buff)) {
        remind_midi();
        for (int i = 1; i < 4; ++i) {
            res[len++] = buff[i];
            if (buff[i] == SYS_EX_END ||
                buff[i] == BPM_CLOCK_STOP_BYTE ||
                buff[i] == BPM_CLOCK_START_BYTE ||
                buff[i] == BPM_CLOCK_BYTE) break;
        }
    }

    if (len == 0) {
        return UNKNOWN;
    }

    if (res[0] == BPM_CLOCK_START_BYTE || res[0] == MUSIC_SELECT) {
        printf("START\n");
        bpm_clock_prepare = BPM_CLOCK_START_BYTE;
        return BPM_CLOCK_INACTIVE;
    }

    if (res[0] == BPM_CLOCK_STOP_BYTE) {
        printf("END\n");
        bpm_clock_prepare = BPM_CLOCK_STOP_BYTE;
        return BPM_CLOCK_DEACTIVATE;
    }

    if (res[0] == BPM_CLOCK_BYTE &&
        (bpm_clock_prepare == BPM_CLOCK_START_BYTE || bpm_clock_prepare == BPM_CLOCK_BYTE)) {
        if (bpm_clock_prepare == BPM_CLOCK_START_BYTE) {
            clk = 0;
            bpm_clock_prepare = BPM_CLOCK_BYTE;
            return BPM_CLOCK_ACTIVATE;
        }
        return midi_clock();
    }


    if (res[0] >= CC_START && res[0] <= CC_END) {
        for (int i = 0; i < length_cc; i++) {
            if (CC[i].num == res[1]) {
                CC[i].action(res[0] - CC_START, res[2]);
                return CUSTOM_COMMAND;
            }
        }
        return UNKNOWN;
    }


    if (res[0] == SYS_EX_START) {
        if (res[1] == PLAYTRONICA_KEY_FIRST && res[2] == PLAYTRONICA_KEY_SECOND) {
            for (int i = 0; i < length_sys; i++) {
                if (sys_com[i].num == res[3]) {
                    uint8_t data[len - 5];
                    for (int j = 4; j < len - 1; j++) {
                        data[j - 4] = res[j];
                    }
                    sys_com[i].action(data, len - 5);
                    return CUSTOM_COMMAND;
                }
            }
        }
        if (res[1] == PLAYTRONICA_SYS_KEY && res[2] == PLAYTRONICA_KEY_FIRST && res[3] == PLAYTRONICA_KEY_SECOND) {
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
        return UNKNOWN;
    }
    return UNKNOWN;
}

