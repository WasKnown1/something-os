#ifndef DLL_LIB_H
#define DLL_LIB_H

#include <smthng_os.h>

static inline u0 smthng_abi_syscall(u32 syscall_num, u32 ecx, u32 edx) {
    __asm__("int $0x80"
            :
            : "a"(syscall_num), "c"(ecx), "d"(edx)
            : "memory", "cc");
}

#endif // DLL_LIB_H