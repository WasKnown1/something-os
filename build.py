import os 

if __name__ == "__main__":
    with open("boot.o", "r+b") as f, open("stage2.o", "r+b") as f2, open("protected_mode.bin", "r+b") as f3, open("long_mode_entry.bin", "r+b") as f4:
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
        
        f.seek(int(os.path.getsize("boot.o")))
        f.write(f4.read())
        
    print(f"Final binary size: {os.path.getsize('boot.o')} bytes")