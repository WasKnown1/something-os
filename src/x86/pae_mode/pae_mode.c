#include <stdint.h>
#include <stdbool.h>

bool cpu_supports_pae(void) {
    uint32_t edx;
    __asm__("mov $1, %%eax\n\t"
            "cpuid\n\t"
            : "=d" (edx));
    return (edx & (1 << 6)) != 0;
}