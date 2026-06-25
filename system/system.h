#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

#define ACPI_SHUTDOWN_PORT_QEMU    0x604
#define ACPI_SHUTDOWN_PORT_BOCHS   0xB004
#define ACPI_SHUTDOWN_PORT_VBOX    0x4004

#define ACPI_SHUTDOWN_VALUE_QEMU   0x2000
#define ACPI_SHUTDOWN_VALUE_BOCHS  0x2000
#define ACPI_SHUTDOWN_VALUE_VBOX   0x3400

#define PIT_CHANNEL0_DATA  0x40
#define PIT_COMMAND        0x43
#define PIT_FREQUENCY      1193182
#define PIT_CMD_MODE2      0x04
#define PIT_CMD_RW_BOTH    0x30
#define PIT_CMD_CHANNEL0   0x00

int system_is_hypervisor(void);
void system_shutdown(void);
void system_reboot(void);
void system_halt(void);
void system_wait(int ms);
void system_init_timer(void);
void timer_handler(void);

#endif
