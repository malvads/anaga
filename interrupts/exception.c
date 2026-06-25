#include "syscall/syscall.h"
#include "display/display.h"
#include <stdint.h>

static const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Security Exception", "Reserved", "Triple Fault?", "Reserved"
};

void kernel_panic(const char* msg, regs_t* r) {
    kprintf("\n!!! KERNEL PANIC: %s !!!\n", msg);
    
    if (r) {
        kprintf("INT: 0x%x  ERR: 0x%x\n", r->int_no, r->err_code);
        kprintf("EIP: 0x%x  CS:  0x%x  EFLAGS: 0x%x\n", r->eip, r->cs, r->eflags);
        kprintf("EAX: 0x%x  EBX: 0x%x  ECX: 0x%x  EDX: 0x%x\n", r->eax, r->ebx, r->ecx, r->edx);
        kprintf("ESI: 0x%x  EDI: 0x%x  EBP: 0x%x\n", r->esi, r->edi, r->ebp);
        kprintf("UESP:0x%x  SS:  0x%x\n", r->useresp, r->ss);
        kprintf("DS:  0x%x  ES:  0x%x  FS:  0x%x  GS: 0x%x\n", r->ds, r->es, r->fs, r->gs);
    }

    __asm__ volatile("cli; hlt");
}

void exception_handler(regs_t* r) {
    if (r->int_no == 0x80) {
        syscall_handler(r);
        return;
    }

    if (r->int_no < 32) {
        kernel_panic(exception_messages[r->int_no], r);
    }
}
