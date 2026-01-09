#include <smthng_os.h>

u0 panic(void (*func)(const char*, ...), const char* msg, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, msg);
    func(msg, args);
    __builtin_va_end(args);
    __asm__("cli\t\n"
            "hlt\t\n");
    while (true)
        __asm__("hlt");
}