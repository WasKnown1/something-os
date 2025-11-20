#ifndef IDT_H
#define IDT_H

#include <stdint.h>

typedef struct idtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_t;

typedef struct {
	uint16_t isr_low;
	uint16_t kernel_cs;
	uint8_t reserved;
	uint8_t attributes;
	uint16_t isr_high;
} __attribute__((packed)) IDTEntry_t;

#define IDT_BASE 0x00
#define IDT_SIZE 0xff

#define IDT_MAX_DESCRIPTORS 256

#endif // IDT_H