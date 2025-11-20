#include <idt.h>
#include <stdbool.h>

__attribute__((aligned(0x10))) static IDTEntry_t idt[IDT_MAX_DESCRIPTORS];
static __attribute__((unused)) idtr_t idtr;

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    IDTEntry_t *descriptor = &idt[vector];
    descriptor->isr_low        = (uint32_t)isr & 0xFFFF;
    descriptor->kernel_cs      = 0x08;
    descriptor->attributes     = flags;
    descriptor->isr_high       = (uint32_t)isr >> 16;
    descriptor->reserved       = 0;
}

static bool vectors[IDT_MAX_DESCRIPTORS];

__asm__(
    ".globl isr_stub_table\n\t"
    "isr_stub_table:\n\t"
    ".long isr_stub_0\n\t"
    ".long isr_stub_1\n\t"
    ".long isr_stub_2\n\t"
    ".long isr_stub_3\n\t"
    ".long isr_stub_4\n\t"
    ".long isr_stub_5\n\t"
    ".long isr_stub_6\n\t"
    ".long isr_stub_7\n\t"
    ".long isr_stub_8\n\t"
    ".long isr_stub_9\n\t"
    ".long isr_stub_10\n\t"
    ".long isr_stub_11\n\t"
    ".long isr_stub_12\n\t"
    ".long isr_stub_13\n\t"
    ".long isr_stub_14\n\t"
    ".long isr_stub_15\n\t"
    ".long isr_stub_16\n\t"
    ".long isr_stub_17\n\t"
    ".long isr_stub_18\n\t"
    ".long isr_stub_19\n\t"
    ".long isr_stub_20\n\t"
    ".long isr_stub_21\n\t"
    ".long isr_stub_22\n\t"
    ".long isr_stub_23\n\t"
    ".long isr_stub_24\n\t"
    ".long isr_stub_25\n\t"
    ".long isr_stub_26\n\t"
    ".long isr_stub_27\n\t"
    ".long isr_stub_28\n\t"
    ".long isr_stub_29\n\t"
    ".long isr_stub_30\n\t"
    ".long isr_stub_31\n\t"
);

extern void* isr_stub_table[];

void idt_init() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(IDTEntry_t) * IDT_MAX_DESCRIPTORS - 1;

    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }

    __asm__ ("lidt %0" : : "m"(idtr));
    __asm__ ("sti");
}