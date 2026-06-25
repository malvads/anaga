#include "system/system.h"
#include "drivers/keyboard/keyboard.h"
#include "io/io.h"
#include "display/display.h"
#include <stdint.h>
#include <stddef.h>

static volatile uint32_t timer_ticks = 0;

#define SLP_EN 0x2000

#define RSDP_SIGNATURE 0x2052545020445352ULL
#define FADT_SIGNATURE 0x50434146

typedef struct {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;
} __attribute__((packed)) acpi_rsdp_t;

typedef struct {
    char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
} __attribute__((packed)) acpi_sdt_header_t;

typedef struct {
    acpi_sdt_header_t h;
    uint32_t FirmwareCtrl;
    uint32_t Dsdt;
    uint8_t Reserved;
    uint8_t PreferredPMProfile;
    uint16_t SciInt;
    uint32_t SmiCmd;
    uint8_t AcpiEnable;
    uint8_t AcpiDisable;
    uint8_t S4BiosReq;
    uint8_t PStateCnt;
    uint32_t PM1aEvtBlk;
    uint32_t PM1bEvtBlk;
    uint32_t PM1aCntBlk;
    uint32_t PM1bCntBlk;
    uint32_t PM2CntBlk;
    uint32_t PMTimerBlk;
    uint32_t GPE0Blk;
    uint32_t GPE1Blk;
    uint8_t PM1EvtLen;
    uint8_t PM1CntLen;
    uint8_t PM2CntLen;
    uint8_t PMTimerLen;
    uint8_t GPE0BlkLen;
    uint8_t GPE1BlkLen;
    uint8_t GPE1Base;
    uint8_t CstCnt;
    uint16_t PLvl2Lat;
    uint16_t PLvl3Lat;
    uint16_t FlushSize;
    uint16_t FlushStride;
    uint8_t DutyOffset;
    uint8_t DutyWidth;
    uint8_t DayAlrm;
    uint8_t MonAlrm;
    uint8_t Century;
    uint16_t IaPcBootArch;
    uint8_t Reserved2;
    uint32_t Flags;
    uint8_t ResetReg[12];  // GenericAddressStructure
    uint8_t ResetValue;
    uint8_t Reserved3[3];
    uint64_t X_FirmwareCtrl;
    uint64_t X_Dsdt;
    uint8_t Reserved4[8];
    uint32_t Version;
} __attribute__((packed)) acpi_table_fadt_t;

int system_is_hypervisor(void) {
    uint32_t eax, ecx;
    eax = 1;
    __asm__ volatile("cpuid"
                     : "=c"(ecx)
                     : "a"(eax)
                     : "ebx", "edx");
    return (ecx >> 31) & 1;
}

acpi_rsdp_t* acpi_find_rsdp(void) {
    for (uint32_t addr = 0x000E0000; addr < 0x00100000; addr += 16) {
        acpi_rsdp_t* rsdp = (acpi_rsdp_t*)addr;
        if (*(uint64_t*)rsdp->Signature == RSDP_SIGNATURE) {
            return rsdp;
        }
    }
    return NULL;
}

acpi_table_fadt_t* acpi_get_fadt(void) {
    acpi_rsdp_t* rsdp = acpi_find_rsdp();
    if (!rsdp) return NULL;

    acpi_sdt_header_t* rsdt = (acpi_sdt_header_t*)(uintptr_t)rsdp->RsdtAddress;
    int entries = (rsdt->Length - sizeof(acpi_sdt_header_t)) / 4;
    uint32_t* table_ptrs = (uint32_t*)((uintptr_t)rsdt + sizeof(acpi_sdt_header_t));

    for (int i = 0; i < entries; i++) {
        acpi_sdt_header_t* header = (acpi_sdt_header_t*)(uintptr_t)table_ptrs[i];
        if (*(uint32_t*)header->Signature == FADT_SIGNATURE) {
            return (acpi_table_fadt_t*)header;
        }
    }
    return NULL;
}

uint8_t acpi_get_s5_sleep_type(void) {
    acpi_table_fadt_t* fadt = acpi_get_fadt();
    if (!fadt) return 0;

    uint8_t* dsdt = (uint8_t*)(uintptr_t)fadt->Dsdt;
    uint32_t length = fadt->h.Length;

    for (uint32_t i = 0; i < length - 4; i++) {
        if (dsdt[i] == '_' && dsdt[i+1] == 'S' && dsdt[i+2] == '5' && dsdt[i+3] == '_') {
            if (dsdt[i+4] == 0x12 || dsdt[i+5] == 0x12) {
                return dsdt[i+5];
            }
        }
    }
    return 0;
}

void system_shutdown(void) {
    if(system_is_hypervisor()) {
        kprint("Shutting down...\n");
        outw(ACPI_SHUTDOWN_PORT_QEMU, ACPI_SHUTDOWN_VALUE_QEMU);
        outw(ACPI_SHUTDOWN_PORT_BOCHS, ACPI_SHUTDOWN_VALUE_BOCHS);
        outw(ACPI_SHUTDOWN_PORT_VBOX, ACPI_SHUTDOWN_VALUE_VBOX);
        system_halt();
    } else {
        kprint("Shutting down (ACPI)...\n");
        acpi_table_fadt_t* fadt = acpi_get_fadt();
        if (!fadt) {
            kprint("ACPI FADT not found, cannot shutdown.\n");
            system_halt();
        }

        uint16_t pm1a_cnt = fadt->PM1aCntBlk;
        uint8_t slp_typa = acpi_get_s5_sleep_type();

        if (pm1a_cnt == 0 || slp_typa == 0) {
            kprint("Invalid PM1a_CNT or SLP_TYP, cannot shutdown.\n");
            system_halt();
        }

        outw(pm1a_cnt, ((uint16_t)slp_typa << 10) | SLP_EN);
        system_halt();
    }
}

void system_reboot(void) {
    kprint("Rebooting...\n");
    uint8_t status;
    do {
        status = inb(KEYBOARD_STATUS_PORT);
        if (status & KEYBOARD_STATUS_OUTPUT_FULL) {
            inb(KEYBOARD_DATA_PORT);
        }
    } while (status & KEYBOARD_STATUS_INPUT_FULL);
    outb(KEYBOARD_STATUS_PORT, 0xFE);
    system_halt();
}

void timer_handler(void) {
    timer_ticks++;
}

void system_init_timer(void) {
    uint32_t divisor = PIT_FREQUENCY / 1000;
    
    outb(PIT_COMMAND, PIT_CMD_CHANNEL0 | PIT_CMD_RW_BOTH | PIT_CMD_MODE2);
    outb(PIT_CHANNEL0_DATA, divisor & 0xFF);
    outb(PIT_CHANNEL0_DATA, (divisor >> 8) & 0xFF);
    
    timer_ticks = 0;
}

void system_wait(int ms) {
    uint32_t target = timer_ticks + ms;
    while (timer_ticks < target) {
        __asm__ volatile("hlt");
    }
}

void system_halt(void) {
    while (1) {
        asm volatile("cli; hlt");
    }
}
