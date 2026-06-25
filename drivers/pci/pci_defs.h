#ifndef PCI_DEFS_H
#define PCI_DEFS_H

#define PCI_CONFIG_ADDR    0xCF8
#define PCI_CONFIG_DATA    0xCFC

#define PCI_VENDOR_ID      0x00
#define PCI_CLASS_REVISION 0x08
#define PCI_BAR0           0x10

#define PCI_CLASS_SERIAL   0x0C
#define PCI_SUBCLASS_USB   0x03

#define PCI_PROGIF_UHCI    0x00
#define PCI_PROGIF_OHCI    0x10
#define PCI_PROGIF_EHCI    0x20
#define PCI_PROGIF_XHCI    0x30

#define PCI_VENDOR_NONE    0xFFFF
#define PCI_BAR_IO_MASK    0xFFFFFFFC
#define PCI_BAR_MEM_MASK   0xFFFFFFF0

#endif
