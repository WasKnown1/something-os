AS = nasm -g -f bin
CC = gcc -c -m32 -Wall -Werror -nostdlib -ffreestanding -nodefaultlibs -mno-red-zone -fno-pic -fno-pie

all: boot stage2 protected_mode
	python helper.py
	qemu-system-i386 -debugcon stdio -no-shutdown -no-reboot -drive format=raw,file=boot.o

boot: src/boot.asm
	$(AS) src/boot.asm -o boot.o

stage2: src/stage2.asm
	$(AS) src/stage2.asm -o stage2.o

protected_mode: src/x86/protected_mode.c
	$(CC) -c src/x86/protected_mode.c -o protected_mode.o
	ld -m elf_i386 -T src/x86/linker.ld protected_mode.o -o protected_mode.elf
	objcopy -O binary protected_mode.elf protected_mode.bin