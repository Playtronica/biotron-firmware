/*!
 *
 * @file commands.h
 * @brief Controls all midi messages
 *
 * You can create your own command, CC or Sys ex
 * If you send message, which device doesn't know, it will resend it on sequencer
 *
 * @author Shadowik
 * @date 2023-07-28
 */

#ifndef PLSDK_COMMANDS_H
#define PLSDK_COMMANDS_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_COUNT_COMMANDS 50

enum SYS_EX_RESPONSE {
    CUSTOM_COMMAND,
    CUSTOM_CC_COMMAND,
    CUSTOM_QUERY_COMMAND,
    MIDI_PACKET_IGNORED,
    BPM_CLOCK_ACTIVATE,
    BPM_CLOCK_DEACTIVATE,
    BPM_CLOCK_PLAY,
    BPM_CLOCK_INACTIVE,
    UNKNOWN,

    TEST_MODE_GREEN_ACTIVATE,
    TEST_MODE_BLUE_ACTIVATE,
    TEST_MODE_DEACTIVATE,
    LOGGER_ACTIVATE,
    LOGGER_DEACTIVATE,
    LIST_OF_COMMANDS_ACTION,
    RESET_DEVICE,
};


typedef struct CC_command_s {
    void (*action)(uint8_t channel, uint8_t value);
    uint8_t num;
} CC_command_s;

typedef struct sys_ex_command_s {
    void (*action)(const uint8_t data[], uint8_t len);
    uint8_t num;
    bool persists;
    uint8_t minimum_length;
} sys_ex_command_s;


/**
 * @note
 * Activates after command midi_init, so it useless for single use.
 * */
void init_commands();

/**
 * @brief Create new CC commands
 *
 * @param action - function, which will be called
 * @param num - CC command number
 *
 * @note
 * void action(uint8_t channel, uint8_t value)
 * */
void add_CC(void action(uint8_t channel, uint8_t value), uint8_t num);

/**
 * @brief Create new SysEx commands
 *
 * @param action - function, which will be called
 * @param num - sys ex command number
 *
 * @note
 * void action(uint8_t data[], uint8_t len)
 * */
void add_sys_ex_com(void action(const uint8_t data[], uint8_t len), uint8_t num);
void add_sys_ex_query(void action(const uint8_t data[], uint8_t len), uint8_t num);
void add_sys_ex_com_len(void action(const uint8_t data[], uint8_t len),
                        uint8_t num, uint8_t minimum_length);
void add_sys_ex_query_len(void action(const uint8_t data[], uint8_t len),
                          uint8_t num, uint8_t minimum_length);

/**
 * @brief Parse value to sys ex format
 *
 * @param data - unsigned num
 *
 * @return Array with 3 nums.
 *
 * @note
 * To convert to a readable number - multiply first on 127^2,
 * second on 127, sum them and add third value
 * */
uint8_t *parse_value_to_sys_ex(uint32_t data);

/**
 * @brief Print info in MIDI + Playtronica format
 *
 * @param data - array with nums (range from 0 to 126 includes)
 * @param len - len of array
 *
 * @return true when the complete message was accepted by the bounded TX queue
 *
 * @note
 * Sends data only on 2 MIDI cable. Also sends data in Playtronica format
 * */
bool print_sys_ex(const uint8_t data[], uint8_t len);

/**
 * @brief Print info in MIDI format
 *
 * @param cable - on which MIDI cable will be send message
 * @param data - array with nums (range from 0 to 126 includes)
 * @param len - len of array
 *
 * @return true when the complete message was accepted by the bounded TX queue
 *
 * */
bool print_pure(uint8_t cable, const uint8_t data[], uint8_t len);

int read_sys_ex();


#endif //PLSDK_COMMANDS_H
