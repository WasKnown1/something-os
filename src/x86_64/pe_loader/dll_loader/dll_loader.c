#include <smthng_os.h>
#include <pe_loader.h>
#include <mono_fs.h>
#include <log.h>

u0 parse_dll(i8 *dll_name) {
    if (!compare_ending(dll_name, ".dll"))
        panic(debug_printf, "[PANIC] %s is not a dll!\n", dll_name);

    FileContent *file_content = get_file_content(dll_name);

    if (file_content == NULL) {
        panic(debug_printf, "[PANIC] unable to find driver at path: %s\n", dll_name);
    } else debug_printf("loaded dll = %s\n", dll_name);

    u0 *image_base = (u0 *)GET_PEHEADER1(file_content->arg1);

    PEHeader1 *pe_header1 = GET_PEHEADER1(file_content->arg1);
    if (pe_header1->ignored != 0x5a4d)
        panic(debug_printf, "[PANIC] %s is not a driver!\n", dll_name);

    PEHeader2 *pe_header2 = GET_PEHEADER2(pe_header1);

    PEHeaderDataDirectory *data_directory = &pe_header2->optional_header.data_directory[PE_HEADER2_OPTIONAL_HEADER_ENTRY_EXPORT];
    if (data_directory->virtual_address == 0)
        panic(debug_printf, "no exports");

    PEHeaderExportDirectory *export_directory = rva_to_pointer(data_directory->virtual_address, pe_header2, image_base);
    u32 *name_rvas = rva_to_pointer(export_directory->addressof_names, pe_header2, image_base);
    u16 *ordinals = rva_to_pointer(export_directory->addressof_name_ordinals, pe_header2, image_base);
    u32 *funcion_rvas = rva_to_pointer(export_directory->addressof_functions, pe_header2, image_base);

    for (u32 i = 0; i < export_directory->numberof_names; i++) {
        i8 *name = rva_to_pointer(name_rvas[i], pe_header2, image_base);
        u16 ordinal = ordinals[i];
        u0 *function_ptr = rva_to_pointer(funcion_rvas[ordinal], pe_header2, image_base);

        debug_printf("export: %s @ %p\n", name, function_ptr);
    }
}