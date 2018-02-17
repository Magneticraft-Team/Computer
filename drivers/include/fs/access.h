//
// Created by cout970 on 11/02/18.
//

#ifndef COMPUTER_ACCESS_H
#define COMPUTER_ACCESS_H

extern ByteBuffer blockBuffer;
extern DiskDrive *currentDiskDrive;

void loadSector(BlockRef sector);

void saveSector(BlockRef sector);

#endif //COMPUTER_ACCESS_H
