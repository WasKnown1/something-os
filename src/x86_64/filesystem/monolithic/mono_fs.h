#ifndef MONO_FS_H
#define MONO_FS_H

#include <stdint.h>

#define MONO_FS_START_ADDRESS (64 * 512 * 3)
/* the fs is appended right at the end of the disk image                                        *
 * then the fs is unziped (with the regular zip algorithm) to the end of the first entry of ram *
 */

void mono_fs_init(void);

typedef struct ZipHeader {
    uint8_t  signature[4];
    uint16_t version_needed;
    uint16_t general_purpose_bit_flag;
    uint16_t compression_method;
    uint16_t last_mod_file_time;
    uint16_t last_mod_file_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t file_name_length;
    uint16_t extra_field_length;
    // followed by file name and extra field
} __attribute__((packed)) ZipHeader;

typedef struct DataDescriptor {
    uint32_t signature; // optional
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
} __attribute__((packed)) DataDescriptor;

typedef struct CDFH {
    uint32_t signature;
    uint16_t version_made_by;
    uint16_t version_needed;
    uint16_t general_purpose_bit_flag;
    uint16_t compression_method;
    uint16_t last_mod_file_time;
    uint16_t last_mod_file_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t file_name_length;
    uint16_t extra_field_length;
    uint16_t file_comment_length;
    uint16_t disk_number_start;
    uint16_t internal_file_attributes;
    uint32_t external_file_attributes;
    uint32_t relative_offset_of_local_header;
} __attribute__((packed)) CDFH;

typedef struct EOCD {
    uint32_t signature;
    uint16_t disk_number;
    uint16_t disk_number_start_of_central_dir;   // number_of_the_disk_with_the_start_of_the_central_directory;
    uint16_t num_entries_in_central_dir_on_disk; // total_number_of_entries_in_the_central_directory_on_this_disk;
    uint16_t num_entries_in_central_dir;         // total_number_of_entries_in_the_central_directory;
    uint32_t sizeof_centeral_dir;                // size_of_the_central_directory;
    uint32_t offset;
    uint16_t zip_file_comment_length;
    // followed by zip file comment
} __attribute__((packed)) EOCD;

extern uint8_t *file_name;   // will be allocated dynamically
extern uint8_t *extra_field; // will be allocated dynamically

#define ZIP_END_MAGIC "\x50\x4b\x05\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"; // marks end of zip file

#endif // MONO_FS_H