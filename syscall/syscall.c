#include "syscall/syscall.h"
#include "display/display.h"
#include "io/io.h"

static int sys_exit(regs_t* r) {
    kprintf("Process exited with code %d\n", (int)r->ebx);
    while (1) __asm__ volatile("hlt");
    return 0;
}

static int sys_read(regs_t* r) {
    (void)r;
    return -1;
}

static int sys_write(regs_t* r) {
    int fd = (int)r->ebx;
    const char* buf = (const char*)r->ecx;
    int count = (int)r->edx;

    if (fd == 1 || fd == 2) {
        for (int i = 0; i < count && buf[i]; i++) {
            kputc(buf[i]);
        }
        return count;
    }
    return -1;
}

static int sys_open(regs_t* r) {
    (void)r;
    return -1;
}

static int sys_close(regs_t* r) {
    (void)r;
    return 0;
}

static int sys_getpid(regs_t* r) {
    (void)r;
    return 1;
}

typedef int (*syscall_fn_t)(regs_t*);

static syscall_fn_t syscall_table[SYSCALL_MAX] = {
    sys_exit,
    sys_read,
    sys_write,
    sys_open,
    sys_close,
    sys_getpid,
};

void syscall_handler(regs_t* regs) {
    if (regs->eax >= SYSCALL_MAX) {
        regs->eax = (uint32_t)-1;
        return;
    }
    regs->eax = (uint32_t)syscall_table[regs->eax](regs);
}

void syscall_init(void) {
}
