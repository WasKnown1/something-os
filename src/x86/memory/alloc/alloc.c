#include <ram.h>
#include <alloc.h>

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
        .last_freed_block_address = NULL,
        .last_freed_block_size = 0,
        .last_allocated_block_address = NULL,
        .last_allocated_block_size = 0
    };
    *((EntryHeader *)((uint32_t)(ram_memmap[0].base))) = first_header;
}

void *malloc(uint32_t size) {
    for (uint8_t i = 0; i < ram_memmap_count; i++) {
        EntryHeader *header = (EntryHeader *)((uint32_t)(ram_memmap[i].base));
        if (header->free_size >= size + sizeof(MemoryBlock)) {
            // found a block with enough free space
            void *alloc_address = (void *)(uint32_t)(ram_memmap[i].base + sizeof(EntryHeader) + header->used_size);
            MemoryBlock block = {
                .size = size,
                .data = alloc_address + sizeof(MemoryBlock)
            };
            *((MemoryBlock *)alloc_address) = block;

            // update header
            header->used_size += size + sizeof(MemoryBlock);
            header->free_size -= size + sizeof(MemoryBlock);
            header->last_allocated_block_address = alloc_address;
            header->last_allocated_block_size = size + sizeof(MemoryBlock);

            return block.data;
        }   
    }
    return NULL; // no suitable block found
}

void free(void *ptr) {
    for (uint8_t i = 0; i < ram_memmap_count; i++) {
        EntryHeader *header = (EntryHeader *)((uint32_t)(ram_memmap[i].base));
        uint32_t block_start = (uint32_t)(ram_memmap[i].base + sizeof(EntryHeader));
        uint32_t block_end = block_start + header->used_size;

        if ((uint32_t)ptr > block_start && (uint32_t)ptr < block_end) {
            // pointer belongs to this block
            MemoryBlock *block = (MemoryBlock *)((uint32_t)ptr - sizeof(MemoryBlock));

            // update header
            header->used_size -= block->size + sizeof(MemoryBlock);
            header->free_size += block->size + sizeof(MemoryBlock);
            header->last_freed_block_address = (void *)block;
            header->last_freed_block_size = block->size + sizeof(MemoryBlock);

            return;
        }
    }
}