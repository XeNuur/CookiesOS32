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
#include "../fs/devfs.h"
#include "../fs/fat.h"

#define COS_VER "2.6.5"

char* mini_shell_prefix = ">> ";

void* shell_prog_ptr[255];
char* shell_prog_names[255];
size_t shell_prog_size = 0;

char* cls_prog_name = "cls";
void cls_prog(void) {
   term_clean();
}

char* devfstest_prog_name = "devfstest";
void devfstest_prog(void) {
   Vfs_t* handle = fopen("/DEV/");
   if(!handle)
      return;

   uint32_t index = 0;
   for(;;) {
       VfsDirent_t dirent;
       if(vread_dir(handle, index, &dirent))
          break;
       term_printf("(%x) %s\n", index, dirent.name);
       ++index;
   }
   fclose(handle);
}

char* syscalltest_prog_name = "syscalltest";
void syscalltest_prog(void);

char* fatreader_prog_name = "fat16tester";
void fatreader_prog(void) {
   const size_t size = 0x2000;
   char* buffer = malloc(size);
   if(!buffer)
      return;
   
   Vfs_t* fat = fopen("/RES/MAFEST");
   if(!fat) {
      yell("cannot find file\n");
      goto ret_me;
   }
   if(vread(fat, 0, size, buffer)) {
      yell("unable to read\n");
      fclose(fat);
      goto ret_me;
   }
   fclose(fat);
   term_printf("%s\n", buffer);
ret_me:
   free(buffer);
}

char* runinit_prog_name = "init";
void runinit_prog(void) {
   term_writestring("Starting /BIN/INIT!\n");
   char* buffer = malloc(0x1000);
   
   Vfs_t* fat = fopen("/BIN/INIT");
   if(!fat) {
      yell("cannot find init binnary\n");
      goto ret_me;
   }
   if(vread(fat, 0, 0x1000, buffer)) {
      yell("unable to read init binnary\n");
      fclose(fat);
      goto ret_me;
   }
   fclose(fat);

   int status = elf_stat(buffer, 0x1000);
   term_printf("\nInit returned: %x!\n", status);
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
        kheap_print_headers();
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

        void *ptr1 = malloc(8);
        term_printf("ptr1: %x\n", ptr1);
        free(ptr1);

        void *ptr2 = malloc(8);
        term_printf("ptr2: %x\n", ptr2);

        void *ptr3 = malloc(16);
        term_printf("ptr3: %x\n", ptr3);
        free(ptr2);
        free(ptr3);
}

char* crashme_prog_name = "crashme";
void crashme_prog(void) {
   term_writestring("You asked for it!");
   size_t size = 0x100;

   int i = 0;
   for(;;) {
        i += size; 
        void *ptr = malloc(size);
        term_printf("Ptr: %x\nLen: %x\n (%x - %x) \n",
              ptr, i, frame_count(), frame_max());
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
        mini_shell_add_entry(devfstest_prog, devfstest_prog_name);
        mini_shell_add_entry(cls_prog, cls_prog_name);

        term_printf("running the build-in shell!\n"
              "type 'init' to rerun init program\n"
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
            term_setcolor(VGA_COLOR_LIGHT_GREY);
            goto parse_quit;

parse_quit:
            term_putchar('\n');
            term_writestring(mini_shell_prefix);
            input_index = 0;
        }
}

int kernel_main(multiboot_info_t *mbi) {
        term_init();
	term_writestring("Starting this junk up!\n");
        term_setcolor(VGA_COLOR_CYAN);

	term_writestring("[Init] Cheaking for memory map\n");
        term_setcolor(VGA_COLOR_LIGHT_CYAN);
        if(!(mbi->flags >> 6 & 0x1)) {
            yell("invalid memory map given by GRUB'en");
        }
        multiboot_memory_map_t* frame_part = (multiboot_memory_map_t*)(mbi->mmap_addr);
        uint32_t frame_start_addr = (frame_part->addr < 0x400) ?0x400 :frame_part->addr;
        uint32_t frame_end_addr = frame_part->addr + frame_part->len;
        if(frame_end_addr < frame_start_addr)
           panic("Invalid memory region part propotions (invalid grub memory map?)");
        term_printf("MEM SIZE: %x \n", mbi->mem_upper);
        term_setcolor(VGA_COLOR_CYAN);

	term_writestring("[Init] Loading GDT\n");
        gdt_init();

	term_writestring("[Init] Loading IDT\n");
        x86_int_off();
        idt_init();
        //init system calls
        set_idt_gate(0x80, syscall_dispatch, INT_GATE_USER_FLAGS);
        //init exceptions handling
        set_idt_gate(0, exception_div_by_zero, TRAP_GATE_FLAGS);
        set_idt_gate(0xD, exception_general_protection_fault, TRAP_GATE_FLAGS);
        set_idt_gate(14, exception_page_fault, TRAP_GATE_FLAGS);

	term_writestring("[Init] Setting up interrupts\n");
        pic_disable();
        pic_init();

        const uint8_t pic_loc = 0x20;
        set_idt_gate(pic_loc, interrupt_pit_timer, INT_GATE_FLAGS);
        pic_IRQ_remove_mask(0); //timer
        pic_IRQ_remove_mask(2); // slave PIC chip

	term_writestring("[Init] Enable paging\n");
        frame_init(frame_start_addr, frame_end_addr);
        paging_init();

        void* ptr = malloc(0x800);
        free(ptr);

	term_writestring("[Init] Setting devices subsystem\n");
        vfs_init();
        devices_init();

        uint32_t dev_id = ata_init(pic_loc);
        term_init_device();

        vmount_device(devices_at(dev_id), "/", 0);
        mount_devfs_device("/DEV/");

        kb_init(pic_loc);
        x86_int_on();

        term_setcolor(VGA_COLOR_LIGHT_GREEN);
	term_writestring("\nWelcome to CookiesOS (ver. "COS_VER")!\n\n");
        term_setcolor(VGA_COLOR_LIGHT_GREY);

        mini_shell();
        runinit_prog();
        return 0;
}

