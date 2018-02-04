//
// Created by cout970 on 27/01/18.
//

#include <motherboard.h>
#include "filesystem.h"
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CEIL(x, y) (((x) > 0)? 1 + ((x) - 1)/(y): ((x) / (y)))

static DiskDrive *drive = NULL;
static Int currentDevice = -1;
static Byte *blockBuffer = NULL;

static void bitmap_set(Byte *bitmap, Int index, Boolean value) {
    int x = index / 8;
    int y = index % 8;
    if (value) {
        bitmap[x] |= 1 << y;
    } else {
        bitmap[x] &= ~(1 << y);
    }
}

static Boolean bitmap_get(Byte *bitmap, Int index) {
    int x = index / 8;
    int y = index % 8;
    return (bitmap[x] & (1 << y)) ? TRUE : FALSE;
}

static void addChildren(INodeRef parent, String *childrenName, INodeRef children, Int parentSize) {
    struct DirectoryEntry newEntry;

    newEntry.inode = children;
    strcpy(newEntry.name, childrenName);

    // append entry
    fs_write(parent, (const ByteBuffer) &newEntry, parentSize, sizeof(struct DirectoryEntry));
}

#ifdef DEBUG_ENV

#include <stdio.h>
#include <debug.h>

static void loadSector(BlockRef sector) {
    FILE *file = fopen("file1.img", "rb");
    fseek(file, sector * 1024, SEEK_SET);
    memset(blockBuffer, 0, 1024);
    fread(blockBuffer, 1024, 1, file);
    fclose(file);
}

