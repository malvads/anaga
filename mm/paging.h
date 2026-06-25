#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PAGE_SIZE        4096
#define PAGE_ENTRIES     1024

#define PAGE_PRESENT     0x001
#define PAGE_RW          0x002
#define PAGE_USER        0x004
#define PAGE_WRITETHROUGH 0x008
#define PAGE_NOCACHE     0x010
#define PAGE_ACCESSED    0x020
#define PAGE_DIRTY       0x040
#define PAGE_4MB         0x080

#define KERNEL_BASE      0x00000000
#define KERNEL_SIZE      0x00400000

typedef uint32_t page_dir_entry_t;
typedef uint32_t page_table_entry_t;

void paging_init(void);
void paging_map_page(uint32_t virt, uint32_t phys, uint32_t flags);

#endif
