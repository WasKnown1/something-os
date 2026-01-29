#include <smthng_os.h>
#include <pe_loader.h>
#include <mono_fs.h>
#include <log.h>
#include <dll_loader.h>
#include <string.h>
#include <alloc.h>

// TODO: fix the DLLParserInfo struct and use the paramerters correctly like in the loop of parse_dll

DLLParserResult *parse_dll(i8 *dll_name) {
    if (!compare_ending(dll_name, ".dll"))
        return &(DLLParserResult){.status = 1, .value = NULL};

    FileContent *file_content = get_file_content(dll_name);

    if (file_content == NULL) {
        return &(DLLParserResult){.status = 2, .value = NULL};
    } else debug_printf("loaded dll = %s\n", dll_name);
    u0 *image_base = (u0 *)GET_PEHEADER1(file_content->arg1);
    
    PEHeader1 *pe_header1 = GET_PEHEADER1(file_content->arg1);
    if (pe_header1->ignored != 0x5a4d)
        return &(DLLParserResult){.status = 3, .value = NULL};

    PEHeader2 *pe_header2 = GET_PEHEADER2(pe_header1);

    PEHeaderDataDirectory *data_directory = &pe_header2->optional_header.data_directory[PE_HEADER2_OPTIONAL_HEADER_ENTRY_EXPORT];
    if (data_directory->virtual_address == 0)
        return &(DLLParserResult) { .status = 4, .value = NULL };

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

    DLLParserInfo *dll_info = debug_alloc(sizeof(DLLParserInfo), "dll_loader: dll info");
    dll_info = &(DLLParserInfo) {
        .exported_function_rvas = funcion_rvas,
        .exported_name_rvas     = name_rvas,
        .exported_name_ordinals = ordinals,
        .number_of_names = (u16)export_directory->numberof_names,
        .pe_header2 = pe_header2,
        .dll_image_base = image_base
    };

    return &(DLLParserResult) {
        .status = 0,
        .value = dll_info
    };
}

u0 *resolve_dll_function_by_ordinal(DLLParserInfo *dll_result, u16 ordinal) {
    u32 function_rva = dll_result->exported_function_rvas[ordinal];
    u0 *function_pointer = rva_to_pointer(function_rva, dll_result->pe_header2, dll_result->dll_image_base);
    return function_pointer;
}

u0 *resolve_dll_function_by_name(DLLParserInfo *dll, const i8 *function_name) {
    for (u16 i = 0; i < dll->number_of_names; i++) {
        i8 *current_name = rva_to_pointer(dll->exported_name_rvas[i], dll->pe_header2, dll->dll_image_base);
        if (strcmp(current_name, function_name) == 0) {
            u16 ordinal = dll->exported_name_ordinals[i];
            u32 function_rva = dll->exported_function_rvas[ordinal];
            return rva_to_pointer(function_rva, dll->pe_header2, dll->dll_image_base);
        }
    }
    return NULL;
}

u0 free_dll_parser_info(DLLParserResult *dll_parser_result) {
    free(dll_parser_result->value);
}