global start

extern panic 
extern kernel_main

section .text
bits 32
start:
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

krnl_exit_code:
   db "kernel_main() returned an error!", 0
