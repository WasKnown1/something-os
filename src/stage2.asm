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

%macro hang 0
    cli
    hlt
    jmp $
    hlt
%endmacro

entry:
    println success_message, message_length

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

    call detect_memory

    ; enable vesa mode 1280×1024 24-bit
    ; mov ax, 0x4f02
    ; mov bx, 0x411b
    ; int 0x10

    ; |-------------------------------------|
    ; |                                     |
    ; |      setting up protected mode      |
    ; |                                     |
    ; |-------------------------------------|
    cli

    xor ax, ax
    mov ds, ax
    lgdt [gdt_desc]

    ; enable a20 line
    in al, 0x93
    or al, 2
    and al, ~1
    out 0x92, al

    mov eax, cr0
    or eax, 0x01
    mov cr0, eax
    jmp 0x08:clear_pipe

    hang

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

detect_memory:
    pusha
    push es
    xor ax, ax
    mov es, ax

    mov di, memory_map_address
    xor ebx, ebx            ; eax = 0 to start
    xor bp, bp              ; bp = entry count
    mov edx, 0x534D4150     ; 'SMAP' magic
    mov eax, 0xE820
    mov ecx, 24             ; ask for 24 bytes
    int 0x15
    
    jc .failed              ; CF set = not supported
    cmp eax, 0x534D4150     ; eax should be 'SMAP'
    jne .failed
    test ebx, ebx           ; ebx = 0 means 1 entry (unlikely)
    je .failed
    jmp .process_entry
.loop:
    mov eax, 0xE820
    mov ecx, 24
    int 0x15
    jc .done                ; CF set = end of list
.process_entry:
    ; entry  at ES:DI
    add di, 24
    inc bp
    test ebx, ebx
    jne .loop
.done:
    mov [memory_map_entries], bp
    pop es
    popa

    clc
    ret
.failed:
    stc
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
	.mode_attributes		  resw 1
	.first_window_attributes  resb 1
	.second_window_attributes resb 1
	.window_granularity	      resw 1 ;	in KB
	.window_size		      resw 1 ;	in KB
	.first_window_segment	  resw 1 ;	0 if not supported
	.second_window_segment	  resw 1 ;	0 if not supported
	.window_function_ptr	  resd 1
	.bytes_per_scan_line	  resw 1

	.width			          resw 1 ;	in pixels(graphics)/columns(text)
	.height			          resw 1 ;	in pixels(graphics)/columns(text)
	.char_width		          resb 1 ;	in pixels
	.char_height		      resb 1 ;	in pixels
	.planes_count		      resb 1
	.bits_per_pixel		      resb 1
	.banks_count		      resb 1
	.memory_model		      resb 1 ;	http://www.ctyme.com/intr/rb-0274.htm#Table82
	.bank_size		          resb 1 ;	in KB
	.image_pages_count	      resb 1 ;	count - 1
	.reserved1		          resb 1 ;	equals 0 in Revision 1.0-2.0, 1 in 3.0

	.red_mask_size		      resb 1
	.red_field_position	      resb 1
	.green_mask_size		  resb 1
	.green_field_position	  resb 1
	.blue_mask_size		      resb 1
	.blue_field_position	  resb 1
	.reserved_mask_size	      resb 1
	.reserved_mask_position	  resb 1
	.direct_color_mode_info	  resb 1

	.lfb_address		      resd 1
	.off_screen_memory_offset resd 1
	.off_screen_memory_size	  resw 1   ;	in KB
	.reserved2		          resb 206 ;	available in Revision 3.0, but useless for now
endstruc

ALIGN(4)
    VesaModeInfoBlockBuffer: istruc VesaModeInfoBlock
		times 256 db 0
	iend

struc E820Entry
    .base resq 1
    .length resq 1
    .type resd 1
    .acpi resd 1
endstruc

gdt_start:
    dq 0
gdt_code:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00
gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00
gdt_end:
gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 1500 - ($ - $$) db 0
memory_map_entries: dw 0
memory_map_address:
times 24*128 db 0 ; space for 128 e820 entries

[bits 32]
clear_pipe:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; qemu magic stdout with port 0xe9 (doesnt work on real machines)
    ; mov al, 'c'
    ; out 0xe9, al

    ; hang