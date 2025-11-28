#ifndef PAE_MODE_H
#define PAE_MODE_H

#include <stdbool.h>

bool cpu_supports_pae(void);

#define CR4_PAE_ENABLE (1 << 5)

void enable_pae_mode(void);

#endif // PAE_MODE_H