__attribute__((section(".entry64"))) void entry64(void)  {
    __asm__("cli\n\t"
            "mov $0x10, %ax\n\t"
            "mov %ax, %ds\n\t"
            "mov %ax, %es\n\t"
            "mov %ax, %fs\n\t"
            "mov %ax, %gs\n\t"
            "mov %ax, %ss\n\t"
    );

    for (int i = 0; i < 5; i++) {
        __asm__("movb $'c', %al\n\t"
                "outb %al, $0xe9\n\t");
    }

    __asm__(
        "cli\n\t"
        "hlt\n\t"
    );

    while (1){
        __asm__("hlt\n\t");
    }
}