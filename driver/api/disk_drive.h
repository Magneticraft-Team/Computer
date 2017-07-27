//
// Created by cout970 on 2017-07-23.
//

#ifndef DRIVER_DISK_DRIVE_H
#define DRIVER_DISK_DRIVE_H

#include "devices.h"

struct disk_drive_header {
/*   0 0x00 */    struct device_header header;
/*   4 0x04 */    i8 signal;
/*   5 0x05 */    const i8 hasDisk;
/*   6 0x06 */    const i8 accessTime;
/*   7 0x07 */    const i8 padding;
/*   8 0x08 */    const i32 numSectors;
/*  12 0x0c */    i32 currentSector;
/*  16 0x10 */    i8 buffer[1024];
};

typedef struct disk_drive_header *DiskDrive;

#define DISK_DRIVE_SIGNAL_NO_ACTION 0
#define DISK_DRIVE_SIGNAL_READ 1
#define DISK_DRIVE_SIGNAL_WRITE 2
#define DISK_DRIVE_SIGNAL_READ_LABEL 3
#define DISK_DRIVE_SIGNAL_WRITE_LABEL 4

void disk_drive_signal(DiskDrive drive, i32 signal);

boolean disk_drive_has_disk(DiskDrive drive);

i32 disk_drive_get_access_time(DiskDrive drive);

i32 disk_drive_get_num_sectors(DiskDrive drive);

i32 disk_drive_get_current_sector(DiskDrive drive);

void disk_drive_set_current_sector(DiskDrive drive, i32 sector);

i8 volatile *disk_drive_get_buffer(DiskDrive drive);

#endif //DRIVER_DISK_DRIVE_H
