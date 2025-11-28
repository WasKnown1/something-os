#include <stdint.h>
#include <stdbool.h>
#include <pae_mode.h>

bool cpu_supports_pae(void) {
    uint32_t edx;
    __asm__("mov $1, %%eax\n\t"
            "cpuid\n\t"
            : "=d" (edx));
    return (edx & (1 << 6)) != 0;
}

void enable_pae_mode(void) {
    if (!cpu_supports_pae())
        return;

    uint32_t cr4;
    __asm__("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= CR4_PAE_ENABLE;
    __asm__("mov %0, %%cr4" : : "r"(cr4));
}