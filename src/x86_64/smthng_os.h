#ifndef SMTHNG_OS_H
#define SMTHNG_OS_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

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

typedef uintptr_t uptr;

typedef struct Node {
    u0 *data;
    struct Node *next;
} Node;

static inline b8 node_has_next(Node *node_ptr) {
    return node_ptr->next != NULL;
}

static inline Node *get_last_node(Node *head) {
    Node *current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    return current;
}

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

#define cli() __asm__("cli")
#define sti() __asm__("sti")

#define PE_HEADER2_OPTIONAL_HEADER_ENTRY_EXPORT 0
#define PE_HEADER2_OPTIONAL_HEADER_ENTRY_IMPORT 1
#define PE_HEADER_SIZEOF_SHORT_NAME 8
#define OFFSET_OF_FIELD(type, field) ((u32) __builtin_offsetof(type, field))
#define PEHEADER_ORDINAL_FLAG32 0x80000000

b8 compare_ending(const i8 *string, const i8 *ending);

#endif // SMTHNG_OS_H