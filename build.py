import os

""""
typedef struct FileHeader {
    uint32_t is_folder;
    uint32_t size;
    uint32_t padding_from_original_size;
    uint16_t file_name_length; // this includes the full directory
    // there goes file name
    // and then goes the file info
} FileHeader;
"""

def build_mono_fs():
    with open("fs.bin", "w+b") as fs:
        fs.seek(0x00)
        fs.write("DEED".encode('ascii')) # fs signiture
        fs.write(b'\x00\x00\x00\x00') # padding for fs start size in the future
        
        for root, dirs, files in os.walk('fs/'):
            # print(f"{files=}, {dirs=}, {root.removeprefix('fs/') == ''}")
            for file in files:
                fs.write(b'\x00') # not a dir
                file_path = os.path.join(root, file)
                file_size = len(file_path.removeprefix('fs/')) + 1 + 8 + len(open(file_path, "r").read()) + 11
                # file size
                fs.write(file_size.to_bytes(4, 'little'))
                print(f"file size = {file_size}")
                # padding 0 at the end of file
                fs.write(int(0).to_bytes(4, 'little'))
                # file name length
                fs.write((len(file_path.removeprefix('fs/')) + 1).to_bytes(2, 'little')) 
                print(f"filename length = {len(file_path.removeprefix('fs/')).to_bytes(2, 'little')}")
                # file name
                fs.write(f"{file_path.removeprefix('fs/')}\0".encode('ascii'))
                # file content
                with open(file_path, 'rb') as fl:
                    fs.write(fl.read())
                    
                # file end header
                fs.write(file_size.to_bytes(4, 'little'))
                fs.write("DEED".encode('ascii'))
                
                print(f"sizeof file {file_path}: {file_size}")
            
            for dr in dirs:
                # is infact a folder
                fs.write(b'\x01')
                file_path = os.path.join(root, dr)
                file_size = len(file_path.removeprefix('fs/')) + 1 + 8 + 11
                # folder size maybe i will change it later to the size of the entire folder idk
                fs.write(file_size.to_bytes(4, 'little')) 
                print(f"folder size = {file_size}")
                # padding at the end of the folder (not really needed since it is a folder)
                fs.write(int(0).to_bytes(4, 'little'))
                # folder name length
                fs.write((len(file_path.removeprefix('fs/')) + 1).to_bytes(2, 'little'))
                print(f"filename length = {len(file_path.removeprefix('fs/')).to_bytes(2, 'little')}")
                # folder name
                fs.write(f"{file_path.removeprefix('fs/')}\0".encode('ascii'))
                
                # file end header
                fs.write(file_size.to_bytes(4, 'little'))
                fs.write("DEED".encode('ascii'))
                
        fs.seek(0x04)
        fs.write(os.path.getsize("fs.bin").to_bytes(4, 'little'))
        print(f"sizeof fs.bin: {os.path.getsize('fs.bin')}")

FS_START_ADDRESS = 64 * 512 * 3

if __name__ == "__main__":
    build_mono_fs()
    
    with open("boot.o", "r+b") as f,               \
         open("stage2.o", "r+b") as f2,            \
         open("protected_mode.bin", "r+b") as f3,  \
         open("long_mode_entry.bin", "r+b") as f4, \
         open("fs.bin", "r+b") as f5:
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
        
        f.seek(FS_START_ADDRESS)
        f.write(f5.read()) # write the filesystem image at the specified LBA
        
    print(f"Final binary size: {os.path.getsize('boot.o')} bytes")