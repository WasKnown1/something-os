#include <ram.h>
#include <alloc.h>
#include <log.h>

void alloc_init() {
    parse_e820_memory_map();

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

void *malloc(uint32_t size) {
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

void print_memory_allocations(void) {
    debug_printf("|-----------------------------------------------------------|\n");
    for (uint8_t i = 0; i < ram_memmap_count; i++) {
        EntryHeader *header = (EntryHeader *)((uint32_t)(ram_memmap[i].base));
        debug_printf("|Memory Block %d: Used: %d Free: %d First Memory Block: %p|\n", 
                     i, header->used_size, header->free_size, header->first_memory_block);
        MemoryBlock *current = header->first_memory_block;
        while (current != NULL) {
            debug_printf("|Allocated Block: Size: %d Address: %p Next: %p            |\n", 
                         current->size, current->data, current->next);
            current = current->next;
        }
    }
    debug_printf("|-----------------------------------------------------------|\n");
}