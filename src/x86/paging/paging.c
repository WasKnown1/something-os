#include <paging.h>
#include <stdint.h> 

unsigned int __attribute__((aligned(4096))) page_directory[1024] = {0};
unsigned int __attribute__((aligned(4096))) page_table_0[1024] = {0};

void load_page_directory(unsigned int *pg_dir) {
    __asm__ ("mov %0, %%cr3" : : "r"(pg_dir));
}

void enable_paging(void) {
    unsigned int cr0;
    __asm__ ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= CR0_PG;
    __asm__ ("mov %0, %%cr0" : : "r"(cr0));
}

void init_paging(void) {
    for (unsigned int i = 0; i < 1024; i++) {
        page_directory[i] = 0x00000002;
    }

    for (unsigned int i = 0; i < 1024; i++) {
        page_table_0[i] = (i * 0x1000) | 3; 
    }

    page_directory[0] = ((unsigned int)page_table_0) | 3;

    load_page_directory(page_directory);
    enable_paging();
}

void disable_paging(void) {
    unsigned int cr0;
    __asm__ ("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~CR0_PG;
    __asm__ ("mov %0, %%cr0" : : "r"(cr0));
}

void clear_page_tables(void) {
    __asm__(
        "cld\n\t"
        "rep stosb\n\t"
        :
        : "D"(PML4T_ADDR),
          "a"(0),
          "c"(PAGE_SIZE / 4)
        : "memory");
}

void link_first_entries_of_each_table(void) {
    uint32_t *pml4 = (uint32_t *)PML4T_ADDR;
    uint32_t *pdpt = (uint32_t *)PDPT_ADDR;
    uint32_t *pdt = (uint32_t *)PD_ADDR;

    pml4[0] = (PDPT_ADDR & PT_ADDR_MASK) | PT_PRESENT | PT_READABLE;
    pdpt[0] = (PD_ADDR & PT_ADDR_MASK) | PT_PRESENT | PT_READABLE;
    pdt[0] = (PT_ADDR & PT_ADDR_MASK) | PT_PRESENT | PT_READABLE;
}

void set_entry_in_page_table(void) {
    uint32_t *pt = (uint32_t*)PT_ADDR;

    uint32_t entry = PT_PRESENT | PT_READABLE;

    for (int i = 0; i < ENTRIES_PER_PT; i++) {
        pt[(i * 2) + 0] = entry;
        pt[(i * 2) + 1] = 0;
        entry += PAGE_SIZE;
    }
}