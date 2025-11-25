#include <stdarg.h>
#ifdef QEMU_DEBUG
#include <qemu_log.h>
#define debug_log(str) qemu_log(str)
#define debug_log_n(str, n) qemu_log_n(str, n)
#endif

void log_num(int n) {
    char buf[12];
    int i = 0;
    if (n < 0) {
        debug_log("-");
        n = -n;
    }
    if (n == 0) {
        debug_log("0");
        return;
    }

    while (n > 0) {
        buf[i++] = (n % 10) + '0';
        n /= 10;
    }
    buf[i] = '\0';

    for (int j = 0; j < i / 2; j++) {
        char temp = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = temp;
    }
    debug_log(buf);
}

int debug_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int count = 0;

    while (*format) {
        if (*format == '%') {
            format++;
            if (*format == 'd') {
                int num = va_arg(args, int);
                log_num(num); 
            } else if (*format == 's') {
                char *s = va_arg(args, char *);
                debug_log(s);
            } else if (*format == 'c') {
                char c = (char)va_arg(args, int);
                debug_log_n(&c, 1);
                count++;
            } else if (*format == 'x') {
                unsigned int num = va_arg(args, unsigned int);
                char buf[9];
                buf[8] = '\0';
                for (int i = 7; i >= 0; i--) {
                    int digit = num & 0xF;
                    if (digit < 10) {
                        buf[i] = '0' + digit;
                    } else {
                        buf[i] = 'a' + (digit - 10);
                    }
                    num >>= 4;
                }
                debug_log(buf);
            } else if (*format == 'p') {
                uintptr_t ptr = (uintptr_t)va_arg(args, void *);
                char buf[9];
                buf[0] = '0';
                buf[1] = 'x';
                buf[10] = '\0';
                for (int i = 9; i >= 2; i--) {
                    int digit = ptr & 0xF;
                    if (digit < 10) {
                        buf[i] = '0' + digit;
                    } else {
                        buf[i] = 'a' + (digit - 10);
                    }
                    ptr >>= 4;
                }
                debug_log(buf);
            } else {
                debug_log_n(format, 1);
                count++;
            }
        } else {
            debug_log_n(format, 1);
            count++;
        }
        format++;
    }
    va_end(args);
    return count;
}