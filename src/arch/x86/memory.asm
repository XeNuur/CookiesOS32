;int get_next_block(MemoryBlock *blk, uint32_t *cnt_id);
global get_next_block
get_next_block:
   ret

;void __attribute__((cdecl)) load_page_directory(uint32_t*);
global load_page_directory
load_page_directory:
   ;load our page dir into cr3 reg
   mov eax, [esp + 4]
   mov cr3, eax

   mov eax, cr4
   or eax, 0x00000010
   mov cr4, eax

   mov ebx, cr0        ; read current cr0
   or  ebx, 0x80000001 ; set PG
   mov cr0, ebx        ; update cr0
   ret

;uint32_t __attribute__((cdecl)) read_cr2();
global read_cr2
read_cr2:
   mov eax, cr2
   ret
