#pragma once
#include <stddef.h>
#include <stdint.h>

#define MAGIC_KHEAP (69420)

typedef struct kheap_header_t{
   uint32_t magic_number;
   uint32_t size;
   int freed;

   struct kheap_header_t* next;
} KheapHeader;

void* malloc(size_t size);
void* realloc(void*, size_t size);
void free(void*);

