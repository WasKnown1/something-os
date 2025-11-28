NASM = nasm -g -f bin
# AS = as --32 -march i386
CC32 = gcc -c -m32 -Wall -Werror -nostdlib -ffreestanding -nodefaultlibs -mno-red-zone -fno-pic -fno-pie -D QEMU_DEBUG
CC64 = gcc -c -m64 -Wall -Werror -nostdlib -ffreestanding -nodefaultlibs -mno-red-zone -fno-pic -fno-pie -D QEMU_DEBUG
LD = ld -m elf_x86_64
OBJ_RAW = objcopy --set-section-flags .bss=alloc,load,contents --set-section-flags .data=alloc,load,contents -O binary 

X86_C_SRC = $(shell find "src/x86/" -type f -name "*.c" ! -name "protected_mode.c")  $(shell find "src/cstd/" -type f -name "*.c")
X64_C_SRC = $(shell find "src/x64/" -type f -name "*.c" ! -name "long_mode_entry.c")
X86_ASM_SRC = $(shell find "src/x86/" -type f -name "*.asm")
X86_O_SRC = $(notdir $(patsubst %.c,%.o,$(X86_C_SRC)) $(patsubst %.asm,%.o,$(X86_ASM_SRC)))
X64_O_SRC = $(notdir $(patsubst %.c,%.o,$(X64_C_SRC)))
X86_C_DIR = $(shell find "src" -type d)
X86_C_INC = $(addprefix -I,$(X86_C_DIR))

all: boot stage2 long_mode python_build
	qemu-system-x86_64 -m 2G -debugcon stdio -no-shutdown -no-reboot -d int -drive format=raw,file=boot.o

boot: src/boot.asm
	$(NASM) src/boot.asm -o boot.o

stage2: src/stage2.asm
	$(NASM) src/stage2.asm -o stage2.o

python_build:
	python build.py

python_linker86:
	python linker86.py

python_linker64:
	python linker64.py

protected_mode: python_linker86
	echo $(X86_O_SRC)
	$(CC32) -c src/x86/protected_mode.c -o protected_mode.o $(X86_C_INC)
	@for file in $(X86_C_SRC); do \
		basename=$$(basename $$file .c); \
		echo "Compiling $$file -> $$basename.o"; \
		$(CC32) $$file -o $$basename.o $(X86_C_INC) || exit 1; \
	done
	@for file in $(X86_ASM_SRC); do \
		basename=$$(basename $$file .asm); \
		echo "Compiling $$file -> $$basename.o"; \
		nasm -f elf32 $$file -o $$basename.o || exit 1; \
	done
	$(LD) protected_mode.o  $(X86_O_SRC) -o protected_mode.elf -T src/x86/tmp86.ld
	rm src/x86/tmp86.ld
	$(OBJ_RAW) protected_mode.elf protected_mode.bin

long_mode: protected_mode python_linker64
	$(CC64) -c src/x64/long_mode_entry.c -o long_mode_entry.o $(X86_C_INC)
	@for file in $(X64_C_SRC); do \
		basename=$$(basename $$file .c); \
		echo "Compiling $$file -> $$basename.o"; \
		$(CC64) $$file -o $$basename.o $(X86_C_INC) || exit 1; \
	done
	$(LD) long_mode_entry.o $(X64_O_SRC) -o long_mode_entry.elf -T src/x64/tmp64.ld
	rm src/x64/tmp64.ld
	$(OBJ_RAW) long_mode_entry.elf long_mode_entry.bin

run:
	qemu-system-x86_64 -m 2G -debugcon stdio -no-shutdown -no-reboot -d int -drive format=raw,file=boot.o

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