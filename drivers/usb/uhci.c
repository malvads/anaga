#include "drivers/usb/uhci.h"
#include "io/io.h"
#include "display/display.h"

void uhci_init(uint32_t io_base) {
    kprint("UHCI: Resetting controller at 0x");
    kprint_num(io_base);
    kprint("\n");

    outw(io_base + UHCI_USBCMD, 0x0004);
    
    for(int i=0; i<10000; i++) asm volatile("nop");
    
    outw(io_base + UHCI_USBCMD, 0x0000);

    outw(io_base + UHCI_USBSTS, 0x003F);

    kprint("UHCI: Reset complete.\n");
}
