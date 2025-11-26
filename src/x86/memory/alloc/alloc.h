#ifndef ALLOC_H
#define ALLOC_H

#include <stdint.h>

typedef struct MemoryBlock {
    uint32_t size;
    void *data;
    struct MemoryBlock *next; // linked list type structure
} MemoryBlock; // this struct will be placed at the start of each allocated block

typedef struct EntryHeader {
    uint32_t used_size;
    uint32_t free_size;
    MemoryBlock *first_memory_block;
} EntryHeader; // this struct will be placed at the start of each ram_memmap block and will track allocations within that block

void alloc_init();
void *malloc(uint32_t size);
void free(void *ptr);
void print_memory_allocations(void);

#endif // ALLOC_H