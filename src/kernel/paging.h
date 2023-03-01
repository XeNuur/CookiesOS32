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

typedef enum {
    PTE_PRESENT       = 0x01,
    PTE_WRITABLE      = 0x02,
    PTE_USER          = 0x04,
    PTE_WRITE_THROUGH = 0x08,
    PTE_CACHE_DISABLE = 0x10,
    PTE_ACCESSED      = 0x20,
    PTE_DIRTY         = 0x40,
    PTE_PAT           = 0x80,
    PTE_GLOBAL        = 0x100,
    PTE_FRAME         = 0x7FFFF000,   // bits 12+
} PageTElags;

typedef enum {
    PDE_PRESENT       = 0x01,
    PDE_WRITABLE      = 0x02,
    PDE_USER          = 0x04,
    PDE_WRITE_THROUGH = 0x08,
    PDE_CACHE_DISABLE = 0x10,
    PDE_ACCESSED      = 0x20,
    PDE_DIRTY         = 0x40,          // 4MB entry only
    PDE_4MB_SIZE      = 0x80,          // 0 = 4KB page, 1 = 4MB page
    PDE_GLOBAL        = 0x100,         // 4MB entry only
    PDE_PAT           = 0x2000,        // 4MB entry only
    PDE_FRAME         = 0x7FFFF000,    // bits 12+
} PageDEFlags;

#define PD_INDEX(address) ((address) >> 22)
#define PT_INDEX(address) (((address) >> 12) & 0x3FF) // Max index 1023 = 0x3FF
#define PAGE_ALIGN(addr) ((((uint32_t)(addr)) & 0xFFFFF000) + 0x1000)
#define PAGE_PHYS_ADDRESS(dir_entry) ((*dir_entry) & ~0xFFF)    // Clear lowest 12 bits, only return frame/address
#define SET_ATTRIBUTE(entry, attr) (*entry |= attr)
#define CLEAR_ATTRIBUTE(entry, attr) (*entry &= ~attr)
#define TEST_ATTRIBUTE(entry, attr) (*entry & attr)
#define SET_FRAME(entry, address) (*entry = (*entry & ~0x7FFFF000) | address)   // Only set address/frame, not flags
#define GET_FRAME(entry) (*entry & ~0x7FFFF000)   // Only set address/frame, not flags

typedef uint32_t PDEntry;
typedef uint32_t PTEntry;

typedef struct {
   PTEntry entries[1024];
} PageTable;

typedef struct {
   PDEntry entries[1024];
} PageDirectory;

static PageDirectory *page_directory = (PageDirectory*)PAGING_MEM_START;
static PageTable *last_page = (PageTable *)PAGING_MEM_START + 0x4000;

void paging_map(uint32_t virt, uint32_t phys);
uint32_t *paging_page_get(uint32_t virt);
void paging_init();

void paging_page_new(PTEntry *page);
