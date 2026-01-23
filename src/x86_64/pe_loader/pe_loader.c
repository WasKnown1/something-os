#include <pe_loader.h>
#include <stddef.h>

u0 *rva_to_pointer(u32 rva, PEHeader2 *pe_header2, u0 *image_base) {
    PEHeaderSectionHeader *section_header = SECTION1(pe_header2);

    for (u32 i = 0; i < pe_header2->file_header.sections_count; i++, section_header++) {
        u32 start = section_header->virtual_address;
        u32 end = start + section_header->misc.virtual_size;

        if (rva >= start && rva < end)
            return (u8 *)image_base + section_header->pointer_to_raw_data + (rva - start);
    }
    return NULL;
 } 