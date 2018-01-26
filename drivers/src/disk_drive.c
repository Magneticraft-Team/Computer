//
// Created by cout970 on 2017-07-23.
//

#include "disk_drive.h"

void disk_drive_signal(DiskDrive *drive, Int signal) {
    ((struct disk_drive_header volatile *) drive)->signal = (Byte) signal;
}

Boolean disk_drive_has_disk(DiskDrive *drive) {
    return drive->hasDisk;
}

Int disk_drive_get_access_time(DiskDrive *drive) {
    return drive->accessTime;
}

Int disk_drive_get_num_sectors(DiskDrive *drive) {
    return drive->numSectors;
}

Int disk_drive_get_current_sector(DiskDrive *drive) {
    return drive->currentSector;
}

void disk_drive_set_current_sector(DiskDrive *drive, Int sector) {
    ((struct disk_drive_header volatile *) drive)->currentSector = sector;
}

Byte volatile *disk_drive_get_buffer(DiskDrive *drive) {
    return drive->buffer;
}

