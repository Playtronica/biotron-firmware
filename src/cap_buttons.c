/*!
* @file cap_buttons.c
* @brief    Capasitive touch buttons driver for rp2040
* Implements possibility to connect simple touch buttons to rp2040
* Main idea was taken from Arduino lib https://playground.arduino.cc/Main/CapacitiveSensor/
* The detection of touch is carried out by the evaluation of time intervals between setting high level
* on out pin and interrupt on receiving pin. If there is no touch, this interval is very small, but if human touches
* button interval increase in more then 100 times
* @author Vladislav Aidarov
* @date   2022-04-12
*/

#include "cap_buttons.h"


//#define MAX_NUM_OF_BUTTONS 5
uint8_t pulse = 5;

/**
 * @struct ButtonState_t
 * @brief structure contents data about one configured button
 *
*/
typedef struct{
    int gpio;               /**< GPIO pin number */
    uint32_t interval;      /**< Last measured interval for this button */
    bool isPressed;         /**< Last state of button pressed or not */
    ButtonsCB_t on_press;   /**< Action to call on button press */
    ButtonsCB_t on_hold;    /**< Action to call on button hold */
    ButtonsCB_t on_release; /**< Action to call on button release */
} ButtonState_t;

/**
 * @param gpio Number of gpio caused interruption
 * @param events Interrupt events
 * @note Handler for buttons interrupts
*/
void _buttons_int_cb(uint gpio, uint32_t events);

/**
 * @brief Check current button state, if need update ButtonState_t::isPushed and invoke callbacks
 * @param button Structure describing button that must be checked
*/
void _check_button(ButtonState_t * button);

/**
 * @brief Initiate next measure cycle
*/
void _start_measure();

/**
 * @brief Set buttons ready for next measure cycle
*/
void _setup_buttons();

/** @brief Counter of registered buttons */
int _num_of_registered_buttons = 0;

/** @brief Array containing structures of configured buttons */
ButtonState_t _buttons[100];

/** @brief Minimal time interval to consider button is pressed, us*/
const uint32_t _minPressInterval = 50;
/** @brief Time interval for one measuring cycle, us*/
const uint32_t _measureInterval = 100000;

/** @brief Start time of last measure cycle, us */
uint32_t _lastPulseTime;

int _outVal = 0;


void stub_cb(){
    return;
}

void buttons_task(){
    if(_outVal == 0){
        _start_measure();
        return;
    }

    if(time_us_32() - _lastPulseTime < _measureInterval){
        return;
    }else{
        gpio_put(pulse, 0);
        _outVal = 0;
        for(int i = 0; i < _num_of_registered_buttons; i++){
            _check_button(&_buttons[i]);
        }
        return;
    }
}


void buttons_init(uint8_t pulse_pin){
    pulse = pulse_pin;
    gpio_init(pulse_pin);
    gpio_set_dir(pulse_pin, GPIO_OUT);

    for(int i = 0; i < _num_of_registered_buttons; i++ ){
        printf("Init button %d\n", _buttons[i].gpio);
        gpio_init(_buttons[i].gpio);
        gpio_set_dir(_buttons[i].gpio, GPIO_IN);
        gpio_disable_pulls(_buttons[i].gpio);

        gpio_set_irq_enabled_with_callback(_buttons[i].gpio, GPIO_IRQ_EDGE_RISE, true, &_buttons_int_cb);
    }
}

void buttons_add_button(int gpio, ButtonsCB_t cb_on_press, ButtonsCB_t cb_on_hold, ButtonsCB_t cb_on_release){
//    if(_num_of_registered_buttons < MAX_NUM_OF_BUTTONS){
        ButtonState_t * button = &_buttons[_num_of_registered_buttons];

        button->gpio = gpio;
        button->interval = 0;
        button->isPressed = 0;

        if(cb_on_press != NULL){
            button->on_press = cb_on_press;
        }else{
            button->on_press = stub_cb;
        }

        if(cb_on_release != NULL){
            button->on_release = cb_on_release;
        }else{
            button->on_release = stub_cb;
        }

        if(cb_on_hold != NULL){
            button->on_hold = cb_on_hold;
        }else{
            button->on_hold = stub_cb;
        }

        _num_of_registered_buttons++;
//    }
}

void _buttons_int_cb(uint gpio, uint32_t event){
    for(int i = 0; i < _num_of_registered_buttons; i++){
        if(_buttons[i].gpio == gpio && _buttons[i].interval == 0){
            _buttons[i].interval = time_us_32() - _lastPulseTime;
        }
    }
}

void _check_button(ButtonState_t * button){
    //printf("Check GPIO %d\nPress interval is %d\n", button->gpio, button->interval);
    if(button->interval > _minPressInterval || button->interval == 0){
        if(button->isPressed){
            button->on_hold();
            return;
        }else{
            button->isPressed = true;
            button->on_press();
        }
    }
    else{
        if(button->isPressed){
            button->on_release();
        }
        button->isPressed = false;
    }
    button->interval = 0;
}

void _start_measure(){
    _setup_buttons();
    gpio_put(pulse, 1);
    _lastPulseTime = time_us_32();
    _outVal = 1;
}

void _setup_buttons(){
    for(int i = 0; i < _num_of_registered_buttons; i++){
        _buttons[i].interval = 0;
    }
}