#include <smthng_os.h>
#include <log.h>

u0 syscall_handler(u0) {
    u32 syscall_num;
    __asm__("" : "=a"(syscall_num));

    debug_printf("syscall %u called!\n", syscall_num);

    switch (syscall_num) {
    case 0:
        debug_printf("syscall 0: print\n");
        break;
    case 1:
        debug_printf("syscall 1: exit\n");
        break;
    default:
        debug_printf("unknown syscall: %u\n", syscall_num);
        break;
    }
}