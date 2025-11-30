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

void enable_huge_paging(void) {
    uint32_t cr4;
    __asm__("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= 0x10;
    __asm__("mov %0, %%cr4" ::"r"(cr4));

    __asm__("mov %0, %%cr3" ::"r"(PDPT_ADDR));

    uint32_t cr0;
    __asm__("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= CR0_PG;
    __asm__("mov %0, %%cr0" ::"r"(cr0));
}

void init_paging_4mb_identity(void) {
    uint32_t *pd = (uint32_t *)PDPT_ADDR;
    for (int i = 0; i < 1024; i++)
        pd[i] = 0;

    pd[0] = (0x00000000 & 0xffc00000) | PT_PRESENT | PT_READABLE | PDE_PS;
    enable_huge_paging();
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

void map_identity_4mb(uint32_t start, uint32_t length) {
    uint32_t *pd = (uint32_t *)PDPT_ADDR;

    uint32_t addr = start;
    uint32_t end  = start + length;

    while (addr < end) {
        uint32_t index = addr >> 22;
        pd[index] = (addr & 0xFFC00000) | 0x083;
        addr += 0x400000;
    }
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
    uint32_t *pdt = (uint32_t *)PDT_ADDR;

    pml4[0] = (PDPT_ADDR & PT_ADDR_MASK) | PT_PRESENT | PT_READABLE;
    pdpt[0] = (PDT_ADDR & PT_ADDR_MASK) | PT_PRESENT | PT_READABLE;
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

bool cpu_supports_pml5(void) {
    uint32_t ecx;
    __asm__("cpuid\n\t"
            : "=c"(ecx)
            : "c"(CPUID_GET_FEATURES),
              "a"(0)
            :);
    return (ecx & CPUID_FEATURE_PML5) != 0;
}

void enable_level5_paging(void) {
    uint32_t cr4;
    __asm__("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= CR4_LA57;
    __asm__("mov %0, %%cr4" : : "r"(cr4));
}