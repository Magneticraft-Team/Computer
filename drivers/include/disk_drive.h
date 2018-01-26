//
// Created by cout970 on 2017-07-23.
//

#ifndef DRIVER_DISK_DRIVE_H
#define DRIVER_DISK_DRIVE_H

#include "devices.h"

#define DISK_DRIVE_SECTOR_SIZE 1024
#define DISK_DRIVE_BUFFER_SIZE 1024

struct disk_drive_header {
/*   0 0x00 */    struct device_header header;
/*   4 0x04 */    Byte signal;
/*   5 0x05 */    const Byte hasDisk;
/*   6 0x06 */    const Byte accessTime;
/*   7 0x07 */    const Byte padding;
/*   8 0x08 */    const Int numSectors;
/*  12 0x0c */    Int currentSector;
/*  16 0x10 */    Byte buffer[DISK_DRIVE_BUFFER_SIZE];
};

typedef struct disk_drive_header DiskDrive;

#define DISK_DRIVE_SIGNAL_NO_ACTION 0
#define DISK_DRIVE_SIGNAL_READ 1
#define DISK_DRIVE_SIGNAL_WRITE 2
#define DISK_DRIVE_SIGNAL_READ_LABEL 3
#define DISK_DRIVE_SIGNAL_WRITE_LABEL 4

void disk_drive_signal(DiskDrive *drive, Int signal);

Boolean disk_drive_has_disk(DiskDrive *drive);

Int disk_drive_get_access_time(DiskDrive *drive);

Int disk_drive_get_num_sectors(DiskDrive *drive);

Int disk_drive_get_current_sector(DiskDrive *drive);

void disk_drive_set_current_sector(DiskDrive *drive, Int sector);

Byte volatile *disk_drive_get_buffer(DiskDrive *drive);

#endif //DRIVER_DISK_DRIVE_H
