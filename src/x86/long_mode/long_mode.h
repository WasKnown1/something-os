#ifndef LONG_MODE_H
#define LONG_MODE_H

#include <stdbool.h>

#define CPUID_EXTENSIONS   0x80000000
#define CPUID_EXT_FEATURES 0x80000001

bool long_mode_support(void);

#endif // LONG_MODE_H