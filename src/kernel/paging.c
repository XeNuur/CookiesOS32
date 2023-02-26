#include "paging.h"

void paging_map(uint32_t virt, uint32_t phys) {
	uint16_t id = virt >> 22;
	for(int i = 0; i < 1024; i++)
	{
		last_page->entries[i] = phys | 3;
		phys += PAGE_SIZE;
	}
	page_directory->entries[id] = ((uint32_t)last_page) | 3;
	last_page = (PageTable*)(((uint32_t)last_page) + PAGE_SIZE);
}

uint32_t *paging_page_get(uint32_t virt) {
   uint32_t* entry = &page_directory->entries[PD_INDEX(virt)];
   PageTable* table = (PageTable*)PAGE_PHYS_ADDRESS(entry);

   return &table->entries[PT_INDEX(virt)];
}

void paging_init() {
	last_page = (PageTable *)0x400000 + 0x4000;

	for(int i = 0; i < 1024; i++) {
		page_directory->entries[i] = 0x2;
	}
	paging_map(0, 0);
	paging_map(0x400000, 0x400000);
        
        load_page_directory((uint32_t)page_directory);
}
