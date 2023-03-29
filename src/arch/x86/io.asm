bits 32
extern x86_outb

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

