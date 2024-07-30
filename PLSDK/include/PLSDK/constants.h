//
// Created by shadowik on 26.07.23.
//

#ifndef PLSDK_CONSTANTS_H
#define PLSDK_CONSTANTS_H

#define MAX_COUNT_CHANNELS 16

#define DEFAULT_BPM 120
#define DEFAULT_VELOCITY 127
#define DEFAULT_PITCH_MSB 63
#define DEFAULT_PITCH_LSB 0

#define CABLE_NUM_MAIN 0
#define CABLE_NUM_EXTRA 1

#define VALUE_LIMIT 0x7F

#define SYS_EX_START 0xF0
#define SYS_EX_END 0xF7

#define CC_START 0xB0
#define CC_VOLUME 7
#define CC_STOP_ALL_NOTES 123
#define CC_END 0xBF
#define CC_LENGTH 3

#define NOTE_OFF 0x80
#define NOTE_ON 0x90
#define CHANGE_PITCH 0xE0

#define PLAYTRONICA_SYS_KEY 0x0B
#define PLAYTRONICA_KEY_FIRST 0x14
#define PLAYTRONICA_KEY_SECOND 0x0D

#define BPM_TO_US(BPM) (int)(1000000.0 / ((BPM) / 60.0))
#define BPM_TO_MS(BPM) (int)(1000.0 / ((BPM) / 60.0))
#define US_TO_BPM(US) (int)(60.0 * 1000000.0 / (US))
#define MS_TO_BPM(MS) (int)(60.0 * 1000.0 / (MS))

#endif //PLSDK_CONSTANTS_H
