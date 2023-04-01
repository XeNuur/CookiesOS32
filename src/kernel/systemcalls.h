#pragma once
#define SYSCALLS_COUNT (2)
#include "framebuffer.h"

const int syscall_cnt = SYSCALLS_COUNT;

void print(char** str) {
   term_writestring(*str);
}

void syscall_2(uint32_t* number) {
   uint32_t data = *number;
   term_printf("Number: %x\n", data);
}

void *syscalls[SYSCALLS_COUNT] = {
   print,
   syscall_2,
};

void syscall_dispatch(void);
