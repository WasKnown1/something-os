#ifndef SMTHNG_OS_H
#define SMTHNG_OS_H

#include <stdbool.h>

typedef float f32;
typedef double f64;
typedef void u0;

typedef char i8;
typedef unsigned char u8;
typedef short i16;
typedef unsigned short u16;
typedef int i32;
typedef unsigned int u32;
typedef long long i64;
typedef unsigned long long u64;

typedef unsigned char b8;
typedef unsigned short b16;
typedef unsigned int b32;
typedef unsigned long long b64;

#define Result(res_type, stts_type) struct { \
    stts_type status; \
    res_type value; \
}

#define Pair(type1, type2) \
    struct                 \
    {                      \
        type1 arg1;        \
        type2 arg2;        \
    }

#define panic(func, msg, ...)     \
    do                            \
    {                             \
        func(msg, ##__VA_ARGS__); \
        __asm__("cli\t\n"         \
                "hlt\t\n");       \
        while (true)              \
            __asm__("hlt");       \
    } while (true);

#endif // SMTHNG_OS_H