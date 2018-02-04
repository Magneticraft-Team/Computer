//
// Created by cout970 on 27/01/18.
//

#ifndef COMPUTER_FILESYSTEM_H
#define COMPUTER_FILESYSTEM_H


// inspired in EXT2

#include <types.h>
#include <disk_drive.h>

typedef Int INodeRef;
typedef Int BlockRef;

#define FS_FLAG_FILE 1
#define FS_FLAG_DIRECTORY 2
#define FS_MAGIC_NUMBER 0xCAFE1234
#define FS_NULL_BLOCK_REF (-1)
#define FS_INODES_PER_TABLE 15
#define FS_NUM_DIRECT_BLOCKS 9
#define FS_MAX_FILE_NAME_SIZE 32

// 28 bytes
struct PACKED BlockGroup {
    Int numberOfBlocks;     // Number of blocks in this group, max 128
    Byte blockBitmap[16];   // Bitmap used check the free blocks in this group, the firsts bits are used for this BlockGroup data (128) / 8
    Int blocksOffset;       // Number of blocks before the start of this group
    BlockRef inodeTable;    // block storing the first part of the INodeTable
};

// 916 bytes maybe
struct PACKED SuperBlock {
    Int magicNumber;        // mark used to detect the disk format
    Int deviceId;           // used to differentiate disks
    Int deviceSize;         // max bytes in the device

    Int numGroups;          // number of groups in the device
    BlockRef nextBlockGroupList; // block storing the next BlockGroup list

    struct BlockGroup blockGroupList[32]; // first 32 block groups, enough for a 4 Mb disk
};

// 64 bytes
struct PACKED INode {
    Int flags;              // Flags: include file type, etc
    Int size;               // Size of the file in bytes
    Int creationTime;       // Moment whe this file was created
    Int accessTime;         // Last time this file was read
    Int modificationTime;   // Last time this file was written

    Int blocksInUse;        // Number of blocks used to store this file content
    BlockRef blocks[FS_NUM_DIRECT_BLOCKS]; // Direct block pointer, address of the block used by this file
    BlockRef indirectBlock; // Pointer to an indirect block, storing the pointers to the rest of the blocks
};

// 968 bytes
struct PACKED INodeTable {
    Int inodeBitmap;        // Bitmap used to check the used inodes in this table
    BlockRef nextTable;     // Block storing the next part of the table, or -1 if there is none
    struct INode inodes[FS_INODES_PER_TABLE];// The inodes in this part of the table
};

struct DirectoryEntry {
    char name[FS_MAX_FILE_NAME_SIZE];
    INodeRef inode;
};

// resets the filesystem cache, and changes the drive/disk in use
void fs_init(DiskDrive *drive);

// Formats the current disk
void fs_format();

// Get the device id of the current disk, or -1 if the disk is not formatted,
// this can be used to check is a disk has changed or the disk doesn't have a format
Int fs_getDevice();

// Return the filesystem root node
INodeRef fs_getRoot();

// Creates a new node, with this parent and name, frags are used to create directories or normal files
INodeRef fs_create(INodeRef parent, String *name, Int flags);

// Removes a file
Int fs_delete(INodeRef parent, INodeRef file);

// Changes the size of a file
Boolean fs_truncate(INodeRef inode, Int size);

// Finds a file inside a directory
INodeRef fs_findFile(INodeRef parent, String *name);

// Get sthe inode information of this file, returns TRUE if success and FALSE otherwise
Boolean fs_getINode(INodeRef inode, struct INode *dst);

// Write to a file
Int fs_write(INodeRef inode, const ByteBuffer buf, Int offset, Int nbytes);

// Read from a file
Int fs_read(INodeRef inode, ByteBuffer buf, Int offset, Int nbytes);

#endif //COMPUTER_FILESYSTEM_H
