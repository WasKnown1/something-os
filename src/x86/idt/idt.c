#include <idt.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

__attribute__((aligned(0x10))) static IDTEntry_t idt[IDT_MAX_DESCRIPTORS];
static idtr_t idtr;

void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags) {
    IDTEntry_t *descriptor = &idt[vector];

    descriptor->base_lo = (uint32_t)isr & 0xFFFF;
    descriptor->sel = 0x08;
    descriptor->always0 = 0;
    descriptor->flags = flags; 
    descriptor->base_hi = (uint32_t)isr >> 16;
}

static bool vectors[IDT_MAX_DESCRIPTORS];
extern void *isr_stub_table[];

void init_idt() {
    idtr.limit = (sizeof(IDTEntry_t) * IDT_MAX_DESCRIPTORS) - 1;
    idtr.base = (uint32_t)&idt[0];

    memset(&idt, 0, sizeof(IDTEntry_t) * IDT_MAX_DESCRIPTORS);

    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8e);
        vectors[vector] = true;
    }

    __asm__("lidt %0" : : "m"(idtr));
    __asm__("sti");
}