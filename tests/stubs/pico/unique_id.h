#ifndef TEST_PICO_UNIQUE_ID_H
#define TEST_PICO_UNIQUE_ID_H

#include <stddef.h>

#define PICO_UNIQUE_BOARD_ID_SIZE_BYTES 8

void pico_get_unique_board_id_string(char *destination, size_t length);

#endif
