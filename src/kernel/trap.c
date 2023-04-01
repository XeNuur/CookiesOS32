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
   yell("DIVISION_BY_ZERO");
}

void exception_page_fault(Registers* frame) {
   int present = !(frame->err_code & 0x1); 
   int rw = frame->err_code & 0x2;           
   int us = frame->err_code & 0x4;           
   int reserved = frame->err_code & 0x8;     
   int id = frame->err_code & 0x10;          

   panic("PAGE_FAULT\n"
         "Caused addres: %x\n"
         "Raw error code: %x\n\n"
         "Details:\n"
         "Present: %x\n"
         "RW: %x\n"
         "User-mode: %x\n"
         "Reserved: %x\n",

         read_cr2(), frame->err_code,
         present,
         rw,
         us,
         reserved);
}

void interrupt_pit_timer(Registers* frame) {
   _INT_BEGIN;
   //term_putchar('.');
   pic_send_eoi(0);
   _INT_END;
}

