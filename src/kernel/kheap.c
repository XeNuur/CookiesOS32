#include "kheap.h"
#include "panic.h"
#include <string.h>

//debug purposes!
#include "framebuffer.h"

extern uint32_t krnl_end;
uint32_t* curr_addr = (uint32_t*)0x2000;

KheapHeader *frist_hh = 0;
KheapHeader *last_hh = 0;

KheapHeader _new_header(uint32_t size, KheapHeader* next) {
   KheapHeader heap = {
      .magic_number = MAGIC_KHEAP,
      .size = size,
      .freed = 0,
      .next = next,

      .zero = 0,
   };
   return heap;
}

void* _find_good_block(size_t size, int reset_free_flag) {
   KheapHeader* current = frist_hh;
   for(;;) {
      if(current->freed && current->size >= size) {
         current->freed = (reset_free_flag)?0:current->freed;
         return current; 
      }

      if(!current->next)
         return 0;
      current = current->next;
   }
}

void* malloc(size_t size) {
   uint32_t abs_size = size;
   uint32_t* addr = curr_addr;
   if(!frist_hh)
      goto create_new_header;

   void* found_addr = _find_good_block(size, 1);
   if(found_addr)
      return found_addr+sizeof(KheapHeader);

create_new_header:
   *(KheapHeader*)curr_addr = _new_header(size, last_hh);
   last_hh = curr_addr;
   if(!frist_hh)
      frist_hh = curr_addr;
   abs_size += sizeof(KheapHeader);

   curr_addr += abs_size;
   return (void*)addr+sizeof(KheapHeader);
}

void free(void* addr) {
   KheapHeader* hh_ptr = addr-sizeof(KheapHeader);
   if(hh_ptr->magic_number != MAGIC_KHEAP) {
      yell("free(...): magic number mismatch");
      return;
   }
   if(hh_ptr->freed)
      yell("free(...): double free detected!");
   hh_ptr->freed = 1;
}

void* realloc(void* addr, size_t size) {
   void* new_addr = malloc(size);
   memcpy(new_addr, addr, size);
   free(addr);
   return new_addr;
}

