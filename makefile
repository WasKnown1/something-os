AS = nasm -f bin 

all: boot stage2
	python helper.py
	qemu-system-i386 -drive format=raw,file=boot.o

boot: src/boot.asm
	$(AS) src/boot.asm -o boot.o

stage2: src/stage2.asm
	$(AS) src/stage2.asm -o stage2.o