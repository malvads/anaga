#ifndef STDLIB_H
#define STDLIB_H

#include "include/stddef.h"
#include <stdint.h>

#define KERNEL_HEAP_START 0x100000  // 1 MB
#define KERNEL_HEAP_SIZE  0x400000  // 4 MB heap

typedef struct kblock {
    size_t size;
    int free;
    struct kblock* next;
} kblock_t;

static kblock_t* heap_start = NULL;

static void kheap_init() {
    if (!heap_start) {
        heap_start = (kblock_t*)KERNEL_HEAP_START;
        heap_start->size = KERNEL_HEAP_SIZE - sizeof(kblock_t);
        heap_start->free = 1;
        heap_start->next = NULL;
    }
}

static inline void* kmalloc(size_t size) {
    kheap_init();
    if (size == 0) return NULL;
    size = (size + 7) & ~7;

    kblock_t* current = heap_start;

    while (current) {
        if (current->free && current->size >= size) {
            if (current->size > size + sizeof(kblock_t)) {
                kblock_t* new_block = (kblock_t*)((uintptr_t)current + sizeof(kblock_t) + size);
                new_block->size = current->size - size - sizeof(kblock_t);
                new_block->free = 1;
                new_block->next = current->next;

                current->size = size;
                current->next = new_block;
            }
            current->free = 0;
            return (void*)((uintptr_t)current + sizeof(kblock_t));
        }
        current = current->next;
    }

    return NULL;
}

static inline void kfree(void* ptr) {
    if (!ptr) return;

    kblock_t* block = (kblock_t*)((uintptr_t)ptr - sizeof(kblock_t));
    block->free = 1;

    kblock_t* current = heap_start;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += sizeof(kblock_t) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

#endif
