//
// Created by cout970 on 2017-07-23.
//
// Api to access to the disk drive, this can be used to access a floppy drive or a hard drive

#ifndef COMPUTER_DISK_DRIVE_H
#define COMPUTER_DISK_DRIVE_H

#include "devices.h"

#define DISK_DRIVE_SECTOR_SIZE 1024
#define DISK_DRIVE_BUFFER_SIZE 1024

struct disk_drive_header {
/*   0 0x00 */    struct device_header header;
/*   4 0x04 */    Byte signal;                  // order to run
/*   5 0x05 */    const Byte hasDisk;           // has disk or is empty
/*   6 0x06 */    const Byte accessTime;        // number of minecraft ticks needed to run a task
/*   7 0x07 */    const Byte padding;           // unused space, needed for alignment
/*   8 0x08 */    const Int numSectors;         // number of sectors of the current disk
/*  12 0x0c */    Int currentSector;            // sector to read/write
/*  16 0x10 */    Byte buffer[DISK_DRIVE_BUFFER_SIZE];  // buffer to store the data before writing or after reading
};

typedef struct disk_drive_header DiskDrive;

#define DISK_DRIVE_SIGNAL_NO_ACTION 0
#define DISK_DRIVE_SIGNAL_READ 1
#define DISK_DRIVE_SIGNAL_WRITE 2
#define DISK_DRIVE_SIGNAL_READ_LABEL 3
#define DISK_DRIVE_SIGNAL_WRITE_LABEL 4

// Runs a task in the drive,
// the device is async so you will need to sleep 'drive.accessTime' ticks
void disk_drive_signal(DiskDrive *drive, Int signal);

// Return TRUe if there is a disk, FALSE otherwise
Boolean disk_drive_has_disk(DiskDrive *drive);

// Amount of minecraft ticks needed to run a task
Int disk_drive_get_access_time(DiskDrive *drive);

// Amount of sectors in the disk, every sector has 1024 Bytes
Int disk_drive_get_num_sectors(DiskDrive *drive);

// Get current selected sector
Int disk_drive_get_current_sector(DiskDrive *drive);

// Set current selected sector
void disk_drive_set_current_sector(DiskDrive *drive, Int sector);

// Returns a pointer to the disk buffer, the volatile is important to avoid dead code elimination from gcc
Byte volatile *disk_drive_get_buffer(DiskDrive *drive);

#endif //COMPUTER_DISK_DRIVE_H
