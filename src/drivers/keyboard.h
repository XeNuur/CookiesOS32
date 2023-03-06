#pragma once
#include <stdint.h>

void kb_init(uint8_t);
char kb_get_curr();

char kb_get();
char kb_wait_get();

