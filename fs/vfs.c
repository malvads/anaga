#include "fs/vfs.h"
#include "fs/file.h"
#include "display/display.h"
#include "drivers/drive/drive.h"
#include "include/string.h"

#define INODE_TABLE_SECTOR 0
#define SECTOR_SIZE 512

static uint32_t current_dir = 0;
static inode_t inode_array[MAX_INODES];

static void vfs_load(void) {
    uint8_t buffer[SECTOR_SIZE];
    int inode_sectors = (sizeof(inode_t) + SECTOR_SIZE - 1) / SECTOR_SIZE;

    for (int i = 0; i < MAX_INODES; i++) {
        for (int s = 0; s < inode_sectors; s++) {
            drive_read_sector(INODE_TABLE_SECTOR + i * inode_sectors + s, (uint16_t*)buffer);
            int copy_size = (s == inode_sectors - 1) ? sizeof(inode_t) - s * SECTOR_SIZE : SECTOR_SIZE;
            memcpy(((uint8_t*)&inode_array[i]) + s * SECTOR_SIZE, buffer, copy_size);
        }
    }
}

static void vfs_sync(void) {
    uint8_t buffer[SECTOR_SIZE];
    int inode_sectors = (sizeof(inode_t) + SECTOR_SIZE - 1) / SECTOR_SIZE;

    for (int i = 0; i < MAX_INODES; i++) {
        for (int s = 0; s < inode_sectors; s++) {
            int copy_size = (s == inode_sectors - 1) ? sizeof(inode_t) - s * SECTOR_SIZE : SECTOR_SIZE;
            memset(buffer, 0, SECTOR_SIZE);
            memcpy(buffer, ((uint8_t*)&inode_array[i]) + s * SECTOR_SIZE, copy_size);
            drive_write_sector(INODE_TABLE_SECTOR + i * inode_sectors + s, (uint16_t*)buffer);
        }
    }
}

void vfs_init(void) {
    vfs_load();

    inode_t* root = &inode_array[0];
    if (root->type != INODE_DIR) {
        memset(root, 0, sizeof(inode_t));
        root->type = INODE_DIR;
        vfs_sync();
    }
    current_dir = 0;
}

int vfs_mkdir(const char* name) {
    int idx = -1;
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_array[i].type == INODE_FREE) { idx = i; break; }
    }
    if (idx < 0) { kprint("No free inodes\n"); return -1; }

    inode_t* parent = &inode_array[current_dir];
    if (parent->num_entries >= MAX_DIR_ENTRIES) { kprint("Directory full\n"); return -1; }
    if (find_inode_in_dir(inode_array, current_dir, name) >= 0) { kprint("Already exists\n"); return -1; }

    inode_t* dir = &inode_array[idx];
    memset(dir, 0, sizeof(inode_t));
    strncpy(dir->name, name, MAX_FILENAME - 1);
    dir->type = INODE_DIR;
    dir->parent_inode = current_dir;

    parent->dir_entries[parent->num_entries++] = idx;
    vfs_sync();
    return idx;
}

void vfs_ls(void) {
    inode_t* dir = &inode_array[current_dir];
    if (dir->num_entries == 0) { kprint("(empty)\n"); return; }

    for (uint32_t i = 0; i < dir->num_entries; i++) {
        kprint(inode_array[dir->dir_entries[i]].name);
        kputc('\n');
    }
}

int vfs_cd(const char* path) {
    if (strcmp(path, "..") == 0) {
        if (current_dir != 0) current_dir = inode_array[current_dir].parent_inode;
        return 0;
    }
    if (strcmp(path, "/") == 0) { current_dir = 0; return 0; }

    int idx = find_inode_in_dir(inode_array, current_dir, path);
    if (idx < 0) { kprint("Not found\n"); return -1; }
    if (inode_array[idx].type != INODE_DIR) { kprint("Not a directory\n"); return -1; }

    current_dir = idx;
    return 0;
}

void vfs_pwd(void) {
    uint32_t idx = current_dir;
    uint32_t path[32];
    int depth = 0;

    while (1) {
        path[depth++] = idx;
        if (idx == 0) break;
        idx = inode_array[idx].parent_inode;
    }

    for (int i = depth - 1; i >= 0; i--) {
        kputc('/');
        if (inode_array[path[i]].name[0]) kprint(inode_array[path[i]].name);
    }
}

int vfs_touch(const char* name) {
    int idx = file_create(name, current_dir, inode_array);
    if (idx < 0) { kprint("Cannot create file\n"); return -1; }
    vfs_sync();
    return 0;
}

int vfs_cat(const char* name) {
    int idx = find_inode_in_dir(inode_array, current_dir, name);
    if (idx < 0) { kprint("File not found\n"); return -1; }
    if (inode_array[idx].type == INODE_DIR) { kprint("Is a directory\n"); return -1; }

    int fd = file_open(idx, inode_array);
    if (fd < 0) { kprint("Cannot open\n"); return -1; }

    char buf[MAX_FILE_SIZE + 1];
    int n = file_read(fd, buf, MAX_FILE_SIZE, inode_array);
    if (n > 0) {
        buf[n] = '\0';
        kprint(buf);
        if (buf[n-1] != '\n') kputc('\n');
    }

    file_close(fd);
    return 0;
}

int vfs_write_file(const char* name, const char* content) {
    int idx = find_inode_in_dir(inode_array, current_dir, name);
    if (idx < 0) {
        idx = file_create(name, current_dir, inode_array);
        if (idx < 0) { kprint("Cannot create\n"); return -1; }
    }

    int fd = file_open(idx, inode_array);
    if (fd < 0) { kprint("Cannot open\n"); return -1; }

    int len = strlen(content);
    int written = file_write(fd, content, len, inode_array);
    file_close(fd);

    if (written < len) { kprint("Write incomplete\n"); return -1; }

    vfs_sync();
    return 0;
}

int vfs_read(const char* name, void* buffer, uint32_t size) {
    int idx = find_inode_in_dir(inode_array, current_dir, name);
    if (idx < 0) return -1;
    if (inode_array[idx].type == INODE_DIR) return -1;
    int fd = file_open(idx, inode_array);
    if (fd < 0) return -1;
    int n = file_read(fd, buffer, size, inode_array);
    file_close(fd);
    return n;
}
int vfs_read_at(const char* name, void* buffer, uint32_t size, uint32_t offset) {
    int idx = find_inode_in_dir(inode_array, current_dir, name);
    if (idx < 0) return -1;
    if (inode_array[idx].type == INODE_DIR) return -1;

    int fd = file_open(idx, inode_array);
    if (fd < 0) return -1;

    file_seek(fd, offset);
    int n = file_read(fd, buffer, size, inode_array);
    file_close(fd);
    return n;
}
