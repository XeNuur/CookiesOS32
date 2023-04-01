#include "elf.h"
#include "../kernel/framebuffer.h"
#include "../kernel/paging.h"
#include <string.h>

int run_code(void* addr) {
   int (*code_fn)(uint32_t) = addr;
   return code_fn(addr);
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
      term_printf("Not a executable!");
      return 0;
   }

   ElfProgramHeader_t *ph = (ElfProgramHeader_t*)(buffer + header->e_phoff);
   for(int i=0; i<header->e_phnum; i++, ph++) {
      term_printf("Found segment %x! (virt: %x, size: %x)\n", ph->p_type, ph->p_vaddr, ph->p_memsz);
      if(ph->p_type != 1) //load
         continue;
      paging_map(ph->p_vaddr, frame_alloc());
      memcpy(ph->p_vaddr, buffer + ph->p_offset, ph->p_filesz);
   }
   term_printf("Running executable: %x",header->e_entry);
   run_code(buffer);
   //run_code(header->e_entry);
   return 1;
}

