#pragma once
#include <stdint.h>

#define ATA_STATUS_BSY   (0x80)
#define ATA_STATUS_RDY   (0x40)
#define ATA_STATUS_DRQ   (0x08)
#define ATA_STATUS_DF    (0x20)
#define ATA_STATUS_ERR   (0x01)

#define ATA_COM_READ     (0x20)
#define ATA_COM_IDENTIFY (0xEC)

#define ATA_MASTER_HD 	 (0xA0)
#define ATA_SLAVE_HD 	 (0xB0)

void ata_init(uint8_t pic_loc);
uint8_t ata_read_block(uint32_t lba);
void ata_read_sector();
