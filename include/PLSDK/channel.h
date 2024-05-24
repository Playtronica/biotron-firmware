/*!
 * @file channel.h
 * @brief Save info about channels
 *
 * @Notes
 * 1) Id of first channel is equal zero
 * 2) Can be easily updated. Add variable in structure "Channel" and create getter and setter
 *
 * @author Shadowik
 * @date 2023-07-27
 */

#ifndef PLAYTRONICA_SDK_CHANNEL_H
#define PLAYTRONICA_SDK_CHANNEL_H

#include "music.h"

/** @brief Struct which describes one channel */
typedef struct {
    uint16_t BPM;
    uint8_t velocity;
    ScaleNums_t scale;
    uint8_t min_note;
    uint8_t start_note;
    uint8_t max_note;
} Channel;

/** @brief Initialize main functionality
 *
 *  @Note Must be put in start of code
 *  @return   Returns nothing
 * */
void init_midi();

/** @brief Remind device about midi
 *
 *  @Note Must be put in main loop, and places where code can be stack for some time
 *  @return   Nothing
 * */
void remind_midi();

/** @brief Change BPM of one channel
 *
 *  @param channel_id - number of channel
 *  @param newBPM - new BPM
 *
 *  @return Nothing
 * */
void set_BPM(uint8_t channel_id, uint16_t newBPM);

/** @brief Get BPM from one channel
 *
 *  @param channel_id - number of channel
 *
 *  @return BPM of chosen channel
 * */
uint16_t get_BPM(uint8_t channel_id);

/** @brief Change velocity of one channel
 *
 *  @param channel_id - number of channel
 *  @param velocity - new velocity
 *
 *  @return Nothing
 * */
void set_velocity(uint8_t channel_id, uint8_t velocity);

/** @brief Get velocity from one channel
 *
 *  @param channel_id - number of channel
 *
 *  @return Velocity of chosen channel
 * */
uint8_t get_velocity(uint8_t channel_id);

/** @brief Change scale of one channel
 *
 *  @param channel_id - number of channel
 *  @param scale - new scale
 *
 *  @return Nothing
 * */
void set_scale(uint8_t channel_id, ScaleNums_t scale);

/** @brief Get scale from one channel
 *
 *  @param channel_id - number of channel
 *
 *  @return Scale of chosen channel
 * */
ScaleNums_t get_scale(uint8_t channel_id);

/** @brief Change min possible note of one channel
 *
 *  @param channel_id - number of channel
 *  @param note - new min note
 *
 *  @return Nothing
 * */
void set_min_note(uint8_t channel_id, uint8_t note);

/** @brief Get min possible note from one channel
 *
 *  @param channel_id - number of channel
 *
 *  @return Min note of chosen channel
 * */
uint8_t get_min_note(uint8_t channel_id);

/** @brief Change start note of one channel
 *
 *  @param channel_id - number of channel
 *  @param note - new start note
 *
 *  @return Nothing
 * */
void set_start_note(uint8_t channel_id, uint8_t note);

/** @brief Get start note from one channel
 *
 *  @param channel_id - number of channel
 *
 *  @return Start note of chosen channel
 * */
uint8_t get_start_note(uint8_t channel_id);

/** @brief Change max possible note of one channel
 *
 *  @param channel_id - number of channel
 *  @param note - new max note
 *
 *  @return Nothing
 * */
void set_max_note(uint8_t channel_id, uint8_t note);

/** @brief Get max possible note from one channel
 *
 *  @param channel_id - number of channel
 *
 *  @return Max note of chosen channel
 * */
uint8_t get_max_note(uint8_t channel_id);


#endif //PLAYTRONICA_SDK_CHANNEL_H
