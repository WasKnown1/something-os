#include <paging.h>

void clear_bss(void);

void entry(void) {
    clear_bss(); // zero out bss if hasnt been already done yet

    __asm__("movb $'c', %al\n\t"
            "outb %al, $0xe9 \n\t");

    init_paging();

    __asm__ (
        "cli\n\t"
        "hlt");

    while (1) {
        __asm__ ("hlt");
    }
}

extern unsigned int __bss_start;
extern unsigned int __bss_end;

void clear_bss(void) {
    unsigned int *bss = &__bss_start;
    unsigned int *bss_end = &__bss_end;
    
    while (bss < bss_end) {
        *bss++ = 0;
    }
}