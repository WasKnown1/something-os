#include <driver_loader/driver_loader.h>
#include <mono_fs.h>
#include <alloc.h>
#include <ata.h>
#include <log.h>
#include <qemu_log.h>
#include <paging.h>
#include <string.h>
#include <dll_loader.h>
#include <stdio.h>

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

    PEHeaderDataDirectory *data_directory = &pe_header2->optional_header.data_directory[PE_HEADER2_OPTIONAL_HEADER_ENTRY_IMPORT];

    debug_printf("import directory va = %u size = %u\n",
        data_directory->virtual_address,
        data_directory->size
    );
    if (data_directory->virtual_address == 0 || data_directory->size == 0)
        debug_printf("no imports\n");

    PEHeaderImportDescriptor *import_directory = rva_to_pointer(
        data_directory->virtual_address,
        pe_header2,
        image_base
    );

    debug_printf("import_directory ptr = %p\n", import_directory);
    debug_printf("first import directory name rva = %u\n", import_directory->name);
    if (import_directory == NULL || import_directory->name == 0)
        debug_printf("import directory is empty\n");

    i8 *dll_name;
    PEHeaderImportDescriptor *import_directory_copy = import_directory;
    for (; import_directory_copy->name != 0; import_directory_copy++) {
        dll_name = rva_to_pointer(import_directory_copy->name, pe_header2, image_base);
        debug_printf("imported dll name: %s\n", dll_name);
    }

    PEHeaderThunkData32 *lookup =
        import_directory->dummy_union_name.original_first_thunk
        ? rva_to_pointer(import_directory->dummy_union_name.original_first_thunk, pe_header2, image_base)
        : rva_to_pointer(import_directory->first_thunk, pe_header2, image_base);

    PEHeaderThunkData32 *iat = rva_to_pointer(import_directory->first_thunk, pe_header2, image_base);

    for (; lookup->u1.addressof_data != 0; lookup++, iat++) {
        if (lookup->u1.ordinal & PEHEADER_ORDINAL_FLAG32) {
            // u32 ordinal = lookup->u1.ordinal & 0xffff;

            // u32 length_of_dll_path = strlen(dll_name) + strlen("dll/") + 1;
            // i8 *dll_path_buffer = aligned_alloc(16, (length_of_dll_path + 15) & ~15);
            // snprintf(dll_path_buffer, length_of_dll_path, "dll/%s", dll_name);
            // debug_printf("resolving dll path: %s\n", dll_path_buffer);

            // DLLParserResult dll_result = parse_dll(dll_path_buffer);
            // if (dll_result.status != 0) {
            //     debug_printf("failed to load dll %s for import by ordinal %u\n", dll_path_buffer, ordinal);
            //     free(dll_path_buffer);
            //     continue;
            // } else debug_printf("loaded dll %s for import by ordinal %u\n", dll_path_buffer, ordinal);

            // u0 *function_address = resolve_dll_function(*dll_result.value, ordinal);
            // if (function_address == NULL) {
            //     debug_printf("failed to resolve function for import by ordinal %u in dll %s\n", ordinal, dll_path_buffer);
            //     free(dll_path_buffer);
            //     continue;
            // } else debug_printf("resolved function for import by ordinal %u in dll %s at address %p\n", ordinal, dll_path_buffer, function_address);

            // free(dll_path_buffer);
            panic(debug_printf, "[PANIC] import by ordinal not supported yet!\n");
        } else {
            PEHeaderImportByName *import_by_name = rva_to_pointer(lookup->u1.addressof_data, pe_header2, image_base);
            debug_printf("import by name: %s\n", import_by_name->name);

            u32 length_of_dll_path = strlen(dll_name) + strlen("dll/") + 1;
            i8 *dll_path_buffer = debug_alloc(length_of_dll_path, "driver_loader: dll path buffer");
            snprintf(dll_path_buffer, length_of_dll_path, "dll/%s", dll_name);

            debug_printf("resolving dll path: %s\n", dll_path_buffer);

            DLLParserResult *dll_result = parse_dll(dll_path_buffer);

            if (dll_result->status != 0) {
                debug_printf("failed to load dll %s for import by name %s\n", dll_path_buffer, import_by_name->name);
                free(dll_path_buffer);
                continue;
            } else debug_printf("loaded dll %s for import by name %s\n", dll_path_buffer, import_by_name->name);

            u0 *function_address = resolve_dll_function_by_name(dll_result->value, import_by_name->name);            
            
            if (function_address == NULL) {
                debug_printf("failed to resolve function for import by name %s in dll %s\n", import_by_name->name, dll_path_buffer);
                free(dll_path_buffer);
                continue;
            } else debug_printf("resolved function for import by name %s in dll %s at address %p\n", import_by_name->name, dll_path_buffer, function_address);

            iat->u1.function = (u32)(uptr)function_address;
            free(dll_path_buffer);
            free_dll_parser_info(dll_result);
        }
    }

    debug_printf("driver loaded at image base: %p\n", image_base);

    free_file_content(file_content);
    return NULL;
}

u0 execute_driver(u0* driver_entry_point) {

}

i8 get_driver_status(u0) {
    return 0;
}