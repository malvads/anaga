#include "drivers/usb/usb.h"
#include "drivers/usb/uhci.h"
#include "display/display.h"

void usb_init(usb_controller_type_t type, uint32_t base_addr) {
    kprint("USB: Found ");
    
    switch(type) {
        case USB_UHCI: 
            kprint("UHCI\n");
            uhci_init(base_addr);
            break;
        case USB_OHCI: kprint("OHCI (Not implemented)\n"); break;
        case USB_EHCI: kprint("EHCI (Not implemented)\n"); break;
        case USB_XHCI: kprint("XHCI (Not implemented)\n"); break;
    }
}
