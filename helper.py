import os 

if __name__ == "__main__":
    with open("boot.o", "r+b") as f, open("stage2.o", "r+b") as f2, open("protected_mode.bin", "r+b") as f3:
        f.seek(510)
        
        f.write(b'\x55\xaa')
        
        f.seek(512)
        
        f.write(f2.read())
        
        print(f"stage2.o size: {os.path.getsize('stage2.o')}")
        
        # print(os.path.getsize("boot.o"))
        
        # f.seek(1790)
        # f.write(b'\x90')
        
        f.seek(int(os.path.getsize("boot.o")))
        f.write(f3.read())
        
    print(f"Final binary size: {os.path.getsize('boot.o')} bytes")
    
    with open("src/x86/linker.ld", "w") as linker, open("src/x86/tmp.ld") as tmp:
        c_offset = 0x7e00 + int(os.path.getsize("stage2.o")) # c is loaded at address 0x7e00 + the size of the second stage
                                                             # if not accounted for the data section is not loaded correctly and each section gcc make has the wrong offset
        linker.write(tmp.read()
                        .replace("#", f"{hex(c_offset)}")
                    )