#include "drivers/drive/drive.h"
#include "io/io.h"
#include "display/display.h"

static void ata_wait_bsy() {
    while (inb(ATA_STATUS) & ATA_STATUS_BSY);
}

static void ata_wait_drq() {
    while (!(inb(ATA_STATUS) & ATA_STATUS_DRQ));
}

void drive_init(void) {
    kprint("ATA Drive initializing...\n");
    outb(ATA_DRIVE_SELECT, ATA_DRIVE_MASTER);
    kprint("ATA Drive initialized (Master)\n");
}

void drive_read_sector(uint32_t lba, uint16_t* buffer) {
    ata_wait_bsy();
    
    outb(ATA_DRIVE_SELECT, ATA_DRIVE_LBA_MODE | ((lba >> 24) & 0x0F));
    outb(ATA_SECTOR_COUNT, 1);
    outb(ATA_LBA_LOW, (uint8_t)lba);
    outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_COMMAND, ATA_CMD_READ);

    ata_wait_bsy();
    ata_wait_drq();

    for (int i = 0; i < 256; i++) {
        buffer[i] = inw(ATA_DATA);
    }
}

void drive_write_sector(uint32_t lba, uint16_t* buffer) {
    ata_wait_bsy();

    outb(ATA_DRIVE_SELECT, ATA_DRIVE_LBA_MODE | ((lba >> 24) & 0x0F));
    outb(ATA_SECTOR_COUNT, 1);
    outb(ATA_LBA_LOW, (uint8_t)lba);
    outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_COMMAND, ATA_CMD_WRITE);

    ata_wait_bsy();
    ata_wait_drq();

    for (int i = 0; i < 256; i++) {
        outw(ATA_DATA, buffer[i]);
    }
}