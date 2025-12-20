#include <ata.h>
#include <pio.h>

void ata_wait_busy() {
    while (inb(0x1f7) & ATA_SR_BSY);
}

void ata_wait_drq() {
    while (!(inb(0x1f7) & ATA_SR_DRQ));
}

bool ata_write28(uint32_t lba, const uint8_t *buffer) {
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
        outw(0x1f0, ((uint16_t*)buffer)[i]);

    outb(0x1f7, 0xf7);
    ata_wait_busy();

    return true;
}

bool ata_read28(uint32_t lba, uint8_t *buffer){
    ata_wait_busy();

    outb(0x1f6, 0xe0 | ((lba >> 24) & 0x0f));
    outb(0x1f2, 1);
    outb(0x1f3, (uint8_t)(lba));
    outb(0x1f4, (uint8_t)(lba >> 8));
    outb(0x1f5, (uint8_t)(lba >> 16));
    outb(0x1f7, 0x20);

    ata_wait_busy();
    ata_wait_drq();

    uint16_t x = 256;

    __asm__ (
        "cld\n\t"
        "rep insw"
        : "+D"(buffer), "+c"(x)
        : "d"(0x1f0)
        : "memory");

    return true;
}
