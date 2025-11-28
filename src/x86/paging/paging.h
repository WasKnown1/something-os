#ifndef PAGING_H
#define PAGING_H

#define CR0_PG (1 << 31)

void init_paging(void);
void disable_paging(void);

#define PML4T_ADDR 0x1000
#define PAGE_SIZE 4096
#define PDPT_ADDR 0x2000
#define PD_ADDR 0x3000
#define PT_ADDR 0x4000

#define PT_ADDR_MASK 0xfffff000u
#define PT_PRESENT 0x1
#define PT_READABLE 0x2

void clear_page_tables(void);
void link_first_entries_of_each_table(void);

#define ENTRIES_PER_PT 512
#define ENTRY_SIZE 8

void set_entry_in_page_table(void);

#endif // PAGING_H