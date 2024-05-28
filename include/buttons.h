//
// Created by Марк Захарченко on 24.05.2024.
//

#ifndef BIOTRON_BUTTONS_H
#define BIOTRON_BUTTONS_H

#include <stdbool.h>

#define BUTTON_FINGER 8
#define BUTTON_BOTTOM 7
#define BUTTON_TOP 6

extern bool button_finger_pressed;
extern bool button_bottom_pressed;
extern bool button_top_pressed;

void init_buttons();
void check_buttons();

#endif //BIOTRON_BUTTONS_H
