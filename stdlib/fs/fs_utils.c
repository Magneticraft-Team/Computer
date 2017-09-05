//
// Created by cout970 on 2017-09-05.
//

#include "fs_utils.h"
#include "../api/string.h"

SuperBlock superBlockCache;

void save_super_block(DiskDrive drive) {
    load_sector(drive, 0);
    memcpy((void *) disk_drive_get_buffer(drive), &superBlockCache, sizeof(superBlockCache));
    save_sector(drive);
}

struct ByteArray byteArrayOf(void *ptr, int len){
    struct ByteArray array = {
            .data = ptr,
            .length = len
    };
    return array;
}

// block allocation

int allocate_sector(DiskDrive drive) {
    if (superBlockCache.lastRemovedBlock != NULL_SECTOR) {
        int ret = superBlockCache.lastRemovedBlock;
        int nextFreeBlock = superBlockCache.lastRemovedBlock++;
        int firstUnusedBlock = superBlockCache.firstUnusedBlock;

        while (1) {
            if (nextFreeBlock == firstUnusedBlock) {
                nextFreeBlock = NULL_SECTOR;
                break;
            }
            load_sector(drive, nextFreeBlock);
            BlockHeader *header = (BlockHeader *) disk_drive_get_buffer(drive);
            if (header->firstBlock == NULL_SECTOR) {
                break;
            }
            nextFreeBlock++;
        }
        superBlockCache.lastRemovedBlock = nextFreeBlock;
        superBlockCache.freeBlocks--;
        superBlockCache.usedBlocks++;
        save_super_block(drive);
        return ret;
    }

    int ret = superBlockCache.firstUnusedBlock;
    superBlockCache.firstUnusedBlock++;
    superBlockCache.freeBlocks--;
    superBlockCache.usedBlocks++;
    save_super_block(drive);
    return ret;
}

void free_sector(DiskDrive drive, int sector) {
    load_sector(drive, sector);
    BlockHeader *header = (BlockHeader *) disk_drive_get_buffer(drive);
    header->firstBlock = NULL_SECTOR;
    save_sector(drive);
    if (superBlockCache.lastRemovedBlock == NULL_SECTOR || superBlockCache.lastRemovedBlock > sector) {
        superBlockCache.lastRemovedBlock = sector;
    }
    superBlockCache.usedBlocks--;
    superBlockCache.freeBlocks++;
    if (sector == superBlockCache.firstUnusedBlock - 1) {
        superBlockCache.firstUnusedBlock--;
    }
    save_super_block(drive);
}

// give a file size, give the amount of block needed to store all the data

int get_blocks_from_size(int size) {
    if ((unsigned int) size < FILE_FIRST_BLOCK_SPACE) {
        return 1;
    }
    int blocks = 1;
    size -= FILE_FIRST_BLOCK_SPACE;

    while (size > 0) {
        size -= FILE_NORMAL_BLOCK_SPACE;
        blocks++;
    }
    return blocks;
}

void alloc_blocks_up_to(DiskDrive drive, File *file, int totalBlocksNeeded) {
    if (totalBlocksNeeded <= 1) return;

    // load first block to the stack
    FileFirstBlock header;
    load_sector(drive, file->firstBlock);
    memcpy(&header, (const void *) disk_drive_get_buffer(drive), sizeof(FileFirstBlock));


    int currentSector = header.metadata.nextBlock;
    int fileBlockCount;

    // tmp save of the last block loaded to update lastBlock.nextBlock
    int lastBlockSector;
    BlockHeader lastBlock;

    // file only has 1 block and totalBlocksNeeded is > 1
    if (header.metadata.nextBlock == NULL_SECTOR) {
        currentSector = allocate_sector(drive);

        // load new block
        lastBlockSector = currentSector;
        load_sector(drive, currentSector);
        memcpy(&lastBlock, (const void *) disk_drive_get_buffer(drive), sizeof(BlockHeader));

        // write header data
        lastBlock.firstBlock = file->firstBlock;
        lastBlock.nextBlock = NULL_SECTOR;

        // save block
        memcpy((void *) disk_drive_get_buffer(drive), &lastBlock, sizeof(BlockHeader));
        save_sector(drive);

        // update file second block pointer
        header.metadata.nextBlock = currentSector;
        file->nextBlock = currentSector;

        // save first block in disk
        load_sector(drive, file->firstBlock);
        memcpy((void *) disk_drive_get_buffer(drive), &header, sizeof(FileFirstBlock));
        save_sector(drive);
    } else {
        // load second block, this is needed to update field nextBlock later
        lastBlockSector = currentSector;
        load_sector(drive, currentSector);
        memcpy(&lastBlock, (const void *) disk_drive_get_buffer(drive), sizeof(BlockHeader));
    }
    // totalBlocksNeeded is > 1 so for now we know that the file has at least 2 blocks
    fileBlockCount = 2;

    // go through blocks loading or allocating if needed
    while (fileBlockCount < totalBlocksNeeded) {
        if (lastBlock.nextBlock == NULL_SECTOR) {
            currentSector = allocate_sector(drive);

            // modify and...
            lastBlock.nextBlock = currentSector;
            // save last block
            load_sector(drive, lastBlockSector);
            memcpy((void *) disk_drive_get_buffer(drive), &lastBlock, sizeof(BlockHeader));
            save_sector(drive);

            //load an init the allocated block
            lastBlockSector = currentSector;
            load_sector(drive, currentSector);
            memcpy(&lastBlock, (const void *) disk_drive_get_buffer(drive), sizeof(BlockHeader));

            lastBlock.firstBlock = file->firstBlock;
            lastBlock.nextBlock = NULL_SECTOR;

            // warning this doesn't save the block
        } else {
            currentSector = lastBlock.nextBlock;
            //load next block
            lastBlockSector = currentSector;
            load_sector(drive, currentSector);
            memcpy(&lastBlock, (const void *) disk_drive_get_buffer(drive), sizeof(BlockHeader));

            // warning this doesn't save the block, just reads it
        }
        fileBlockCount++;
    }

    // save last block of the file, because the branches in the while don't do it
    load_sector(drive, lastBlockSector);
    memcpy((void *) disk_drive_get_buffer(drive), &lastBlock, sizeof(BlockHeader));
    save_sector(drive);
}

void load_sector(DiskDrive drive, int sector) {
    disk_drive_set_current_sector(drive, sector);
    disk_drive_signal(drive, DISK_DRIVE_SIGNAL_READ);
    motherboard_sleep((i8) disk_drive_get_access_time(drive));
}

void save_sector(DiskDrive drive) {
    disk_drive_signal(drive, DISK_DRIVE_SIGNAL_WRITE);
    motherboard_sleep((i8) disk_drive_get_access_time(drive));
}