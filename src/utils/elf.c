#include "elf.h"
#include "../kernel/framebuffer.h"
#include "../kernel/paging.h"
#include "../kernel/kheap.h"
#include <string.h>

int run_code(void* addr) {
   int (*code_fn)(void) = addr;
   return code_fn();
}

int elf_probe(char* buffer) {
   ElfHeader_t *header = (ElfHeader_t*)buffer;
   const char target_sig[4] = {
      0x7f,
      0x45,
      0x4c,
      0x46,
   };
   for(int i=0; i<4;++i) {
      if(header->e_ident[i] != target_sig[i])
         return 0;
   }
   return 1;
}

int elf_stat(char* buffer, size_t size) {
   if(!elf_probe(buffer) || size < 4) {
      term_printf("Not a elf format");
      return 0;
   }
   ElfHeader_t *header = (ElfHeader_t*)buffer;

   if(header->e_type != 0x2) {
      term_printf("Not a executable!\n");
      return 0;
   }

   ElfProgramHeader_t *ph = (ElfProgramHeader_t*)(buffer + header->e_phoff);
   const int frames = 1;

   void* phys_loc = frame_alloc_ex(frames);
   for(int i=0; i<header->e_phnum; i++, ph++) {
      if(ph->p_type != 1) //load
         continue;
      /*
      term_printf("LOAD: OFFSET: %x, VIRT: %x, PHYS: %x, FILESZ: %x, MEMSZ: %x \n",
            ph->p_offset, ph->p_vaddr, ph->p_paddr, ph->p_filesz, ph->p_memsz);
      */

      paging_map(ph->p_vaddr, phys_loc);
      memcpy(ph->p_vaddr, buffer + ph->p_offset, ph->p_filesz);
   }
   int retcode = run_code((void*)header->e_entry);
   frame_free_ex(phys_loc, frames);

   return retcode;
}

