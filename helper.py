if __name__ == "__main__":
    with open("boot.o", "r+b") as f:
        f.seek(510)
        
        f.write(b'\x55\xaa')
        
        with open("stage2.o", "r+b") as f2:
            f.seek(512)
            
            f.write(f2.read())