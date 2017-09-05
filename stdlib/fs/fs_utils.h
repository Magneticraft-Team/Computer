//
// Created by cout970 on 2017-09-05.
//

#ifndef MAGNETICRAFTCOMPUTER_FSUTILS_H
#define MAGNETICRAFTCOMPUTER_FSUTILS_H

#include "filesystem.h"

extern SuperBlock superBlockCache;

void save_super_block(DiskDrive drive);


// alloc and fre blocks
int allocate_sector(DiskDrive drive);

void free_sector(DiskDrive drive, int sector);

// blocks needed for 'size' bytes
int get_blocks_from_size(int size);

void alloc_blocks_up_to(DiskDrive drive, File *file, int totalBlocksNeeded);

void load_sector(DiskDrive drive, int sector);

void save_sector(DiskDrive drive);

#endif //MAGNETICRAFTCOMPUTER_FSUTILS_H
