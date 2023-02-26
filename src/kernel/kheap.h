#pragma once
#include <stddef.h>
#include <stdint.h>

#define MAGIC_KHEAP (69420)

typedef struct kheap_header_t{
   uint32_t magic_number;
   uint32_t size;
   uint8_t freed;

   struct kheap_header_t* next;
   uint32_t zero;
} KheapHeader;

void* malloc(size_t size);
void free(void*);
