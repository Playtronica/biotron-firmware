/*!
* @file cap_buttons.h
* @brief    Capasitive touch buttons driver for rp2040
* Implements possibility to connect simple touch buttons to rp2040
* Main idea was taken from Arduino lib https://playground.arduino.cc/Main/CapacitiveSensor/
* The detection of touch is carried out by the evaluation of time intervals between setting high level
* on out pin and interrupt on receiving pin. If there is no touch, this interval is very small, but if human touches
* button interval increase in more then 100 times
* @author Vladislav Aidarov
* @date   2022-04-12
*
 * @note
 * Pulse pin initiates in function buttons_init (no define) and don't have any limit.
 * @author Shadowik
 * @date 2023-07-28
*/

#ifndef THEREMIN_CAP_BUTTONS_H
#define THEREMIN_CAP_BUTTONS_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"

#ifndef DEBUG_LAYOUT_BUILD
/** @brief Number of outut pin */
#else
#define pulse 22
#endif
/** @brief Callback type to specify actions for buttons */
typedef void (*ButtonsCB_t)();

extern uint32_t button_states[];

/*!
    * @brief    Manager for buttons.
    * @return   Returns nothing
    * @note     Must be called periodicaly to start new measuring cycles or to proccess touches
*/
void buttons_task();

/*!
    * @brief    Initialize all added buttons as inputs with interrupts
    *
    * @param    pulse_pin
    *
    * @return   Returns nothing

*/
void buttons_init(uint8_t pulse_pin);

/*!
    * @brief    Add button processing.
    * @param    gpio number of GPIO pin of the button input
    * @param    cb_on_press callback action on button press
    * @param    cb_on_hold callback action on button hold
    * @param    cb_on_release callback action on button release
    * @return   Returns nothing
    * @note     Must be called before buttons_init()
*/
void buttons_add_button(int gpio, int min_press, ButtonsCB_t cb_on_press, ButtonsCB_t cb_on_hold, ButtonsCB_t cb_on_release);



#endif //THEREMIN_CAP_BUTTONS_H