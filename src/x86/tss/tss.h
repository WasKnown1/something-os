#ifndef TSS_H
#define TSS_H

#include <stdint.h>
#define GDT_START (void **)(0x7e00 + 1500)
#define GDT_TSS_INDEX 3
#define GDT_TSS_SELECTOR (uint16_t)(GDT_TSS_INDEX << 3)

typedef struct {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed)) Tss32;

typedef struct __attribute__((packed)) {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  gran;
    uint8_t  base_high;
} gdt_entry_t;

void tss_init(uint32_t kernel_stack_top);

#endif // TSS_H