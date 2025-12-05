#include <paging.h>
#include <idt.h>
#include <qemu_log.h>
#include <log.h>
#include <pae_mode.h>
#include <long_mode.h>
#include <ram.h>
#include <alloc.h>
#include <compatibility_mode.h>
#include <gdt64.h>
#include <mono_fs.h>
#include <stdio.h>

extern unsigned int __bss_start;
extern unsigned int __bss_end;

void clear_bss(void) {
    unsigned int *bss = &__bss_start;
    unsigned int *bss_end = &__bss_end;

    while (bss < bss_end) {
        *bss++ = 0;
    }
}

__attribute__((section(".entry"))) void entry(void)  {
    clear_bss(); // zero out bss if hasnt been already done yet
    init_paging();

    if (cpu_supports_pae()) {
        debug_printf("cpu supports pae!\n");
    } else {
        debug_printf("cpu doesnt support pae :(\n");
    }

    if (cpu_supports_long_mode() && cpu_supports_pae()) {
        debug_printf("cpu supports long mode!\n");
        disable_paging();
        clear_page_tables();
        link_first_entries_of_each_table();
        set_entry_in_page_table();
        enable_pae_mode();

        if (cpu_supports_pml5()) {
            debug_printf("cpu supports pml5!\n");
            // enable_level5_paging(); // this can only be done in long mode
            enable_compatibility_mode();
            set_gdt64();
            jump_to_long_mode();
        } else {
            debug_printf("cpu doesnt support pml5 :(\n");
        }

    } else if (!cpu_supports_pae()) {
        debug_printf("long mode requires pae support!\n");
    } else {
        debug_printf("cpu doesnt support long mode :(\n");
    }

    alloc_init();
    for (uint8_t i = 0; i < ram_memmap_count; i++) {
        debug_printf("Usable RAM %d: Base: 0x%x Length: 0x%x\n", 
                     i, (unsigned int)ram_memmap[i].base, (unsigned int)ram_memmap[i].length);
    }

    __asm__("cli\n\t"); // disable interrupts before setting idt
    init_idt();

    void *ptr1 = malloc(10);
    *(int *)ptr1 = 42;
    debug_printf("Allocated ptr1 at 0x%x with value %d\n", (unsigned int)ptr1, *(int *)ptr1);
    void *ptr2 = malloc(20);
    *(int *)ptr2 = 43;
    debug_printf("Allocated ptr2 at 0x%x with value %d\n", (unsigned int)ptr2, *(int *)ptr2);
    void *ptr3 = malloc(30);
    *(int *)ptr3 = 84;
    debug_printf("Allocated ptr3 at 0x%x with value %d\n", (unsigned int)ptr3, *(int *)ptr3);
    print_memory_allocations();
    free(ptr2);
    print_memory_allocations();
    ptr2 = malloc(15);
    print_memory_allocations();
    free(ptr1);
    free(ptr2);
    free(ptr3);

    disable_paging();
    init_paging_4mb_identity();

    mono_fs_init();

    FILE *fd = fopen("testdir/testdirfile.txt", "dasdasd");
    debug_printf("fd = %d\n", (uint32_t)fd->ptr);
    // __asm__("mov $1, %eax\n\t"
    //         "xor %ebx, %ebx\n\t"
    //         "div %ebx\n\t");
    // __asm__("int $0x10\n\t");

    __asm__ (
        "cli\n\t"
        "hlt\n\t");

    while (1) {
        __asm__ ("hlt\n\t");
    }
}