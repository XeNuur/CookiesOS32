#include "frame.h"
#include "panic.h"
#include <string.h>

static uint32_t *frames = 0;
static uint32_t frames_max = 0;

uint32_t alloced_frames = 0;

uint32_t frame_count() {
   return alloced_frames;
}

uint32_t frame_max() {
   return frames_max; 
}

void frame_set(uint32_t i) {
   frames[i/ 32] |= (1 << (i%32));  
}

void frame_clear(uint32_t i) {
   frames[i/ 32] &= ~(1 << (i%32));  
}

int frame_test(uint32_t i) {
   return frames[i/ 32] & (1 << (i%32));  
}

uint32_t frame_frist_free(uint32_t blocks_num) {
   uint32_t cnt = 0;
   uint32_t bi=0, bj=0; 

   for (uint32_t i = 0; i < frames_max / 32; i++) {
       if (frames[i] != 0xFFFFFFFF) { // nothing free, exit early.
           for (uint32_t j = 0; j < 32; j++) { // at least one bit is free here.
               uint32_t toTest = 0x1 << j;
               if ( !(frames[i]&toTest) ) {
                  if(!cnt)
                     bi=i; bj=j;
                  ++cnt;
                  if(cnt >= blocks_num)
                     return bi*32+bj;
                  continue;
               }
               cnt = 0;
           }
       }
   }
   return -1;
}

uint32_t frame_alloc_ex(uint32_t blocks_num) {
   uint32_t idx = frame_frist_free(blocks_num);
   if(idx == -1)
      panic("Out of frames:\n Allocated frames: %x\n Max frames: %x", alloced_frames, frames_max);

   for(uint32_t i=0; i<blocks_num; i++)
      frame_set(idx+i);
   uint32_t phys_addr = idx * FRAME_BLOCK_SIZE;

   alloced_frames+=blocks_num;
   return phys_addr;
}

void frame_free_ex(uint32_t addr, uint32_t blocks_num) {
   uint32_t index = addr / FRAME_BLOCK_SIZE;
   for(uint32_t i=0; i<blocks_num; i++)
      frame_clear(index);
   alloced_frames-=blocks_num;
}

void frame_init(uint32_t start_addr, uint32_t end_addr) {
   frames = (uint32_t*)start_addr;
   frames_max = end_addr/ FRAME_BLOCK_SIZE;
   
   memset(frames, 0x0, frames_max / FRAME_BLOCKS_PER_BYTE);
}

