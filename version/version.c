#include "version/version.h"
#include "display/display.h"

void print_kernel_version(void) {
    kprint("Anaga Kernel v");
    kprint_num(ANAGA_VERSION_MAJOR);
    kputc('.');
    kprint_num(ANAGA_VERSION_MINOR);
    kputc('.');
    kprint_num(ANAGA_VERSION_PATCH);
    kputc('.');
    kprint_num(ANAGA_VERSION_BUILD);
    kputc('\n');
}
