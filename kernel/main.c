#include "system/system.h"
#include "interrupts/idt.h"
#include "mm/paging.h"
#include "mm/tss.h"
#include "syscall/syscall.h"
#include "drivers/pci/pci.h"
#include "drivers/drive/drive.h"
#include "drivers/command/command.h"
#include "display/display.h"
#include "io/kinput.h"
#include "version/version.h"
#include "fs/vfs.h"

extern void tss_flush(void);

extern uint32_t stack_top;

static void kinit(void) {
    paging_init();
    idt_init();
    tss_init((uint32_t)&stack_top);
    tss_flush();
    syscall_init();
    system_init_timer();
    pci_scan();
    drive_init();
    vfs_init();
}

void kmain(void) {
    kclean_display();
    kinit();
    print_kernel_version();
    kprint("Syscalls ready (int 0x80)\n");
    input_prompt();

    char cmd[MAX_COMMAND_LEN];
    while (1) {
        if (input_poll(cmd, MAX_COMMAND_LEN)) {
            handle_command(cmd);
            input_prompt();
        }
    }
}
