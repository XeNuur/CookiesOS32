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

void ata_interrupt_primary_bus() {
   pic_send_eoi(14);
}

void ata_interrupt_secondary_bus() {
   pic_send_eoi(15);
}


void _ata_wait_bsy() { //Wait for bsy to be 0
   while(x86_inb(0x1F7)&ATA_STATUS_BSY);
}

void _ata_wait_drq() { //Wait fot drq to be 1
   while(!(x86_inb(0x1F7)&ATA_STATUS_RDY));
}

uint8_t _ide_identify(uint16_t drive) {
   x86_outb(0x1F6, drive);

   x86_outb(0x1F1, 0x00);
   x86_outb(0x1F2, (unsigned char) 0x1); //sectors

   x86_outb(0x1F3, 0x0); //lba
   x86_outb(0x1F4, 0x0);
   x86_outb(0x1F5, 0x0);
   x86_outb(0x1F7, ATA_COM_IDENTIFY); //send identify command
                        
   uint8_t status = x86_inb(0x1F7);
   if(!status)
      return ATA_ERR_BUS_FETCH;

   _ata_wait_bsy();
   status = x86_inb(0x1F7);
   if(status & ATA_STATUS_ERR)
      return ATA_ERR_DRIVE_FETCH;
   return 0;
}

void _ata_check() {
   uint8_t status = x86_inb(0x1F7);
   if (status & ATA_STATUS_ERR)
      yell("ata device faliure");
}

void ata_init(uint8_t pic_loc) {
   //set_idt_gate
   set_idt_gate(pic_loc+14, ata_interrupt_primary_bus, INT_GATE_FLAGS);
   set_idt_gate(pic_loc+15, ata_interrupt_secondary_bus, INT_GATE_FLAGS);
   pic_IRQ_remove_mask(14);
   pic_IRQ_remove_mask(15);

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
   uint8_t drive = 0xE0;
   if(!ata_pm) drive = 0xF0;

   x86_outb(0x1F6, drive | ((lba >> 24) & 0xF));

   x86_outb(0x1F1, 0x00);
   x86_outb(0x1F2, (unsigned char) 0x1); //sectors

   x86_outb(0x1F3, (unsigned char) lba); //lba
   x86_outb(0x1F4, (unsigned char) (lba >> 8));
   x86_outb(0x1F5, (unsigned char) (lba >> 16));
   x86_outb(0x1F7, ATA_COM_READ); //send read command

   _ata_wait_bsy();
   _ata_check();
   return x86_inb(0x1F0);
}

