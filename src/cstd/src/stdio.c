#include <stdio.h>

void snprintf(char *buffer, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    size_t written = 0;

    while (*format && written < size - 1) {
        if (*format == '%') {
            format++;
            if (*format == 'd') {
                int num = va_arg(args, int);
                char num_buf[12];
                int num_i = 0;
                if (num < 0) {
                    if (written < size - 1) {
                        buffer[written++] = '-';
                    }
                    num = -num;
                }
                if (num == 0) {
                    num_buf[num_i++] = '0';
                } else {
                    while (num > 0) {
                        num_buf[num_i++] = (num % 10) + '0';
                        num /= 10;
                    }
                }
                for (int j = num_i - 1; j >= 0 && written < size - 1; j--) {
                    buffer[written++] = num_buf[j];
                }
            } else if (*format == 's') {
                char *s = va_arg(args, char *);
                while (*s && written < size - 1) {
                    buffer[written++] = *s++;
                }
            } else if (*format == 'c') {
                char c = (char)va_arg(args, int);
                if (written < size - 1) {
                    buffer[written++] = c;
                }
            } else if (*format == 'x') {
                unsigned int num = va_arg(args, unsigned int);
                char hex_buf[9];
                hex_buf[8] = '\0';
                for (int i = 7; i >= 0; i--) {
                    int digit = num & 0xF;
                    if (digit < 10) {
                        hex_buf[i] = '0' + digit;
                    } else {
                        hex_buf[i] = 'a' + (digit - 10);
                    }
                    num >>= 4;
                }
                for (int j = 0; j < 8 && written < size - 1; j++) {
                    buffer[written++] = hex_buf[j];
                }
            } else if (*format == '%') {
                if (written < size - 1) {
                    buffer[written++] = '%';
                }
            } else if (*format == 'u') {
                unsigned int num = va_arg(args, unsigned int);
                char num_buf[11];
                int num_i = 0;
                if (num == 0) {
                    num_buf[num_i++] = '0';
                } else {
                    while (num > 0) {
                        num_buf[num_i++] = (num % 10) + '0';
                        num /= 10;
                    }
                }
                for (int j = num_i - 1; j >= 0 && written < size - 1; j--) {
                    buffer[written++] = num_buf[j];
                }
            } else if (*format == 'p') {
                void *ptr = va_arg(args, void *);
                unsigned long addr = (unsigned long)ptr;
                char hex_buf[17];
                hex_buf[16] = '\0';
                for (int i = 15; i >= 0; i--) {
                    int digit = addr & 0xF;
                    if (digit < 10) {
                        hex_buf[i] = '0' + digit;
                    } else {
                        hex_buf[i] = 'a' + (digit - 10);
                    }
                    addr >>= 4;
                }
                if (written < size - 1) {
                    buffer[written++] = '0';
                }
                if (written < size - 1) {
                    buffer[written++] = 'x';
                }
                for (int j = 0; j < 16 && written < size - 1; j++) {
                    buffer[written++] = hex_buf[j];
                }
            }
        } else {
            buffer[written++] = *format;
        }
        format++;
    }
    buffer[written] = '\0';
    va_end(args);
}