if __name__ == "__main__":
    with open("boot.o", "r+b") as f:
        f.seek(0x0200 - 2)
        
        f.write(b'\x55\xaa')