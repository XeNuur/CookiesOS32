#pragma once
#include <stdint.h>
#include "../fs/vfs.h"

uint32_t kb_init(uint8_t);
char kb_get_curr();

char kb_get();
char kb_wait_get();

int keyboard_read_callback (Vfs_t*, uint32_t, uint32_t, char* );
