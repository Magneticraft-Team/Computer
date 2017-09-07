//
// Created by cout970 on 2017-08-24.
//

#include "fs_utils.h"
#include "../api/math.h"
#include "../api/string.h"
#include "../api/stdio.h"
#include "filesystem.h"

void makeFs(DiskDrive drive) {

    load_sector(drive, 0);
    SuperBlock *buffer = (SuperBlock *) disk_drive_get_buffer(drive);

    //create superblock
    buffer->magicNumber = FILE_SYSTEM_MAGIC_NUMBER;
    buffer->freeBlocks = disk_drive_get_num_sectors(drive) - 2;
    buffer->usedBlocks = 1;
    buffer->rootDirectory = 1;
    buffer->firstUnusedBlock = 2;
    buffer->lastRemovedBlock = NULL_SECTOR;
    memcpy(&superBlockCache, buffer, sizeof(SuperBlock));
    save_sector(drive);

    // create root directory
    load_sector(drive, 1);
    FileFirstBlock *root = (FileFirstBlock *) disk_drive_get_buffer(drive);
    root->metadata.firstBlock = 1;
    root->metadata.nextBlock = NULL_SECTOR;
    strcpy(root->metadata.name, "/");
    root->metadata.parent = NULL_SECTOR;
    root->metadata.type = FILE_TYPE_DIRECTORY;
    root->metadata.size = 0;
    root->metadata.lastModified = motherboard_get_minecraft_world_time();

    save_sector(drive);

    // add .
    DirectoryEntry thisEntry;
    memset(&thisEntry, 0, sizeof(DirectoryEntry));
    thisEntry.firstBlock = root->metadata.firstBlock;
    memcpy(thisEntry.name, ".", 2);
    file_append(drive, (File *) root, byteArrayOf(&thisEntry, sizeof(DirectoryEntry)));

    // add ..
    DirectoryEntry parentEntry;
    memset(&parentEntry, 0, sizeof(DirectoryEntry));
    parentEntry.firstBlock = root->metadata.firstBlock;
    memcpy(parentEntry.name, "..", 3);
    file_append(drive, (File *) root, byteArrayOf(&parentEntry, sizeof(DirectoryEntry)));
}

void file_truncate(DiskDrive drive, File *file, int size) {
    if (file->size < size) {
        int totalBlocksNeeded = get_blocks_from_size(size);
        // allocate new blocks if needed
        alloc_blocks_up_to(drive, file, totalBlocksNeeded);

        // update file size
        load_sector(drive, file->firstBlock);
        FileFirstBlock *block = (FileFirstBlock *) disk_drive_get_buffer(drive);
        block->metadata.size = size;
        file->size = size;

        save_sector(drive);
    } else if (file->size > size) {

        int currentBlocks = get_blocks_from_size(file->size);
        int neededBlocks = get_blocks_from_size(size);
        int blocksToRemove = currentBlocks - neededBlocks;

        if (blocksToRemove > 0) {
            int sector = file->nextBlock, aux;
            int count = 2;
            BlockHeader *header;

            // iterate over all blocks
            while (count <= currentBlocks) {
                aux = sector;
                load_sector(drive, sector);
                header = (BlockHeader *) disk_drive_get_buffer(drive);
                sector = header->nextBlock;
                if (count > neededBlocks) {
                    free_sector(drive, aux);
                }
                count++;
            }
        }

        // update file size
        load_sector(drive, file->firstBlock);
        FileFirstBlock *block = (FileFirstBlock *) disk_drive_get_buffer(drive);
        block->metadata.size = size;
        file->size = size;
        if (neededBlocks == 1) {
            block->metadata.nextBlock = NULL_SECTOR;
            file->nextBlock = NULL_SECTOR;
        }

        save_sector(drive);
    }
}

struct BlockBuffer {
    int block;
    int nextBlock;
    int fileOffset;
    struct ByteArray data;
};

struct BlockBuffer nextBlockBuffer(DiskDrive drive, struct BlockBuffer old, int second) {
    load_sector(drive, old.nextBlock);
    BlockHeader *header = (BlockHeader *) disk_drive_get_buffer(drive);

    struct BlockBuffer slice = {
            .data = {
                    .data = (char *) header->content,
                    .length = FILE_NORMAL_BLOCK_SPACE
            },
            .fileOffset = old.fileOffset + (second ? FILE_FIRST_BLOCK_SPACE : FILE_NORMAL_BLOCK_SPACE),
            .block = old.nextBlock,
            .nextBlock = header->nextBlock
    };
    return slice;
}

