#pragma once
#include <stdint.h>

void x86_outb(uint16_t port, uint8_t val);
uint8_t x86_inb(uint16_t port);
uint16_t x86_inw(uint16_t port);

void __attribute__((cdecl)) x86_io_wait(void);
void __attribute__((cdecl)) x86_int_on();
void __attribute__((cdecl)) x86_int_off();

