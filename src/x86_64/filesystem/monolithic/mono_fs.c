#include <mono_fs.h>
#include <log.h>
#include <paging.h>
#include <ram.h>
#include <string.h>
#include <alloc.h>

void *mono_fs_address;

void mono_fs_init(void) {
    mono_fs_address = (void *)((uint32_t)ram_memmap[0].base + (uint32_t)ram_memmap[0].length);
    map_identity_4mb((uint32_t)mono_fs_address, 0x4000000); // map 64MB after ram for mono fs access

    debug_printf("Initializing mono filesystem... at %p\n", MONO_FS_START_ADDRESS);
    FsHeader *fs_header = (FsHeader *)MONO_FS_START_ADDRESS;
    debug_printf("signiture = %d, %d\n", fs_header->signiture, MONO_FS_START_SIGNITURE);

    if (fs_header->signiture == MONO_FS_START_SIGNITURE) {
        debug_printf("found the filesystem!\n");
    }

    debug_printf("filesystem start size = %d\n", fs_header->size);

    uint32_t offset = sizeof(FsHeader);
    
    while (offset < fs_header->size) {
        FileHeader *file_header = (FileHeader *)(MONO_FS_START_ADDRESS + offset);
        
        debug_printf("file header: is_folder=%d, size=%d, name_len=%d\n", 
                     file_header->is_folder, 
                     file_header->size, 
                     file_header->file_name_length);
        
        char *filename_ptr = (char *)file_header + sizeof(FileHeader);
        char *file_name = malloc(file_header->file_name_length + 1);
        memcpy(file_name, filename_ptr, file_header->file_name_length);
        debug_printf("  name: %s\n", file_name);
        free(file_name);
        offset += sizeof(FileHeader) + file_header->file_name_length + file_header->size;
    }
}