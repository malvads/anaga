#include "include/string.h"
#include "include/stdlib.h"
#include "display/display.h"
#include "drivers/network/e1000.h"

int e1000_init(struct e1000 *dev, uint64_t mmio_base,
               struct e1000_rx_desc *rx_phys_mem,
               struct e1000_tx_desc *tx_phys_mem) {

    if (!dev || !rx_phys_mem || !tx_phys_mem) {
        kprint("e1000_init error: invalid pointer(s) provided\n");
        return -1;
    }

    kprint("e1000_init: Starting initialization...\n");

    memset(dev, 0, sizeof(*dev));
    dev->mmio_base = mmio_base;
    kprintf("MMIO Base set to 0x0%lx\n", dev->mmio_base);

    dev->rx_ring.size = RX_RING_SIZE;
    dev->rx_desc_mem = rx_phys_mem;
    dev->rx_ring.base = (uint64_t)(uintptr_t)rx_phys_mem;
    dev->rx_ring.head = 0;
    dev->rx_ring.tail = 0;
    kprintf("RX Ring initialized: base=0x%lx, size=%d\n", dev->rx_ring.base, RX_RING_SIZE);

    for (int i = 0; i < RX_RING_SIZE; i++) {
        dev->rx_desc_mem[i].buffer_addr = 0;
        dev->rx_desc_mem[i].status = 0;
        dev->rx_desc_mem[i].errors = 0;
        dev->rx_desc_mem[i].length = 0;
        dev->rx_desc_mem[i].checksum = 0;
        dev->rx_desc_mem[i].special = 0;
    }

    dev->tx_ring.size = TX_RING_SIZE;
    dev->tx_desc_mem = tx_phys_mem;
    dev->tx_ring.base = (uint64_t)(uintptr_t)tx_phys_mem;
    dev->tx_ring.head = 0;
    dev->tx_ring.tail = 0;
    kprintf("TX Ring initialized: base=0x%lx, size=%d\n", dev->tx_ring.base, TX_RING_SIZE);

    for (int i = 0; i < TX_RING_SIZE; i++) {
        dev->tx_desc_mem[i].buffer_addr = 0;
        dev->tx_desc_mem[i].status = 0x1;
        dev->tx_desc_mem[i].length = 0;
        dev->tx_desc_mem[i].cso = 0;
        dev->tx_desc_mem[i].cmd = 0;
        dev->tx_desc_mem[i].css = 0;
        dev->tx_desc_mem[i].special = 0;
    }

    kprint("e1000_init: Initialization complete\n");
    kprintf("Final state: MMIO=0x%lx, RX base=0x%lx, TX base=0x%lx\n",
            dev->mmio_base, dev->rx_ring.base, dev->tx_ring.base);

    return 0;
}

void e1000_destroy(struct e1000 *dev) {
    if (!dev) return;

    kprint("e1000_destroy: Resetting device state...\n");

    dev->rx_desc_mem = NULL;
    dev->tx_desc_mem = NULL;

    dev->rx_ring.base = 0;
    dev->rx_ring.size = 0;
    dev->rx_ring.head = 0;
    dev->rx_ring.tail = 0;

    dev->tx_ring.base = 0;
    dev->tx_ring.size = 0;
    dev->tx_ring.head = 0;
    dev->tx_ring.tail = 0;

    dev->mmio_base = 0;

    kprint("e1000_destroy: Device state reset complete\n");
}
