#include "framebuffer.h"
#include "systemcalls.h"
#include "trap.h"
#include "paging.h"
#include "frame.h"
#include "kheap.h"

#include <x86/idt.h>
#include <x86/gdt.h>
#include <x86/pic.h>
#include <x86/io.h>

#include "../drivers/ata.h"
#include "../utils/multiboot.h"

#define COS32_VER "v0.0.7"

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
        set_idt_gate(pic_loc+1, interrupt_kb_timer, INT_GATE_FLAGS);
        pic_IRQ_remove_mask(0); //timer
        pic_IRQ_remove_mask(1); //keyboard
        pic_IRQ_remove_mask(2); // slave PIC chip
	term_writestring("initing... Interrupts\n");

	term_writestring("initing... Paging\n");
        frame_init(frame_start_addr, frame_end_addr);
        paging_init();

	term_writestring("initing... ATA driver\n");
        ata_init(pic_loc);

        x86_int_on();
        term_setcolor(VGA_COLOR_LIGHT_BLUE);
	term_writestring("\nCookiesOS32! "COS32_VER"\n");
	term_writestring("Created by Jan Lomozik\n");
        term_setcolor(VGA_COLOR_WHITE);

        //term_puthex(frame_alloc()); term_putchar('\n');
        term_writestring("\n[i]testing malloc: \n");
        void *ptr1 = malloc(32);
	term_writestring("d: ");term_puthex(ptr1); term_putchar('\n');
        free(ptr1);

        void *ptr2 = malloc(28);
	term_writestring("d: ");term_puthex(ptr2); term_putchar('\n');

        void *ptr3 = malloc(16);
	term_writestring("d: ");term_puthex(ptr3); term_putchar('\n');
        free(ptr2);
        free(ptr3);

        term_writestring("\n[i]testing ata drive: \n");
        for(size_t i=0; i<10; i++) {
            uint8_t block = ata_read_block(i);
            term_puthex(block); term_putchar(' ');
        }
        term_putchar('\n');
        return 0;
}

