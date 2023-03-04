#pragma once
#include <x86/idt.h>

void general_exception_handler(Registers*);
void general_exception_handler_err(Registers*);

void exception_div_by_zero(Registers*);
void exception_page_fault(Registers*);

void interrupt_pit_timer(Registers*);
void interrupt_kb_timer(Registers*);

