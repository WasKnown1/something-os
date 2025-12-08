#ifndef MONO_FS_H
#define MONO_FS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>

#define MONO_FS_START_ADDRESS (64 * 512 * 3)
/* the fs is appended right at the end of the disk image                                        *
 * then the fs is unziped (with the regular zip algorithm) to the end of the first entry of ram *
 */

#define MONO_FS_START_SIGNITURE 0x44454544 // encoded ascii dead, could be anything honestly

extern void *mono_fs_address;

void mono_fs_init(void);

typedef struct FsHeader {
    uint32_t signiture;
    uint32_t size;
} __attribute__((packed)) FsHeader;

typedef struct FileHeader {
    uint8_t is_folder;
    uint32_t size; // including the file end header
    uint32_t padding_from_original_size;
    uint16_t file_name_length; // this includes the full directory
    // there goes file name
    // and then goes the file info
} __attribute__((packed)) FileHeader;

typedef struct FileEndHeader {
    uint32_t size;
    uint32_t signiture; // also DEED
} __attribute__((packed)) FileEndHeader;

FILE *get_file(const char *filename, const char *restrict mode);
int close_file(FILE *file);

#endif // MONO_FS_H