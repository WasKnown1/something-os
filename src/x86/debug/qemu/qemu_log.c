#include <stdint.h>

void qemu_log(const char* str) {
    while (*str) {
        __asm__("outb %%al, $0xe9\n\t"
                :   
                : "a" (*str));
        str++;
    }
}

void qemu_log_n(const char* str, uint16_t n) {
    for (uint16_t i = 0; i < n; i++, str++)
    {
        __asm__("outb %%al, $0xe9\n\t"
                :
                : "a"(*str));
    }
}

void qemu_log_char(uint8_t c) {
    __asm__("outb %%al, $0xe9\n\t"
            :
            : "a"(c));
}