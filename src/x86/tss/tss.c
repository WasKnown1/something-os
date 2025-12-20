#include <string.h>
#include <tss.h>
#include <log.h>

static Tss32 tss;

void tss_init(uint32_t kernel_stack_top) {
    memset(&tss, 0, sizeof(Tss32));

    tss.ss0 = 0x10;
    tss.esp0 = kernel_stack_top;
    tss.iomap_base = sizeof(Tss32);

    debug_printf("TSS variable at: 0x%x\n", &tss);
    debug_printf("GDT_START points to: 0x%x\n", GDT_START);
    debug_printf("*GDT_START = 0x%x\n", *GDT_START);

    uint32_t gdt_base = (uint32_t)(*GDT_START);
    debug_printf("GDT base: 0x%x\n", gdt_base);

    gdt_entry_t *tss_desc = (gdt_entry_t *)(gdt_base + GDT_TSS_SELECTOR);
    debug_printf("TSS descriptor at: 0x%x\n", tss_desc);

    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(Tss32) - 1;

    debug_printf("setting TSS base to: 0x%x, limit: 0x%x\n", base, limit);

    tss_desc->limit_low = limit & 0xFFFF;
    tss_desc->base_low = base & 0xFFFF;
    tss_desc->base_mid = (base >> 16) & 0xFF;
    tss_desc->access = 0x89;
    tss_desc->gran = (limit >> 16) & 0x0F;
    tss_desc->base_high = (base >> 24) & 0xFF;

    debug_printf("TSS descriptor contents:\n");
    debug_printf("  limit_low: 0x%x\n", tss_desc->limit_low);
    debug_printf("  base_low: 0x%x\n", tss_desc->base_low);
    debug_printf("  base_mid: 0x%x\n", tss_desc->base_mid);
    debug_printf("  base_high: 0x%x\n", tss_desc->base_high);

    __asm__ ("ltr %w0" : : "r"((uint16_t)GDT_TSS_SELECTOR));

    debug_printf("TSS loaded\n");
}
