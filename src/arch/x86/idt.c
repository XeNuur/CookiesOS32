#include "idt.h"
#include "../../kernel/trap.h"

extern void* isr_stub_table[];

void set_idt_gate(int index, void* isr, uint8_t flags) {
   IdtEntry_t* it = &the_idt[index];

   it->isr_low = (uint32_t)isr & 0xFFFF;
   it->kernel_cs = 0x08; //KERNEL CODE SECTION FROM GDT
   it->reserved = 0;
   it->attributes = flags;
   it->isr_high = ((uint32_t)isr >> 16) & 0xFFFF;
}

void idt_init() {
   idtr.base = (uintptr_t)&the_idt[0];
   idtr.limit = (uint16_t)sizeof(IdtEntry_t)*256;

   //exeption hanlers
   for (uint8_t i=0; i<32; ++i) {
      set_idt_gate(i, isr_stub_table[i], TRAP_GATE_FLAGS);
   }
   /*regural interupts/*
   for (uint8_t i=32; i<256; ++i) {
      set_idt_gate(i, general_interrupt_handler, INT_GATE_FLAGS);
   }
   */

   load_idt(&idtr);
}

