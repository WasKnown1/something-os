#include <gdt64.h>

void set_gdt64(void) {
    __asm__("lgdt ([gdt_pointer])"
            :
            : [gdt_pointer] "m"(gdt_pointer)
            : "memory");
}