#ifndef GDT64_H
#define GDT64_H

void set_gdt64(void);

extern void *gdt_pointer;

#endif // GDT64_H