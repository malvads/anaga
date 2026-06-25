#include "user/libc/syscall.h"

void _start(void) {
    const char* msg = "Hello from userland!\n";
    write(1, msg, 21);
    exit(0);
}
