#ifndef SYSCALL_H
#define SYSCALL_H

#include <smthng_os.h>

static inline void syscall0(u32 num) {
    __asm__ ("int $0x80" 
             : 
             : "a"(num)
             : "memory", "cc");
}

#endif // SYSCALL_H