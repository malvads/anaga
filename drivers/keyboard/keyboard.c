#include "drivers/keyboard/keyboard.h"
#include "io/io.h"

static int shift_state = 0;

static const unsigned char scancode_ascii[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0
};

static const unsigned char scancode_ascii_shift[] = {
    0,  27, '!', '@', '#', '$', '%', '^', '/', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0
};

uint8_t get_scancode(void) {
    while (!(inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_OUTPUT_FULL));
    return inb(KEYBOARD_DATA_PORT);
}

char getchar(void) {
    uint8_t scancode = get_scancode();
    uint8_t press = !(scancode & KEYBOARD_SCANCODE_RELEASE);
    uint8_t code = scancode & ~KEYBOARD_SCANCODE_RELEASE;

    if (code == SC_SHIFT_LEFT || code == SC_SHIFT_RIGHT) {
        shift_state = press;
        return 0;
    }

    if (!press) return 0;
    if (code >= sizeof(scancode_ascii)) return 0;

    return shift_state ? scancode_ascii_shift[code] : scancode_ascii[code];
}
