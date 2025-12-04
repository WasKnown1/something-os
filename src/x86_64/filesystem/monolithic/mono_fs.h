#ifndef MONO_FS_H
#define MONO_FS_H

#include <stdint.h>
#include <stdbool.h>

#define MONO_FS_START_ADDRESS (64 * 512 * 3)
/* the fs is appended right at the end of the disk image                                        *
 * then the fs is unziped (with the regular zip algorithm) to the end of the first entry of ram *
 */

#define MONO_FS_START_SIGNITURE 0x44454544 // encoded ascii dead

void mono_fs_init(void);

typedef struct FsHeader {
    uint32_t signiture;
    uint32_t size;
} __attribute__((packed)) FsHeader;

typedef struct FileHeader {
    uint8_t is_folder;
    uint32_t size;
    uint16_t file_name_length; // this includes the full directory
    // there goes file name
    // and then goes the file info
} __attribute__((packed)) FileHeader;

extern void *mono_fs_address;

#endif // MONO_FS_H