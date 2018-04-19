//
// Created by cout970 on 27/01/18.
//

#include <motherboard.h>
#include <fs/filesystem.h>
#include <string.h>
#include <macros.h>
#include <fs/access.h>
#include <util/bitmap.h>
#include <debug.h>

static Int currentDevice = -1;

static void addChild(INodeRef parent, String *childrenName, INodeRef children, Int parentSize) {
    struct DirectoryEntry newEntry;

    newEntry.inode = children;
    strcpy(newEntry.name, childrenName);

    // append entry
    fs_write(parent, (const ByteBuffer) &newEntry, parentSize, sizeof(struct DirectoryEntry));
}

static void removeChildren(INodeRef parent) {
    struct DirectoryEntry entry;
    int read, index = 0;

    while (1) {
        read = fs_read(parent, (const ByteBuffer) &entry,
                       sizeof(struct DirectoryEntry) * index++, sizeof(struct DirectoryEntry));

        if (read == 0) return;
        if (strcmp(entry.name, "..") != 0 && strcmp(entry.name, ".") != 0) {
            fs_delete(parent, entry.inode);
        }
    }
}

static BlockRef allocBlock() {
    loadSector(0);
    struct SuperBlock *block = (struct SuperBlock *) blockBuffer;

    struct BlockGroup *group = &block->blockGroupList[0];

    for (int i = 0; i < group->numberOfBlocks; ++i) {
        if (!bitmap_get(group->blockBitmap, i)) {
            bitmap_set(group->blockBitmap, i, TRUE);
            saveSector(0);
//            kdebug("allocBlock success, block: %d\n", i + group->blocksOffset);
            return i + group->blocksOffset;
        }
    }

    kdebug("allocBlock failed\n");
    return FS_NULL_BLOCK_REF;
}

static void freeBlock(BlockRef ref) {
    loadSector(0);
    struct SuperBlock *block = (struct SuperBlock *) blockBuffer;

    struct BlockGroup *group = &block->blockGroupList[0];
    int index = ref - group->blocksOffset;
    if (index < 0 || index >= group->numberOfBlocks) {
        kdebug("freeBlock %d failed, invalid index: %d\n", ref, index);
        return;
    }

    bitmap_set(group->blockBitmap, index, FALSE);
    saveSector(0);
//    kdebug("freeBlock %d success\n", ref);
}

void fs_init(DiskDrive *_drive) {
    currentDiskDrive = _drive;
    blockBuffer = (Byte *) disk_drive_get_buffer(currentDiskDrive);
    currentDevice = fs_getDevice();
}

void fs_format() {
    if (currentDiskDrive == NULL) return;
    if (!disk_drive_has_disk(currentDiskDrive)) return;

    int sectorCount = disk_drive_get_num_sectors(currentDiskDrive);
    int time = motherboard_get_minecraft_world_time();

    // first block
    struct SuperBlock *block = (struct SuperBlock *) blockBuffer;
    {
        memset(blockBuffer, 0, DISK_DRIVE_BUFFER_SIZE);
        block->magicNumber = FS_MAGIC_NUMBER;
        block->deviceId = time;
        block->deviceSize = sectorCount * DISK_DRIVE_BUFFER_SIZE;
        block->numGroups = 1;
        block->nextBlockGroupList = FS_NULL_BLOCK_REF;
        currentDevice = block->deviceId;

        struct BlockGroup *group = &block->blockGroupList[0];
        memset(group->blockBitmap, 0, FS_BLOCK_GROUP_BITMAP_BYTES);

        bitmap_set(group->blockBitmap, 0, TRUE); // inode table
        bitmap_set(group->blockBitmap, 1, TRUE); // root directory

        group->inodeTable = 1;
        group->blocksOffset = 1;
        group->numberOfBlocks = MIN(sectorCount - 1, 128);
    }
    saveSector(0); // first block

    // second block
    struct INodeTable *inodes = (struct INodeTable *) blockBuffer;
    {
        memset(blockBuffer, 0, DISK_DRIVE_BUFFER_SIZE);

        inodes->nextTable = FS_NULL_BLOCK_REF;
        inodes->inodeBitmap = 0;
        bitmap_set((Byte *) &inodes->inodeBitmap, 0, TRUE);

        // set all inodes BlockRefs to NULL
        for (int i = 0; i < FS_INODES_PER_TABLE; ++i) {
            inodes->inodes[i].indirectBlock = FS_NULL_BLOCK_REF;
            memset(inodes->inodes[i].blocks, FS_NULL_BLOCK_REF, FS_NUM_DIRECT_BLOCKS * sizeof(BlockRef));
        }

        // setup root folder
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
        memset(blockBuffer, 0, DISK_DRIVE_BUFFER_SIZE);
        entries[0].inode = 0;
        strcpy(entries[0].name, "..");
        entries[1].inode = 0;
        strcpy(entries[1].name, ".");
    }
    saveSector(2); // third block
}

