#pragma once
#include <stdint.h>

#define FRAME_BLOCKS_PER_BYTE    (8)
#define FRAME_BLOCK_SIZE	(4096)

#define MEM_END_ADR (0x500000) // Let's assume that pc that we are currently running have 16mb
#define MEM_START_ADR (0x1000) // Let's assume that pc that we are currently running have 16mb

static uint32_t *frames = 0;
static uint32_t frames_max = 0;
static uint32_t frames_size = 0;

void frame_set(uint32_t);
void frame_clear(uint32_t);
int frame_test(uint32_t);

uint32_t frame_frist_free(uint32_t blocks_num);
void frame_init();

uint32_t frame_alloc_ex(uint32_t);
#define frame_alloc() frame_alloc_ex(1) //ik...ik... but its for styling purposes
void frame_free_ex(uint32_t, uint32_t);
#define frame_free() frame_free_ex(1)

