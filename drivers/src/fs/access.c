//
// Created by cout970 on 11/02/18.
//


#include <types.h>
#include <fs/filesystem.h>
#include <string.h>
#include <motherboard.h>

ByteBuffer blockBuffer = NULL;
DiskDrive *currentDiskDrive = NULL;

#ifdef DEBUG_ENV
#include <stdio.h>

void loadSector(BlockRef sector) {
    FILE *file = fopen("file1.img", "rb");
    if(!file){
        fclose(fopen("file1.img", "wb"));
        file = fopen("file1.img", "rb");
    }
    fseek(file, sector * 1024, SEEK_SET);
    memset(blockBuffer, 0, 1024);
    fread(blockBuffer, 1024, 1, file);
    fclose(file);
}

void saveSector(BlockRef sector) {
    if (sector < 0) {
        *(Byte *) 0x0 = 0;
    }
    FILE *file = fopen("file1.img", "rb+");
    if (!file) {
        fclose(fopen("file1.img", "wb+"));
        file = fopen("file1.img", "rb+");
    }
    fseek(file, sector * 1024, SEEK_SET);
    fwrite(blockBuffer, 1024, 1, file);
    fflush(file);
    fclose(file);
}

#else
void loadSector(BlockRef sector) {
    disk_drive_set_current_sector(currentDiskDrive, sector);
    disk_drive_signal(currentDiskDrive, DISK_DRIVE_SIGNAL_READ);
    motherboard_sleep((Byte) disk_drive_get_access_time(currentDiskDrive));
}

void saveSector(BlockRef sector) {
    disk_drive_set_current_sector(currentDiskDrive, sector);
    disk_drive_signal(currentDiskDrive, DISK_DRIVE_SIGNAL_WRITE);
    motherboard_sleep((Byte) disk_drive_get_access_time(currentDiskDrive));
}
#endif