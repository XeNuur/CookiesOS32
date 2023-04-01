#include "framebuffer.h"
#include "systemcalls.h"
#include "trap.h"
#include "paging.h"
#include "frame.h"
#include "kheap.h"
#include "panic.h"
#include "device.h"

#include <x86/idt.h>
#include <x86/gdt.h>
#include <x86/pic.h>
#include <x86/io.h>
#include <string.h>

#include "../drivers/ata.h"
#include "../drivers/keyboard.h"
#include "../utils/multiboot.h"
#include "../utils/elf.h"
#include "../fs/vfs.h"
#include "../fs/fat.h"

#define COS_VER "v2.1"

int exec_code(void* addr) {
   int (*code_fn)() = addr;
   return code_fn();
}

char* mini_shell_prefix = ">> ";

void* shell_prog_ptr[255];
char* shell_prog_names[255];
size_t shell_prog_size = 0;

char* syscalltest_prog_name = "syscalltest";
void syscalltest_prog(void);

char* fatreader_prog_name = "fat16tester";
void fatreader_prog(void) {
   char* buffer = malloc(256);
   
   Vfs_t* fat = fopen("/RES/MAFEST");
   if(!fat) {
      yell("cannot find file\n");
      goto ret_me;
   }
   if(!vread(fat, 0, 256, buffer)) {
      yell("unable to read\n");
      free(fat);
      goto ret_me;
   }
   term_printf("%s\n", buffer);
   free(fat);
ret_me:
   free(buffer);
}

char* runinit_prog_name = "init";
void runinit_prog(void) {
   char* buffer = malloc(100);
   
   Vfs_t* fat = fopen("/BIN/INIT");
   if(!fat) {
      yell("cannot find init binnary\n");
      goto ret_me;
   }
   if(!vread(fat, 0, 100, buffer)) {
      yell("unable to read init binnary\n");
      free(fat);
      goto ret_me;
   }
   free(fat);

   term_printf("Running from the '/BIN/INIT'!\n");
   int status = exec_code(buffer);
   term_printf("\ninit program returned: %x\n", status);
ret_me:
   free(buffer);
}

char* help_prog_name = "help";
void help_prog(void) {
        for(size_t i=0; i<shell_prog_size; ++i)
           term_printf("%s\n", shell_prog_names[i]);
}

char* helloworld_prog_name = "helloworld";
void helloworld_prog(void) {
        term_writestring("Hello, World!");
}

char* ataread_prog_name = "ataread";
void ataread_prog(void) {
        term_writestring("Reading from ata drive!\n");
        ataChar_t block[ATA_SECTOR_SIZE];

        vread(devices_at(0), 0, 1, block);
            
        for(uint32_t i=0; i<ATA_SECTOR_SIZE; ++i) {
            ataChar_t ch = block[i];
            term_printf("%x(%c) ", ch, ch);
        }
}

char* malloctest_prog_name = "malloctest";
void malloctest_prog(void) {
        term_writestring("Testing malloc: \n");

        void *ptr1 = malloc(32);
        term_printf("ptr1: %x\n", ptr1);
        free(ptr1);

        void *ptr2 = malloc(28);
        term_printf("ptr2: %x\n", ptr2);

        void *ptr3 = malloc(16);
        term_printf("ptr3: %x\n", ptr3);
        free(ptr2);
        free(ptr3);
}

char* crashme_prog_name = "crashme";
void crashme_prog(void) {
   term_writestring("You asked for it!");
   size_t size = 0x500;

   int i = 0;
   for(;;) {
        i += size; 
        void *ptr = malloc(size);
        term_printf("Ptr: %x\nLen: %x\n", ptr, i);
   }
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
        mini_shell_add_entry(crashme_prog, crashme_prog_name); 
        mini_shell_add_entry(fatreader_prog, fatreader_prog_name); 
        mini_shell_add_entry(runinit_prog, runinit_prog_name); 
        mini_shell_add_entry(syscalltest_prog, syscalltest_prog_name); 

        term_printf("running the build-in shell!\n"
              "type 'init' to run init program\n"
              "type 'help' to see other commands\n");
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
        uint32_t frame_start_addr = (frame_part->addr < 0x400) ?0x400 :frame_part->addr;
        uint32_t frame_end_addr = frame_part->addr + frame_part->len;
        if(frame_end_addr < frame_start_addr)
           panic("Invalid memory region part propotions (invalid grub memory map?)");
        term_printf("UPPER MEM SIZE: %x \n", mbi->mem_upper);
        term_printf("LOWER MEM SIZE: %x \n", mbi->mem_lower);

        term_writestring("MEM MAP: \n");
        for(int i=0; i < mbi->mmap_length; i+= sizeof(multiboot_memory_map_t)) {
            multiboot_memory_map_t* mmm = (multiboot_memory_map_t*)(mbi->mmap_addr+i);

            uint32_t addr = mmm->addr;
            uint32_t len = mmm->len;
            uint32_t size = mmm->size;
            uint32_t type = mmm->type;

            term_printf("Start: %x | Len: %x | Size: %x | Type %x \n", 
                  addr, len, size, type
            );
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

	term_writestring("initing... Devices & Vfs\n");
        vfs_init();
        devices_init();

	term_writestring("initing... ATA driver\n");
        uint32_t dev_id = ata_init(pic_loc);
        term_init_device();

        vmount_device(devices_at(dev_id), "/");

	term_writestring("initing... Keyboard driver\n");
        kb_init(pic_loc);

        x86_int_on();
        term_setcolor(VGA_COLOR_LIGHT_BLUE);
	term_writestring("\n");
	term_writestring("Buggy & Dirty(?)...\n");
	term_writestring("I mean enjoyable!\n");
	term_writestring("Welcome in CookiesOS ver. "COS_VER"!\n");
        term_setcolor(VGA_COLOR_WHITE);
        term_putchar('\n');

        mini_shell();
        return 0;
}

