#ifndef RAM_H
#define RAM_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t acpi;
} __attribute__((packed)) E820_Entry;

#define E820_MEMMAP_PTR (0x7e00 + 1500 + 6)
#define E820_ENTRY_COUNT_PTR (0x7e00 + 1500 + 4)

void parse_e820_memory_map(void);
uint32_t get_ram_size(void);

extern E820_Entry ram_memmap[20];
extern uint8_t ram_memmap_count;
extern uint32_t ram_size;

#endif // RAM_H