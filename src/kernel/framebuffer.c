#include "framebuffer.h"
#include <string.h>

size_t term_row = 0;
size_t term_column = 0;
uint8_t term_color = 0;
uint16_t* term_buffer = (uint16_t*) 0xB8000;

void term_put_char_at(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	term_buffer[index] = vga_entry(c, color);
}
 
void term_init(void) {
	term_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
	for (size_t y = 0; y < VGA_HEIGHT; y++)
		for (size_t x = 0; x < VGA_WIDTH; x++)
                        term_put_char_at(' ', term_color, x, y);
}
 
void term_setcolor(uint8_t color) {
	term_color = color;
}

void term_putchar(char c) {
        x86_outb(0xE9, c);
        switch(c) {
            case '\n':
               term_column = 0;
               goto newline_callback;
            default:
	       term_put_char_at(c, term_color, term_column, term_row);
        }

	if (!(++term_column >= VGA_WIDTH))
                return;
        term_column = 0;
newline_callback:
        if (!(++term_row >= VGA_HEIGHT))
                return;
        term_row = VGA_HEIGHT-1;
        term_newline();
}
 
void term_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		term_putchar(data[i]);
}
 
void term_writestring(const char* data) {
	term_write(data, strlen(data));
}

void term_reset_cursor() {
   term_row = 0;
   term_color = 0;
}

const uint16_t hex_digits[] = {
    u'0', u'1', u'2', u'3', u'4', u'5', u'6', u'7', u'8', u'9', u'a', u'b', u'c', u'd', u'e', u'f'
};

void term_puthex(uint32_t hex) {
   char buffer[9];
   char *p = buffer;

   for (int i = 7; i >= 0; --i) {
      uint8_t nibble = (hex & (0xf << (i*4))) >> (i*4);
      *p++ = hex_digits[nibble];
   }
   *p = 0;
   term_writestring(buffer);
}

void term_newline() {
   memcpy(term_buffer, term_buffer+VGA_WIDTH, VGA_WIDTH*(VGA_HEIGHT-1)*2);
   for(size_t i=0; i<VGA_WIDTH; ++i) {
      term_put_char_at(' ', term_color, i, VGA_HEIGHT-1);
   }
}

void term_printf(const char* fmt, ...) {
   va_list ap;
   va_start(ap, fmt);
   term_vprintf(fmt, ap);
   va_end(ap);
}

void term_vprintf(const char* fmt, va_list arg) {
   int* arg_ptr = (int*)&fmt;
   arg_ptr += sizeof(fmt) / sizeof(int);

   while(*fmt) {
      if(*fmt != '%') {
         term_putchar(*fmt);
         fmt++;
         continue;
      }
      fmt++;
      switch(*fmt) {
         case 'c':
            term_putchar(va_arg(arg, int));
            break;
         case 's':
            term_writestring(va_arg(arg, char*));
            break;
         case 'x':
            term_puthex(va_arg(arg, uint32_t));
            break;
      }
      arg_ptr++;
      fmt++;
   }
}

