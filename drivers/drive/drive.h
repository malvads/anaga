#ifndef DRIVE_H
#define DRIVE_H

#include <stdint.h>

#define ATA_DATA           0x1F0
#define ATA_ERROR          0x1F1
#define ATA_SECTOR_COUNT   0x1F2
#define ATA_LBA_LOW        0x1F3
#define ATA_LBA_MID        0x1F4
#define ATA_LBA_HIGH       0x1F5
#define ATA_DRIVE_SELECT   0x1F6
#define ATA_STATUS         0x1F7
#define ATA_COMMAND        0x1F7

#define ATA_CMD_READ       0x20
#define ATA_CMD_WRITE      0x30

#define ATA_STATUS_BSY     0x80
#define ATA_STATUS_DRQ     0x08

#define ATA_DRIVE_MASTER   0xA0
#define ATA_DRIVE_LBA_MODE 0xE0

void drive_init(void);
void drive_read_sector(uint32_t lba, uint16_t* buffer);
void drive_write_sector(uint32_t lba, uint16_t* buffer);

#endif
