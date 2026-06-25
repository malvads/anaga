#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define KEYBOARD_DATA_PORT           0x60
#define KEYBOARD_STATUS_PORT         0x64
#define KEYBOARD_STATUS_OUTPUT_FULL  0x01
#define KEYBOARD_STATUS_INPUT_FULL   0x02
#define KEYBOARD_SCANCODE_RELEASE    0x80
#define SC_SHIFT_LEFT                0x2A
#define SC_SHIFT_RIGHT               0x36

uint8_t get_scancode(void);
char getchar(void);

#endif
