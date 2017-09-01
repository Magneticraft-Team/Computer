//
// Created by cout970 on 2017-08-24.
//

#include "api/math.h"
#include "filesystem.h"
#include "api/string.h"
#include "util/static_list.h"

static SuperBlock superBlockCache;

int file_equals(File* a, File* b){
    return a->firstBlock == b->firstBlock;
}

// non malloc list of 'File' up to MAX_OPEN_FILES
StaticList(open_files, File, MAX_OPEN_FILES, file_equals);

static void load_sector(DiskDrive drive, int sector) {
    disk_drive_set_current_sector(drive, sector);
    disk_drive_signal(drive, DISK_DRIVE_SIGNAL_READ);
    motherboard_sleep((i8) disk_drive_get_access_time(drive));
}

static void save_sector(DiskDrive drive) {
    disk_drive_signal(drive, DISK_DRIVE_SIGNAL_WRITE);
    motherboard_sleep((i8) disk_drive_get_access_time(drive));
}

static void save_super_block(DiskDrive drive) {
    load_sector(drive, 0);
    memcpy((void *) disk_drive_get_buffer(drive), &superBlockCache, sizeof(superBlockCache));
    save_sector(drive);
}

static int allocate_sector(DiskDrive drive) {
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

static void free_sector(DiskDrive drive, int sector) {
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

static int get_file_in_folder(DiskDrive drive, File *folder, const char *name) {
    int entryCount = folder->size / sizeof(DirectoryEntry);
    DirectoryEntry entry;
    for (int i = 0; i < entryCount; ++i) {
        file_read(drive, folder, &entry, sizeof(DirectoryEntry) * i, sizeof(DirectoryEntry));
        if (strcmp(entry.name, name) == 0) {
            return entry.firstBlock;
        }
    }
    return NULL_SECTOR;
}

static int open_files_count() {
    return open_files.used;
}

static int get_blocks_from_size(int size) {
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

static void alloc_blocks_up_to(DiskDrive drive, File *file, int totalBlocksNeeded) {
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

static int file_is_open(File* file){
    return open_files_index_of(file) != -1;
}

void makeFileSystem(DiskDrive drive) {

    load_sector(drive, 0);
    SuperBlock *buffer = (SuperBlock *) disk_drive_get_buffer(drive);

    if (buffer->magicNumber == FILE_SYSTEM_MAGIC_NUMBER) {
        memcpy(&superBlockCache, buffer, sizeof(SuperBlock));
        return;
    }

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

    open_files_add(&root->metadata);

    save_sector(drive);
}

File *file_get_root(DiskDrive drive) {
    if (open_files_count() == 0) {
        makeFileSystem(drive);
    }
    load_sector(drive, superBlockCache.rootDirectory);
    FileFirstBlock *block = (FileFirstBlock *) disk_drive_get_buffer(drive);
    memcpy(&open_files.items[0].data, block, sizeof(File));

    return &open_files.items[0].data;
}

// Create/Delete
File *file_create(DiskDrive drive, File *parent, const char *name, int type) {
    if (parent == NULL) return NULL;
    if (parent->type != FILE_TYPE_DIRECTORY) return NULL;
    if (strlen(name) >= FILE_MAX_NAME_SIZE) return NULL;
    if (open_files_count() >= MAX_OPEN_FILES) return NULL;

    int sector = allocate_sector(drive);
    load_sector(drive, sector);
    FileFirstBlock *header = (FileFirstBlock *) disk_drive_get_buffer(drive);

    header->metadata.firstBlock = sector;
    header->metadata.nextBlock = NULL_SECTOR;
    strcpy(header->metadata.name, name);
    header->metadata.parent = parent->firstBlock;
    header->metadata.type = type;
    header->metadata.size = 0;
    header->metadata.lastModified = motherboard_get_minecraft_world_time();

    File *file = open_files_add(&header->metadata);

    save_sector(drive);

    DirectoryEntry newEntry;
    newEntry.firstBlock = file->firstBlock;
    memcpy(newEntry.name, file->name, FILE_MAX_NAME_SIZE);

    file_append(drive, parent, &newEntry, sizeof(DirectoryEntry));

    // add parent folder link
    if(type == FILE_TYPE_DIRECTORY){
        DirectoryEntry parentEntry;
        parentEntry.firstBlock = parent->firstBlock;
        memcpy(parentEntry.name, "..", 3);
        file_append(drive, file, &parentEntry, sizeof(DirectoryEntry));
    }

    return file;
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

int file_remove_child(DiskDrive drive, File *parent, File *child) {
    if (parent == NULL) return 0;
    if (parent->type != FILE_TYPE_DIRECTORY) return 0;

    // remove file from parent folder
    DirectoryEntry entry;
    int offset = 0;
    int read;
    do {
        read = file_read(drive, parent, &entry, offset, sizeof(DirectoryEntry));
        if (read == sizeof(DirectoryEntry) && strcmp(entry.name, child->name) == 0) {
            // start moving all entries
            do {
                read = file_read(drive, parent, &entry, offset + sizeof(DirectoryEntry), sizeof(DirectoryEntry));
                if (read == 0) break;
                file_write(drive, parent, &entry, offset, sizeof(DirectoryEntry));
                offset += sizeof(DirectoryEntry);
            } while (1);

            file_truncate(drive, parent, parent->size - sizeof(DirectoryEntry));
            return 1;
        }
        offset += sizeof(DirectoryEntry);
    } while (read != 0);

    return 0;
}

void file_delete(DiskDrive drive, File *parent, File *file) {
    // you can delete the root directory
    if(file == &open_files.items[0].data) return;
    if (!file_remove_child(drive, parent, file)) return;

    if (file->type == FILE_TYPE_DIRECTORY) {
        DirectoryEntry entry;
        int offset = 0;
        int read;
        do {
            read = file_read(drive, file, &entry, offset, sizeof(DirectoryEntry));
            if(read == sizeof(DirectoryEntry)){
                File entryFile;
                load_sector(drive, entry.firstBlock);
                memcpy(&entryFile, (const void *) disk_drive_get_buffer(drive), sizeof(File));
                file_delete(drive, file, &entryFile);
            }
            offset += sizeof(DirectoryEntry);
        } while (read != 0);
    }
    file_truncate(drive, file, 0);
    free_sector(drive, file->firstBlock);

    // remove file from openFiles
    file_close(drive, file);
}

// Open/Close
File *file_open(DiskDrive drive, File *parent, const char *name) {
    if (parent == NULL) return NULL;
    if (parent->type != FILE_TYPE_DIRECTORY) return NULL;
    if (strlen(name) >= FILE_MAX_NAME_SIZE) return NULL;
    if (open_files_count() >= MAX_OPEN_FILES) return NULL;

    int sector = get_file_in_folder(drive, parent, name);
    if (sector == NULL_SECTOR) return NULL;

    load_sector(drive, sector);
    FileFirstBlock *header = (FileFirstBlock *) disk_drive_get_buffer(drive);\

    return open_files_add(&header->metadata);
}

void file_close(DiskDrive drive IGNORED, File *file) {
    if (file == NULL) return;
    // root folder cache
    if (file == &open_files.items[0].data) return;
    if (open_files_count() <= 1) return;

    if(file_is_open(file)){
        open_files_remove(file);
    }
}

// Read/Write
int file_read(DiskDrive drive, File *file, void *buff, int offset, int bufSize) {
    if (buff == NULL) return 0;
    if (bufSize <= 0) return 0;

    load_sector(drive, file->firstBlock);
    FileFirstBlock *firstBuffer = (FileFirstBlock *) disk_drive_get_buffer(drive);
    BlockHeader *currentBuff = (BlockHeader *) firstBuffer;

    int size = firstBuffer->metadata.size;
    if (offset >= size) return 0;

    int sector;
    int read = 0;
    int currentOffset = offset;

    //start reading from the first block
    if ((unsigned int)currentOffset < FILE_FIRST_BLOCK_SPACE) {
        int available = min((size_t)size, FILE_FIRST_BLOCK_SPACE) - currentOffset;
        int toRead = min(bufSize, available);

        memcpy(buff, firstBuffer->content + currentOffset, (size_t) toRead);
        read += toRead;
        currentOffset += toRead;

        sector = firstBuffer->metadata.nextBlock;
        while (bufSize > read && read < size) {
            load_sector(drive, sector);
            currentBuff = (BlockHeader *) disk_drive_get_buffer(drive);

            available = min(size - currentOffset, FILE_NORMAL_BLOCK_SPACE);
            toRead = min(bufSize - read, available);

            memcpy(buff + read, currentBuff->content, (size_t) toRead);
            read += toRead;
            currentOffset += toRead;
            sector = currentBuff->nextBlock;
        }
        return read;
    }

    int behind = FILE_FIRST_BLOCK_SPACE;
    sector = firstBuffer->metadata.nextBlock;
    while (currentOffset >= behind) {
        load_sector(drive, sector);
        currentBuff = (BlockHeader *) disk_drive_get_buffer(drive);
        behind += FILE_NORMAL_BLOCK_SPACE;
        sector = currentBuff->nextBlock;
    }

    int available = min(size - currentOffset, FILE_NORMAL_BLOCK_SPACE);
    int toRead = min(bufSize, available);

    memcpy(buff, firstBuffer->content, (size_t) toRead);
    read += toRead;
    currentOffset += toRead;

    sector = currentBuff->firstBlock;
    while (bufSize > read && read < size) {
        load_sector(drive, sector);
        currentBuff = (BlockHeader *) disk_drive_get_buffer(drive);

        available = min(size - currentOffset, FILE_NORMAL_BLOCK_SPACE);
        toRead = min(bufSize - read, available);

        memcpy(buff + read, currentBuff->content, (size_t) toRead);
        read += toRead;
        currentOffset += toRead;
        sector = currentBuff->nextBlock;
    }
    return read;
}

int file_write(DiskDrive drive, File *file, void *buff, int offset, int size) {
    if (buff == NULL) return 0;
    if (size <= 0) return 0;

    int end = offset + size;

    // make sure the file has enough block to store all data
    if (end > file->size) {
        file_truncate(drive, file, end);
    }

    // fist block to start writing
    int startBlock = get_blocks_from_size(offset);
    // ptr to the buffer, so it can be different depending on the block type (first or normal)
    char *writeBufferPtr;
    // writeBuffer size
    int writeBufferSize;
    // offset of the buffer respect to the file start
    int bufferFileOffset;
    // byte writen
    int writen = 0;
    // next block to load if necesary
    int nextBlock = file->nextBlock;
    // blocks loaded
    int blockCount = 1;

    if (startBlock == 1) {
        // start writing in the first block

        load_sector(drive, file->firstBlock);
        FileFirstBlock *block = (FileFirstBlock *) disk_drive_get_buffer(drive);

        bufferFileOffset = 0;
        writeBufferPtr = block->content;
        writeBufferSize = FILE_FIRST_BLOCK_SPACE;

    } else {
        BlockHeader *block = NULL;
        bufferFileOffset = FILE_FIRST_BLOCK_SPACE;

        // invalidate first while loop incrementing writeBuffFileOffset
        bufferFileOffset -= FILE_NORMAL_BLOCK_SPACE;
        // iterate until reach the desired block
        do {
            bufferFileOffset += FILE_NORMAL_BLOCK_SPACE;

            load_sector(drive, nextBlock);
            block = (BlockHeader *) disk_drive_get_buffer(drive);

            nextBlock = block->nextBlock;
            blockCount++;

        } while (blockCount < startBlock);

        writeBufferPtr = block->content;
        writeBufferSize = FILE_NORMAL_BLOCK_SPACE;
    }

    int offsetRelativeToBuffer = offset - bufferFileOffset;
    int toWrite = min(writeBufferSize - offsetRelativeToBuffer, size);
    memcpy(writeBufferPtr + offsetRelativeToBuffer, buff, (size_t) toWrite);
    writen += toWrite;
    // move offset and buffer to write more data
    offset += toWrite;
    buff += toWrite;
    // decrement size to break next while lopp if needed
    size -= toWrite;

    // save the buffer after writing
    save_sector(drive);

    // this should only be added once for the firs block
    if (startBlock == 1) {
        bufferFileOffset += FILE_FIRST_BLOCK_SPACE;
    } else {
        bufferFileOffset += FILE_NORMAL_BLOCK_SPACE;
    }
    while (size > 0) {

        BlockHeader *block = NULL;

        load_sector(drive, nextBlock);
        block = (BlockHeader *) disk_drive_get_buffer(drive);
        nextBlock = block->nextBlock;
        blockCount++;

        writeBufferPtr = block->content;
        writeBufferSize = FILE_NORMAL_BLOCK_SPACE;


        offsetRelativeToBuffer = offset - bufferFileOffset;
        toWrite = min(writeBufferSize - offsetRelativeToBuffer, size);
        memcpy(writeBufferPtr + offsetRelativeToBuffer, buff, (size_t) toWrite);
        writen += toWrite;

        offset += toWrite;
        buff += toWrite;

        size -= toWrite;
        bufferFileOffset += FILE_NORMAL_BLOCK_SPACE;

        save_sector(drive);
    }

    return writen;
}

int file_append(DiskDrive drive, File *file, void *input, int bufSize) {
    return file_write(drive, file, input, file->size, bufSize);
}