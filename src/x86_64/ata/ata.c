#include <ata.h>
#include <pio.h>

u0 ata_wait_busy(u0) {
    while (inb(0x1f7) & ATA_SR_BSY);
}

u0 ata_wait_drq(u0) {
    while (!(inb(0x1f7) & ATA_SR_DRQ));
}

u0 ata_write28(u32 lba, const u8 *buffer) {
    ata_wait_busy();

    outb(0x1f6, 0xe0 | ((lba >> 24) & 0x0f));
    outb(0x1f2, 1);
    outb(0x1f3, (uint8_t)(lba));
    outb(0x1f4, (uint8_t)(lba >> 8));
    outb(0x1f5, (uint8_t)(lba >> 16));
    outb(0x1f7, 0x30);

    ata_wait_busy();
    ata_wait_drq();

    for (int i = 0; i < 256; i++)
        outw(0x1f0, ((u16*)buffer)[i]);

    outb(0x1f7, 0xf7);
    ata_wait_busy();
}

u0 ata_read28(u32 lba, u8 *buffer){
    ata_wait_busy();

    outb(0x1f6, 0xe0 | ((lba >> 24) & 0x0f));
    outb(0x1f2, 1);
    outb(0x1f3, (u8)(lba));
    outb(0x1f4, (u8)(lba >> 8));
    outb(0x1f5, (u8)(lba >> 16));
    outb(0x1f7, 0x20);

    ata_wait_busy();
    ata_wait_drq();

    u16 x = 256;

    __asm__ (
        "cld\n\t"
        "rep insw"
        : "+D"(buffer), "+c"(x)
        : "d"(0x1f0)
        : "memory");
}

u0 ata_read_lbas(u32 lba, u32 sector_count, u8 *buffer) {
    for (u32 i = 0; i < sector_count; i++) {
        ata_read28(lba + i, buffer + (i * 512));
    }
}