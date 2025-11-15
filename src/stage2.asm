[bits 16]
[org 0x7e00]

%macro println 2 ; msg, len
    mov dh, [line_counter] ; row
    mov dl, 0x00 ; col

    mov ah, 0x13
    mov al, 0x01
    mov bh, 0x00
    mov bl, 0x07
    mov cx, %2
    mov bp, %1
    int 0x10

    inc byte [line_counter]
%endmacro

entry:
    println success_message, message_length

    ; enable vesa mode
    ; mov ax, 0x4f02
    ; mov bx, 0x4118
    ; int 0x10

    push ds
    pop es
    mov di, VesaInfoBlockBuffer
    call get_vesa_info

    mov dx, VesaInfoBlockBuffer + 4
    call print_hex16

    ; qemu magic stdout with port 0xe9 (doesnt work on real machines)
    mov al, 'c'
    out 0xe9, al

    cli
    hlt
    jmp $
    hlt

get_vesa_info:
	clc
	mov ax, 0x4f00
	int 0x10
	cmp ax, 0x004f
	jne .failed
    println vesa_info_success, vesa_info_success_length
	ret
.failed:
    println vesa_info_failed, vesa_info_failed_length
    stc
    ret

print_hex16:
    pusha
    mov cx, 4
    mov si, hex_buffer + 3
    mov bx, dx
.convert_loop:
    mov ax, bx
    and ax, 0x000f
    add al, 0x30
    cmp al, 0x39
    jle .store_digit
    add al, 0x07
.store_digit:
    mov [si], al
    shr bx, 4
    dec si
    loop .convert_loop
    
    mov dh, [line_counter] ; row
    mov dl, 0x00           ; col
    mov ah, 0x13
    mov al, 0x01
    mov bh, 0x00
    mov bl, 0x07
    mov cx, 4
    mov bp, hex_buffer
    int 0x10
    
    inc byte [line_counter]
    popa
    ret

success_message: db "Successfully read more sectors!"
message_length equ ($ - success_message)
vesa_info_failed: db "Failed to get vesa info"
vesa_info_failed_length equ ($ - vesa_info_failed)
vesa_info_success: db "Successfully got vesa info"
vesa_info_success_length equ ($ - vesa_info_success)

line_counter: db 0x01
hex_buffer: times 4 db 0

struc VesaInfoBlock	; VesaInfoBlock_size = 512 bytes
	.signature		          resb 4 ; must be 'VESA'
	.version		          resw 1
	.oem_name_ptr		      resd 1
	.capabilities		      resd 1

	.video_modes_offset	      resw 1
	.video_modes_segment      resw 1

	.count_of64k_blocks	      resw 1
	.oem_software_revision    resw 1
	.oem_vendor_name_ptr      resd 1
	.oem_product_name_ptr     resd 1
	.oem_product_revision_ptr resd 1
	.reserved		          resb 222
	.oem_data		          resb 256
endstruc

ALIGN(4)
    VesaInfoBlockBuffer: istruc VesaInfoBlock
        at VesaInfoBlock.signature, db "VESA"
        times 508 db 0
    iend