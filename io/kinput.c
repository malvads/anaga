#include "io/kinput.h"
#include "display/display.h"
#include "drivers/keyboard/keyboard.h"

void input_prompt(void) {
    kprint("> ");
}

int input_poll(char *buffer, size_t buffer_size) {
    static int idx = 0;

    char c = getchar();
    if (c == 0) return 0;
    if (c == '\r') c = '\n';

    if (c == '\n') {
        kputc('\n');
        buffer[idx] = '\0';
        idx = 0;
        return 1;
    }

    if (c == '\b' || c == 127) {
        if (idx > 0) {
            idx--;
            kputc('\b');
            kputc(' ');
            kputc('\b');
        }
        return 0;
    }

    if (c == 0x15) {
        while (idx > 0) {
            idx--;
            kputc('\b');
            kputc(' ');
            kputc('\b');
        }
        return 0;
    }

    if ((size_t)idx < buffer_size - 1) {
        buffer[idx++] = c;
        kputc(c);
    }
    return 0;
}
