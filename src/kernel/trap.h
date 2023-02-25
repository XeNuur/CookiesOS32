#pragma once

struct interrupt_frame* frame;
void general_exception_handler(struct interrupt_frame*);
void general_exception_handler_err(struct interrupt_frame*);

void exception_div_by_zero(struct interrupt_frame*);
void exception_page_fault(struct interrupt_frame*);

void __attribute__ ((interrupt))interrupt_pit_timer(struct interrupt_frame*);
void __attribute__ ((interrupt))interrupt_kb_timer(struct interrupt_frame*);
