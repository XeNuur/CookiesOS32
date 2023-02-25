#pragma once
#include <stdint.h>

typedef struct {
   uint64_t base; 
   uint64_t len;
   uint32_t type; 
   uint32_t acpi;
} MemoryBlock;

typedef enum {
   USABLE = 1,
   RESERVED = 2,
   APCI_RECLAIMABLE = 3,
   ACPI_NVS = 4,
   BAD_MEM = 5,
} MemoryBlockType;

//TODO: complete
int __attribute__((cdecl)) get_next_block(MemoryBlock *blk, uint32_t *cnt_id);
//
void __attribute__((cdecl)) load_page_directory(uint32_t);
uint32_t __attribute__((cdecl)) read_cr2();
