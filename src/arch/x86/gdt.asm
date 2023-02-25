global gdt_init 

CODE_SEG equ GDT_code - GDT_start
DATA_SEG equ GDT_data - GDT_start

gdt_init:
   lgdt [GDT_descriptor]
   jmp CODE_SEG:.continue

.continue:
   mov ax, DATA_SEG ; offset of data segment
   mov ds, ax
   mov es, ax
   mov fs, ax
   mov gs, ax
   mov ss, ax

   ret

GDT_start:                          ; must be at the end of real mode code
    GDT_null:
        dd 0x0
        dd 0x0

    GDT_code:
        dw 0xffff
        dw 0x0
        db 0x0
        db 0b10011010
        db 0b11001111
        db 0x0

    GDT_data:
        dw 0xffff
        dw 0x0
        db 0x0
        db 0b10010010
        db 0b11001111
        db 0x0

GDT_end:

GDT_descriptor:
    dw GDT_end - GDT_start - 1
    dd GDT_start

