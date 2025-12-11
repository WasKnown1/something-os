#ifndef QEMU_LOG_H
#define QEMU_LOG_H

#include <stdint.h>

void qemu_log(const char *str);
void qemu_log_n(const char *str, uint16_t n);
void qemu_log_char(uint8_t c);

#endif // QEMU_LOG_H