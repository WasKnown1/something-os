#include <ram.h>
#include <log.h>

E820_Entry ram_memmap[20];
uint8_t ram_memmap_count = 0;

void parse_e820_memory_map(void) {
    E820_Entry *map = (E820_Entry *)E820_MEMMAP_PTR;
    uint8_t entry_count = *(uint8_t *)E820_ENTRY_COUNT_PTR;

    debug_printf("E820 Memory Map Entries: %d\n", entry_count);
    for (uint8_t i = 1; i < entry_count; i++) { // trim first entry which is bogus
        if (map[i].type == 1) {
            debug_printf("Entry %d: Base: 0x%x Length: 0x%x Type: %d (Usable RAM)\n", 
                         i, (unsigned int)map[i].base, (unsigned int)map[i].length, map[i].type);
            ram_memmap[ram_memmap_count++] = map[i];
        } else {
            debug_printf("Entry %d: Base: 0x%x Length: 0x%x Type: %d\n", 
                         i, (unsigned int)map[i].base, (unsigned int)map[i].length, map[i].type);
        }
    }
}