#include "keyboard.h"
#include "../kernel/framebuffer.h"
#include <x86/pic.h>
#include <x86/idt.h>
#include <x86/io.h>

#define KB_DATA_PORT (0x60)
#define KB_CMD_PORT  (0x64)

/* KBDUS means US Keyboard Layout. This is a scancode table
*  used to layout a standard US keyboard. I have left some
*  comments in to give you an idea of what key is what, even
*  though I set it's array index to 0. You can change that to
*  whatever you want using a macro, if you wish! */
unsigned char kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};	

char current_ch = 0;

char buffer[255];
int handled_index = 0;
int is_handled = 1;

void kb_interrupt_hander(Registers* regs) {
   _INT_BEGIN;

   unsigned char code = x86_inb(0x60);
   if(code & 0x80) //special chars
      goto end;

   current_ch = kbdus[code];
   buffer[handled_index++] = current_ch;
   is_handled = 0;

end:
   pic_send_eoi(1);
   _INT_END;
}

void kb_init(uint8_t pic_loc) {
   set_idt_gate(pic_loc+1, kb_interrupt_hander, INT_GATE_FLAGS);
   pic_IRQ_remove_mask(1); //keyboard
}

char kb_get_curr() { return current_ch; }

char kb_get() {
   if(!handled_index)
      return kb_get_curr();
   return buffer[--handled_index];
}

char kb_wait_get() {
   while(is_handled);
   is_handled = 1;
   return kb_get();
}

