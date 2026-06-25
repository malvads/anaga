section .multiboot
align 4
    dd 0x1BADB002
    dd 0x00
    dd -(0x1BADB002)

section .bss
align 16
stack_bottom:
    resb 16384
global stack_top
stack_top:

section .data
gdt_start:
    dq 0x0                          ; null descriptor

gdt_code:                           ; 0x08 - kernel code
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10011010b                    ; present, ring 0, code
    db 11001111b
    db 0x0

gdt_data:                           ; 0x10 - kernel data
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10010010b                    ; present, ring 0, data
    db 11001111b
    db 0x0

gdt_user_code:                      ; 0x18 - user code (0x1B with RPL 3)
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 11111010b                    ; present, ring 3, code
    db 11001111b
    db 0x0

gdt_user_data:                      ; 0x20 - user data (0x23 with RPL 3)
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 11110010b                    ; present, ring 3, data
    db 11001111b
    db 0x0

gdt_tss:                            ; 0x28 - TSS
    dw 0x0068                       ; limit (104 bytes)
    dw 0x0                          ; base low (filled at runtime)
    db 0x0                          ; base mid
    db 10001001b                    ; present, ring 0, TSS
    db 00000000b
    db 0x0                          ; base high

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ 0x08
DATA_SEG equ 0x10
USER_CODE_SEG equ 0x18
USER_DATA_SEG equ 0x20
TSS_SEG equ 0x28

section .text
global _start
global gdt_tss
global tss_flush
extern kmain
extern tss

_start:
    mov esp, stack_top
    lgdt [gdt_descriptor]
    jmp CODE_SEG:.reload_cs

.reload_cs:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    call setup_tss
    call kmain

    cli
.hang:
    hlt
    jmp .hang

setup_tss:
    mov eax, tss
    mov word [gdt_tss + 2], ax      ; base low
    shr eax, 16
    mov byte [gdt_tss + 4], al      ; base mid
    shr eax, 8
    mov byte [gdt_tss + 7], al      ; base high
    ret

tss_flush:
    mov ax, TSS_SEG | 3
    ltr ax
    ret
