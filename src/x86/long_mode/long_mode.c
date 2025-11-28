#include <long_mode.h>
#include <stdint.h>

bool extended_functions_cpuid_support(void) {
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

bool cpu_supports_long_mode(void) {
    uint32_t edx;
    if (extended_functions_cpuid_support() == false) {
        return false;
    }

    __asm__("cpuid"
            : "=d"(edx)
            : "a"(CPUID_EXT_FEATURES));

    if (edx & CPUID_EDX_EXT_FEAT_ML) {
        return true;
    }
    return false;
}