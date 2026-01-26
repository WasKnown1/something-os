#include <driver_loader/driver_loader.h>
#include <mono_fs.h>
#include <alloc.h>
#include <ata.h>
#include <log.h>
#include <qemu_log.h>
#include <paging.h>
#include <string.h>
#include <dll_loader.h>

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

    PEHeaderImportDirectory *import_directory = rva_to_pointer(
        data_directory->virtual_address,
        pe_header2,
        image_base
    );

    debug_printf("import_directory ptr = %p\n", import_directory);
    debug_printf("first import directory name rva = %u\n", import_directory->name);
    if (import_directory == NULL || import_directory->name == 0)
        debug_printf("import directory is empty\n");

    for (; import_directory->name != 0; import_directory++) {
        i8 *dll_name = rva_to_pointer(import_directory->name, pe_header2, image_base);
        // parse_dll(dll_name);
        debug_printf("imported dll name: %s\n", dll_name);
    }

    free_file_content(file_content);
    return NULL;
}

u0 execute_driver(u0* driver_entry_point) {

}

i8 get_driver_status(u0) {
    return 0;
}