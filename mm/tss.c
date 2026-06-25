#include "mm/tss.h"

tss_t tss __attribute__((aligned(4096)));

extern void tss_flush(void);

void tss_init(uint32_t kernel_stack) {
    for (int i = 0; i < (int)sizeof(tss); i++) {
        ((uint8_t*)&tss)[i] = 0;
    }

    tss.ss0 = 0x10;
    tss.esp0 = kernel_stack;
    tss.cs = 0x0B;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;
    tss.iomap_base = (uint16_t)sizeof(tss);
}

void tss_set_kernel_stack(uint32_t stack) {
    tss.esp0 = stack;
}
