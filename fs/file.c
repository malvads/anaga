#include "fs/file.h"
#include "include/string.h"
#include <stdbool.h>

static file_descriptor_t fd_table[MAX_OPEN_FILES];

int find_inode_in_dir(inode_t* inode_array, uint32_t dir_inode, const char* name) {
    if (dir_inode >= MAX_INODES) return -1;
    inode_t* dir = &inode_array[dir_inode];
    if (dir->type != INODE_DIR) return -1;

    for (uint32_t i = 0; i < dir->num_entries; i++) {
        uint32_t child_idx = dir->dir_entries[i];
        if (child_idx < MAX_INODES) {
            inode_t* child = &inode_array[child_idx];
            if (strcmp(child->name, name) == 0) return child_idx;
        }
    }
    return -1;
}

static int find_free_inode(inode_t* inode_array) {
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_array[i].type == INODE_FREE) return i;
    }
    return -1;
}

static int find_free_fd(void) {
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!fd_table[i].in_use) return i;
    }
    return -1;
}

int file_create(const char* name, uint32_t parent_inode, inode_t* inode_array) {
    int inode_idx = find_free_inode(inode_array);
    if (inode_idx < 0) return -1;

    inode_t* parent = &inode_array[parent_inode];
    if (parent->type != INODE_DIR) return -1;
    if (parent->num_entries >= MAX_DIR_ENTRIES) return -1;
    if (find_inode_in_dir(inode_array, parent_inode, name) >= 0) return -1;

    inode_t* new_file = &inode_array[inode_idx];
    memset(new_file, 0, sizeof(inode_t));
    strncpy(new_file->name, name, MAX_FILENAME - 1);
    new_file->name[MAX_FILENAME - 1] = '\0';
    new_file->type = INODE_FILE;
    new_file->size = 0;
    new_file->parent_inode = parent_inode;
    new_file->num_entries = 0;

    parent->dir_entries[parent->num_entries++] = inode_idx;
    return inode_idx;
}

int file_open(uint32_t inode_index, inode_t* inode_array) {
    if (inode_index >= MAX_INODES) return -1;
    inode_t* inode = &inode_array[inode_index];
    if (inode->type != INODE_FILE) return -1;

    int fd = find_free_fd();
    if (fd < 0) return -1;

    fd_table[fd].in_use = true;
    fd_table[fd].inode_index = inode_index;
    fd_table[fd].position = 0;
    return fd;
}

int file_read(int fd, void* buffer, uint32_t size, inode_t* inode_array) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fd_table[fd].in_use) return -1;
    inode_t* inode = &inode_array[fd_table[fd].inode_index];

    uint32_t pos = fd_table[fd].position;
    uint32_t bytes_to_read = size;
    if (pos + bytes_to_read > inode->size) bytes_to_read = inode->size - pos;

    memcpy(buffer, inode->data + pos, bytes_to_read);
    fd_table[fd].position += bytes_to_read;
    return bytes_to_read;
}

int file_write(int fd, const void* buffer, uint32_t size, inode_t* inode_array) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fd_table[fd].in_use) return -1;
    inode_t* inode = &inode_array[fd_table[fd].inode_index];

    uint32_t pos = fd_table[fd].position;
    uint32_t bytes_to_write = size;
    if (pos + bytes_to_write > MAX_FILE_SIZE) bytes_to_write = MAX_FILE_SIZE - pos;

    memcpy(inode->data + pos, buffer, bytes_to_write);
    fd_table[fd].position += bytes_to_write;
    if (fd_table[fd].position > inode->size) inode->size = fd_table[fd].position;

    return bytes_to_write;
}

void file_close(int fd) {
    if (fd >= 0 && fd < MAX_OPEN_FILES) fd_table[fd].in_use = false;
}

int file_seek(int fd, uint32_t offset) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fd_table[fd].in_use) return -1;
    fd_table[fd].position = offset;
    return 0;
}
