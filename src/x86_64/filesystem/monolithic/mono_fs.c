#include <mono_fs.h>
#include <qemu_log.h>
#include <log.h>
#include <string.h>
#include <alloc.h>
#include <ata.h>

static DiskAddress *dumb_file_search(const i8* file_name) {
    u32 fs_offset = sizeof(FsHeader);
    u16 lba = MONO_FS_START_ADDRESS / LBA_SIZE;
    u8 sector[LBA_SIZE] = {0};
    ata_read28(lba, sector);

    while (true) {
        FileHeader *file_header = (FileHeader *)(sector + (fs_offset % LBA_SIZE));

        u16 file_name_length = file_header->file_name_length;
        i8 *file_name_buffer = malloc(file_name_length);
        memcpy(file_name_buffer, sector + (fs_offset % LBA_SIZE) + sizeof(FileHeader), file_name_length);
        file_name_buffer[file_header->file_name_length] = '\0'; // no need -1 since we are setting the last byte

        if (strcmp(file_name_buffer, file_name) == 0) {
            free(file_name_buffer);
            DiskAddress *disk_address = malloc(sizeof(DiskAddress));
            *disk_address = (DiskAddress){.arg1 = lba, .arg2 = fs_offset % LBA_SIZE};
            return disk_address;
        }

        u32 file_size = file_header->size;
        fs_offset += file_size;
        lba += file_size / LBA_SIZE;
        ata_read28(lba, sector);
        free(file_name_buffer);
    }

    return NULL;
}

u0 print_mono_fs(u0) {
    DiskAddress *fs_structrue = dumb_file_search("fs_structure.json");

    if (fs_structrue == NULL)
        panic(debug_printf, "enable to find fs_structure.json!\n");

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
    qemu_log_n((i8 *)file_data, file_data_size);
    debug_printf("\n");

    free(buffer);
}

u0 mono_fs_init(u0) {
    u8* read_fs_header_sector = malloc(LBA_SIZE);
    ata_read_lbas(MONO_FS_START_ADDRESS / LBA_SIZE, 1, read_fs_header_sector);
    FsHeader* fs_header = malloc(sizeof(FsHeader));
    memcpy((void*)fs_header, (void*)read_fs_header_sector, sizeof(FsHeader));
    free(read_fs_header_sector);

    if (fs_header->signiture != MONO_FS_START_SIGNITURE) {
        debug_printf("mono fs signiture invalid! got 0x%x expected 0x%x\n", fs_header->signiture, MONO_FS_START_SIGNITURE);
        free(fs_header);
        return;
    }

    debug_printf("mono fs signiture valid! signiture: %u bytes\n", fs_header->signiture);

    debug_printf("found testdir/testdirfile.txt = %p\n", dumb_file_search("testdir/testdirfile.txt"));
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