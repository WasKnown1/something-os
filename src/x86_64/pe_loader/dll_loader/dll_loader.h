#ifndef DLL_LOADER_H
#define DLL_LOADER_H

#include <smthng_os.h>

typedef struct {
    u32 *exported_function_rvas;
    u32 *exported_name_rvas;
    u16 *exported_name_ordinals;
    u16 number_of_names;
    PEHeader2 *pe_header2;
    u0 *dll_image_base;
} DLLParserInfo;

typedef Result(DLLParserInfo *, i8) DLLParserResult;

DLLParserResult *parse_dll(i8 *dll_name);
u0 *resolve_dll_function_by_ordinal(DLLParserInfo *dll_result, u16 ordinal);
u0 *resolve_dll_function_by_name(DLLParserInfo *dll_result, const i8 *function_name);
u0 free_dll_parser_info(DLLParserResult *dll_parser_result);

#endif // DLL_LOADER_H