#pragma once
#define SYSCALLS_COUNT (11)
#include <string.h>
#include "framebuffer.h"
#include "kheap.h"
#include "panic.h"
#include "../fs/vfs.h"

const int syscall_cnt = SYSCALLS_COUNT;
int retval = 0;

typedef struct syscall_regs_t{
    uint32_t esp;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
    uint32_t eax;
} __attribute__ ((packed)) SyscallRegs_t;

/* eax 0: dbg_print: 
   ebx: char* str
*/
void sc_print(SyscallRegs_t regs) {
   term_writestring((char*)regs.ebx);
}

/* eax 1: open: 
   ebx: char* path
   ecx: Vfs_t** target
*/
int sc_open(SyscallRegs_t regs) {
   Vfs_t* node = fopen((char*)regs.ebx);
   *(Vfs_t**)(regs.ecx) = node;

   if(!node)
      return 0; 
   return 1;
}

/* eax 2: close: 
   ebx: Vfs* node 
*/
int sc_close(SyscallRegs_t regs) {
   Vfs_t* node = (Vfs_t*)regs.ebx;
   return fclose(node);
}

/* eax 3: read: 
   ebx: Vfs* node,
   ecx: u32 size,
   edx: char* buffer 
*/
int sc_read(SyscallRegs_t regs) {
   Vfs_t* node = (Vfs_t*)regs.ebx;
   uint32_t size = regs.ecx;
   char* buffer = (char*)regs.edx;

   return vread(node, 0, size, buffer);
}

/* eax 4: write: 
   ebx: Vfs* node
   ecx: u32 size
   edx: char* buffer
*/
int sc_write(SyscallRegs_t regs) {
   Vfs_t* node = (Vfs_t*)regs.ebx;
   uint32_t size = regs.ecx;
   char* buffer = (char*)regs.edx;

   return vwrite(node, 0, size, buffer);
}

/* eax 5: readdir: 
   ebx: Vfs* node,
   ecx: u32 index
   edx: char* outname 
   esi: u32 outsize
*/
int sc_readdir(SyscallRegs_t regs) {
   Vfs_t* node = (Vfs_t*)regs.ebx;
   uint32_t index = regs.ecx;
   char* outname = (char*)regs.edx;
   uint32_t outsize = regs.esi;

   VfsDirent_t dirent;
   vread_dir(node, index, &dirent);
   memcpy(dirent.name, outname, outsize);
   return 0;
}

//TODO Fix
/* eax 6: finddir
   ebx: Vfs* node
   ecx: char* name
   edx: Vfs_t** outname 
*/
int sc_finddir(SyscallRegs_t regs) {
   Vfs_t* node = (Vfs_t*)regs.ebx;
   char* name = (char*)regs.ecx;
   Vfs_t** target = (Vfs_t**)regs.edx;

   *target = malloc(sizeof(Vfs_t));
   vfind_dir(node, name, *target);
   vopen(*target);
   return 0;
}

/* eax 7: kmalloc: 
   ebx: size_t bytes 
   ecx: void** address
*/
int sc_kmalloc(SyscallRegs_t regs) {
   size_t size = regs.ebx;
   void** ptr = (void**)regs.ecx;

   (*ptr) = malloc(size);
   return 0;
}

/* ext 8: kfree: 
   ebx: void* pointer_to_free
*/
int sc_kfree(SyscallRegs_t regs) {
   void* ptr = (void*)regs.ebx;
   free(ptr);
   return 0;
}

/* ext 9: krealloc: 
   ebx: size_t new_size
   ecx: void* pointer_to_resize,
   edx: void** return_address
*/
int sc_realloc(SyscallRegs_t regs) {
   size_t size = regs.ebx;
   void*  oldptr = (void*)regs.ecx;
   void** newptr = (void**)regs.edx;

   (*newptr) = realloc(oldptr, size);
   return 0;
}

/* ext 10: sysc_status
 * ebx: int* staus_of_last_syscall
*/
int sc_sysc_status(SyscallRegs_t regs) {
   int* laststat = (int*)regs.ebx;
   *laststat = retval;
   return 0;
}


void *syscalls[SYSCALLS_COUNT] = {
   sc_print,

   sc_open,
   sc_close,
   sc_read,
   sc_write,

   sc_readdir,
   sc_finddir,

   sc_kmalloc,
   sc_kfree,
   sc_realloc,

   sc_sysc_status,
};

void syscall_dispatch(void);

