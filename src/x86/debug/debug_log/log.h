#ifndef LOG_H
#define LOG_H

#ifdef QEMU_DEBUG
#include <qemu_log.h>
#define debug_log(str) qemu_log(str)
#define debug_log_n(str, n) qemu_log_n(str, n)
#endif

void log_num(int n);
int debug_printf(const char *format, ...);

#endif // LOG_H