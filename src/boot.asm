[bits 16]
[org 0x7c00]

times 10 nop ; i have read that some bios` add sum junk in here idk

_start:
    jmp boot_entry

hello_message: db "Hello, World from 16 bits!" ; , 0x0a
message_length equ ($ - hello_message)

boot_entry:
    push dx
    cld
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti

    mov ax, cs
    mov ds, ax
    mov es, ax

    mov ah, 0x06
    mov al, 0x00
    mov bh, 0x07
    mov cx, 0x0000
    mov dh, 24
    mov dl, 79
    int 0x10

    mov ah, 0x13
    mov al, 0x01
    mov bh, 0x00
    mov bl, 0x07
    mov cx, message_length
    mov dh, 0x00
    mov dl, 0x00
    mov bp, hello_message
    int 0x10

    ; mov ah, 0x02 ; set cursor position
    ; mov bh, 0
    ; mov dh, 1 ; y
    ; mov dl, 0 ; x
    ; int 0x10

    ; pop dx
    ; mov dx, cs
    ; call print_hex16

    mov ah, 0x42 ; newer (still legacy bios though works on uefi emulation of legacy boot)
    mov dl, 0x80
    mov si, dap
    int 0x13

.load_fs: ; have to wait before reading disk again
    mov cx, 6
.retry:
    push cx
    mov ah, 0x42
    mov dl, 0x80
    mov si, dap_fs
    int 0x13
    pop cx
    jnc .done_fs

    xor ah, ah
    mov dl, 0x80
    int 0x13
    
    loop .retry
.done_fs:

    call $+3 ; get ip (instructiom pointer)
    pop dx
    cmp dx, 0x7c00
    jg 0x7e00

    ; call print_hex16

    cli
    hlt
    jmp $
    hlt

dap:
    db 0x10
    db 0x00
    dw 127 ; changed to have more memory
    dd 0x00007e00
    dq 0x01

; so for some reason i have to do this to read non ram momory areas...
dap_fs:
    db 0x10
    db 0x00
    dw 64
    dw 0x0000 
    dw 0x1800
    dq (0x18000 / 512)

times 510 - ($ - $$) db 0