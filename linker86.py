import os

with open("src/x86/linker86.ld") as linker, open("src/x86/tmp86.ld", "w") as tmp:
    c_offset = 0x7e00 + int(os.path.getsize("stage2.o")) # c is loaded at address 0x7e00 + the size of the second stage
                                                         # if not accounted for the data section is not loaded correctly and each section gcc make has the wrong offset
    tmp.write(linker.read()
                    .replace("##", f"{hex(c_offset)}")
                )