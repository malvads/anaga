#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include "fs/file.h"

void vfs_init(void);
int vfs_mkdir(const char* name);
int vfs_read(const char* name, void* buffer, uint32_t size);
int vfs_read_at(const char* name, void* buffer, uint32_t size, uint32_t offset);
void vfs_ls(void);
int vfs_cd(const char* path);
void vfs_pwd(void);
int vfs_touch(const char* name);
int vfs_cat(const char* name);
int vfs_write_file(const char* name, const char* content);

#endif
