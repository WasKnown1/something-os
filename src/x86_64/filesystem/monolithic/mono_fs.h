#ifndef MONO_FS_H
#define MONO_FS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <smthng_os.h>

#define MONO_FS_START_ADDRESS (64 * 512 * 3)
/* the fs is appended right at the end of the disk image                                        *
 * then the fs is unziped (with the regular zip algorithm) to the end of the first entry of ram *
 */

#define MONO_FS_START_SIGNITURE 0x44454544 // encoded ascii dead, could be anything honestly
#define LBA_SIZE 512

typedef Pair(u16, u32) DiskAddress;
typedef Pair(u8 *, u32) FileContent;
// #define ERROR_DISK_ADDRESS (DiskAddress){.arg1 = 0, .arg2 = 0};

typedef struct FsHeader {
    uint32_t signiture;
    u32 size;
} __attribute__((packed)) FsHeader;

typedef struct FileHeader {
    uint8_t is_folder;
    uint32_t size; // including the file end header
    uint32_t padding_from_original_size;
    uint16_t file_name_length; // this includes the full directory
    // there goes file name
    // and then goes the file info
} __attribute__((packed)) FileHeader;

DiskAddress *dumb_file_search(const i8 *file_name);
FileContent *get_file_content(const i8 *file_name);
u0 free_file_content(FileContent *file_content);
u0 print_mono_fs(u0);
u0 mono_fs_init(u0);
FILE *get_file(const char *filename);
int flush_file(FILE *file);
int write_char(uint8_t c, FILE *file);
int close_file(FILE *file);

#endif // MONO_FS_H