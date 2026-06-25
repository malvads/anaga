#ifndef E1000_H
#define E1000_H

#include <stdint.h>

#define RX_RING_SIZE 128
#define TX_RING_SIZE 128

struct e1000_rx_desc {
    uint64_t buffer_addr;
    uint16_t length;
    uint16_t checksum;
    uint8_t status;
    uint8_t errors;
    uint16_t special;
} __attribute__((packed));

struct e1000_tx_desc {
    uint64_t buffer_addr;
    uint16_t length;
    uint8_t cso;
    uint8_t cmd;
    uint8_t status;
    uint8_t css;
    uint16_t special;
} __attribute__((packed));

struct ring {
    uint64_t base;
    uint32_t size;
    uint32_t head;
    uint32_t tail;
};

struct e1000 {
    struct ring rx_ring;
    struct ring tx_ring;
    uint64_t mmio_base;

    struct e1000_rx_desc *rx_desc_mem;
    struct e1000_tx_desc *tx_desc_mem;
};

int e1000_init(struct e1000 *dev, uint64_t mmio_base,
               struct e1000_rx_desc *rx_phys_mem,
               struct e1000_tx_desc *tx_phys_mem);

void e1000_destroy(struct e1000 *dev);

#endif
