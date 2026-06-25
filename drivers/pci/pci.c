#include "drivers/pci/pci.h"
#include "io/io.h"
#include "display/display.h"
#include "drivers/usb/usb.h"
#include "drivers/pci/vendor.h"
#include "drivers/network/e1000.h"

#define MAX_PCI_BUS  256
#define MAX_PCI_SLOT 32
#define PCI_MULTIFUNCTION_FLAG 0x80
#define PCI_HEADER_TYPE 0x0E
#define PCI_BAR1 0x14

static uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)(
        ((uint32_t)bus  << 16) |
        ((uint32_t)slot << 11) |
        ((uint32_t)func << 8)  |
        (offset & 0xFC)        |
        0x80000000u
    );

    outl(PCI_CONFIG_ADDR, address);
    return inl(PCI_CONFIG_DATA);
}

static int pci_device_exists(uint8_t bus, uint8_t slot, uint8_t func) {
    uint32_t vendor_device = pci_read_config(bus, slot, func, PCI_VENDOR_ID);
    return (vendor_device & 0xFFFF) != PCI_VENDOR_NONE;
}

static void pci_handle_usb(uint8_t bus, uint8_t slot, uint8_t func) {
    uint32_t class_rev = pci_read_config(bus, slot, func, PCI_CLASS_REVISION);
    uint8_t base_class = (class_rev >> 24) & 0xFF;
    uint8_t sub_class  = (class_rev >> 16) & 0xFF;
    uint8_t prog_if    = (class_rev >> 8) & 0xFF;

    if (base_class != PCI_CLASS_SERIAL || sub_class != PCI_SUBCLASS_USB) return;

    uint32_t bar0 = pci_read_config(bus, slot, func, PCI_BAR0);
    uint32_t base_addr = bar0 & PCI_BAR_IO_MASK;

    usb_controller_type_t type;
    switch (prog_if) {
        case PCI_PROGIF_UHCI: type = USB_UHCI; break;
        case PCI_PROGIF_OHCI: type = USB_OHCI; break;
        case PCI_PROGIF_EHCI: type = USB_EHCI; break;
        case PCI_PROGIF_XHCI: type = USB_XHCI; break;
        default: return;
    }

    usb_init(type, base_addr);
}

static int is_qemu_network_device(uint16_t vendor, uint16_t device) {
    if (vendor != PCI_VENDOR_INTEL) return 0;

    switch (device) {
        case E1000_DEV_82540EM: // QEMU default NIC
        case E1000_DEV_82545EM:
        case E1000_DEV_82543GC:
        case E1000_DEV_82574L:
        case E1000_DEV_I217:
            return 1;
        default:
            return 0;
    }
}
static void pci_print_device(uint8_t bus, uint8_t slot, uint8_t func) {
    uint32_t vd = pci_read_config(bus, slot, func, PCI_VENDOR_ID);
    uint16_t vendor = vd & 0xFFFF;
    uint16_t device = (vd >> 16) & 0xFFFF;
    kprintf("PCI Device: bus=%d slot=%d func=%d vendor=0x%x device=0x%x\n", bus, slot, func, vendor, device);
}

static void pci_handle_network(uint8_t bus, uint8_t slot, uint8_t func) {
    uint32_t vd = pci_read_config(bus, slot, func, PCI_VENDOR_ID);
    uint16_t vendor = vd & 0xFFFF;
    uint16_t device = (vd >> 16) & 0xFFFF;

    if (!is_qemu_network_device(vendor, device)) return;

    switch (device) {
        case E1000_DEV_82540EM:
            kprintf("Found Intel 82540EM at bus %d, slot %d, func %d\n", bus, slot, func);
            break;
        case E1000_DEV_82545EM:
            kprintf("Found Intel 82545EM at bus %d, slot %d, func %d\n", bus, slot, func);
            break;
        case E1000_DEV_82543GC:
            kprintf("Found Intel 82543GC at bus %d, slot %d, func %d\n", bus, slot, func);
            break;
        case E1000_DEV_82574L:
            kprintf("Found Intel 82574L at bus %d, slot %d, func %d\n", bus, slot, func);
            break;
        case E1000_DEV_I217:
            kprintf("Found Intel I217 at bus %d, slot %d, func %d\n", bus, slot, func);
            break;
        default:
            return;
    }

    uint32_t bar0 = pci_read_config(bus, slot, func, PCI_BAR0);
    uint32_t bar1 = pci_read_config(bus, slot, func, PCI_BAR1);

    uint64_t mmio_base;
    if (bar0 & 0x1) {
        mmio_base = bar0 & 0xFFFFFFFC;
    } else {
        mmio_base = (bar0 & 0xFFFFFFF0) |
                    ((uint64_t)(bar1 & 0xFFFFFFFF) << 32);
    }

    struct e1000_rx_desc rx_ring_mem[RX_RING_SIZE] __attribute__((aligned(16)));
    struct e1000_tx_desc tx_ring_mem[TX_RING_SIZE] __attribute__((aligned(16)));

    struct e1000 dev;
    int ret = e1000_init(&dev, mmio_base, rx_ring_mem, tx_ring_mem);
    if (ret != 0) {
        kprint("Failed to initialize e1000\n");
    } else {
        kprint("e1000 initialized successfully\n");
    }
}

static void pci_scan_function(uint8_t bus, uint8_t slot, uint8_t func) {
    if (!pci_device_exists(bus, slot, func)) return;
    pci_print_device(bus, slot, func);
    pci_handle_usb(bus, slot, func);
    pci_handle_network(bus, slot, func);
}

static void pci_scan_slot(uint16_t bus, uint8_t slot) {
    uint8_t header_type = (pci_read_config(bus, slot, 0, PCI_HEADER_TYPE) >> 16) & 0xFF;
    pci_scan_function((uint8_t)bus, slot, 0);
    if (header_type & PCI_MULTIFUNCTION_FLAG) {
        for (uint8_t func = 1; func < 8; func++) {
            pci_scan_function((uint8_t)bus, slot, func);
        }
    }
}

void pci_scan(void) {
    kprint("Scanning PCI bus...\n");
    for (uint16_t bus = 0; bus < MAX_PCI_BUS; bus++) {
        for (uint8_t slot = 0; slot < MAX_PCI_SLOT; slot++) {
            pci_scan_slot(bus, slot);
        }
    }
    kprint("PCI scan complete.\n");
}
