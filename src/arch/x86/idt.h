#pragma once
#include <stdint.h>

#define TRAP_GATE_FLAGS     0x8F    // P = 1, DPL = 00, S = 0, Type = 1111 (32bit trap gate)
#define INT_GATE_FLAGS      0x8E    // P = 1, DPL = 00, S = 0, Type = 1110 (32bit interrupt gate)
#define INT_GATE_USER_FLAGS 0xEE    // P = 1, DPL = 11, S = 0, Type = 1110 (32bit interrupt gate, called from PL 3)

#define _INT_BEGIN __asm__("pusha");
#define _INT_END __asm__("popa; leave; iret"); 
                                   
//idt entry
typedef struct{
	uint16_t    isr_low;      // The lower 16 bits of the ISR's address
	uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t     reserved;     // Set to zero
	uint8_t     attributes;   // Type and attributes; see the IDT page
	uint16_t    isr_high;     // The higher 16 bits of the ISR's address
} __attribute__((packed)) IdtEntry_t;

//idt layout
typedef struct {
	uint16_t	limit;
	uint32_t	base;
} __attribute__((packed)) IdtR_t;

IdtEntry_t the_idt[256];
IdtR_t idtr;

void __attribute__((cdecl)) load_idt(IdtR_t*);

void set_idt_gate(int index, void* isr, uint8_t flags);
void idt_init();

//These are for traps & exceptios
typedef struct {
    unsigned int gs, fs, es, ds;      /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    unsigned int int_no, err_code;    /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
} Registers;

