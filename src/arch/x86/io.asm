bits 32

global x86_outb
x86_outb:
   mov dx, [esp + 4]
   mov al, [esp + 8]
   out dx, al
   ret

global x86_inb
x86_inb:
   mov dx, [esp + 4]
   xor eax, eax
   in al, dx
   ret

global x86_io_wait
x86_io_wait:
   push 0x80 ;An unused port 
   push 0
   call x86_outb
   ret

global x86_int_on
x86_int_on:
   sti
   ret

global x86_int_off
x86_int_off:
   cli
   ret

