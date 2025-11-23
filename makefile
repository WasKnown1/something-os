NASM = nasm -g -f bin
# AS = as --32 -march i386
CC = gcc -c -m32 -Wall -Werror -nostdlib -ffreestanding -nodefaultlibs -mno-red-zone -fno-pic -fno-pie -D QEMU_DEBUG
LD = ld -m elf_i386 -T src/x86/linker.ld
OBJ_RAW = objcopy --set-section-flags .bss=alloc,load,contents --set-section-flags .data=alloc,load,contents -O binary 

X86_C_src = $(shell find "src/x86/" -type f -name "*.c" ! -name "protected_mode.c")
X86_ASM_src = $(shell find "src/x86/" -type f -name "*.asm")
X86_C_dir = $(shell find "src" -type d)
X86_C_inc = $(addprefix -I,$(X86_C_dir))

all: boot stage2 protected_mode python
	qemu-system-x86_64 -debugcon stdio -no-shutdown -no-reboot -d int -drive format=raw,file=boot.o

boot: src/boot.asm
	$(NASM) src/boot.asm -o boot.o

stage2: src/stage2.asm
	$(NASM) src/stage2.asm -o stage2.o

python:
	python helper.py

protected_mode: src/x86/protected_mode.c
	echo $(X86_C_inc)
	$(CC) -c src/x86/protected_mode.c -o protected_mode.o $(X86_C_inc)
	@for file in $(X86_C_src); do \
		basename=$$(basename $$file .c); \
		echo "Compiling $$file -> $$basename.o"; \
		$(CC) -c $$file -o $$basename.o $(X86_C_inc) || exit 1; \
	done
	@for file in $(X86_ASM_src); do \
		basename=$$(basename $$file .asm); \
		echo "Compiling $$file -> $$basename.o"; \
		nasm -f elf32 $$file -o $$basename.o || exit 1; \
	done
	$(eval X86_O_src = $(shell find "./" -type f -name "*.o" ! -name "boot.o" ! -name "stage2.o" ! -name "protected_mode.o"))
# the protected_mode.o file always has to be the first file to be linked (idk why but setting entry = 0x0; doesnt do anything so i have to force link it to be the first argument so the entry function is the first function it sees)
	$(LD) protected_mode.o $(X86_O_src) -o protected_mode.elf
	$(OBJ_RAW) protected_mode.elf protected_mode.bin

run:
	qemu-system-x86_64 -debugcon stdio -no-shutdown -no-reboot -d int -drive format=raw,file=boot.o

clean:
	mv boot.o src/boot.o 
	rm *.o 
	rm *.elf 
	rm *.bin 
	mv src/boot.o boot.o

clean_all:
	rm *.o 
	rm *.elf 
	rm *.bin 