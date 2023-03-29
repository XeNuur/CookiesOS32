#pragma once
#include <stdint.h>
#include "../fs/vfs.h"

#define ATA_STATUS_BSY   (0x80)
#define ATA_STATUS_RDY   (0x40)
#define ATA_STATUS_DRQ   (0x08)
#define ATA_STATUS_DF    (0x20)
#define ATA_STATUS_ERR   (0x01)

#define ATA_COM_READ     (0x20)
#define ATA_COM_IDENTIFY (0xEC)

#define ATA_MASTER_HD 	 (0xA0)
#define ATA_SLAVE_HD 	 (0xB0)

#define ATA_PRIMARY_IO     (0x1F0)
#define ATA_SECONDARY_IO   (0x170)

#define ATA_PRIMARY_DEVCTL   (0x3F6)
#define ATA_SECONDARY_DEVCTL (0x376)

// registers offset
#define ATA_REG_DATA       (0x00)
#define ATA_REG_ERROR      (0x01)
#define ATA_REG_FEATURES   (0x01)
#define ATA_REG_SECCOUNT0  (0x02)
#define ATA_REG_LBA0       (0x03)
#define ATA_REG_LBA1       (0x04)
#define ATA_REG_LBA2       (0x05)
#define ATA_REG_HDDEVSEL   (0x06)
#define ATA_REG_COMMAND    (0x07)
#define ATA_REG_STATUS     (0x07)
#define ATA_REG_SECCOUNT1  (0x08)
#define ATA_REG_LBA3       (0x09)
#define ATA_REG_LBA4       (0x0A)
#define ATA_REG_LBA5       (0x0B)
#define ATA_REG_CONTROL    (0x0C)
#define ATA_REG_ALTSTATUS  (0x0C)
#define ATA_REG_DEVADDRESS (0x0D)

#define ATA_SECTOR_SIZE (512)
typedef uint8_t ataChar_t;

void ata_init(uint8_t pic_loc);
uint8_t ata_read_sector(uint32_t lba, ataChar_t* data);
int ata_read_callback (Vfs_t*, uint32_t, uint32_t, char* );

