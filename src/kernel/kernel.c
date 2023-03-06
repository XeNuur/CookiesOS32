#include "framebuffer.h"
#include "systemcalls.h"
#include "trap.h"
#include "paging.h"
#include "frame.h"
#include "kheap.h"
#include "panic.h"

#include <x86/idt.h>
#include <x86/gdt.h>
#include <x86/pic.h>
#include <x86/io.h>
#include <string.h>

#include "../drivers/ata.h"
#include "../drivers/keyboard.h"
#include "../utils/multiboot.h"

#define COS32_VER "v0.0.8"

char* mini_shell_prefix = ">> ";

void* shell_prog_ptr[255];
char* shell_prog_names[255];
size_t shell_prog_size = 0;

char* help_prog_name = "help";
void help_prog(void) {
        for(size_t i=0; i<shell_prog_size; ++i) {
           term_writestring(shell_prog_names[i]);
           term_putchar('\n');
        }
}

char* helloworld_prog_name = "helloworld";
void helloworld_prog(void) {
        term_writestring("Hello, World!");
}

char* ataread_prog_name = "ataread";
void ataread_prog(void) {
        term_writestring("Reading from ata drive!\n");
        for(size_t i=0; i<25; i++) {
            uint8_t block = ata_read_block(i);
            term_puthex(block); term_putchar(' ');
        }
}

char* malloctest_prog_name = "malloctest";
void malloctest_prog(void) {
        term_writestring("Testing malloc: \n");

        void *ptr1 = malloc(32);
	term_writestring("ptr1: ");term_puthex(ptr1); term_putchar('\n');
        free(ptr1);

        void *ptr2 = malloc(28);
	term_writestring("ptr2: ");term_puthex(ptr2); term_putchar('\n');

        void *ptr3 = malloc(16);
	term_writestring("ptr3: ");term_puthex(ptr3); term_putchar('\n');
        free(ptr2);
        free(ptr3);
}

void mini_shell_add_entry(void (*prog)(void), char* prog_name) {
   shell_prog_ptr[shell_prog_size] = (void*)prog;
   shell_prog_names[shell_prog_size] = prog_name;
   shell_prog_size++;
}

void mini_shell(void) {
        mini_shell_add_entry(help_prog, help_prog_name);
        mini_shell_add_entry(helloworld_prog, helloworld_prog_name);
        mini_shell_add_entry(ataread_prog, ataread_prog_name); 
        mini_shell_add_entry(malloctest_prog, malloctest_prog_name); 
        term_writestring(mini_shell_prefix);

        char input_buffer[255];
        uint8_t input_index = 0;
        for(;;) {
            char ch = kb_wait_get();
            switch(ch) {
               case '\b':
                  if(!input_index) 
                     continue;

                  input_index--;
                  term_putchar('\n');

                  term_writestring(mini_shell_prefix);
                  for(int i=0; i<input_index; ++i)
                     term_putchar(input_buffer[i]);
                  continue;
               case '\n':
                  term_putchar('\n');
                  goto parse_input;
            }
            input_buffer[input_index++] = ch;
            term_putchar(ch);
            continue;
parse_input:
            for(size_t i=0; i<shell_prog_size; ++i) {
               if(memcmp(input_buffer, shell_prog_names[i], input_index) != 0)
                  continue;
               void (*callback)(void) = shell_prog_ptr[i];
               callback();
               goto parse_quit;
            }

            term_setcolor(VGA_COLOR_LIGHT_RED);
            term_writestring("Input error!");
            term_setcolor(VGA_COLOR_WHITE);
            goto parse_quit;

parse_quit:
            term_putchar('\n');
            term_writestring(mini_shell_prefix);
            input_index = 0;
        }
}

int kernel_main(multiboot_info_t *mbi) {
        term_init();
	term_writestring("CookiesOS32 init start!\n");
        term_setcolor(VGA_COLOR_LIGHT_GREEN);

	term_writestring("initing... MEMORY MAP:\n");
        term_setcolor(VGA_COLOR_GREEN);
        if(!(mbi->flags >> 6 & 0x1)) {
            yell("invalid memory map given by GRUB'en");
        }
        multiboot_memory_map_t* frame_part = (multiboot_memory_map_t*)(mbi->mmap_addr);
        uint32_t frame_start_addr = (frame_part->addr < 0x800) ?0x800 :frame_part->addr;
        uint32_t frame_end_addr = frame_part->addr + frame_part->len;
        if(frame_end_addr < frame_start_addr)
           panic("Invalid memory region part propotions (invalid grub memory map?)");

        term_writestring("UPPER MEM SIZE: "); term_puthex(mbi->mem_upper); term_putchar('\n');
        term_writestring("LOWER MEM SIZE: "); term_puthex(mbi->mem_lower); term_putchar('\n');
        term_writestring("MEM MAP: \n");
        for(int i=0; i < mbi->mmap_length; i+= sizeof(multiboot_memory_map_t)) {
            multiboot_memory_map_t* mmm = (multiboot_memory_map_t*)(mbi->mmap_addr+i);

	    term_writestring("Start: "); term_puthex(mmm->addr);
	    term_writestring("| Len:  "); term_puthex(mmm->len);
	    term_writestring("| Size: "); term_puthex(mmm->size);
	    term_writestring("| Type: "); term_puthex(mmm->type);
            term_putchar('\n');
        }
        term_setcolor(VGA_COLOR_LIGHT_GREEN);

	term_writestring("initing... GDT\n");
        gdt_init();

	term_writestring("initing... IDT\n");
        x86_int_off();
        idt_init();
        //init system calls
        set_idt_gate(0x80, syscall_dispatch, INT_GATE_USER_FLAGS);
        //init exceptions handling
        set_idt_gate(0, exception_div_by_zero, TRAP_GATE_FLAGS);
        set_idt_gate(14, exception_page_fault, TRAP_GATE_FLAGS);

	term_writestring("initing... PIC\n");
        pic_disable();
        pic_init();

        const uint8_t pic_loc = 0x20;
        set_idt_gate(pic_loc, interrupt_pit_timer, INT_GATE_FLAGS);
        pic_IRQ_remove_mask(0); //timer
        pic_IRQ_remove_mask(2); // slave PIC chip
	term_writestring("initing... Interrupts\n");

	term_writestring("initing... Paging\n");
        frame_init(frame_start_addr, frame_end_addr);
        paging_init();

	term_writestring("initing... ATA driver\n");
        ata_init(pic_loc);

	term_writestring("initing... Keyboard driver\n");
        kb_init(pic_loc);

        x86_int_on();
        term_setcolor(VGA_COLOR_LIGHT_BLUE);
	term_writestring("\nWelcome in CookiesOS32 ver. "COS32_VER"!\n");
	term_writestring("Created by Jan Lomozik\n");
        term_setcolor(VGA_COLOR_WHITE);

        term_putchar('\n');
        mini_shell();
        return 0;
}

