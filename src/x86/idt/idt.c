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

__attribute__((unused)) static bool vectors[IDT_MAX_DESCRIPTORS];

extern void* isr_stub_table[];

void init_idt() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(IDTEntry_t) * IDT_MAX_DESCRIPTORS - 1;

    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }

    __asm__ ("lidt %0" : : "m"(idtr));
    __asm__ ("sti");
}