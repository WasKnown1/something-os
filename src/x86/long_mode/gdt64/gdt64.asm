[bits 32]

PRESENT   equ 1 << 7
NOT_SYS   equ 1 << 4
EXEC      equ 1 << 3
DC        equ 1 << 2
RW        equ 1 << 1
ACCESSED  equ 1 << 0

GRAN_4K   equ 1 << 7
SIZE_32   equ 1 << 6
LONG_MODE equ 1 << 5

global gdt_pointer

section .data
gdt:
.null: equ $ - gdt
    dq 0
.code: equ $ - gdt
    .code.limit_lo: dw 0xffff
    .code.base_lo:  dw 0
    .code.base_mid: db 0
    .code.access:   db PRESENT | NOT_SYS | EXEC | RW
    .code.flags:    db GRAN_4K | LONG_MODE | 0xf   ; Flags & Limit (high, bits 16-19)
    .code.base_hi:  db 0
.data: equ $ - gdt
    .data.limit_lo: dw 0xffff
    .data.base_lo:  dw 0
    .data.base_mid: db 0
    .data.access:   db PRESENT | NOT_SYS | RW
    .data.flags:    db GRAN_4K | SIZE_32 | 0xf       ; Flags & Limit (high, bits 16-19)
    .data.base_hi:  db 0
gdt_pointer:
    dw $ - gdt - 1
    dq gdt