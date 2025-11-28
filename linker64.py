import os

with open("src/x64/linker64.ld") as linker, open("src/x64/tmp64.ld", "w") as tmp:
    c_offset = 0x7c00 + int(os.path.getsize("stage2.o")) + int(os.path.getsize("protected_mode.bin")) # c is loaded at address 0x7e00 + the size of the second stage
                                                         # if not accounted for the data section is not loaded correctly and each section gcc make has the wrong offset
    tmp.write(linker.read()
                    .replace("##", f"{hex(c_offset)}")
                )