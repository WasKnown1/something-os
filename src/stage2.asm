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

    mov dx, [VesaInfoBlockBuffer + 4]
    call print_hex16

    push word [VesaInfoBlockBuffer + VesaInfoBlock.video_modes_segment]
    pop es
    mov di, VesaModeInfoBlockBuffer
    ; mov bx, [VesaInfoBlockBuffer + VesaInfoBlock.video_modes_offset] ; not need since i dont use the offset as the mode
    mov cx, 0x011b ; get vesa mode info for 1280×1024 24-bit
    call get_vesa_mode_info

    mov dx, word [VesaModeInfoBlockBuffer + 18] ; width
    call print_hex16
    mov dx, word [VesaModeInfoBlockBuffer + 20] ; height
    call print_hex16

    movzx dx, byte [VesaModeInfoBlockBuffer + 25] ; bits_per_pixel
    call print_hex16

    ; qemu magic stdout with port 0xe9 (doesnt work on real machines)
    mov al, 'c'
    out 0xe9, al

hang:
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

get_vesa_mode_info:
	clc
	mov ax, 0x4f01
	int 0x10
	cmp ax, 0x004f
	jne .failed
    println vesa_mode_info_success, vesa_mode_info_success_length
	ret
.failed:
    println vesa_mode_info_failed, vesa_mode_info_failed_length
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
vesa_mode_info_failed: db "Failed to get vesa mode info"
vesa_mode_info_failed_length equ ($ - vesa_mode_info_failed)
vesa_mode_info_success: db "Successfully got vesa mode info"
vesa_mode_info_success_length equ ($ - vesa_mode_info_success)

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

struc VesaModeInfoBlock	;	VesaModeInfoBlock_size = 256 bytes
	.ModeAttributes		    resw 1
	.FirstWindowAttributes	resb 1
	.SecondWindowAttributes	resb 1
	.WindowGranularity	    resw 1 ;	in KB
	.WindowSize		        resw 1 ;	in KB
	.FirstWindowSegment	    resw 1 ;	0 if not supported
	.SecondWindowSegment	resw 1 ;	0 if not supported
	.WindowFunctionPtr	    resd 1
	.BytesPerScanLine	    resw 1

	; Added in Revision 1.2
	.Width			        resw 1 ;	in pixels(graphics)/columns(text)
	.Height			        resw 1 ;	in pixels(graphics)/columns(text)
	.CharWidth		        resb 1 ;	in pixels
	.CharHeight		        resb 1 ;	in pixels
	.PlanesCount		    resb 1
	.BitsPerPixel		    resb 1
	.BanksCount		        resb 1
	.MemoryModel		    resb 1 ;	http://www.ctyme.com/intr/rb-0274.htm#Table82
	.BankSize		        resb 1 ;	in KB
	.ImagePagesCount	    resb 1 ;	count - 1
	.Reserved1		        resb 1 ;	equals 0 in Revision 1.0-2.0, 1 in 3.0

	.RedMaskSize		    resb 1
	.RedFieldPosition	    resb 1
	.GreenMaskSize		    resb 1
	.GreenFieldPosition	    resb 1
	.BlueMaskSize		    resb 1
	.BlueFieldPosition	    resb 1
	.ReservedMaskSize	    resb 1
	.ReservedMaskPosition	resb 1
	.DirectColorModeInfo	resb 1

	;	Added in Revision 2.0
	.LFBAddress		        resd 1
	.OffscreenMemoryOffset	resd 1
	.OffscreenMemorySize	resw 1   ;	in KB
	.Reserved2		        resb 206 ;	available in Revision 3.0, but useless for now
endstruc

ALIGN(4)
    VesaModeInfoBlockBuffer: istruc VesaModeInfoBlock
		times 256 db 0
	iend