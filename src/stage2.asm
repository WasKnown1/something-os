[bits 16]
[org 0x7e00]

entry: 
    mov ah, 0x13
    mov al, 0x01
    mov bh, 0x00
    mov bl, 0x07
    mov cx, message_length
    mov dh, 0x01 ; row
    mov dl, 0x00 ; col
    mov bp, success_message
    int 0x10

    cli
    hlt
    jmp $
    hlt

success_message: db "Successfully read more sectors!"
message_length equ ($ - success_message)