Int fs_getFreeBlocks() {
    if (currentDiskDrive == NULL) return -1;
    if (!disk_drive_has_disk(currentDiskDrive)) return -1;

    loadSector(0);
    struct SuperBlock *block = (struct SuperBlock *) blockBuffer;

    // Disk has changed set drive haven't been called
    if (block->deviceId != currentDevice) {
        return -1;
    }

    int count = 0;

    for (int i = 0; i < block->numGroups; ++i) {
        struct BlockGroup *group = &block->blockGroupList[i];
        for (int j = 0; j < group->numberOfBlocks; ++j) {
            if (!bitmap_get(group->blockBitmap, j)) count++;
        }
    }

    return count;
}

Int fs_getDevice() {
    if (currentDiskDrive == NULL) return -1;
    if (!disk_drive_has_disk(currentDiskDrive)) return -1;
    loadSector(0);
    struct SuperBlock *block = (struct SuperBlock *) blockBuffer;
    if (block->magicNumber != FS_MAGIC_NUMBER)
        return -1;

    return block->deviceId;
}

INodeRef fs_getRoot() {
    return 0;
}

static Boolean fs_findFreeINode(BlockRef inodeTableRef, INodeRef *result) {
    loadSector(inodeTableRef);

    struct INodeTable *table = (struct INodeTable *) blockBuffer;

    for (int i = 0; i < FS_INODES_PER_TABLE; ++i) {
        if (!bitmap_get((Byte *) &table->inodeBitmap, i)) {
            *result = i;
            return TRUE;
        }
    }
    *result = table->nextTable;
    return FALSE;
}

INodeRef fs_create(INodeRef parent, String *name, Int flags) {
    if (currentDiskDrive == NULL) return FS_NULL_INODE_REF;
    if (!disk_drive_has_disk(currentDiskDrive)) return FS_NULL_INODE_REF;

    struct INode parentNode;

    if (!fs_getINode(parent, &parentNode))
        return FS_NULL_INODE_REF;

    // If not a folder, return error
    if (parentNode.flags != FS_FLAG_DIRECTORY)
        return FS_NULL_INODE_REF;

    // If the a file with that name already exists, return error
    if (fs_findFile(parent, name) != FS_NULL_INODE_REF)
        return FS_NULL_INODE_REF;

    loadSector(0);
    struct SuperBlock *block = (struct SuperBlock *) blockBuffer;

    // Disk has changed set drive haven't been called
    if (block->deviceId != currentDevice) {
        return FS_NULL_INODE_REF;
    }

    int newINode = -1;
    BlockRef result, tableBlock = block->blockGroupList[0].inodeTable, inodeOffset = 0;

    do {
        if (!fs_findFreeINode(tableBlock, &result)) {
            // table full, result is the next table
            int newTableBlock = result;
            inodeOffset += FS_INODES_PER_TABLE;

            if (newTableBlock == FS_NULL_INODE_REF) {
                // allocate new block for the table
                newTableBlock = allocBlock();
                if (newTableBlock == FS_NULL_BLOCK_REF)
                    return FS_NULL_INODE_REF;

                loadSector(tableBlock);
                {
                    struct INodeTable *table = (struct INodeTable *) blockBuffer;
                    table->nextTable = newTableBlock;
                }
                saveSector(tableBlock);

                // init the new table
                loadSector(newTableBlock);
                {
                    struct INodeTable *table = (struct INodeTable *) blockBuffer;
                    memset(blockBuffer, 0, DISK_DRIVE_BUFFER_SIZE);
                    table->nextTable = FS_NULL_BLOCK_REF;
                    table->inodeBitmap = 0;
                }
                saveSector(newTableBlock);
            }
            tableBlock = newTableBlock;
        } else {
            // found empty spot, result is the spot
            newINode = result + inodeOffset;
        }
    } while (newINode == -1);

    loadSector(tableBlock);
    {
        struct INodeTable *table = (struct INodeTable *) blockBuffer;

        bitmap_set((Byte *) &table->inodeBitmap, newINode - inodeOffset, TRUE);
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
    saveSector(tableBlock);

    // add file to the parent directory
    addChild(parent, name, newINode, parentNode.size);

    // Add .. and .
    if (flags == FS_FLAG_DIRECTORY) {
        struct DirectoryEntry entries[2];

        entries[0].inode = parent;
        memset(entries[0].name, 0, FS_MAX_FILE_NAME_SIZE);
        strcpy(entries[0].name, "..");
        entries[1].inode = newINode;
        memset(entries[1].name, 0, FS_MAX_FILE_NAME_SIZE);
        strcpy(entries[1].name, ".");

        fs_write(newINode, (ByteBuffer const) &entries, 0, sizeof(struct DirectoryEntry) * 2);
    }

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

    struct INode fileNode;
    if (!fs_getINode(file, &fileNode))
        return -1;

    if (fileNode.flags == FS_FLAG_DIRECTORY) {
        removeChildren(file);
    }

    // Free all blocks of this file
    if (!fs_truncate(file, 0))
        return -1;

    loadSector(0);
    struct SuperBlock *block = (struct SuperBlock *) blockBuffer;
    BlockRef inodeTableRef = block->blockGroupList[0].inodeTable;

    // Disable inode

    int tableIndex = file / FS_INODES_PER_TABLE;

    for (int i = 0; i < tableIndex; ++i) {
        loadSector(inodeTableRef);
        struct INodeTable *table = (struct INodeTable *) blockBuffer;
        inodeTableRef = table->nextTable;
        if (inodeTableRef == FS_NULL_BLOCK_REF)
            return -1;
    }

    loadSector(inodeTableRef);
    {
        struct INodeTable *table = (struct INodeTable *) blockBuffer;
        bitmap_set((Byte *) &table->inodeBitmap, file, FALSE);
    }
    saveSector(inodeTableRef);

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
    return fs_truncate(parent, parentNode.size - sizeof(struct DirectoryEntry)) ? 0 : 1;
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
            if (strcmp(entries[i].name, name) == 0) {
                return entries[i].inode;
            }
        }
    } while (1);

    return FS_NULL_BLOCK_REF;
}

