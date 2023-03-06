[bits 32]

;void general_interrupt_handler(struct interrupt_frame*);
extern general_exception_handler_err
extern general_exception_handler

global load_idt
load_idt:
   ;New call frame
   push ebp
   mov ebp, esp
   
   ;Load our bug chungus
   mov eax, [ebp + 8]
   lidt [eax]

   ;Restore old call frame
   mov esp, ebp
   pop ebp

   ret

;Isr exeptions
%macro isr_noerr 1
isr%1:
    call general_exception_handler
    iret
%endmacro
%macro isr_err 1
isr%1:
    call general_exception_handler_err
    iret
%endmacro

isr_noerr 0
isr_noerr 1
isr_noerr 2
isr_noerr 3
isr_noerr 4
isr_noerr 5
isr_noerr 6
isr_noerr 7
isr_err 8
isr_noerr 9
isr_err 10
isr_err 11
isr_err 12
isr_err 13
isr_err 14
isr_noerr 15
isr_noerr 16
isr_noerr 17
isr_noerr 18
isr_noerr 19
isr_noerr 20
isr_noerr 21
isr_noerr 22
isr_noerr 23
isr_noerr 24
isr_noerr 25
isr_noerr 26
isr_noerr 27
isr_noerr 28
isr_noerr 29
isr_noerr 30
isr_noerr 31
%define isr_stub_count 32 

global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    isr_stub_count
    dd isr%+i ; use DQ instead if targeting 64-bit
%assign i i+1 
%endrep

