#include <ata.h>
#include <pio.h>

void ata_wait_busy() {
    while (inb(0x1F7) & ATA_SR_BSY);
}

void ata_wait_drq() {
    while (!(inb(0x1F7) & ATA_SR_DRQ));
}

bool ata_write28(uint32_t lba, const uint8_t *buffer) {
    ata_wait_busy();

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)(lba));
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x30);

    ata_wait_busy();
    ata_wait_drq();

    for (int i = 0; i < 256; i++)
        outw(0x1F0, ((uint16_t*)buffer)[i]);

    outb(0x1F7, 0xE7);
    ata_wait_busy();

    return true;
}

bool ata_read28(uint32_t lba, uint8_t *buffer){
    ata_wait_busy();

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)(lba));
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x20);

    ata_wait_busy();
    ata_wait_drq();

    uint16_t x = 256;

    __asm__ (
        "cld\n\t"
        "rep insw"
        : "+D"(buffer), "+c"(x)
        : "d"(0x1F0)
        : "memory");

    return true;
}
