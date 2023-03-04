#include "framebuffer.h"

void panic(char* msg) {
   term_reset_cursor();

   term_setcolor(vga_entry_color(VGA_COLOR_RED, VGA_COLOR_BLUE));
   term_writestring("[KERNEL PANIC]: \n");
   term_writestring("reason: \n");

   term_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLUE));
   term_writestring(msg);

   term_setcolor(vga_entry_color(VGA_COLOR_RED, VGA_COLOR_BLUE));
   term_writestring("\n...hanging...\n");

   __asm__ volatile ("cli; hlt");
   while(true){};
}

void yell(char* msg) {
   term_reset_cursor();

   term_setcolor(vga_entry_color(VGA_COLOR_RED, VGA_COLOR_BLUE));
   term_writestring("[INTERNAL ERROR]: \n");
   term_writestring("reason: \n");

   term_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLUE));
   term_writestring(msg);
   term_putchar('\n');
   term_setcolor(VGA_COLOR_WHITE);
}