Boolean fs_getINode(INodeRef inode, struct INode *dst) {
    if (currentDiskDrive == NULL) return FALSE;
    if (!disk_drive_has_disk(currentDiskDrive)) return FALSE;

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
    int tableIndex = inode / FS_INODES_PER_TABLE;

    for (int i = 0; i < tableIndex; ++i) {
        loadSector(inodeTableRef);
        struct INodeTable *table = (struct INodeTable *) blockBuffer;
        inodeTableRef = table->nextTable;
        if (inodeTableRef == FS_NULL_BLOCK_REF)
            return FALSE;
    }

    loadSector(inodeTableRef);
    struct INodeTable *table = (struct INodeTable *) blockBuffer;
    int index = inode - tableIndex * FS_INODES_PER_TABLE;

    if (!bitmap_get((Byte *) &table->inodeBitmap, index)) {
        return FALSE;
    }

    memcpy(dst, &table->inodes[inode], sizeof(struct INode));
    return TRUE;
}

Boolean fs_setINode(INodeRef inode, struct INode *src) {
    if (currentDiskDrive == NULL) return FALSE;
    if (!disk_drive_has_disk(currentDiskDrive)) return FALSE;

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
    int tableIndex = inode / FS_INODES_PER_TABLE;

    for (int i = 0; i < tableIndex; ++i) {
        loadSector(inodeTableRef);
        struct INodeTable *table = (struct INodeTable *) blockBuffer;
        inodeTableRef = table->nextTable;
        if (inodeTableRef == FS_NULL_BLOCK_REF)
            return FALSE;
    }

    loadSector(inodeTableRef);
    struct INodeTable *table = (struct INodeTable *) blockBuffer;
    int index = inode - tableIndex * FS_INODES_PER_TABLE;

    if (!bitmap_get((Byte *) &table->inodeBitmap, index)) {
        return FALSE;
    }

    memcpy(&table->inodes[inode], src, sizeof(struct INode));
    saveSector(inodeTableRef);
    return TRUE;
}

