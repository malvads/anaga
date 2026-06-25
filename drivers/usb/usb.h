#ifndef USB_H
#define USB_H

#include <stdint.h>

typedef enum {
    USB_UHCI,
    USB_OHCI,
    USB_EHCI,
    USB_XHCI
} usb_controller_type_t;

void usb_init(usb_controller_type_t type, uint32_t base_addr);

#endif
