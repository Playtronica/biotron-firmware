#include "buttons.h"
#include "PLSDK/cap_buttons.h"
#include "params.h"
#include <stdlib.h>


bool button_finger_pressed = false;
void change_mute_mode() {
    button_finger_pressed = true;
    mute_state = (mute_state + 1) % 4;
//    printf("BUTTON_EVENT Mute\n");
}

void release_finger() {
    button_finger_pressed = false;
}

bool button_bottom_pressed = false;
void change_same_mode() {
    button_bottom_pressed = true;
    settings.same_note = !(bool)settings.same_note;
    save_settings();
//    printf("BUTTON_EVENT Same note\n");
}

void release_bottom() {
    button_bottom_pressed = false;
}

bool button_top_pressed = false;
void change_scale() {
    button_top_pressed = true;
    settings.scale = (settings.scale + 1) % 12;
    save_settings();
//    printf("BUTTON_EVENT Scale\n");
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

void check_buttons() {
    buttons_task();
}