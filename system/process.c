#include <stdint.h>

void process_exec(uint32_t entry) {
    __asm__ volatile(
        "movl %0, %%edx\n"
        "mov $0x23, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "pushl $0x23\n"      // SS
        "pushl $0x300000\n"  // ESP (user stack at 3MB)
        "pushl $0x202\n"     // EFLAGS (IF=1)
        "pushl $0x1B\n"      // CS
        "pushl %%edx\n"      // EIP
        "iret\n"
        :
        : "m"(entry)          // Use memory constraint to avoid register clobbering
        : "eax", "edx", "memory"
    );
}
