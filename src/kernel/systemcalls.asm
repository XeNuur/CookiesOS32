global syscall_dispatch

extern syscalls
extern syscall_cnt;
extern retval

syscall_dispatch:
   ;Basic syscall system 
   ; already on stack: ss, sp, flags, cs, ip.
   ; need to push ax, gs, fs, es, ds, -ENOSYS, bp, di, si, dx, cx, and bx
   cmp eax, syscall_cnt-1
   ja .bad_opcode

   push eax
   push gs
   push fs
   push es
   push ds
   push ebp
   push edi
   push esi
   push edx
   push ecx
   push ebx
   push esp

   ;Make a system call
   call [syscalls+eax*4]
   mov [retval], eax
   add esp, 4

   pop ebx
   pop ecx
   pop edx
   pop esi
   pop edi
   pop ebp
   pop ds
   pop es
   pop fs
   pop gs

   add esp, 4
   iretd
.bad_opcode:
   iretd

