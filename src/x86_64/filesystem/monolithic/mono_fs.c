#include <mono_fs.h>
#include <qemu_log.h>
#include <log.h>
#include <string.h>
#include <alloc.h>
#include <ata.h>

static FsHeader fs_header = {.signiture = 0, .size = 0};

static void fs_read_bytes(u32 offset, u0 *out, u32 size) {
    u8 sector[LBA_SIZE];
    u32 remaining = size;
    u8 *dst = out;

    while (remaining > 0) {
        u32 lba = (MONO_FS_START_ADDRESS + offset) / LBA_SIZE;
        u32 sector_offset = offset % LBA_SIZE;

        ata_read28(lba, sector);

        u32 to_copy = LBA_SIZE - sector_offset;
        if (to_copy > remaining)
            to_copy = remaining;

        memcpy(dst, sector + sector_offset, to_copy);

        dst += to_copy;
        offset += to_copy;
        remaining -= to_copy;
    }
}

DiskAddress *dumb_file_search(const i8 *file_name) {
    u32 fs_offset = sizeof(FsHeader);

    while (fs_offset < fs_header.size) {
        FileHeader file_header;
        fs_read_bytes(fs_offset, &file_header, sizeof(FileHeader));

        u16 name_len = file_header.file_name_length;

        i8 *name = malloc(name_len + 1);
        fs_read_bytes(fs_offset + sizeof(FileHeader), name, name_len);
        name[name_len] = '\0';

        if (strcmp(name, file_name) == 0) {
            free(name);

            DiskAddress *addr = malloc(sizeof(DiskAddress));
            addr->arg1 = (MONO_FS_START_ADDRESS + fs_offset) / LBA_SIZE;
            addr->arg2 = (MONO_FS_START_ADDRESS + fs_offset) % LBA_SIZE;
            return addr;
        }
        free(name);
        fs_offset += file_header.size;
    }

    return NULL;
}

u0 free_file_content(FileContent *file_content) {
    free(file_content->arg1);
    free(file_content);
}

FileContent *get_file_content(const i8 *file_name) {
    DiskAddress *fs_structrue = dumb_file_search(file_name);
    if (fs_structrue == NULL)
        return NULL;

    u8 sector[LBA_SIZE] = {0};
    ata_read28(fs_structrue->arg1, sector);

    FileHeader *file_header = (FileHeader *)(sector + fs_structrue->arg2);
    u32 total_bytes = file_header->size + fs_structrue->arg2;
    u32 lbas = (total_bytes + LBA_SIZE - 1) / LBA_SIZE;

    u8 *buffer = malloc(lbas * LBA_SIZE);
    ata_read_lbas(fs_structrue->arg1, lbas, buffer);

    u8 *file_base = buffer + fs_structrue->arg2;
    u8 *file_data = file_base + sizeof(FileHeader) + file_header->file_name_length;

    u32 file_data_size = file_header->size - sizeof(FileHeader) - file_header->file_name_length;
    u8 *file_raw = malloc(file_data_size);
    memcpy(file_raw, file_data, file_data_size);
    free(buffer);

    FileContent *file_content = malloc(sizeof(FileContent));
    file_content->arg1 = file_raw;
    file_content->arg2 = file_data_size;

    free(fs_structrue);

    return file_content;
}

u0 print_mono_fs(u0) {
    FileContent *fs_structure_json = get_file_content("fs_structure.json");

    if (fs_structure_json == NULL)
        panic(debug_printf, "[PANIC] unable to find fs_structure.json!\n");

    qemu_log_n((i8 *)fs_structure_json->arg1, fs_structure_json->arg2);
    debug_printf("\n");

    free_file_content(fs_structure_json);
}

u0 mono_fs_init(u0) {
    u8* read_fs_header_sector = malloc(LBA_SIZE);
    ata_read_lbas(MONO_FS_START_ADDRESS / LBA_SIZE, 1, read_fs_header_sector);
    memcpy((u0 *)&fs_header, (u0 *)read_fs_header_sector, sizeof(FsHeader));
    free(read_fs_header_sector);

    if (fs_header.signiture != MONO_FS_START_SIGNITURE) {
        debug_printf("mono fs signiture invalid! got 0x%x expected 0x%x\n", fs_header.signiture, MONO_FS_START_SIGNITURE);
        return;
    }

    debug_printf("mono fs signiture valid! signiture: %u bytes\n", fs_header.signiture);
    debug_printf("mono fs size %u bytes\n", fs_header.size);
}

FILE *get_file(const char *filename) { // maybe i will change this to return a handle in the future like in windows
    (void)filename;
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
    (void)file;
    return 0;
}

int write_char(uint8_t c, FILE *file) {
    (void)c;
    (void)file;
    return 0;
}

int close_file(FILE* file) {
    return 0;
}