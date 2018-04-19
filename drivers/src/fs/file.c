//
// Created by cout970 on 26/01/18.
//

#include <types.h>
#include <file.h>
#include <util/bitmap.h>
#include <string.h>
#include "fs/filesystem.h"

#define MAX_OPEN_FILES 8

// MEMORY

#define FD_TYPE_FILE 0
#define FD_TYPE_STD 1

typedef struct FileDescriptorEntry {
    Int type;
    union {
        INodeRef inode;     // file
        Int io;             // stdin, stdout, stderr
    };
    Int readPtr;
    Int writePtr;
} FileDescriptorEntry;

static struct FileDescriptorTable {
    FileDescriptorEntry entries[MAX_OPEN_FILES];
    Byte bitmap[MAX_OPEN_FILES / 8];
} FDTable;

static INodeRef getINodeFromPath(String *path, Int create, Int exclusive) {
    Int pathLen = strlen(path);
    if (pathLen == 0) return FS_NULL_INODE_REF;
    if (pathLen > 1 && path[0] == '/') {
        path++;
        pathLen--;
    }
    String buffer[FS_MAX_FILE_NAME_SIZE];
    Int i;
    INodeRef folder = fs_getRoot();

    do {
        for (i = 0; i < pathLen; ++i) {
            if (path[i] == '/') break;
            buffer[i] = path[i];
        }
        buffer[i] = '\0';
        pathLen -= i;
        path += i;

        if (pathLen > 0) {
            INodeRef file = fs_findFile(folder, buffer);
            if (file == FS_NULL_INODE_REF) return FS_NULL_INODE_REF;
            struct INode node;
            if (!fs_getINode(file, &node)) return FS_NULL_INODE_REF;
            if (node.flags != FS_FLAG_DIRECTORY) return FS_NULL_INODE_REF;
            folder = file;
        } else {
            INodeRef file = fs_findFile(folder, buffer);
            if (file == FS_NULL_INODE_REF) {
                // doesn't exist
                if (create) {
                    return fs_create(folder, buffer, FS_FLAG_FILE);
                }
                return FS_NULL_INODE_REF;
            } else {
                // exist
                if (create && exclusive) {
                    return FS_NULL_INODE_REF;
                }
                return file;
            }
        }
    } while (pathLen > 0);

    return FS_NULL_INODE_REF;
}

FD file_open_inode(INodeRef inode, Int flags) {
    // Find empty file descriptor
    Int fd;
    for (fd = 0; fd < MAX_OPEN_FILES; ++fd) {
        if (!bitmap_get(FDTable.bitmap, fd)) {
            break;
        }
    }
    if (fd == MAX_OPEN_FILES) return ERROR;

    if (flags & FILE_OPEN_TRUNCATE) {
        fs_truncate(inode, 0);
    }

    // Allocate
    FileDescriptorEntry *entry = &FDTable.entries[fd];
    entry->type = FD_TYPE_FILE;
    if (flags & FILE_OPEN_APPEND) {
        struct INode node;
        if (!fs_getINode(entry->inode, &node)) return ERROR;
        entry->writePtr = node.size;
        entry->readPtr = node.size;
    } else {
        entry->writePtr = 0;
        entry->readPtr = 0;
    }
    entry->inode = inode;
    bitmap_set(FDTable.bitmap, fd, TRUE);
    return fd;
}

FD file_open(String *path, Int flags) {
    if (path == NULL) return ERROR;

    // Get file inode
    INodeRef inode = getINodeFromPath(path, flags & FILE_OPEN_CREATE, flags & FILE_OPEN_EXCLUSIVE);
    if (inode == FS_NULL_INODE_REF) return ERROR;

    return file_open_inode(inode, flags);
}

