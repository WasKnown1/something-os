#include <pio.h>
#include <pci.h>

uint32_t pci_config_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address =
        (1U << 31)             |
        ((uint32_t)bus << 16)  |
        ((uint32_t)slot << 11) |
        ((uint32_t)func << 8)  |
        (offset & 0xfc);

    outl(0xcf8, address);
    return inl(0xcfc);
}

void pci_config_write_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address =
        (1U << 31)             |
        ((uint32_t)bus << 16)  |
        ((uint32_t)slot << 11) |
        ((uint32_t)func << 8)  |
        (offset & 0xfc);

    outl(0xcf8, address);
    outl(0xcfc, value);
}