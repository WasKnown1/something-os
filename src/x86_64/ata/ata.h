#ifndef ATA_H
#define ATA_H

#include <stdbool.h>
#include <smthng_os.h>

#define ATA_SR_BSY 0x80
#define ATA_SR_DRQ 0x08
#define ATA_SR_ERR 0x01

u0 ata_read28(u32 lba, u8 *buffer);
u0 ata_write28(u32 lba, const u8 *buffer);
u0 ata_read_lbas(u32 lba, u32 sector_count, u8 *buffer);

#endif // ATA_H