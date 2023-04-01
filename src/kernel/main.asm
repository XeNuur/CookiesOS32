global start

extern panic 
extern kernel_main

section .text
bits 32
start:
        push ebx
        call kernel_main
        ;check if kernel_main() returned error code
        cmp eax, 0
        jz halt
        push krnl_exit_code
        call panic

        halt:
            jmp halt
        cli
        hlt

global syscalltest_prog
syscalltest_prog:
   mov eax, 0
   mov ebx, krnl_exit_code
   int 0x80

   mov eax, 1
   mov ebx, 9
   int 0x80

   ret

krnl_exit_code:
   db "kernel_main() returned an error!", 0
