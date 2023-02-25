#include "frame.h"
#include "panic.h"

extern uint32_t krnl_end;

//macros for bitset algorythm
#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

void _frame_get_(uint32_t addr, uint32_t* idx, uint32_t* off) {
   uint32_t frame = addr/0x1000;
   *idx = INDEX_FROM_BIT(frame);
   *off = OFFSET_FROM_BIT(frame);
}

void frame_set(uint32_t addr) {
   uint32_t idx = 0, off = 0;
   _frame_get_(addr, &idx, &off);
   frames[idx] |= (0x1 << off);  
}

void frame_clear(uint32_t addr) {
   uint32_t idx = 0, off = 0;
   _frame_get_(addr, &idx, &off);
   frames[idx] &= ~(0x1 << off);
}

int frame_test(uint32_t addr) {
   uint32_t idx = 0, off = 0;
   _frame_get_(addr, &idx, &off);
   return (frames[idx] & (0x1 << off));
}

uint32_t frame_frist_free() {
   uint32_t i, j;
   for (i = 0; i < INDEX_FROM_BIT(frames_size); i++) {
       if (frames[i] != 0xFFFFFFFF) { // nothing free, exit early.
           for (j = 0; j < 32; j++) { // at least one bit is free here.
               uint32_t toTest = 0x1 << j;
               if ( !(frames[i]&toTest) ) 
                   return i*32+j;
           }
       }
   }
}

uint32_t frame_alloc() {
   uint32_t idx = frame_frist_free();
   frame_set(idx*0x1000);
   if(idx == (uint32_t)-1) {
      panic("Out of frames");
   }
   return idx;
}

void frame_free(uint32_t addr) {
   frame_clear(addr);
}

void frame_init() {
   frames_size = MEM_END_ADR/0x1000;
   frames = (uint32_t*)(krnl_end + INDEX_FROM_BIT(frames_size));
}

