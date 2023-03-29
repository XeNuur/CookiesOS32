#include "ata.h"
#include <x86/io.h>
#include <x86/pic.h>
#include <x86/idt.h>
#include <string.h>
#include "../kernel/panic.h"
#include "../kernel/trap.h"
#include "../kernel/framebuffer.h"
#include "../kernel/device.h"

#define ATA_ERR_BUS_FETCH   (-1)
#define ATA_ERR_DRIVE_FETCH (-2)

int is_master_exist = 0;

uint8_t _ata_poll();
void _ata_delay400ns();
uint8_t _ata_wait_drq();
void _ata_wait_bsy();

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
}

uint8_t _ide_identify() {
   x86_outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, ATA_MASTER_HD);
   x86_outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT0, 0x0); //sectors

   x86_outb(ATA_PRIMARY_IO + ATA_REG_LBA0, 0x0); //lba
   x86_outb(ATA_PRIMARY_IO + ATA_REG_LBA1, 0x0);
   x86_outb(ATA_PRIMARY_IO + ATA_REG_LBA2, 0x0);
   x86_outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_COM_IDENTIFY); //send identify command
                        
   uint8_t status = x86_inb(ATA_PRIMARY_IO + ATA_REG_STATUS);
   _ata_delay400ns();
   while (status & ATA_STATUS_BSY) {
      _ata_delay400ns();
      status = x86_inb(ATA_PRIMARY_IO + ATA_REG_STATUS);
   }
   _ata_wait_bsy();
   if(!status) return ATA_ERR_BUS_FETCH;
   _ata_wait_bsy();

   uint8_t mid = x86_inb(ATA_PRIMARY_IO + ATA_REG_LBA1);
   uint8_t hi  = x86_inb(ATA_PRIMARY_IO + ATA_REG_LBA2);

   if(mid || hi) return ATA_ERR_DRIVE_FETCH;
   if(_ata_poll()) return ATA_ERR_DRIVE_FETCH;

   _ata_wait_bsy();
   uint8_t identify_data[256];
   for(int i=0; i<256; ++i) {
      uint8_t stat = x86_inw(ATA_PRIMARY_IO + ATA_REG_DATA);

      identify_data[i] = stat;
   }
   return 0;
}

uint8_t _ata_poll() {
   _ata_delay400ns();
   _ata_wait_bsy();
   uint8_t status = _ata_wait_drq();
   if (status & ATA_STATUS_ERR) {
      yell("ata device faliure");
      return 1;
   }
   return 0;
}

void ata_init(uint8_t pic_loc) {
   set_idt_gate(pic_loc+14, ata_interrupt_primary_bus, TRAP_GATE_FLAGS);
   set_idt_gate(pic_loc+15, ata_interrupt_primary_bus, TRAP_GATE_FLAGS);
   pic_IRQ_remove_mask(14); 
   pic_IRQ_remove_mask(15); 

   _ata_reset();
   uint8_t status = _ide_identify();
   if(status != 0) {
      yell("Can't fetch ata bus");
      return;
   }
   is_master_exist = 1;

   Vfs_t device_handler = vfs_node_new();
   const char* name = "/dev/ata0";
   memcpy(device_handler.name, name, strlen(name));
   device_handler.read = ata_read_callback;
   device_handler.flag = VFS_BLOCK_DEV;

   devices_add(device_handler);
}

uint8_t ata_read_sector(uint32_t lba, ataChar_t* data) {
   if(!is_master_exist) 
      return -1;
   _ata_wait_bsy();

   x86_outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xE0 | ((lba >> 24) & 0xF));
   x86_outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT0, 0x1); //sectors

   x86_outb(ATA_PRIMARY_IO + ATA_REG_LBA0, lba); //lba
   x86_outb(ATA_PRIMARY_IO + ATA_REG_LBA1, (lba >> 8));
   x86_outb(ATA_PRIMARY_IO + ATA_REG_LBA2, (lba >> 16));

   x86_outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_COM_READ); //send read command
   
   _ata_wait_bsy();
   _ata_wait_drq();

   uint32_t index = 0;
   for(uint32_t i=0; i<256; ++i) {
      uint16_t out = x86_inw(ATA_PRIMARY_IO + ATA_REG_DATA);
      ataChar_t bytes[2] = { //endian spec
         out & 0xFF,
         (out >> 8) & 0xFF,
      };

      for(uint32_t j=0; j<2; ++j) {
         data[index] = bytes[j];
         ++index;
      }
   }
   return 0;
}

int ata_read_callback (Vfs_t* _node, uint32_t offset, uint32_t size, char* ptr) {
   for(uint32_t i=0; i<size; ++i) {
      ata_read_sector(offset+i, ptr);
      ptr += ATA_SECTOR_SIZE;
   }
}

