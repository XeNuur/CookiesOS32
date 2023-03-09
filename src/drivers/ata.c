#include "ata.h"
#include <x86/io.h>
#include <x86/pic.h>
#include <x86/idt.h>
#include "../kernel/panic.h"
#include "../kernel/trap.h"

#define ATA_ERR_BUS_FETCH   (-1)
#define ATA_ERR_DRIVE_FETCH (-2)

//check for existment
uint8_t ata_pm = 0;
uint8_t ata_ps = 0;

void _ata_delay400ns() {
   for(int _i=0; _i<4; ++_i)
      x86_inb(0x1FC);
}

void _ata_reset() {
    x86_outb(ATA_PRIMARY_DEVCTL, 0x04);
    _ata_delay400ns();
    x86_outb(ATA_PRIMARY_DEVCTL, 0x00);
}

void ata_interrupt_primary_bus() {
   _INT_BEGIN;
   yell("14");
   pic_send_eoi(14);
   _INT_END;
}

void ata_interrupt_secondary_bus() {
   _INT_BEGIN;
   pic_send_eoi(15);
   _INT_END;
}

void _ata_wait_bsy() { //Wait for bsy to be 0
   while(x86_inb(ATA_PRIMARY_IO + ATA_REG_STATUS)&ATA_STATUS_BSY);
}

uint8_t _ata_wait_drq() { //Wait fot drq to be 1
   while(!(x86_inb(ATA_PRIMARY_IO + ATA_REG_STATUS)&ATA_STATUS_RDY));
   return x86_inb(ATA_PRIMARY_IO + ATA_REG_STATUS);
}

uint8_t _ide_identify(uint16_t drive) {
   x86_outb(0x1F6, drive);

   x86_outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, drive);
   x86_outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT0, 0x0); //sectors

   x86_outb(ATA_PRIMARY_IO + ATA_REG_LBA0, 0x0); //lba
   x86_outb(ATA_PRIMARY_IO + ATA_REG_LBA1, 0x0);
   x86_outb(ATA_PRIMARY_IO + ATA_REG_LBA2, 0x0);
   x86_outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_COM_IDENTIFY); //send identify command
                        
   uint8_t status = x86_inb(0x1F7);
   if(!status)
      return ATA_ERR_BUS_FETCH;
   status = _ata_wait_drq();
   if(status & ATA_STATUS_ERR)
      return ATA_ERR_DRIVE_FETCH;
   return 0;
}

void _ata_poll() {
   _ata_delay400ns();
   _ata_wait_bsy();
   uint8_t status = _ata_wait_drq();
   if (status & ATA_STATUS_ERR)
      yell("ata device faliure");
}

void ata_init(uint8_t pic_loc) {
   //set_idt_gate
   _ata_reset();
   uint8_t status = _ide_identify(ATA_MASTER_HD);
   if(status == ATA_ERR_BUS_FETCH) {
      yell("Can't fetch ata bus");
      return;
   }
   ata_pm = (status==0)?1:0;
   ata_ps = (_ide_identify(ATA_SLAVE_HD)==0)?1:0;

   if(!ata_pm && !ata_ps) {
      yell("Can't fetch any ata drives");
      return;
   }
}

uint8_t ata_read_block(uint32_t lba) {
   x86_outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xE0 | ((lba >> 24) & 0xF));
   x86_outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT0, 0x1); //sectors

   x86_outb(ATA_PRIMARY_IO + ATA_REG_LBA0, lba & 0xFF); //lba
   x86_outb(ATA_PRIMARY_IO + ATA_REG_LBA1, (lba >> 8) & 0xFF);
   x86_outb(ATA_PRIMARY_IO + ATA_REG_LBA2, (lba >> 16) & 0xFF);

   x86_outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_COM_READ); //send read command
   _ata_poll();
   
   uint8_t sector = x86_inb(0x1F0);
   _ata_wait_bsy();
   return sector;
}

