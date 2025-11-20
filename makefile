AS = nasm -g -f bin
CC = gcc -c -m32 -Wall -Werror -nostdlib -ffreestanding -nodefaultlibs -mno-red-zone -fno-pic -fno-pie
LD = ld -m elf_i386 -T src/x86/linker.ld
OBJ_RAW = objcopy --set-section-flags .bss=alloc,load,contents --set-section-flags .data=alloc,load,contents -O binary 

X86_C_src = $(shell find "src/x86/" -type f -name "*.c" ! -name "protected_mode.c")
X86_C_dir = $(shell find "src" -type d)
X86_C_inc = $(addprefix -I,$(X86_C_dir))

all: boot stage2 protected_mode
	python helper.py
	qemu-system-i386 -debugcon stdio -no-shutdown -no-reboot -d int -drive format=raw,file=boot.o

boot: src/boot.asm
	$(AS) src/boot.asm -o boot.o

stage2: src/stage2.asm
	$(AS) src/stage2.asm -o stage2.o

protected_mode: src/x86/protected_mode.c
	echo $(X86_C_inc)
	$(CC) -c src/x86/protected_mode.c -o protected_mode.o $(X86_C_inc)
	@for file in $(X86_C_src); do \
		basename=$$(basename $$file .c); \
		echo "Compiling $$file -> build/$$basename.o"; \
		$(CC) -c $$file -o $$basename.o $(X86_C_inc) || exit 1; \
	done
	$(LD) protected_mode.o paging.o -o protected_mode.elf
	$(OBJ_RAW) protected_mode.elf protected_mode.bin

run:
	qemu-system-i386 -debugcon stdio -no-shutdown -no-reboot -d int -drive format=raw,file=boot.o

clean:
	mv boot.o src/boot.o 
	rm *.o 
	rm *.elf 
	rm *.bin 
	mv src/boot.o boot.o