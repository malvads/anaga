import struct
import sys
import os

MAX_INODES = 32
SECTOR_SIZE = 512
MAX_FILENAME = 32
MAX_DIR_ENTRIES = 16
MAX_FILE_SIZE = 8192

# inode_type_t: FREE=0, FILE=1, DIR=2
TYPE_FREE = 0
TYPE_FILE = 1
TYPE_DIR = 2

INODE_SIZE = 32 + 4 + 4 + 4 + 4 + (MAX_DIR_ENTRIES * 4) + MAX_FILE_SIZE
INODE_SECTORS = (INODE_SIZE + SECTOR_SIZE - 1) // SECTOR_SIZE

def create_disk(output_file, size_mb):
    size = size_mb * 1024 * 1024
    with open(output_file, 'wb') as f:
        f.write(b'\x00' * size)

def write_inode(f, index, inode_data):
    f.seek(index * INODE_SECTORS * SECTOR_SIZE)
    f.write(inode_data)

def pack_inode(name, itype, size, parent, num_entries, dir_entries, data):
    # struct format: 32s I I I I 16I 1024s
    # I = 4 byte unsigned int
    fmt = f"<{MAX_FILENAME}sIIII{MAX_DIR_ENTRIES}I{MAX_FILE_SIZE}s"
    # Ensure name is null-padded and fits
    name_bytes = name.encode('utf-8')[:MAX_FILENAME-1]
    name_bytes = name_bytes.ljust(MAX_FILENAME, b'\x00')
    
    # Pad dir entries
    de = list(dir_entries) + [0] * (MAX_DIR_ENTRIES - len(dir_entries))
    
    # Pad data
    d = data[:MAX_FILE_SIZE].ljust(MAX_FILE_SIZE, b'\x00')
    
    return struct.pack(fmt, name_bytes, itype, size, parent, num_entries, *de, d)

def main():
    if len(sys.argv) < 4:
        print("Usage: python3 mkfs.py <disk_img> <file_in_host> <path_in_vfs>")
        return

    disk_img = sys.argv[1]
    host_file = sys.argv[2]
    vfs_path = sys.argv[3] # simple filename for now

    if not os.path.exists(host_file):
        print(f"Error: {host_file} not found")
        return

    with open(host_file, 'rb') as hf:
        content = hf.read()

    if len(content) > MAX_FILE_SIZE:
        print(f"Warning: File too large ({len(content)}). Truncating to {MAX_FILE_SIZE}")
        content = content[:MAX_FILE_SIZE]

    with open(disk_img, 'r+b') as f:
        # 1. Read or create root inode (index 0)
        # For simplicity, we'll recreate a basic image with root + 1 file
        # Root (Index 0)
        root = pack_inode("", TYPE_DIR, 0, 0, 1, [1], b"")
        write_inode(f, 0, root)
        
        # File (Index 1)
        file_node = pack_inode(vfs_path, TYPE_FILE, len(content), 0, 0, [], content)
        write_inode(f, 1, file_node)
        
        print(f"Installed {vfs_path} at inode 1")

if __name__ == "__main__":
    main()
