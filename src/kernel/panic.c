#include "framebuffer.h"

void panic(char* msg, ...) {
   term_reset_cursor();

   term_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_RED));
   term_clean();
   term_writestring("\n[!KERNEL PANIC!]: \n\n");

   va_list ap;
   va_start(ap, msg);
   term_vprintf(msg, ap);
   va_end(ap);

   term_writestring("\n\n...Halt...\n");

   __asm__ volatile ("cli; hlt");
   while(true){};
}

void yell(char* msg, ...) {
   term_reset_cursor();
   term_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_RED));
   term_writestring("\n[INTERNAL ERROR]: \n\n");

   va_list ap;
   va_start(ap, msg);
   term_vprintf(msg, ap);
   va_end(ap);

   term_putchar('\n');
   term_setcolor(VGA_COLOR_LIGHT_GREY);
}

