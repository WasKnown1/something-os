OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
ENTRY(entry)

SECTIONS 
{
    . = 0x0;
    
    .text 0x0 : AT(0x0) {
        KEEP(*(.entry))
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
}