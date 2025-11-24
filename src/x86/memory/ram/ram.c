#include <ram.h>
#include <log.h>

void parse_e820_memory_map(void) {
    E820_ENTRY *map = (E820_ENTRY *)E820_MEMMAP_PTR;
    uint8_t entry_count = *(uint8_t *)E820_ENTRY_COUNT_PTR;

    debug_printf("E820 Memory Map Entries: %d\n", entry_count);
    for (uint8_t i = 0; i < entry_count; i++) {
        if (map[i].type == 1) {
            debug_printf("Entry %d: Base: 0x%x Length: 0x%x Type: %d (Usable RAM)\n", 
                         i, (unsigned int)map[i].base, (unsigned int)map[i].length, map[i].type);
        } else {
            debug_printf("Entry %d: Base: 0x%x Length: 0x%x Type: %d\n", 
                         i, (unsigned int)map[i].base, (unsigned int)map[i].length, map[i].type);
        }
    }
}