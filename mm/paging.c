#include "mm/paging.h"

static page_dir_entry_t page_directory[PAGE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static page_table_entry_t first_page_table[PAGE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

void paging_init(void) {
    for (int i = 0; i < PAGE_ENTRIES; i++) {
        first_page_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
    }

    page_directory[0] = ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_RW | PAGE_USER;

    for (int i = 1; i < PAGE_ENTRIES; i++) {
        page_directory[i] = 0;
    }

    __asm__ volatile(
        "mov %0, %%cr3\n"
        "mov %%cr0, %%eax\n"
        "or $0x80000000, %%eax\n"
        "mov %%eax, %%cr0\n"
        :
        : "r"(page_directory)
        : "eax"
    );
}

void paging_map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t pd_idx = virt >> 22;
    uint32_t pt_idx = (virt >> 12) & 0x3FF;

    if (!(page_directory[pd_idx] & PAGE_PRESENT)) {
        return;
    }

    page_table_entry_t* pt = (page_table_entry_t*)(page_directory[pd_idx] & 0xFFFFF000);
    pt[pt_idx] = (phys & 0xFFFFF000) | (flags & 0xFFF) | PAGE_PRESENT;

    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}
