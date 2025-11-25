#ifndef ALLOC_H
#define ALLOC_H

#include <stdint.h>

typedef struct MemoryBlock {
    uint32_t size;
    void *data;
} MemoryBlock; // this struct will be placed at the start of each allocated block

typedef struct EntryHeader {
    uint32_t used_size;
    uint32_t free_size;
    void *last_freed_block_address;
    uint32_t last_freed_block_size;
    void *last_allocated_block_address;
    uint32_t last_allocated_block_size;
} EntryHeader; // this struct will be placed at the start of each ram_memmap block and will track allocations within that block

void alloc_init();

void *malloc(uint32_t size);
void free(void *ptr);

#endif // ALLOC_H