static void saveSector(BlockRef sector) {
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
static void strcpy(String *dst, const String *src) {
    for (; *src; dst++, src++) *dst = *src;
}

static void loadSector(BlockRef sector) {
    disk_drive_set_current_sector(drive, sector);
    disk_drive_signal(drive, DISK_DRIVE_SIGNAL_READ);
}

static  void saveSector(BlockRef sector) {
    disk_drive_set_current_sector(drive, sector);
    disk_drive_signal(drive, DISK_DRIVE_SIGNAL_WRITE);
}
#endif

static BlockRef allocBlock() {
    loadSector(0);
    struct SuperBlock *block = (struct SuperBlock *) blockBuffer;

    struct BlockGroup *group = &block->blockGroupList[0];

    for (int i = 0; i < group->numberOfBlocks; ++i) {
        if (!bitmap_get(group->blockBitmap, i)) {
            bitmap_set(group->blockBitmap, i, TRUE);
            saveSector(0);
            return i + group->blocksOffset;
        }
    }

    return FS_NULL_BLOCK_REF;
}

static void freeBlock(BlockRef ref) {
    loadSector(0);
    struct SuperBlock *block = (struct SuperBlock *) blockBuffer;

    struct BlockGroup *group = &block->blockGroupList[0];
    int index = ref - group->blocksOffset;
    if (index < 0 || index >= group->numberOfBlocks)
        return;

    bitmap_set(group->blockBitmap, index, FALSE);
    saveSector(0);
}

void fs_init(DiskDrive *_drive) {
    drive = _drive;
    blockBuffer = (Byte *) disk_drive_get_buffer(drive);
    currentDevice = fs_getDevice();
}

void fs_format() {
    if (drive == NULL) return;
    if (!disk_drive_has_disk(drive)) return;

    int sectorCount = disk_drive_get_num_sectors(drive);
    int time = motherboard_get_minecraft_world_time();

    // first block
    struct SuperBlock *block = (struct SuperBlock *) blockBuffer;
    {
        block->magicNumber = FS_MAGIC_NUMBER;
        block->deviceId = time;
        block->deviceSize = sectorCount * DISK_DRIVE_BUFFER_SIZE;
        block->numGroups = 1;
        block->nextBlockGroupList = FS_NULL_BLOCK_REF;
        currentDevice = time;

        struct BlockGroup *group = &block->blockGroupList[0];
        for (int i = 1; i < 16; ++i)
            group->blockBitmap[i] = 0;

        bitmap_set(group->blockBitmap, 0, TRUE);
        group->inodeTable = 1;
        group->blocksOffset = 3;
        group->numberOfBlocks = sectorCount - 2;
    }
    saveSector(0); // first block

    // second block
    struct INodeTable *inodes = (struct INodeTable *) blockBuffer;
    {
        inodes->nextTable = FS_NULL_BLOCK_REF;
        inodes->inodeBitmap = 0;
        bitmap_set((Byte *) &inodes->inodeBitmap, 0, TRUE);

        struct INode *node = &inodes->inodes[0];

        node->flags = FS_FLAG_DIRECTORY;
        node->size = sizeof(struct DirectoryEntry) * 2;
        node->creationTime = time;
        node->accessTime = time;
        node->modificationTime = time;
        node->blocksInUse = 1;
        node->blocks[0] = 2; // third block

    }
    saveSector(1); // second block

    // third block
    struct DirectoryEntry *entries = (struct DirectoryEntry *) blockBuffer;
    {
        entries[0].inode = 0;
        strcpy(entries[0].name, "..");
        entries[1].inode = 0;
        strcpy(entries[1].name, ".");
    }
    saveSector(2); // third block
}

Int fs_getDevice() {
    if (drive == NULL) return -1;
    if (!disk_drive_has_disk(drive)) return -1;
    loadSector(0);
    struct SuperBlock *block = (struct SuperBlock *) blockBuffer;
    if (block->magicNumber == FS_MAGIC_NUMBER) return -1;
    return block->deviceId;
}

INodeRef fs_getRoot() {
    return 0;
}

INodeRef fs_create(INodeRef parent, String *name, Int flags) {
    if (drive == NULL) return FS_NULL_BLOCK_REF;
    if (!disk_drive_has_disk(drive)) return FS_NULL_BLOCK_REF;

    struct INode parentNode;

    if (!fs_getINode(parent, &parentNode))
        return FS_NULL_BLOCK_REF;

    // If not a folder, return error
    if (parentNode.flags != FS_FLAG_DIRECTORY)
        return FS_NULL_BLOCK_REF;

    // If the a file with that name already exists, return error
    if (fs_findFile(parent, name) != FS_NULL_BLOCK_REF)
        return FS_NULL_BLOCK_REF;

    loadSector(0);
    struct SuperBlock *block = (struct SuperBlock *) blockBuffer;

    // Disk has changed set drive haven't been called
    if (block->deviceId != currentDevice) {
        return FS_NULL_BLOCK_REF;
    }

    BlockRef inodeTableRef = block->blockGroupList[0].inodeTable;

    int newINode = -1;
    loadSector(inodeTableRef);
    {
        struct INodeTable *table = (struct INodeTable *) blockBuffer;

        for (int i = 0; i < FS_INODES_PER_TABLE; ++i) {
            if (!bitmap_get((Byte *) &table->inodeBitmap, i)) {
                newINode = i;
                break;
            }
        }
        // TODO allocate new inode tables
        if (newINode == -1) return FS_NULL_BLOCK_REF;

        bitmap_set((Byte *) &table->inodeBitmap, newINode, TRUE);
        struct INode *node = &table->inodes[newINode];
        int time = motherboard_get_minecraft_world_time();

        node->flags = flags;
        node->size = 0;
        node->creationTime = time;
        node->accessTime = time;
        node->modificationTime = time;
        node->blocksInUse = 0;
        node->indirectBlock = FS_NULL_BLOCK_REF;
    }
    saveSector(inodeTableRef);

    // add file to the parent directory
    addChildren(parent, name, newINode, parentNode.size);

    return newINode;
}

Int fs_delete(INodeRef parent, INodeRef file) {
    if (parent == FS_NULL_BLOCK_REF || file == FS_NULL_BLOCK_REF || parent == file) return -1;

    struct INode parentNode;
    if (!fs_getINode(parent, &parentNode))
        return -1;

    // If not a folder, return error
    if (parentNode.flags != FS_FLAG_DIRECTORY)
        return -1;

    // Free all blocks of this file
    if (fs_truncate(file, 0))
        return -1;

    loadSector(0);
    struct SuperBlock *block = (struct SuperBlock *) blockBuffer;

    // Disk has changed set drive haven't been called
    if (block->deviceId != currentDevice) {
        return -1;
    }

    BlockRef inodeTableRef = block->blockGroupList[0].inodeTable;

    // Disable inode
    if (file < FS_INODES_PER_TABLE) {
        loadSector(inodeTableRef);
        {
            struct INodeTable *table = (struct INodeTable *) blockBuffer;
            bitmap_set((Byte *) &table->inodeBitmap, file, FALSE);
        }
        saveSector(inodeTableRef);
    } else {
        // TODO check other inode tables
        return -1;
    }

    // Remove entry in parent
    int read = 0, index = 0;
    struct DirectoryEntry entry;

    // read entries until the file is found
    do {
        read = fs_read(parent, (ByteBuffer) &entry, index * sizeof(struct DirectoryEntry),
                       sizeof(struct DirectoryEntry));

        if (read == 0) return -1;
        if (entry.inode == file) break;
        index++;

    } while (1);

    // Move all the other entries to fill the gap
    do {
        read = fs_read(parent, (ByteBuffer) &entry, (index + 1) * sizeof(struct DirectoryEntry),
                       sizeof(struct DirectoryEntry));

        if (read == 0) break;

        fs_write(parent, (ByteBuffer const) &entry, index * sizeof(struct DirectoryEntry),
                 sizeof(struct DirectoryEntry));

        index++;
    } while (1);

    // Reduce the parent size
    return fs_truncate(parent, parentNode.size - sizeof(struct DirectoryEntry));
}

INodeRef fs_findFile(INodeRef parent, String *name) {
    if (name == NULL) return FS_NULL_BLOCK_REF;
    if (parent < 0) return FS_NULL_BLOCK_REF;

    struct INode parentNode;
    if (!fs_getINode(parent, &parentNode))
        return FS_NULL_BLOCK_REF;

    int read = 0, index = 0;
    // read 8 entries at the time so there is less overhead for read operations
    struct DirectoryEntry entries[8];

    // reads all entries until the end of file or until we find the entry we want
    do {
        read = fs_read(parent, (ByteBuffer) &entries, index * sizeof(struct DirectoryEntry) * 8,
                       sizeof(struct DirectoryEntry) * 8);

        if (read == 0) break;

        index++;
        int readEntries = read / sizeof(struct DirectoryEntry);

        for (int i = 0; i < readEntries; ++i) {
            if (strcmp(entries[i].name, name)) {
                return entries[i].inode;
            }
        }
    } while (1);

    return FS_NULL_BLOCK_REF;
}

Boolean fs_getINode(INodeRef inode, struct INode *dst) {
    if (drive == NULL) return FALSE;
    if (!disk_drive_has_disk(drive)) return FALSE;

    if (inode < 0) {
        return FALSE;
    }
    loadSector(0);
    struct SuperBlock *block = (struct SuperBlock *) blockBuffer;

    // Disk has changed set drive haven't been called
    if (block->deviceId != currentDevice) {
        return FALSE;
    }

    BlockRef inodeTableRef = block->blockGroupList[0].inodeTable;

    loadSector(inodeTableRef);
    struct INodeTable *table = (struct INodeTable *) blockBuffer;

    if (inode >= FS_INODES_PER_TABLE) {
        return FALSE;
    }

    if (!bitmap_get((Byte *) &table->inodeBitmap, inode)) {
        return FALSE;
    }

    memcpy(dst, &table->inodes[inode], sizeof(struct INode));
    return TRUE;
}

Boolean fs_setINode(INodeRef inode, struct INode *src) {
    if (drive == NULL) return FALSE;
    if (!disk_drive_has_disk(drive)) return FALSE;

    if (inode < 0) {
        return FALSE;
    }
    loadSector(0);
    struct SuperBlock *block = (struct SuperBlock *) blockBuffer;

    // Disk has changed set drive haven't been called
    if (block->deviceId != currentDevice) {
        return FALSE;
    }

    BlockRef inodeTableRef = block->blockGroupList[0].inodeTable;

    loadSector(inodeTableRef);
    struct INodeTable *table = (struct INodeTable *) blockBuffer;

    if (inode >= FS_INODES_PER_TABLE) {
        return FALSE;
    }

    if (!bitmap_get((Byte *) &table->inodeBitmap, inode)) {
        return FALSE;
    }

    memcpy(&table->inodes[inode], src, sizeof(struct INode));
    saveSector(inodeTableRef);
    return TRUE;
}

Boolean fs_truncate(INodeRef inode, Int size) {
    struct INode node;

    if (!fs_getINode(inode, &node))
        return FALSE;

    if (node.size < size) {
        int newBlockCount = CEIL(size, DISK_DRIVE_BUFFER_SIZE);
        if (newBlockCount > node.blocksInUse) {

            if (node.blocksInUse < FS_NUM_DIRECT_BLOCKS) {
                for (int i = node.blocksInUse; i < newBlockCount; ++i) {
                    int newBlock = allocBlock();
                    if (newBlock == FS_NULL_BLOCK_REF) {
                        fs_setINode(inode, &node);
                        return FALSE;
                    }

                    node.blocks[i] = newBlock;
                    node.size += DISK_DRIVE_BUFFER_SIZE;
                    node.blocksInUse++;
                }
            }
            if (newBlockCount >= FS_NUM_DIRECT_BLOCKS) {

                for (int i = node.blocksInUse; i < newBlockCount; ++i) {
                    int newBlock = allocBlock();
                    if (newBlock == FS_NULL_BLOCK_REF) {
                        fs_setINode(inode, &node);
                        return FALSE;
                    }
                    int pointerArrayIndex = i - FS_NUM_DIRECT_BLOCKS;

                    if (node.indirectBlock == FS_NULL_BLOCK_REF) {
                        node.indirectBlock = allocBlock();

                        // Error allocating a new block
                        if (node.indirectBlock == FS_NULL_BLOCK_REF) {
                            fs_setINode(inode, &node);
                            return FALSE;
                        }
                    }

                    loadSector(node.indirectBlock);
                    {
                        ((int *) blockBuffer)[pointerArrayIndex] = newBlock;
                    }
                    saveSector(node.indirectBlock);

                    node.size += DISK_DRIVE_BUFFER_SIZE;
                    node.blocksInUse++;
                }
            }
        }

        node.size = size;
        return fs_setINode(inode, &node);
    } else {
        int newBlockCount = CEIL(size, DISK_DRIVE_BUFFER_SIZE);
        if (node.blocksInUse < FS_NUM_DIRECT_BLOCKS) {
            for (int i = node.blocksInUse - 1; i >= newBlockCount; --i) {
                freeBlock(node.blocks[i]);
                node.blocksInUse--;
            }
        } else {
            // TODO
        }
        node.size = size;
        return fs_setINode(inode, &node);
    }
    return FALSE;
}

Int fs_write(INodeRef inode, const ByteBuffer buf, Int offset, Int nbytes) {
    if (inode < 0 || nbytes < 0 || offset < 0 || buf == NULL) return 0;

    struct INode node;
    if (!fs_getINode(inode, &node))
        return 0;

    if (node.size <= offset + nbytes) {
        if (!fs_truncate(inode, offset + nbytes))
            return 0;
        fs_getINode(inode, &node);
    }

    ByteBuffer bufPtr = buf;
    int written = 0;
    do {
        int blockNum = offset / DISK_DRIVE_BUFFER_SIZE;
        int blockOff = offset % DISK_DRIVE_BUFFER_SIZE;

        int blockToWrite;
        int index = blockNum - FS_NUM_DIRECT_BLOCKS;

        if (index < 0) {
            blockToWrite = node.blocks[blockNum];

        } else if (index < (DISK_DRIVE_BUFFER_SIZE / 4)) {

            loadSector(node.indirectBlock);
            {
                blockToWrite = ((int *) blockBuffer)[index];
            }

            if (blockToWrite == FS_NULL_BLOCK_REF)
                return written;

        } else {
            return written;
        }

        loadSector(blockToWrite);
        {
            int amount = MIN(nbytes, DISK_DRIVE_BUFFER_SIZE - blockOff);
            memcpy(blockBuffer + blockOff, bufPtr, (size_t) amount);
            offset += amount;
            bufPtr += amount;
            written += amount;
            nbytes -= amount;
        }
        saveSector(blockToWrite);

    } while (nbytes > 0);

    return written;
}

Int fs_read(INodeRef inode, ByteBuffer buf, Int offset, Int nbytes) {
    if (inode < 0 || nbytes < 0 || offset < 0 || buf == NULL) return 0;

    struct INode node;
    if (!fs_getINode(inode, &node))
        return 0;

    ByteBuffer bufPtr = buf;
    int read = 0;
    do {
        int canRead = node.size - offset;
        if (canRead <= 0) return 0;

        int blockNum = offset / DISK_DRIVE_BUFFER_SIZE;
        int blockOff = offset % DISK_DRIVE_BUFFER_SIZE;

        int blockToRead;

        if (blockNum < FS_NUM_DIRECT_BLOCKS) {
            blockToRead = node.blocks[blockNum];

        } else if (blockNum - FS_NUM_DIRECT_BLOCKS > (DISK_DRIVE_BUFFER_SIZE / 4)) {
            loadSector(node.indirectBlock);
            {
                blockToRead = ((int *) blockBuffer)[blockNum - FS_NUM_DIRECT_BLOCKS];
            }
            saveSector(node.indirectBlock);

            if (blockToRead == FS_NULL_BLOCK_REF)
                return read;

        } else {
            return read;
        }

        loadSector(blockToRead);
        {
            int amount = MIN(canRead, nbytes);
            memcpy(bufPtr, blockBuffer + blockOff, (size_t) amount);
            offset += amount;
            bufPtr += amount;
            read += amount;
            nbytes -= amount;
        }
        saveSector(blockToRead);

    } while (nbytes > 0);

    return read;
}