/* this is a dummy linker script, dont be fooled though it is used just after replacing the # symbol with the starting address */
OUTPUT_FORMAT(elf64-x86-64)
/* OUTPUT_ARCH(x86_64) */
ENTRY(entry64)

SECTIONS 
{
    . = ##;
    
    .text ## : AT(##) {
        KEEP(*(.entry64))
        *(.text)
        *(.text.*)
    }
    
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }
    
    .data : {
        *(.data)
        *(.data.*)
    }
    
    .bss : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    }
    
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note*)
    }

    __kernel_end = .;
}