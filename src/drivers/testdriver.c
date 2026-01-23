int main(void) {
    __asm__(
        "mov $-1, %ecx\n\t"
        "mov $150, %edx\n\t"
        "mov $0x2C, %eax\n\t"
        "int $0x2e");
    return 0;
}