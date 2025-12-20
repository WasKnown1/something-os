#include <mono_fs.h>
#include <log.h>
#include <paging.h>
#include <ram.h>
#include <string.h>
#include <alloc.h>

void *mono_fs_address;
FsHeader *fs_header;

void print_mono_fs(void) {
    debug_printf("fs_header->size: %d\n", fs_header->size);
    uint32_t offset = sizeof(FsHeader);
    
    while (offset < fs_header->size) {
        FileHeader *file_header = (FileHeader *)(mono_fs_address + offset);
        debug_printf("file_header->size: %d\n", file_header->size);

        if (file_header->is_folder != 1 && file_header->file_name_length > 0) {
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
            debug_log_n(file_content_ptr, file_header->size - sizeof(FileEndHeader) - sizeof(FileHeader) - file_header->file_name_length);
            debug_log("\n");

            debug_printf("sizeof file %s: %d\n", filename_ptr, file_header->size);
        } else if (file_header->is_folder == 1)
            debug_printf("found folder!\n");
        offset += file_header->size;
        debug_printf("offset: %d\n", offset);
    }
}

static bool mono_fs_unpacked = false;

void mono_fs_init(void) {
    mono_fs_address = (void *)((uint32_t)ram_memmap[0].base + (uint32_t)ram_memmap[0].length);
    map_identity_4mb((uint32_t)mono_fs_address, 0x4000000); // map 64MB after ram for mono fs access

    debug_printf("Initializing mono filesystem... at %p\n", MONO_FS_START_ADDRESS);
    fs_header = (FsHeader *)MONO_FS_START_ADDRESS;
    debug_printf("signiture = %d, %d\n", fs_header->signiture, MONO_FS_START_SIGNITURE);

    if (fs_header->signiture != MONO_FS_START_SIGNITURE) {
        debug_printf("didnt find the filesystem!\n");
        return;
    }

    if (!mono_fs_unpacked)  // only unpack when first boot...
        memcpy(mono_fs_address, (char *)MONO_FS_START_ADDRESS, fs_header->size);
    else
        debug_printf("file system was unpacked!\n");
    mono_fs_unpacked = true; // true
    fs_header = (FsHeader *)mono_fs_address;

    debug_printf("filesystem start size = %d\n", fs_header->size);
    debug_printf("mono_fs at address: %p\n", fs_header);
    debug_printf("sizeof fpos_t: %d\n", sizeof(fpos_t));

    print_mono_fs();
}

FILE *get_file(const char *filename) { // maybe i will change this to return a handle in the future like in windows
    uint32_t allocated_file_size = get_ram_size() / 100;
    if (get_largest_entry_ram_size() > allocated_file_size + sizeof(EntryHeader) + sizeof(MemoryBlock)) {

        uint32_t offset = sizeof(FsHeader);

        while (offset < fs_header->size){
            FileHeader *file_header = (FileHeader *)(mono_fs_address + offset);
            void *allocated_file = malloc(allocated_file_size);

            char *filename_ptr = (char *)file_header + sizeof(FileHeader);
            char *filename_buffer = malloc(file_header->file_name_length);
            memcpy(filename_buffer, filename_ptr, file_header->file_name_length);
            if (strcmp(filename_buffer, filename) == 0 && 
                file_header->is_folder != 1 &&
                file_header->file_name_length > 0) {
                memcpy(allocated_file, (char *)file_header, file_header->size);
                free(filename_buffer); // forgot to do this and leeked memory (very nice!)
                FILE *file_ptr = malloc(sizeof(FILE));
                *file_ptr = (FILE){.file_ptr = mono_fs_address + offset,
                                   .stream = allocated_file,
                                   .stream_size = file_header->size};
                debug_printf("file at pointer: %p\n", mono_fs_address + offset);
                return file_ptr;
            }

            offset += file_header->size;
        }
    } else debug_printf("not enough ram to open file!\n");

    return NULL; // file not found
}

// FILE* create_file(const char* restrict filename, uint32_t file_size, const FILEMODE mode) {
//     void *allocated_file = malloc(file_size);
//     FileHeader file_header = (FileHeader){
//         .is_folder = 0x01,
//         .size = file_size,
//         .padding_from_original_size = 0,
//         .file_name_length = strlen(filename)
//     };
//     memcpy((char *)allocated_file, (void *)&file_header, sizeof(FileHeader));
//     memcpy((char *)allocated_file + sizeof(FileHeader), filename, strlen(filename));
//     uint32_t file_content_size = file_size - sizeof(FileHeader) - sizeof(FileEndHeader) - strlen(filename);
//     memset((char *)allocated_file + sizeof(FileHeader) + strlen(filename), 0, file_content_size);
//     FileEndHeader file_end_header = (FileEndHeader){
//         .size = file_size,
//         .signiture = MONO_FS_START_SIGNITURE
//     };
//     memcpy((char *)allocated_file + file_size - sizeof(FileEndHeader), (void *)&file_end_header, sizeof(FileEndHeader));
//     FILE *file = malloc(sizeof(FILE));
// }

int flush_file(FILE* file) {
    FileHeader* file_ptr = (FileHeader*)file->file_ptr;

    if (file->stream_size > file_ptr->size) {
        memset((char *)file_ptr + sizeof(FileHeader), 0, file_ptr->size - sizeof(FileHeader) - sizeof(FileEndHeader));
        memcpy((char *)mono_fs_address + fs_header->size, file->stream, file->stream_size);
        file_ptr->file_name_length = 0;
        FileHeader *file_header = (FileHeader *)((char *)mono_fs_address + fs_header->size);
        file_header->size = file->stream_size;
        debug_printf("sizeof stream: %d\n", file->stream_size);
        FileEndHeader file_end_header = (FileEndHeader){
            .size = file->stream_size,
            .signiture = MONO_FS_START_SIGNITURE};
        memcpy((char *)mono_fs_address + fs_header->size + file->stream_size - sizeof(FileEndHeader), (char *)&file_end_header, sizeof(FileEndHeader));
        fs_header->size += file_header->size; 
    }
    return 0;
}

int write_char(uint8_t c, FILE *file) {
    ((uint8_t *)file->stream)[file->stream_size - sizeof(FileEndHeader)] = c;
    file->stream_size++;
    return 0;
}

int close_file(FILE* file) {
    return 0;
}