#pragma once
#include <stdint.h>

#define FRAME_BLOCKS_PER_BYTE    (8)
#define FRAME_BLOCK_SIZE	(4096)

void frame_set(uint32_t);
void frame_clear(uint32_t);
int frame_test(uint32_t);

uint32_t frame_frist_free(uint32_t blocks_num);
void frame_init(uint32_t, uint32_t);

uint32_t frame_alloc_ex(uint32_t);
#define frame_alloc() frame_alloc_ex(1) //ik...ik... but its for styling purposes
void frame_free_ex(uint32_t, uint32_t);
#define frame_free(addr) frame_free_ex(addr, 1)

uint32_t frame_count();
uint32_t frame_max();
