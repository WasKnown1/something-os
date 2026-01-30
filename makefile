NASM         = nasm -g -f bin
# AS = as --32 -march i386
CC32         = gcc -ggdb -c -m32 -Wall -Werror -nostdlib -ffreestanding -nodefaultlibs -mno-red-zone -fno-pic -fno-pie -D QEMU_DEBUG
CC64         = gcc -c -m64 -Wall -Werror -nostdlib -ffreestanding -nodefaultlibs -mno-red-zone -fno-pic -fno-pie -D QEMU_DEBUG
MINGWCC32    = x86_64-w64-mingw32-gcc -c -m32 -Wall -Werror -nostdlib -nodefaultlibs -mno-red-zone -fno-pic -fno-pie -Wl,--export-all-symbols -Wl,--gc-sections
LD32         = ld -m elf_i386
LD64         = ld -m elf_x86_64 
MINGWLD32    = x86_64-w64-mingw32-gcc -m32 -Wall -Werror -nostdlib -nodefaultlibs -mno-red-zone -fno-pic -fno-pie -Wl,--gc-sections -T /usr/i686-w64-mingw32/lib/ldscripts/i386pe.x
OBJ_RAW      = objcopy --set-section-flags .bss=alloc,load,contents --set-section-flags .data=alloc,load,contents -O binary
# MINGWDLL32   = $(MINGWLD32) -shared
MINGW32FLAGS = -Lfs/dll/ -lsmthngdll -Wl,--entry,_main -Wl,-e,_main
MINGW32LDDLL = $(MINGWLD32) -shared 

X86_C_SRC    = $(shell find "src/x86/" -type f -name "*.c" ! -name "protected_mode.c")  $(shell find "src/cstd/" -type f -name "*.c") $(shell find "src/x86_64/" -type f -name "*.c")
X64_C_SRC    = $(shell find "src/x64/" -type f -name "*.c" ! -name "long_mode_entry.c")
X86_ASM_SRC  = $(shell find "src/x86/" -type f -name "*.asm")
X86_O_SRC    = $(notdir $(patsubst %.c,%.o,$(X86_C_SRC)) $(patsubst %.asm,%.o,$(X86_ASM_SRC)))
X64_O_SRC    = $(notdir $(patsubst %.c,%.o,$(X64_C_SRC)))
X86_C_DIR    = $(shell find "src" -type d)
X86_C_INC    = $(addprefix -I,$(X86_C_DIR))
DRIVER_C_SRC = $(shell find "src/drivers/" -type f -name "*.c" ! -name "driver_lib.c")
DRIVER_C_DIR = $(shell find "src/drivers/" -type d)
DRIVER_C_INC = $(addprefix -I,$(DRIVER_C_DIR))
DLLS_C_SRC   = $(shell find "src/dlls/" -type f -name "*.c" ! -name "dll_lib.c")
DLLS_C_DIR   = $(shell find "src/dlls/" -type d)
DLLS_C_INC   = $(addprefix -I,$(DLLS_C_DIR))

build_only: boot stage2 long_mode python_build

all: boot stage2 long_mode python_build
	qemu-system-x86_64 -m 2G -debugcon stdio -no-shutdown -no-reboot -d int -drive format=raw,file=boot.o

boot: src/boot.asm
	$(NASM) src/boot.asm -o boot.o

stage2: src/stage2.asm
	$(NASM) src/stage2.asm -o stage2.o

python_build: dlls drivers
	python build.py

python_linker86:
	python linker86.py

python_linker64:
	python linker64.py

protected_mode: python_linker86
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
	$(LD32) protected_mode.o  $(X86_O_SRC) -o protected_mode.elf -T src/x86/tmp86.ld
	rm src/x86/tmp86.ld
	$(OBJ_RAW) protected_mode.elf protected_mode.bin

long_mode: protected_mode python_linker64
	$(CC64) -c src/x64/long_mode_entry.c -o long_mode_entry.o $(X86_C_INC)
	@for file in $(X64_C_SRC); do \
		basename=$$(basename $$file .c); \
		echo "Compiling $$file -> $$basename.o"; \
		$(CC64) $$file -o $$basename.o $(X86_C_INC) || exit 1; \
	done
	$(LD64) long_mode_entry.o $(X64_O_SRC) -o long_mode_entry.elf -T src/x64/tmp64.ld
	rm src/x64/tmp64.ld
	$(OBJ_RAW) long_mode_entry.elf long_mode_entry.bin

drivers:
	$(MINGWCC32) src/drivers/driver_lib.c -o src/drivers/driver_lib.o $(DRIVER_C_INC) $(X86_C_INC)
	@for file in $(DRIVER_C_SRC); do \
		basename=$$(basename $$file .c); \
		echo "Compiling $$file -> $$basename.kdr"; \
		$(MINGWCC32) $(MINGW32FLAGS) $$file -o $$basename.o $(DRIVER_C_INC) $(X86_C_INC) || exit 1; \
		$(MINGWLD32) $(MINGW32FLAGS) $$basename.o src/drivers/driver_lib.o -o $$basename.kdr; \
		mv $$basename.kdr fs/drivers/; \
		rm $$basename.o; \
	done
	rm src/drivers/driver_lib.o

dlls:
	$(MINGWCC32) src/dlls/dll_lib.c -o src/dlls/dll_lib.o $(DLLS_C_INC) $(X86_C_INC)
	@for file in $(DLLS_C_SRC); do \
		basename=$$(basename $$file .c); \
		echo "Compiling $$file -> $$basename.kdr"; \
		$(MINGWCC32) $$file -o $$basename.o $(DRIVER_C_INC) $(X86_C_INC) $(DLLS_C_INC) || exit 1; \
		$(MINGW32LDDLL) $$basename.o src/dlls/dll_lib.o -o $$basename.dll; \
		mv $$basename.dll fs/dll/; \
		rm $$basename.o; \
	done
	rm src/dlls/dll_lib.o

run:
	qemu-system-x86_64 -m 2G -debugcon stdio -no-shutdown -no-reboot -d int -drive format=raw,file=boot.o

run-i386:
	qemu-system-i386 -m 2G -debugcon stdio -no-shutdown -no-reboot -d int -drive format=raw,file=boot.o -device pci-ohci,id=ohci -device usb-mouse -device usb-kbd

debug-run-i386:
	qemu-system-i386 -s -S -m 2G -debugcon stdio -no-shutdown -no-reboot -d int -drive format=raw,file=boot.o -device pci-ohci,id=ohci -device usb-mouse -device usb-kbd

build-run-i386: boot stage2 long_mode python_build
	qemu-system-i386 -m 2G -debugcon stdio -no-shutdown -no-reboot -d in_asm,int -drive format=raw,file=boot.o

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