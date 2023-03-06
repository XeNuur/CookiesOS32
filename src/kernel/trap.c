#include "trap.h"
#include "framebuffer.h"
#include "panic.h"
#include <x86/pic.h>
#include <x86/memory.h>

void general_exception_handler_err(Registers* frame) {
   panic("CPU exception was trigged!");
}

void general_exception_handler(Registers* frame) {
   yell("An unexpected error happened!");
}

void exception_div_by_zero(Registers* frame) {
   yell("DIVITION BY ZERO");
}

void exception_page_fault(Registers* frame) {
   term_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
   term_writestring("A addres before tragedy: ");
   term_puthex(read_cr2()); term_putchar('\n');
   panic("PAGE FAULT");
}

void interrupt_pit_timer(Registers* frame) {
   _INT_BEGIN;
   //term_putchar('.');
   pic_send_eoi(0);
   _INT_END;
}