// Writes nbytes from the buffer into the file, returns the amount of bytes written
Int file_write(FD fd, const ByteBuffer buf, Int nbytes) {
    if (fd < 0 || fd > MAX_OPEN_FILES) return 0;
    if (buf == NULL || nbytes == 0) return 0;
    if (!bitmap_get(FDTable.bitmap, fd)) return 0;
    FileDescriptorEntry *entry = &FDTable.entries[fd];

    if (entry->type == FD_TYPE_FILE) {
        Int amount = fs_write(entry->inode, buf, entry->writePtr, nbytes);
        entry->writePtr += amount;
        return amount;
    } else {
        //TODO stdin, stdout
        return 0;
    }
}

// Reads nbytes from the file into the buffer, returns the amount of bytes read
Int file_read(FD fd, ByteBuffer buf, Int nbytes) {
    if (fd < 0 || fd > MAX_OPEN_FILES) return 0;
    if (buf == NULL || nbytes == 0) return 0;
    if (!bitmap_get(FDTable.bitmap, fd)) return 0;
    FileDescriptorEntry *entry = &FDTable.entries[fd];

    if (entry->type == FD_TYPE_FILE) {
        Int amount = fs_read(entry->inode, buf, entry->readPtr, nbytes);
        entry->readPtr += amount;
        return amount;
    } else {
        //TODO stdin, stdout
        return 0;
    }
}

// Reads the file info, see file_stat, returns FALSE on error
Boolean file_stat(FD fd, struct file_stat *ref) {
    if (fd < 0 || fd > MAX_OPEN_FILES) return FALSE;
    if (ref == NULL) return FALSE;
    if (!bitmap_get(FDTable.bitmap, fd)) return FALSE;
    FileDescriptorEntry *entry = &FDTable.entries[fd];

    if (entry->type != FD_TYPE_FILE) return FALSE;
    struct INode node;
    if (!fs_getINode(entry->inode, &node)) return FALSE;

    ref->inode = entry->inode;
    ref->size = node.size;
    ref->atime = node.accessTime;
    ref->mtime = node.modificationTime;
    ref->device = fs_getDevice();
    ref->type = node.flags == FS_FLAG_DIRECTORY ? FILE_TYPE_DIRECTORY : FILE_TYPE_REGULAR;
    return TRUE;
}

// Sets the read and write pointers, see FILE_SEEK_* for 'whence'
Boolean file_seek(FD fd, Int offset, Int whence) {
    if (fd < 0 || fd > MAX_OPEN_FILES) return FALSE;
    if (whence < 0 || whence > FILE_SEEK_END) return FALSE;
    if (!bitmap_get(FDTable.bitmap, fd)) return FALSE;
    FileDescriptorEntry *entry = &FDTable.entries[fd];

    if (entry->type != FD_TYPE_FILE) return FALSE;

    if (whence == FILE_SEEK_SET) {
        entry->readPtr = offset;
        entry->writePtr = offset;
    } else if (whence == FILE_SEEK_CURRENT) {
        entry->readPtr += offset;
        entry->writePtr += offset;
    } else {// FILE_SEEK_END
        struct INode node;
        if (!fs_getINode(entry->inode, &node)) return FALSE;
        entry->readPtr = node.size - offset;
        entry->writePtr = node.size - offset;
    }
    return TRUE;
}

// Close a file, return FALSE on error
Boolean file_close(FD fd) {
    if (fd < 0 || fd > MAX_OPEN_FILES) return FALSE;
    if (!bitmap_get(FDTable.bitmap, fd)) return FALSE;

    bitmap_set(FDTable.bitmap, fd, FALSE);
    return TRUE;
}

Boolean file_eof(FD fd) {
    if (fd < 0 || fd > MAX_OPEN_FILES) return TRUE;
    if (!bitmap_get(FDTable.bitmap, fd)) return TRUE;
    FileDescriptorEntry *entry = &FDTable.entries[fd];

    if (entry->type == FD_TYPE_FILE) {
        struct INode res;
        if (!fs_getINode(entry->inode, &res)) {
            return TRUE;
        }

        return entry->readPtr >= res.size;
    } else {
        //TODO stdin, stdout
        return TRUE;
    }
}