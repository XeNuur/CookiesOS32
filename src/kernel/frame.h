#pragma once
#include <stdint.h>

#define MEM_END_ADR (0x1000000) // Let's assume that pc that we are currently running have 16mb

static uint32_t *frames = 0;
static uint32_t frames_size = 0;

void frame_set(uint32_t);
void frame_clear(uint32_t);
int frame_test(uint32_t);

uint32_t frame_frist_free();

uint32_t frame_alloc();
void frame_free(uint32_t);
void frame_init();

