[bits 16]
[org 0x7c00]

_start:
    jmp boot_entry

hello_message_start:
    db "Hello, World from 16 bits!"
hello_message_end:

boot_entry:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti

    mov ah, 0x13
    mov al, 0x01
    mov bh, 0x00
    mov bl, 0x07
    mov cx, (hello_message_end - hello_message_start)
    mov dh, 0x00
    mov dl, 0x00
    mov bp, hello_message_start
    int 0x10

    cli
    hlt
    jmp $
    hlt

times 510 - ($ - $$) db 0