#include <driver_loader/driver_loader.h>
#include <mono_fs.h>
#include <alloc.h>
#include <ata.h>
#include <log.h>
#include <qemu_log.h>
#include <paging.h>
#include <string.h>

DriverLoadResult *load_driver(const i8 *driver_path) {
    FileContent *file_content = get_file_content(driver_path);

    if (file_content == NULL) {
        panic(debug_printf, "[PANIC] unable to find driver at path: %s\n", driver_path);
    } else
        debug_printf("loaded driver = %s\n", driver_path);

    u0 *image_base = (u0 *)GET_PEHEADER1(file_content->arg1);

    PEHeader1 *pe_header1 = GET_PEHEADER1(file_content->arg1);
    if (pe_header1->ignored != 0x5a4d)
        panic(debug_printf, "[PANIC] %s is not a driver!\n", driver_path);

    debug_printf("found signiture = ");
    qemu_log_n((i8 *)pe_header1, 2);
    debug_printf("\n");

    debug_printf("offset of next header: %u\n", pe_header1->e_lfanew);
    PEHeader2 *pe_header2 = GET_PEHEADER2(pe_header1);

    debug_printf("coff signiture = ");
    qemu_log_n((i8 *)&pe_header2->ignored, 4);
    debug_printf("\n");

    PEHeader3 *pe_header3 = GET_PEHEADER3(pe_header2);
    debug_printf("offset to symbol table = %u\n", pe_header3->ptr_symbol_table);
    // hexdump((u8 *)pe_header2 + pe_header3->ptr_symbol_table, 240);

    PEHeaderDataDir *data_directory = &pe_header2->optional_header.data_directory[PE_HEADER2_OPTIONAL_HEADER_ENTRY_EXPORT];
    if (data_directory->virtual_address == 0)
        panic(debug_printf, "no exports");

    PEHeaderExportDir *export_directory = rva_to_pointer(data_directory->virtual_address, pe_header2, image_base);
    u32 *name_rvas = rva_to_pointer(export_directory->addressof_names, pe_header2, image_base);
    u16 *ordinals  = rva_to_pointer(export_directory->addressof_name_ordinals, pe_header2, image_base);
    u32 *funcion_rvas = rva_to_pointer(export_directory->addressof_functions, pe_header2, image_base);

    for (u32 i = 0; i < export_directory->numberof_names; i++) {
        i8 *name = rva_to_pointer(name_rvas[i], pe_header2, image_base);
        u16 ordinal = ordinals[i];
        u0 *function_ptr = rva_to_pointer(funcion_rvas[ordinal], pe_header2, image_base);

        debug_printf("export: %s @ %p\n", name, function_ptr);
    }

    free_file_content(file_content);
    return NULL;
}

u0 execute_driver(u0* driver_entry_point) {

}

i8 get_driver_status(u0) {
    return 0;
}