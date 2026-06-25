#include "fs/elf.h"
#include "fs/vfs.h"
#include "display/display.h"
#include "mm/paging.h"
#include "include/string.h"

int elf_load(const char* path, uint32_t* entry_point) {
    Elf32_Ehdr header;
    if (vfs_read_at(path, &header, sizeof(Elf32_Ehdr), 0) != sizeof(Elf32_Ehdr)) {
        kprint("ELF: Failed to read header\n");
        return -1;
    }

    if (header.e_ident[0] != 0x7F || header.e_ident[1] != 'E' || 
        header.e_ident[2] != 'L' || header.e_ident[3] != 'F') {
        kprint("ELF: Invalid magic\n");
        return -1;
    }

    *entry_point = header.e_entry;

    for (int i = 0; i < header.e_phnum; i++) {
        Elf32_Phdr phdr;
        uint32_t phdr_offset = header.e_phoff + (i * header.e_phentsize);
        
        if (vfs_read_at(path, &phdr, sizeof(Elf32_Phdr), phdr_offset) != sizeof(Elf32_Phdr)) {
            kprint("ELF: Failed to read phdr\n");
            return -1;
        }

        if (phdr.p_type == PT_LOAD) {
            kprintf("ELF: Loading segment at 0x%x (size: %d)\n", phdr.p_vaddr, phdr.p_filesz);
            
            int read = vfs_read_at(path, (void*)phdr.p_vaddr, phdr.p_filesz, phdr.p_offset);
            if (read < 0 || (uint32_t)read != phdr.p_filesz) {
                kprintf("ELF: Failed to load segment (read %d of %d)\n", read, phdr.p_filesz);
                return -1;
            }

            if (phdr.p_memsz > phdr.p_filesz) {
                memset((void*)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
            }
        }
    }

    return 0;
}
