#include "io.h"

void x86_outb(uint16_t port, uint8_t val) {
   asm volatile("outb %%al, %%dx": :"d" (port), "a" (val));
}

uint8_t x86_inb(uint16_t port) {
   uint8_t ret;
   asm volatile("inb %%dx, %%al":"=a"(ret):"d"(port));
   return ret;
}

uint16_t x86_inw(uint16_t port) {
   uint16_t ret;
   asm volatile("inw %%dx, %%ax":"=a"(ret):"d"(port));
   return ret;
}


