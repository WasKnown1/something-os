#include <mono_fs.h>
#include <log.h>
#include <paging.h>
#include <ram.h>
#include <string.h>
#include <alloc.h>

void *mono_fs_address;
FileHeader **dynamicly_allocated_file = NULL;
FsHeader *fs_header;

void mono_fs_init(void)
{
    mono_fs_address = (void *)((uint32_t)ram_memmap[0].base + (uint32_t)ram_memmap[0].length);
    map_identity_4mb((uint32_t)mono_fs_address, 0x4000000); // map 64MB after ram for mono fs access

    debug_printf("Initializing mono filesystem... at %p\n", MONO_FS_START_ADDRESS);
    fs_header = (FsHeader *)MONO_FS_START_ADDRESS;
    debug_printf("signiture = %d, %d\n", fs_header->signiture, MONO_FS_START_SIGNITURE);

    if (fs_header->signiture != MONO_FS_START_SIGNITURE) {
        debug_printf("didnt find the filesystem!\n");
        return;
    }

    memcpy(mono_fs_address, (char *)MONO_FS_START_ADDRESS, fs_header->size);
    fs_header = (FsHeader *)mono_fs_address;

    debug_printf("filesystem start size = %d\n", fs_header->size);

    uint32_t offset = sizeof(FsHeader);
    
    while (offset < fs_header->size) {
        FileHeader *file_header = (FileHeader *)(MONO_FS_START_ADDRESS + offset);
        
        debug_printf("file header: is_folder=%d, size=%d, name_len=%d\n", 
                     file_header->is_folder, 
                     file_header->size, 
                     file_header->file_name_length - 1);
        
        char *filename_ptr = (char *)file_header + sizeof(FileHeader);
        debug_log("file name: ");
        debug_log_n(filename_ptr, file_header->file_name_length);
        debug_log("\n");

        char *file_content_ptr = (char *)file_header + sizeof(FileHeader) + file_header->file_name_length;
        debug_log("file content: ");
        debug_log_n(file_content_ptr, file_header->size - sizeof(FileEndHeader));
        debug_log("\n");
        offset += sizeof(FileHeader) + file_header->file_name_length + file_header->size;
    }
}

void *get_file_handle(const char *filename) { // maybe i will change this to return a handle in the future like in windows
    uint32_t offset = sizeof(FsHeader);

    while (offset < fs_header->size){
        FileHeader *file_header = (FileHeader *)(MONO_FS_START_ADDRESS + offset);

        char *filename_ptr = (char *)file_header + sizeof(FileHeader);
        char *filename_buffer = malloc(file_header->file_name_length);
        memcpy(filename_buffer, filename_ptr, file_header->file_name_length);
        if (strcmp(filename_buffer, filename) == 0)
            return (void *)file_header;
        offset += sizeof(FileHeader) + file_header->file_name_length + file_header->size;
    }

    return NULL; // file not found
}