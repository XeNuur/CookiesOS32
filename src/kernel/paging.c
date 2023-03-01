#include "paging.h"
#include <string.h>

void paging_page_new(PTEntry *page){
   uint32_t frame = frame_alloc();
   if(!frame) 
      return;

   SET_FRAME(page, frame);
   SET_ATTRIBUTE(page, PTE_PRESENT);
}

void paging_map(uint32_t virt, uint32_t phys) {
   PDEntry* pd_entry = &page_directory->entries[PD_INDEX(virt)];

   if(TEST_ATTRIBUTE(pd_entry, PTE_PRESENT) != PTE_PRESENT) {
      PageTable* newtab = frame_alloc();
      if(!newtab) return;
      memset(newtab, 0x0, sizeof(PageTable));
      
      PDEntry* pd_entry = &page_directory->entries[PD_INDEX(virt)];
      SET_ATTRIBUTE(pd_entry, PDE_PRESENT);
      SET_ATTRIBUTE(pd_entry, PDE_WRITABLE);
   }

   PageTable* table = PAGE_PHYS_ADDRESS(pd_entry);
   PTEntry *page = &table->entries[PT_INDEX(virt)];

   SET_ATTRIBUTE(page, PTE_PRESENT);
   SET_FRAME(page, phys);
}

uint32_t *paging_page_get(uint32_t virt) {
   uint32_t* entry = &page_directory->entries[PD_INDEX(virt)];
   PageTable* table = (PageTable*)PAGE_PHYS_ADDRESS(entry);

   return &table->entries[PT_INDEX(virt)];
}

void paging_init() {
        PageTable* table = (PageTable*)frame_alloc();
	for (int i=0, frame=0x0, virt=0x00000000; i<1024; i++, frame+=4096, virt+=4096) {
		PTEntry page=0;
		SET_ATTRIBUTE(&page, PTE_PRESENT);
 		SET_FRAME(&page, frame);

		table->entries[PT_INDEX (virt) ] = page;
	}
        page_directory = frame_alloc_ex(3);
	for(int i = 0; i < 1024; i++)
		page_directory->entries[i] = 0x2;

        PDEntry *entry = &page_directory->entries[PD_INDEX(0x0)];
        SET_ATTRIBUTE(entry, PDE_PRESENT);
        SET_ATTRIBUTE(entry, PDE_WRITABLE);
        SET_FRAME(entry, (uint32_t)table);
        
        load_page_directory((uint32_t)page_directory);
}

