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

FILE *get_file(const char *filename, const char *restrict mode) { // maybe i will change this to return a handle in the future like in windows
    uint32_t allocated_file_size = get_ram_size() / 100;
    if (get_largest_entry_ram_size() > allocated_file_size + sizeof(EntryHeader) + sizeof(MemoryBlock)) {

        uint32_t offset = sizeof(FsHeader);

        while (offset < fs_header->size){
            FileHeader *file_header = (FileHeader *)(MONO_FS_START_ADDRESS + offset);
            void *allocated_file = malloc(allocated_file_size);

            char *filename_ptr = (char *)file_header + sizeof(FileHeader);
            char *filename_buffer = malloc(file_header->file_name_length);
            memcpy(filename_buffer, filename_ptr, file_header->file_name_length);
            if (strcmp(filename_buffer, filename) == 0 && file_header->is_folder != 1) {
                memcpy(allocated_file, (char *)file_header, file_header->size);
                free(filename_buffer); // forgot to do this and leeked memory (very nice!)
                FILE *file_ptr = malloc(sizeof(FILE));
                *file_ptr = (FILE){.pfile = NULL, .stream = allocated_file, .flags = (FILEMODE)mode};
                return file_ptr;
            }

            offset += sizeof(FileHeader) + file_header->file_name_length + file_header->size;
        }
    } else debug_printf("not enough ram to open file!\n");

    return NULL; // file not found
}

int close_file(FILE* file) {
    free(file->stream);
    free(file);
    return 0;
}