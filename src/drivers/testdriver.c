__declspec(dllimport) void kill_process(void *handle, unsigned char status);

int main(void) {
    // __asm__(
    //     "mov $-1, %ecx\n\t"
    //     "mov $150, %edx\n\t"
    //     "mov $0x2C, %eax\n\t"
    //     "int $0x2e");
    // kill_process((void *)-1, 150);
    return 0x67;
}