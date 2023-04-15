#include "kheap.h"
#include "panic.h"
#include "frame.h"
#include <string.h>
#include "paging.h"

//debug purposes!
#include "framebuffer.h"

extern uint32_t krnl_end;
uint32_t* curr_addr = (uint32_t*)&krnl_end;
uint32_t* max_page_addr = (uint32_t*)&krnl_end;

KheapHeader *frist_hh = 0;
KheapHeader *last_hh = 0;

KheapHeader _new_header(uint32_t size, KheapHeader* next) {
   KheapHeader heap = {
      .magic_number = MAGIC_KHEAP,
      .size = size,
      .freed = 0,
      .next = next,
   };
   return heap;
}

KheapHeader* _split_block(KheapHeader* header, size_t size) {
   int new_size = header->size - size - sizeof(KheapHeader);
   if(!(new_size > 0))
      return NULL;

   KheapHeader *new_header = (KheapHeader*)((void*)header + sizeof(KheapHeader) + size);
   *new_header = _new_header(new_size, header->next);
   new_header->freed = 1;

   header->size = size;
   header->next = new_header;

   if(!new_header->next)
      last_hh = new_header;
   return new_header;
}

void* _find_good_block(size_t size, int reset_free_flag) {
   KheapHeader* next = frist_hh;

   do {
      if(!next->freed)
         continue;
      if(next->size >= size) {
         next->freed = (reset_free_flag)?0:next->freed;
         _split_block(next, size);
         return next; 
      }
   } while ((next = next->next));
   return 0;
}

void _merge_free_blocks() {
   KheapHeader* next = frist_hh;
   KheapHeader* current = 0;

   do {
      if(!next->freed) {
         current = 0;
         continue;
      }
      if(!current) {
         current = next;
         continue;
      }
      
      current->size += next->size + sizeof(KheapHeader);
      current->next = next->next;
   } while((next = next->next));

   if(current)
      last_hh = current;
}


void* malloc(size_t size) {
   uint32_t abs_size = size;
   uint32_t* addr = curr_addr;
   if(frist_hh) {
      void* found_addr = _find_good_block(size, 1);
      if(found_addr)
         return found_addr+sizeof(KheapHeader);
   }
/*
   uint32_t pages_needed = 1;
   pages_needed += size/PAGE_SIZE;

   for(int i=0; i<pages_needed; ++i) {
      paging_map((uint32_t)(curr_addr+i*PAGE_SIZE), frame_alloc());
      max_page_addr += PAGE_SIZE;
   }
*/

   *(KheapHeader*)curr_addr = _new_header(size, 0);
   if(!frist_hh) {
      frist_hh = (KheapHeader*)curr_addr;
      last_hh = (KheapHeader*)curr_addr;
   } else {
      last_hh->next = (KheapHeader*)curr_addr;
      last_hh = (KheapHeader*)curr_addr;
   }
   abs_size += sizeof(KheapHeader);

alloc_pages_end:
   curr_addr += abs_size;
   return (void*)addr+sizeof(KheapHeader);
}

void free(void* addr) {
   KheapHeader* hh_ptr = addr-sizeof(KheapHeader);
   if(hh_ptr->magic_number != MAGIC_KHEAP) {
      yell("free(...): magic number mismatch (%x != %x)", hh_ptr->magic_number, MAGIC_KHEAP);
      return;
   }
   if(hh_ptr->freed)
      yell("free(...): double free detected!");
   hh_ptr->freed = 1;
   _merge_free_blocks();
}

void* realloc(void* addr, size_t size) {
   void* new_addr = malloc(size);
   memcpy(new_addr, addr, size);
   free(addr);
   return new_addr;
}

void kheap_print_headers() {
   KheapHeader* next = frist_hh;

   do {
      term_printf("\n[addr %x (real: %x) ]:\n", next, next+sizeof(KheapHeader));
      term_printf("size: %x\n", next->size);

      char* freed_msg = (next->freed)?"Yes":"No";
      term_printf("freed: %s\n", freed_msg);
   } while ((next = next->next));
}

