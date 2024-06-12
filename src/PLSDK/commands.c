#include <class/midi/midi_device.h>
#include <malloc.h>
#include <pico/time.h>
#include "PLSDK/commands.h"
#include "PLSDK/channel.h"
#include "PLSDK/constants.h"
#include "params.h"
#include "global.h"
#include <pico/bootrom.h>

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
void midi_clock() {
    if (clk++ < 25) return;
    clk = 1;
    if (status == BPMClockActive) play_music();
}

void read_sys_ex() {
    uint8_t res[300];
    uint8_t buff[4];
    uint32_t len = 0;
    static uint8_t bpm_clock_prepare = 0;


    while (tud_midi_packet_read(buff)) {
        remind_midi();
        for (int i = 1; i < 4; ++i) {
            res[len++] = buff[i];
            if (buff[i] == SYS_EX_END) break;
        }
    }

    if (len == 0) {
        return;
    }
//    printf("%d", res[0]);
    if (res[0] == 250 || res[0] == 242) {
        bpm_clock_prepare = 250;
//        printf("1\n");
        return;
    }

    if (res[0] == 252) {
        bpm_clock_control(false);
//        printf("2\n");
        bpm_clock_prepare = 252;
        return;
    }

    if (res[0] == 248) {
//        printf("3\n");
        if (bpm_clock_prepare == 250) {
            bpm_clock_control(true);
            clk = 0;
        }
        midi_clock();
        bpm_clock_prepare = 248;
        return;
    }


//    printf("%d %d %d %d %d\n", res[0], res[1], res[2], res[3], res[4]);
    if (res[0] >= CC_START && res[0] <= CC_END) {
        for (int i = 0; i < length_cc; i++) {
            if (CC[i].num == res[1]) {
                CC[i].action(res[0] - CC_START, res[2]);
                return;
            }
        }
        print_pure(CABLE_NUM_MAIN, res, len);
        save_settings();
        return;
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
                    save_settings();
                }
            }
        }
        if (res[1] == PLAYTRONICA_SYS_KEY && res[2] == PLAYTRONICA_KEY_FIRST && res[3] == PLAYTRONICA_KEY_SECOND) {
            switch (res[4]) {
                case 125:
                    // TODO Scala reader
                    break;
                case 126:
                    load_settings();
                    break;
                case 127:
                    clear_flash();
                    reset_usb_boot(0, 0);
                    break;
            }
        }
        print_pure(CABLE_NUM_EXTRA, res, len);
        return;
    }

    if (res[0] >= VALUE_LIMIT) {
        print_pure(CABLE_NUM_MAIN, res, len);
        return;
    }
}

static bool _repeating_timer_callback_t_settings(repeating_timer_t *rt) {
    read_sys_ex();
    return true;
}

struct repeating_timer settingsTimer;

void init_commands() {
    add_repeating_timer_ms(10,
                           _repeating_timer_callback_t_settings,
                           NULL, &settingsTimer);
}