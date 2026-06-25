#ifndef FILE_H
#define FILE_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_INODES 32
#define MAX_OPEN_FILES 32
#define MAX_FILE_SIZE 8192
#define MAX_DIR_ENTRIES 16
#define MAX_FILENAME 32

typedef enum { INODE_FREE, INODE_FILE, INODE_DIR } inode_type_t;

typedef struct {
    char name[MAX_FILENAME];
    inode_type_t type;
    uint32_t size;
    uint32_t parent_inode;
    uint32_t num_entries;
    uint32_t dir_entries[MAX_DIR_ENTRIES];
    uint8_t data[MAX_FILE_SIZE];
} inode_t;

typedef struct {
    bool in_use;
    uint32_t inode_index;
    uint32_t position;
} file_descriptor_t;

int file_create(const char* name, uint32_t parent_inode, inode_t* inode_array);
int file_open(uint32_t inode_index, inode_t* inode_array);
int file_read(int fd, void* buffer, uint32_t size, inode_t* inode_array);
int file_write(int fd, const void* buffer, uint32_t size, inode_t* inode_array);
void file_close(int fd);
int file_seek(int fd, uint32_t offset);
int find_inode_in_dir(inode_t* inode_array, uint32_t dir_inode, const char* name);

#endif
