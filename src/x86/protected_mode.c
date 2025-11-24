#include <paging.h>
#include <idt.h>
#include <qemu_log.h>
#include <log.h>
#include <pae_mode.h>
#include <long_mode.h>
#include <ram.h>

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

    parse_e820_memory_map();

    // __asm__("movb $'c', %al\n\t"
    //         "outb %al, $0xe9 \n\t");

    init_paging();
    __asm__("cli\n\t"); // disable interrupts before setting idt
    init_idt();

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