#ifndef RAM_H
#define RAM_H

#include <stdint.h>

typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t acpi;
} __attribute__((packed)) E820_ENTRY;

#define E820_MEMMAP_PTR (0x7e00 + 1500 + 2)
#define E820_ENTRY_COUNT_PTR (0x7e00 + 1500)

void parse_e820_memory_map(void);

#endif // RAM_H