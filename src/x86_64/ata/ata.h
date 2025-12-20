#ifndef ATA_H
#define ATA_H

#include <stdbool.h>
#include <stdint.h>

#define ATA_SR_BSY 0x80
#define ATA_SR_DRQ 0x08
#define ATA_SR_ERR 0x01

bool ata_read28(uint32_t lba, uint8_t *buffer);
bool ata_write28(uint32_t lba, const uint8_t *buffer);

#endif // ATA_H