#ifndef PAGING_H
#define PAGING_H

#include <stdbool.h>
#include <stdint.h>

#define CR0_PG (1 << 31)

void enable_paging(void);
void init_paging_4mb_identity(void);
void init_paging(void);
void map_identity_4mb(uint32_t start, uint32_t length);
void disable_paging(void);

#define PML4T_ADDR 0x1000
#define PAGE_SIZE 4096
#define PDPT_ADDR 0x2000
#define PDT_ADDR 0x3000
#define PT_ADDR 0x4000

#define PT_ADDR_MASK 0xfffff000u
#define PT_PRESENT 0x1
#define PT_READABLE 0x2
#define PDE_PS 0x80

void clear_page_tables(void);
void link_first_entries_of_each_table(void);

#define ENTRIES_PER_PT 512
#define ENTRY_SIZE 8

void set_entry_in_page_table(void);

#define CPUID_GET_FEATURES 0x7
#define CPUID_FEATURE_PML5 (1 << 16)

bool cpu_supports_pml5(void);

#define CR4_LA57 (1 << 12)

void enable_level5_paging(void);

#endif // PAGING_H