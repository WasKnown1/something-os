unsigned int __attribute__((aligned(4096))) page_directory[1024];
unsigned int __attribute__((aligned(4096))) page_table_0[1024];

void load_page_directory(unsigned int *pg_dir) {
    __asm__ ("mov %0, %%cr3" : : "r"(pg_dir));
}

void enable_paging(void) {
    unsigned int cr0;
    __asm__ ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
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