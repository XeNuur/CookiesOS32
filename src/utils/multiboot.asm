MODULEALIGN equ  1<<0					; align loaded modules on page boundaries
MEMINFO		equ  1<<1					; provide memory map
FLAGS		equ  MODULEALIGN | MEMINFO	; this is the Multiboot 'flag' field
MAGIC		equ  0x1BADB002				; 'magic number' lets bootloader find the header
CHECKSUM	equ  -(MAGIC + FLAGS)		; checksum required

section .multiboot_header
header_start:
	dd MAGIC
	dd FLAGS
	dd CHECKSUM
