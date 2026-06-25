CC = i686-elf-gcc
AS = nasm
LD = i686-elf-ld

CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -I.
ASFLAGS = -f elf32
LDFLAGS = -T linker.ld -m elf_i386

BUILD_DIR = build

BOOT_SRC = boot/boot.asm
ISR_SRC = interrupts/isr.asm
KERNEL_SRCS = $(shell find . -name "*.c" -not -path "./user/*" -not -path "./$(BUILD_DIR)/*")
BOOT_OBJ = $(BUILD_DIR)/boot.o
ISR_OBJ = $(BUILD_DIR)/interrupts/isr.o
KERNEL_OBJS = $(patsubst ./%.c, $(BUILD_DIR)/%.o, $(KERNEL_SRCS))
KERNEL_BIN = $(BUILD_DIR)/anaga.bin
DRIVE_IMG = $(BUILD_DIR)/drive.img

SECTOR_SIZE = 512
DRIVE_SIZE_MB = 1

.PHONY: all clean run

all: $(KERNEL_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BOOT_OBJ): $(BOOT_SRC) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(ISR_OBJ): $(ISR_SRC) | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/%.o: ./%.c | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_BIN): $(BOOT_OBJ) $(ISR_OBJ) $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

$(DRIVE_IMG): $(BUILD_DIR)/user/init/init.o
	@mkdir -p $(BUILD_DIR)
	dd if=/dev/zero of=$(DRIVE_IMG) bs=$(SECTOR_SIZE) count=$(shell echo $$(($(DRIVE_SIZE_MB)*1024*1024/$(SECTOR_SIZE))))
	$(CC) $(CFLAGS) -nostdlib -Wl,-e,_start -Wl,-Ttext,0x200000 user/init/init.c -o $(BUILD_DIR)/init_elf
	python3 tools/mkfs.py $(DRIVE_IMG) $(BUILD_DIR)/init_elf init
	@echo "Disk image created with /init ELF"

run: $(KERNEL_BIN) $(DRIVE_IMG)
	qemu-system-i386 \
        -kernel $(KERNEL_BIN) \
        -drive file=$(DRIVE_IMG),format=raw \
        -serial stdio

clean:
	rm -rf $(BUILD_DIR)
