#include <long_mode.h>
#include <stdint.h>

bool long_mode_support(void) {
    uint32_t eax, edx;
    
    __asm__ ("cpuid"
             : "=a"(eax)
             : "a"(CPUID_EXTENSIONS));
    
    if (eax < CPUID_EXT_FEATURES)
        return false;
    
    __asm__("cpuid"
            : "=d"(edx)
            : "a"(CPUID_EXT_FEATURES));
    
    return (edx & (1 << 29)) != 0;
}