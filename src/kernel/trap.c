#include "trap.h"
#include "framebuffer.h"
#include "panic.h"
#include <x86/pic.h>
#include <x86/memory.h>

void general_exception_handler_err(struct interrupt_frame* frame) {
   panic("CPU exception was trigged!");
}

void general_exception_handler(struct interrupt_frame* frame) {
   yell("An unexpected error happened!");
}

void exception_div_by_zero(struct interrupt_frame* frame) {
   yell("DIVITION BY ZERO");
}

void exception_page_fault(struct interrupt_frame* frame) {
   term_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
   term_writestring("A addres before tragedy: ");
   term_puthex(read_cr2()); term_putchar('\n');
   panic("PAGE FAULT");
}

void interrupt_pit_timer(struct interrupt_frame* frame) {
   term_putchar('.');
   pic_send_eoi(0);
}

void interrupt_kb_timer(struct interrupt_frame* frame) {
   //term_putchar(x86_inb(0x60));
   pic_send_eoi(1);
}
