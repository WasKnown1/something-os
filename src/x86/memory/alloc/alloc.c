#include <ram.h>
#include <alloc.h>
#include <log.h>
#include <string.h>
#include <stdio.h>
#include <paging.h>

void alloc_init() {
    parse_e820_memory_map();
    get_ram_size();

    if (ram_memmap_count == 0) { // no usable RAM found
        return;
    }

    if (ram_memmap[0].base != 0x100000) { // first usable RAM block should start at 1MB
        return;
    }

    EntryHeader first_header = {
        .used_size = 0,
        .free_size = (uint32_t)(ram_memmap[0].length),
        .first_memory_block = NULL
    };
    *((EntryHeader *)((uint32_t)(ram_memmap[0].base))) = first_header;
}

void *alloc(uint32_t size) {
    for (uint8_t i = 0; i < ram_memmap_count; i++) {
        EntryHeader *header = (EntryHeader *)((uint32_t)(ram_memmap[i].base));
        if (header->first_memory_block == NULL) {
            if (header->free_size >= size + sizeof(MemoryBlock)) {
                MemoryBlock *block = (MemoryBlock *)((uint32_t)(ram_memmap[i].base) + sizeof(EntryHeader));
                block->size = size;
                block->data = (void *)((uint32_t)block + sizeof(MemoryBlock));
                block->next = NULL;
                header->first_memory_block = block;
                header->used_size += size + sizeof(MemoryBlock);
                header->free_size -= size + sizeof(MemoryBlock);
                return block->data;
            }
        } else {
            MemoryBlock *current = header->first_memory_block;
            while (current->next != NULL) {
                if ((uint32_t)current->data + current->size + sizeof(MemoryBlock) + size <=
                    (uint32_t)(current->next->data)) {
                    MemoryBlock *block = (MemoryBlock *)((uint32_t)current->data + current->size);
                    block->size = size;
                    block->data = (void *)((uint32_t)block + sizeof(MemoryBlock));
                    block->next = current->next;
                    current->next = block;
                    header->used_size += size + sizeof(MemoryBlock);
                    header->free_size -= size + sizeof(MemoryBlock);
                    return block->data;
                }
                current = current->next;
            }
            uint32_t block_end = (uint32_t)current->data + current->size;
            if ((block_end + sizeof(MemoryBlock) + size) <= 
                ((uint32_t)(ram_memmap[i].base) + (uint32_t)(ram_memmap[i].length))) {
                MemoryBlock *block = (MemoryBlock *)(block_end);
                block->size = size;
                block->data = (void *)((uint32_t)block + sizeof(MemoryBlock));
                block->next = NULL;
                current->next = block;
                header->used_size += size + sizeof(MemoryBlock);
                header->free_size -= size + sizeof(MemoryBlock);
                return block->data;
            }
        }
    }
    return NULL; // no suitable block found
}

void *malloc(uint32_t size) {
    void *ptr = alloc(size);
    uint32_t start = (uint32_t)ptr & ~0x3FFFFF;  //round down to 4MB
    uint32_t end = ((uint32_t)ptr + size + 0x3FFFFF) & ~0x3FFFFF;  //round up
    
    for (uint32_t addr = start; addr < end; addr += 0x400000)
        map_identity_4mb(addr, addr);
    
    return ptr;
}

void free(void *ptr) {
    for (uint8_t i = 0; i < ram_memmap_count; i++) {
        EntryHeader *header = (EntryHeader *)((uint32_t)(ram_memmap[i].base));
        MemoryBlock *current = header->first_memory_block;
        MemoryBlock *prev = NULL;
        while (current != NULL) {
            if (current->data == ptr) {
                if (prev == NULL) {
                    header->first_memory_block = current->next;
                } else {
                    prev->next = current->next;
                }
                header->used_size -= current->size + sizeof(MemoryBlock);
                header->free_size += current->size + sizeof(MemoryBlock);
                return;
            }
            prev = current;
            current = current->next;
        }
    }
}

void *realloc(void *ptr, uint32_t new_size) {
    free(ptr);
    void *new_ptr = malloc(new_size);
    if (new_ptr != NULL) {
        return new_ptr;
    }
    return NULL;
}

void *save_realloc(void *ptr, uint32_t old_size, uint32_t new_size) {
    void* new_ptr = malloc(new_size);
    if (new_ptr != NULL) {
        memcpy(new_ptr, ptr, old_size);
    }
    free(ptr);
    return new_ptr;
}

void print_memory_allocations(void) {
    char buf[95] = {0};
    memset(buf, '-', sizeof(buf) - 1);
    buf[0] = '|';
    buf[sizeof(buf) - 3] = '|';
    buf[sizeof(buf) - 2] = '\n';
    buf[sizeof(buf) - 1] = '\0';
    debug_log(buf);
    for (uint8_t i = 0; i < ram_memmap_count; i++) {
        EntryHeader *header = (EntryHeader *)((uint32_t)(ram_memmap[i].base));
        memset(buf, ' ', sizeof(buf));
        snprintf(buf, sizeof(buf), "|Memory Block %d: Used: %d Free: %d First Memory Block: %p", 
                     i, header->used_size, header->free_size, header->first_memory_block);
        for (int j = strlen(buf); j < sizeof(buf); j++)
            buf[j] = ' ';
        buf[sizeof(buf) - 3] = '|';        
        buf[sizeof(buf) - 2] = '\n';
        buf[sizeof(buf) - 1] = '\0';
        debug_log(buf);
        MemoryBlock *current = header->first_memory_block;
        while (current != NULL) {
            memset(buf, ' ', sizeof(buf));
            snprintf(buf, sizeof(buf), "|  Allocated Block: Size: %d Address: %p Next: %p", 
                         current->size, current->data, current->next);
            for (int j = strlen(buf); j < sizeof(buf); j++)
                buf[j] = ' ';
            buf[sizeof(buf) - 3] = '|';
            buf[sizeof(buf) - 2] = '\n';
            buf[sizeof(buf) - 1] = '\0';
            debug_log(buf);
            current = current->next;
        }
    }
    memset(buf, '-', sizeof(buf) - 1);
    buf[0] = '|';
    buf[sizeof(buf) - 3] = '|';
    buf[sizeof(buf) - 2] = '\n';
    buf[sizeof(buf) - 1] = '\0';
    debug_log(buf);
}

uint32_t get_largest_entry_ram_size(void) {
    uint32_t max_entry_ram_size = 0;
    for (size_t i = 0; i < ram_memmap_count; i++)
        if ((uint32_t)ram_memmap[i].length > max_entry_ram_size)
            max_entry_ram_size = (uint32_t)ram_memmap[i].length;
    return max_entry_ram_size;
}