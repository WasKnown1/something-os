#ifndef LONG_MODE_H
#define LONG_MODE_H

#include <stdbool.h>
#include <stdint.h>

#define CPUID_EXTENSIONS   0x80000000
#define CPUID_EXT_FEATURES 0x80000001

#define CPUID_EDX_EXT_FEAT_ML (1 << 29)

bool extended_functions_cpuid_support(void);
bool cpu_supports_long_mode(void);

extern uint32_t __kernel_end;

void jump_to_long_mode(void);

#endif // LONG_MODE_H