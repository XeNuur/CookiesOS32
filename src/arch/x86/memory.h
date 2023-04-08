#pragma once
#include <stdint.h>

void __attribute__((cdecl)) load_page_directory(uint32_t);
void __attribute__((cdecl)) enable_paging(void);
uint32_t __attribute__((cdecl)) read_cr2();

