#ifndef COMPATIBILITY_MODE_H
#define COMPATIBILITY_MODE_H

#define EFER_MSR 0xc0000080
#define EFER_LM_ENABLE (1 << 8)

void enable_compatibility_mode(void);

#endif // COMPATIBILITY_MODE_H