#include <paging.h>

void entry(void) {
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