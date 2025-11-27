#include <paging.h>
#include <idt.h>
#include <qemu_log.h>
#include <log.h>
#include <pae_mode.h>
#include <long_mode.h>
#include <ram.h>
#include <alloc.h>

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

    debug_printf("hello %s, your number is %d and char is %c\n", "world!", 123, 'X');
    if (cpu_support_pae()) {
        debug_printf("cpu supports pae!\n");
    } else {
        debug_printf("cpu doesnt support pae :(\n");
    }

    if (long_mode_support()) {
        debug_printf("cpu supports long mode!\n");
    } else {
        debug_printf("cpu doesnt support long mode :(\n");
    }

    alloc_init();
    for (uint8_t i = 0; i < ram_memmap_count; i++) {
        debug_printf("Usable RAM %d: Base: 0x%x Length: 0x%x\n", 
                     i, (unsigned int)ram_memmap[i].base, (unsigned int)ram_memmap[i].length);
    }

    // __asm__("movb $'c', %al\n\t"
    //         "outb %al, $0xe9 \n\t");

    init_paging();
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

    __asm__("mov $1, %eax\n\t"
            "xor %ebx, %ebx\n\t"
            "div %ebx\n\t");
    // __asm__("int $0x10\n\t");

    __asm__ (
        "cli\n\t"
        "hlt\n\t");

    while (1) {
        __asm__ ("hlt\n\t");
    }
}