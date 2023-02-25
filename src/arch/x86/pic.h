#pragma once

#define PIC1		   0x20		/* IO base address for master PIC */
#define PIC1_CMD	   PIC1
#define PIC1_DATA	   (PIC1+1)

#define PIC2		   0xA0		/* IO base address for slave PIC */
#define PIC2_CMD	   PIC2
#define PIC2_DATA	   (PIC2+1)

#define PIC_EOI		   0x20		/* End-of-interrupt command code */

#define PIC_IRQ_OFFSET     (0x20)

void pic_send_eoi(int irq_line);
void pic_remap(int new_master_loc, int new_slave_loc);
void pic_IRQ_set_mask(unsigned char irq_line);
void pic_IRQ_remove_mask(unsigned char irq_line);

static inline void pic_init(void) {
   pic_remap(PIC_IRQ_OFFSET, PIC_IRQ_OFFSET + 0x8);
}
void pic_disable(void);

