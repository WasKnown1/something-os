#ifndef ALLOC_H
#define ALLOC_H

#include <stdint.h>
#include <stddef.h>

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
void *realloc(void *ptr, uint32_t new_size);
void *save_realloc(void *ptr, uint32_t old_size, uint32_t new_size);
void *aligned_alloc(size_t alignment, size_t size);
void print_memory_allocations(void);
uint32_t get_largest_entry_ram_size(void);

#define DynamicArray(type) struct { \
    type *data; \
    size_t size; \
    size_t capacity; \
}
#define DYNAMIC_ARRAY_INIT { NULL, 0, 0 }
#define dynamic_array_push(type, arr, value) do { \
    if ((arr).size >= (arr).capacity) { \
        size_t new_capacity = (arr).capacity == 0 ? 4 : (arr).capacity * 2; \
        type *new_data = (type *)realloc((arr).data, new_capacity * sizeof(type)); \
        if (new_data) { \
            (arr).data = new_data; \
            (arr).capacity = new_capacity; \
        } \
    } \
    if ((arr).size < (arr).capacity) { \
        (arr).data[(arr).size++] = (value); \
    } \
} while(0)
#define dynamic_array_free(arr) do { \
    free((arr).data); \
    (arr).data = NULL; \
    (arr).size = 0; \
    (arr).capacity = 0; \
} while(0)
#define dynamic_array_get(arr, index) ((arr).data[(index)])
#define dynamic_array_size(arr) ((arr).size)
// #define DynamicArrayCapacity(arr) ((arr).capacity)
#define dynamic_array_print(arr, print_func) do { \
    for (size_t i = 0; i < (arr).size; i++) { \
        print_func((arr).data[i]); \
    } \
} while(0)

#endif // ALLOC_H