struct BlockBuffer getSectorData(DiskDrive drive, File *file, int index) {

    load_sector(drive, file->firstBlock);
    struct BlockBuffer buffer = {
            .data = {
                    .data = ((FileFirstBlock *) disk_drive_get_buffer(drive))->content,
                    .length = FILE_FIRST_BLOCK_SPACE
            },
            .fileOffset = 0,
            .block = file->firstBlock,
            .nextBlock = file->nextBlock
    };

    int count = 0;
    while (count < index) {
        buffer = nextBlockBuffer(drive, buffer, count == 0);
        count++;
    }
    return buffer;
}

int arrayToFile(struct BlockBuffer buffer, struct ByteArray src, int offset) {
    // Offset in the BlockBuffer
    int offsetRelativeToBuffer = offset - buffer.fileOffset;
    // Space in the buffer to write
    int bufferSpace = buffer.data.length - offsetRelativeToBuffer;
    // Max bytes to write,
    int toWrite = min(bufferSpace, src.length);
    // copy data
    memcpy(buffer.data.data + offsetRelativeToBuffer, src.data, (size_t) toWrite);

    return toWrite;
}

int fileToArray(struct BlockBuffer buffer, struct ByteArray src, int offset) {
    // Offset in the BlockBuffer
    int offsetRelativeToBuffer = offset - buffer.fileOffset;
    // Space in the buffer to write
    int bufferSpace = buffer.data.length - offsetRelativeToBuffer;
    // Max bytes to write,
    int toWrite = min(bufferSpace, src.length);
    // copy data
    memcpy(src.data, buffer.data.data + offsetRelativeToBuffer, (size_t) toWrite);

    return toWrite;
}

// Read/Write

int file_read(DiskDrive drive, File *file, struct ByteArray dst, int offset) {
    if (dst.data == NULL) {
        printf("[file_read] Null data buffer\n");
        return 0;
    }
    if (dst.length <= 0) {
        printf("[file_read] Invalid length: %d\n", dst.length);
        return 0;
    }
    if (offset >= file->size) {
        if (offset > file->size){
            printf("[file_read] Offset out of file: offset = %d, fileSize = %d\n", offset, file->size);
        }
        return 0;
    }

    // fist block to start writing
    int startBlock = get_blocks_from_size(offset) - 1;
    int writen = 0;

    // getSectorData leaves the sector in the drive buffer
    struct BlockBuffer fileBuffer = getSectorData(drive, file, startBlock);

    while (dst.length > 0) {
        int bytesWriten = fileToArray(fileBuffer, dst, offset);

        dst.data += bytesWriten;
        dst.length -= bytesWriten;
        offset += bytesWriten;

        writen += bytesWriten;

        if (dst.length > 0) {
            fileBuffer = nextBlockBuffer(drive, fileBuffer, file->firstBlock == fileBuffer.block);
        }
    }

    return writen;
}

int file_write(DiskDrive drive, File *file, struct ByteArray src, int offset) {
    if (src.data == NULL) return 0;
    if (src.length <= 0) return 0;

    int end = offset + src.length;

    // make sure the file has enough block to store all data
    if (end > file->size) {
        file_truncate(drive, file, end);
    }

    // fist block to start writing
    int startBlock = get_blocks_from_size(offset) - 1;
    int writen = 0;

    // getSectorData leaves the sector in the drive buffer
    struct BlockBuffer fileBuffer = getSectorData(drive, file, startBlock);

    while (src.length > 0) {
        int bytesWriten = arrayToFile(fileBuffer, src, offset);
        save_sector(drive);

        src.data += bytesWriten;
        src.length -= bytesWriten;
        offset += bytesWriten;

        writen += bytesWriten;

        if (src.length > 0) {
            fileBuffer = nextBlockBuffer(drive, fileBuffer, file->firstBlock == fileBuffer.block);
        }
    }

    // update file lastModified
    if(writen > 0){
        load_sector(drive, file->firstBlock);
        FileFirstBlock *block = (FileFirstBlock *) disk_drive_get_buffer(drive);
        file->lastModified = motherboard_get_minecraft_world_time();
        block->metadata.lastModified = file->lastModified;
        save_sector(drive);
    }

    return writen;
}

int file_append(DiskDrive drive, File *file, struct ByteArray src) {
    if(file == NULL) return 0;
//    printf("[file_append] file = %s (0x%x), size = %d, src = { data = 0x%x, length = %d}\n",
//           file->name, (unsigned int) file, file->size,
//           (unsigned int) src.data, src.length);

    return file_write(drive, file, src, file->size);
}