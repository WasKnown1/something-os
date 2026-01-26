#ifndef PE_LOADER_H
#define PE_LOADER_H

#include <smthng_os.h>
#include <stdint.h>

typedef Result(void*, i8) DriverLoadResult;

typedef struct {
    u16 ignored;
    u16 e_cblp;
    u16 e_cp;
    u16 e_crlc;
    u16 e_cparhdr;
    u16 e_minalloc;
    u16 e_maxalloc;
    u16 e_ss;
    u16 e_sp;
    u16 e_csum;
    u16 e_ip;
    u16 e_cs;
    u16 e_lfarlc;
    u16 e_ovno;
    u16 e_res[4];
    u16 e_oemid;
    u16 e_oeminfo;
    u16 e_res2[10];
    u32 e_lfanew;   // offset to 3rd header
} __attribute__((packed)) PEHeader1;

typedef struct {
    u16 machine;
    u16 sections_count;
    u32 time_data_stamp;
    u32 ptr_symbol_table;
    u32 symbols_count;
    u16 sizeof_optional_header;
    u16 flags;
} __attribute__((packed)) PEHeader3;

typedef struct {
    u32 virtual_address;
    u32 size;
} __attribute__((packed)) PEHeaderDataDirectory;

#define DIRECTORY_ENTRIES_COUNT 16

typedef struct {
    u16 ignored1;
    u8 maj_ld_ver;
    u8 min_ld_ver;
    u32 sizeof_code;
    u32 sizeof_init_data;
    u32 sizeof_uninit_data;
    u32 adr_entry_point;
    u32 base_code;
    u32 base_data;
    u32 img_base;
    u32 section_alignment;
    u32 file_alignment;
    // major os version should probably be ignored too but well see later on
    u16 maj_os_ver; 
    u16 min_os_ver;
    u16 maj_img_ver;
    u16 min_img_ver;
    u16 maj_subsys_ver;
    u16 min_subsys_ver;
    u32 ignored2;
    u32 sizeof_image;
    u32 sizeof_headers;
    u32 chechsum;
    u16 subsystem;
    u16 dll_flags;
    u32 sizeof_stack_reserve;
    u32 sizeof_stack_commit;
    u32 sizeof_heap_reserve;
    u32 sizeof_heap_commit;
    u32 loader_flags;
    u32 rva_and_sizes_count;
    PEHeaderDataDirectory data_directory[DIRECTORY_ENTRIES_COUNT];
} __attribute__((packed)) PEOptionalHeader32;

// this is not used right now...
typedef struct {
    u16 ignored1;
    u8 maj_ld_ver;
    u8 min_ld_ver;
    u32 sizeof_code;
    u32 sizeof_init_data;
    u32 sizeof_uninit_data;
    u32 adr_entry_point;
    u32 base_code;
    u32 base_data;
    u64 img_base;
    u32 section_alignment;
    u32 file_alignment;
    // major os version should probably be ignored too but well see later on
    u16 maj_os_ver;
    u16 min_os_ver;
    u16 maj_img_ver;
    u16 min_img_ver;
    u16 maj_subsys_ver;
    u16 min_subsys_ver;
    u32 ignored2;
    u32 sizeof_image;
    u32 sizeof_headers;
    u32 chechsum;
    u16 subsystem;
    u16 dll_flags;
    u64 sizeof_stack_reserve;
    u64 sizeof_stack_commit;
    u64 sizeof_heap_reserve;
    u64 sizeof_heap_commit;
    u32 loader_flags;
    u32 rva_and_sizes_count;
    PEHeaderDataDirectory data_directory[DIRECTORY_ENTRIES_COUNT];
} __attribute__((packed)) PEOptionalHeader64;

typedef struct {
    u32 ignored;
    PEHeader3 file_header;
    PEOptionalHeader32 optional_header;
} __attribute__((packed)) PEHeader2;

typedef struct  {
    u8 name[PE_HEADER_SIZEOF_SHORT_NAME];
    union {
        u32 physical_address;
        u32 virtual_size;
    } misc;
    u32 virtual_address;
    u32 sizeof_raw_data;
    u32 pointer_to_raw_data;
    u32 pointer_to_relocations;
    u32 pointer_to_linenumbers;
    u16 numberof_relocations;
    u16 numberof_linenumbers;
    u32 characteristics;
} __attribute__((packed)) PEHeaderSectionHeader;

typedef struct {
    u32 characteristics;
    u32 timedate_stamp;
    u16 major_version;
    u16 minor_version;
    u32 name;
    u32 base;
    u32 numberof_functions;
    u32 numberof_names;
    u32 addressof_functions;
    u32 addressof_names;
    u32 addressof_name_ordinals;
} __attribute__((packed)) PEHeaderExportDirectory;

typedef struct {
    union {
        u32 characteristics;
        u32 original_first_thunk;
    } dummy_union_name;
    u32 time_date_stamp;
    u32 forwarder_chain;
    u32 name;
    u32 first_thunk;
} __attribute__((packed)) PEHeaderImportDirectory;



#define SECTION1(n) ((PEHeaderSectionHeader *)((uintptr_t)n + OFFSET_OF_FIELD(PEHeader2, optional_header) + ((PEHeader2 *)(n))->file_header.sizeof_optional_header))

#define GET_PEHEADER1(file)       (PEHeader1 *)file
#define GET_PEHEADER2(pe_header1) (PEHeader2 *)((u8 *)pe_header1 + pe_header1->e_lfanew)
#define GET_PEHEADER3(pe_header2) (PEHeader3 *)&pe_header2->file_header

u0 *rva_to_pointer(u32 rva, PEHeader2 *pe_header2, u0 *image_base);

#endif // PE_LOADER_H