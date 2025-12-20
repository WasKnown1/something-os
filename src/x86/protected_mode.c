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
#include <ata.h>
#include <tss.h>
#include <pio.h>

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
    // mask all irqs for slave/master irq pic
    outb(0x21, 0xff);
    outb(0xa1, 0xff);

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

    // mono_fs_init();

    // FILE* fd = fopen("testdir/testdirfile.txt", "dasdasd");
    // if (fd == NULL)
    //     debug_printf("file not found!\n");
    // else
    //     debug_printf("file found!\n");
    // for (uint16_t i = 0; i < 36; i++)
    //     fputc('A', fd);
    // fflush(fd);
    // if (fclose(fd) == 0)
    //     debug_printf("successfully closed the file!\n");
    // else
    //     debug_printf("failed to close the file!\n");
    // print_mono_fs();
    // hexdump(mono_fs_address, 300);

    tss_init(0x9fc00);

    __asm__("cli");

    uint8_t buf[512] __attribute__((aligned(2)));
    debug_printf("about to call ata_read28\n");
    debug_printf("  function pointer: 0x%x\n", ata_read28);
    debug_printf("  stack: 0x%x\n", &buf);

    debug_printf("attempting direct inb(0x1F7)...\n");
    uint8_t test = inb(0x1F7);
    debug_printf("  result: 0x%x\n", test);
    debug_printf("still alive after inb!\n");

    if (ata_read28(0, buf)) {
        debug_printf("read successful!\n");
    }

    buf[509] = 0x90;
    ata_write28(0, buf);

    ata_read28(0, buf);

    __asm__("sti");
    hexdump(buf, sizeof(buf));

    __asm__ (
        "cli\n\t"
        "hlt\n\t");

    while (1) {
        __asm__ ("hlt\n\t");
    }
}