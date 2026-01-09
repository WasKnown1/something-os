import os
import json
import subprocess
import struct

def build_tree(path):
    files = []
    dirs = []
    for name in sorted(os.listdir(path)):
        full = os.path.join(path, name)
        if os.path.isdir(full):
            dirs.append(name)
        else:
            files.append(name)
    items = []
    items.extend(files)
    for d in dirs:
        items.append({d: build_tree(os.path.join(path, d))})
    return items

def dir2jsonfile(path, json_file):
    tree = build_tree(path)
    with open(json_file, "w", encoding="utf-8") as jf:
        json.dump(tree, jf, indent=4)

""""
typedef struct FileHeader {
    uint8_t is_folder;
    uint32_t size; // including the file end header
    uint32_t padding_from_original_size;
    uint16_t file_name_length; // this includes the full directory
    // there goes file name
    // and then goes the file info
} __attribute__((packed)) FileHeader;
"""

"""
typedef struct FsHeader {
    uint32_t signiture;
} __attribute__((packed)) FsHeader;
"""

def build_mono_fs():
    dir2jsonfile('fs/', 'fs_structure.json')
    subprocess.run(['mv', 'fs_structure.json', 'fs/fs_structure.json'])
    
    with open("fs.bin", "w+b") as fs:
        fs.seek(0x00)
        fs.write("DEED".encode('ascii'))           # fs signiture
        
        for root, dirs, files in os.walk('fs/'):
            for file in files:
                file_path = os.path.join(root, file)
                file_content = open(file_path, "r").read().encode('ascii')
                file_size = len(file_content) + 11 + len(file_path.removeprefix('fs/'))
                #                               ^ file header size
                byte_struct = struct.pack('<BLLH',
                                            int(False),                                    # is_folder
                                            file_size,                                     # file_size
                                            int(0),                                        # padding at end of file
                                            len(file_path.removeprefix('fs/')),            # fille name length
                                          )
                fs.write(byte_struct)
                fs.write(file_path.removeprefix('fs/').encode('ascii'))
                fs.write(file_content)
            
            for dr in dirs:
                file_path = os.path.join(root, dr)
                file_size = 11 + len(file_path.removeprefix('fs/'))
                #            ^ file header size
                byte_struct = struct.pack('<BLLH',
                                            int(True),                                     # is_folder
                                            file_size,                                     # file_size
                                            int(0),                                        # padding at end of file
                                            len(file_path.removeprefix('fs/')),            # fille name length
                                          )
                fs.write(byte_struct)
                fs.write(file_path.removeprefix('fs/').encode('ascii'))
                
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