#include <compatibility_mode.h>
#include <stdint.h>

void enable_compatibility_mode(void) {
    uint32_t eax;
    __asm__("rdmsr\n\t"
            : "=a"(eax)
            : "c"(EFER_MSR)
    );

    eax |= EFER_LM_ENABLE;
    eax |= EFER_LM_ENABLE;
    __asm__("wrmsr\n\t"
            :
            : "a"(eax), "c"(EFER_MSR)
    );
}
