#include <mono_fs.h>
#include <log.h>
#include <paging.h>
#include <ram.h>

uint8_t *file_name;   // will be allocated dynamically
uint8_t *extra_field; // will be allocated dynamically

void mono_fs_init(void) {
    map_identity_4mb((uint32_t)ram_memmap[0].base + (uint32_t)ram_memmap[0].length, 0x4000000); // map 64MB after ram for mono fs access

    debug_printf("Initializing mono filesystem... at %p\n", MONO_FS_START_ADDRESS);
    ZipHeader *zip_header = (ZipHeader *)(uint32_t)(MONO_FS_START_ADDRESS);
    if (zip_header->signature[0] != 0x50 ||
        zip_header->signature[1] != 0x4b ||
        zip_header->signature[2] != 0x03 ||
        zip_header->signature[3] != 0x04) {
        debug_printf("Unable to find mono filesystem signature!\n");
        char sig[4] = {
            zip_header->signature[0],
            zip_header->signature[1],
            zip_header->signature[2],
            zip_header->signature[3]
        };
        debug_printf("Found signature: %d %d %d %d\n",
                     sig[0], sig[1], sig[2], sig[3]);
        return;
    }
}