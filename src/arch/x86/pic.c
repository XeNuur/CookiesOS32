#include "pic.h"
#include "io.h"
#include <stdint.h>

void pic_disable(void) {
   x86_outb(PIC1_DATA, 0xFF);
   x86_outb(PIC2_DATA, 0xFF);
}

void pic_send_eoi(int irq_line) {
   if (irq_line >= 8)
      x86_outb(PIC2_CMD, PIC_EOI);
   x86_outb(PIC1_CMD, PIC_EOI);
}

void pic_remap(int new_master_loc, int new_slave_loc) {
   pic_disable();
   //mask it all
   /*Init (in cascade mode)(ICW1)
    * 0x10 - Init
    * 0x01 - Send up to ICW4
    * */
   x86_outb(PIC1_CMD, 0x11);
   x86_outb(PIC2_CMD, 0x11);

   //Now move irq location to new place(ICW2)
   x86_outb(PIC1_DATA, new_master_loc);
   x86_outb(PIC2_DATA, new_slave_loc);

   //Tell the master that theres the slave PIC at IRQ2 (ICW3)
   x86_outb(PIC1_DATA, 4);
   //Tell the slave Pic its cascade identity (ICW3)
   x86_outb(PIC2_DATA, 2);

   //Junk on the x86 mode on the PIC'es (ICW4)
   x86_outb(PIC1_DATA, 0x01);
   x86_outb(PIC2_DATA, 0x01);

   //reset flags
   pic_disable();
}

uint16_t _pic_IRQ_fix_line_and_port(unsigned char* line) {
   if(*line < 8)
      return PIC1_DATA;
   (*line) -= 8;
   return PIC2_DATA;
}

void pic_IRQ_set_mask(unsigned char irq_line) {
   uint16_t port = _pic_IRQ_fix_line_and_port(&irq_line);
   uint8_t value = x86_inb(port) | (1 << irq_line);
   x86_outb(port, value);
}

void pic_IRQ_remove_mask(unsigned char irq_line) {
   uint16_t port = _pic_IRQ_fix_line_and_port(&irq_line);
   uint8_t value = x86_inb(port) & ~(1 << irq_line);
   x86_outb(port, value);
}

