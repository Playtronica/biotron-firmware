#include "buttons.h"
#include "PLSDK/cap_buttons.h"
#include <stdlib.h>


bool button_finger_pressed = false;
void change_mute_mode() {
    button_finger_pressed = true;
}

void release_finger() {
    button_finger_pressed = false;
}

bool button_bottom_pressed = false;
void change_same_mode() {
    button_bottom_pressed = true;
}

void release_bottom() {
    button_bottom_pressed = false;
}

bool button_top_pressed = false;
void change_scale() {
    button_top_pressed = true;
}

void release_top() {
    button_top_pressed = false;
}

void init_buttons() {
    buttons_add_button(BUTTON_FINGER, 60,  change_mute_mode, NULL, release_finger);
    buttons_add_button(BUTTON_BOTTOM, 40, change_same_mode, NULL, release_bottom);
    buttons_add_button(BUTTON_TOP, 40, change_scale, NULL, release_top);

    buttons_init(5);
}