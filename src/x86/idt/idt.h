#ifndef IDT_H
#define IDT_H

#include <stdint.h>

typedef struct idtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_t;

typedef struct {
	uint16_t base_lo;
	uint16_t sel; /* kernel segment goes here*/
	uint8_t always0; 
	uint8_t flags;
	uint16_t base_hi;
} __attribute__((packed)) IDTEntry_t;

#define IDT_MAX_DESCRIPTORS 256

void init_idt();

#endif // IDT_H