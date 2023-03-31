#pragma once
#define SYSCALLS_COUNT (2)
#include "framebuffer.h"

const int syscall_cnt = SYSCALLS_COUNT;

void syscall_1(void) {
   term_writestring("syscall nr.1\n");
}

void syscall_2(void) {
   term_writestring("syscall nr.2\n");
}

void *syscalls[SYSCALLS_COUNT] = {
   syscall_1,
   syscall_2,
};

void syscall_dispatch(void);
