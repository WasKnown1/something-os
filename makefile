AS = nasm -f bin 

all: boot
	python helper.py
	qemu-system-i386 -drive format=raw,file=boot.o

boot: src/boot.asm
	$(AS) src/boot.asm -o boot.o