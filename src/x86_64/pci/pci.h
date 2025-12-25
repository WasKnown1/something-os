#ifndef PCI_H
#define PCI_H

#include <stdint.h>

uint32_t pci_config_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_config_write_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);

#endif // PCI_H