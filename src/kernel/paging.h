#pragma once
#include <stdint.h>
#include <x86/memory.h>
#include "frame.h"

/* Paging now will be really simple
 * we reserve 0-8MB for kernel stuff
 * heap will be from approx 1mb to 4mb
 * and paging stuff will be from 4mb
 */
#define PAGING_MEM_START (0x400000)
#define PAGE_SIZE (4096)

#define PD_INDEX(address) ((address) >> 22)
#define PT_INDEX(address) (((address) >> 12) & 0x3FF) // Max index 1023 = 0x3FF
#define PAGE_ALIGN(addr) ((((uint32_t)(addr)) & 0xFFFFF000) + 0x1000)
#define PAGE_PHYS_ADDRESS(dir_entry) ((*dir_entry) & ~0xFFF)    // Clear lowest 12 bits, only return frame/address
#define SET_ATTRIBUTE(entry, attr) (*entry |= attr)
#define CLEAR_ATTRIBUTE(entry, attr) (*entry &= ~attr)
#define TEST_ATTRIBUTE(entry, attr) (*entry & attr)
#define SET_FRAME(entry, address) (*entry = (*entry & ~0x7FFFF000) | address)   // Only set address/frame, not flags
#define GET_FRAME(entry) (*entry & ~0x7FFFF000)   // Only set address/frame, not flags

typedef struct {
   uint32_t entries[1024];
} PageTable;

typedef struct {
   uint32_t entries[1024];
} PageDirectory;

static PageDirectory *page_directory = (PageDirectory*)PAGING_MEM_START;
static PageTable *last_page = (PageTable *)PAGING_MEM_START + 0x4000;

void paging_map(uint32_t virt, uint32_t phys);
uint32_t *paging_page_get(uint32_t virt);
void paging_init();
