#ifndef BIOTRON_LEDS_H
#define BIOTRON_LEDS_H

#define GROUP_BlUE_LED_CENTER 1
#define GROUP_BlUE_LED_LEFT 0
#define GROUP_BlUE_LED_RIGHT 2

#define FIRST_GROUP_GREEN_LED_1 9
#define FIRST_GROUP_GREEN_LED_2 3
#define FIRST_GROUP_GREEN_LED_3 4

#define SECOND_GROUP_GREEN_LED_1 14
#define SECOND_GROUP_GREEN_LED_2 10
#define SECOND_GROUP_GREEN_LED_3 11

#define LED_MASK ((1 << GROUP_BlUE_LED_CENTER) | (1 << GROUP_BlUE_LED_LEFT) | (1 << GROUP_BlUE_LED_RIGHT) | \
                 (1 << FIRST_GROUP_GREEN_LED_1) | (1 << FIRST_GROUP_GREEN_LED_2) | (1 << FIRST_GROUP_GREEN_LED_3) | \
                 (1 << SECOND_GROUP_GREEN_LED_1) | (1 << SECOND_GROUP_GREEN_LED_2) | (1 << SECOND_GROUP_GREEN_LED_3))


#define MAX_LIGHT ((1 << 16) - 100)
#define MIN_LIGHT 10
#define ASYNC_LEDS 200
#define NOTE_STRONG 414

void init_leds();
void intro_leds();
void led_loop();

void light_note_observer();
#endif //BIOTRON_LEDS_H