Boolean fs_truncate(INodeRef inode, Int size) {
    struct INode node;

    // non-existent inode
    if (!fs_getINode(inode, &node))
        return FALSE;

    if (node.size < size) {
        // increase size

        // new amount of blocks that need to be allocated
        int newBlockCount = CEIL_DIV(size, DISK_DRIVE_BUFFER_SIZE);

        if (newBlockCount > node.blocksInUse) {

            // use the direct blocks field to store the new blocks addresses
            if (node.blocksInUse < FS_NUM_DIRECT_BLOCKS) {
                int directBlocks = MIN(newBlockCount, FS_NUM_DIRECT_BLOCKS);

                for (int i = node.blocksInUse; i < directBlocks; ++i) {
                    int newBlock = allocBlock();
                    if (newBlock == FS_NULL_BLOCK_REF) {
                        // make sure the system fails without an inconsistent state
                        fs_setINode(inode, &node);
                        return FALSE;
                    }

                    node.blocks[i] = newBlock;
                    node.size += DISK_DRIVE_BUFFER_SIZE;
                    node.blocksInUse++;
                }
            }

            // use the indirect block for the extra block addresses
            if (newBlockCount >= FS_NUM_DIRECT_BLOCKS) {

                for (int i = node.blocksInUse; i < newBlockCount; ++i) {
                    int newBlock = allocBlock();
                    if (newBlock == FS_NULL_BLOCK_REF) {
                        // make sure the system fails without an inconsistent state
                        fs_setINode(inode, &node);
                        return FALSE;
                    }

                    if (node.indirectBlock == FS_NULL_BLOCK_REF) {
                        node.indirectBlock = allocBlock();

                        // Error allocating a new block
                        if (node.indirectBlock == FS_NULL_BLOCK_REF) {
                            fs_setINode(inode, &node);
                            return FALSE;
                        }
                    }

                    int pointerArrayIndex = i - FS_NUM_DIRECT_BLOCKS;

                    if (pointerArrayIndex >= (int) (DISK_DRIVE_BUFFER_SIZE / sizeof(BlockRef))) {
                        // Indirect block overflow, reached max file size
                        fs_setINode(inode, &node);
                        return FALSE;
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
        // decrease size

        int newBlockCount = CEIL_DIV(size, DISK_DRIVE_BUFFER_SIZE);

        // do we need to free blocks?
        if (node.blocksInUse > newBlockCount) {

            // free indirect blocks
            if (node.blocksInUse > FS_NUM_DIRECT_BLOCKS) {
                int indirectBlocksToKeep = MAX(0, newBlockCount - FS_NUM_DIRECT_BLOCKS);
                int blockToFree;

                for (int i = node.blocksInUse - 1 - +FS_NUM_DIRECT_BLOCKS; i >= indirectBlocksToKeep; --i) {
                    loadSector(node.indirectBlock);
                    {
                        blockToFree = ((int *) blockBuffer)[i];
                        ((int *) blockBuffer)[i] = FS_NULL_BLOCK_REF;
                    }
                    saveSector(node.indirectBlock);

                    freeBlock(blockToFree);
                    node.blocksInUse--;
                }


                if (indirectBlocksToKeep == 0) {
                    freeBlock(node.indirectBlock);
                    node.indirectBlock = FS_NULL_BLOCK_REF;
                }
            }

            for (int i = node.blocksInUse - 1; i >= newBlockCount; --i) {
                freeBlock(node.blocks[i]);
                node.blocks[i] = FS_NULL_BLOCK_REF;
                node.blocksInUse--;
            }
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
    } else {
        node.modificationTime = motherboard_get_minecraft_world_time();
        fs_setINode(inode, &node);
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

    node.accessTime = motherboard_get_minecraft_world_time();
    fs_setINode(inode, &node);

    ByteBuffer bufPtr = buf;
    int read = 0;
    do {
        int canRead = node.size - offset;
        if (canRead <= 0) return read;

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

void fs_iterInit(INodeRef directory, struct DirectoryIterator *iter) {
    iter->index = 0;
    iter->entry.inode = FS_NULL_BLOCK_REF;
    iter->entry.name[0] = '\0';
    struct INode node;
    if (fs_getINode(directory, &node) && node.flags == FS_FLAG_DIRECTORY) {
        iter->directory = directory;
    } else {
        iter->directory = -1;
    }
}

Boolean fs_iterNext(struct DirectoryIterator *iter) {
    if (iter == NULL || iter->directory == -1 || iter->index < 0) return FALSE;

    int read = fs_read(iter->directory, (ByteBuffer) &iter->entry,
                       sizeof(struct DirectoryEntry) * iter->index, sizeof(struct DirectoryEntry));

    iter->index++;
    return read == sizeof(struct DirectoryEntry);
}
