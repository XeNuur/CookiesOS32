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

#define COS32_VER "v0.0.5"

extern uint32_t krnl_end;

int kernel_main(void) {
        term_init();
	term_writestring("CookiesOS32 init start!\n");
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

        set_idt_gate(0x20, interrupt_pit_timer, INT_GATE_FLAGS);
        set_idt_gate(0x20+1, interrupt_kb_timer, INT_GATE_FLAGS);
        pic_IRQ_remove_mask(0); //timer
        pic_IRQ_remove_mask(1); //keyboard
        pic_IRQ_remove_mask(2); // slave PIC chip
	term_writestring("initing... Interrupts\n");

	term_writestring("initing... Paging\n");
        frame_init();
        paging_init();

	term_writestring("initing... ATA driver\n");
        ata_init();

        x86_int_on();
        term_setcolor(VGA_COLOR_LIGHT_BLUE);
	term_writestring("\nCookiesOS32! "COS32_VER"\n");
	term_writestring("Created by Jan Lomozik\n\n");

        term_setcolor(VGA_COLOR_WHITE);
	term_writestring("Krnl end: ");term_puthex(krnl_end); term_putchar('\n');
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
        for(size_t i=0; i<3; i++) {
            uint8_t block = ata_read_block(i);
            term_puthex(block); term_putchar(' ');
        }
        term_putchar('\n');
        return 0;
}
