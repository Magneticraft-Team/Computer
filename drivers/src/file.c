//
// Created by cout970 on 26/01/18.
//

#include <types.h>
#include <file.h>
#include "filesystem.h"

#define MAX_OPEN_FILES 8

// MEMORY

typedef struct FileDescriptorEntry {
    Int type;
    union {
        INodeRef inode;     // file
        Int io;             // stdin, stdout, stderr
    };
    Int readPtr;
    Int writePtr;
} FileDescriptorEntry;

typedef struct {
    FileDescriptorEntry entries[MAX_OPEN_FILES];
    Byte bitmap[MAX_OPEN_FILES / 8];
} FileDescriptorTable;

FD file_open(String *path, Int flags);

Int file_write(FD fd, const ByteBuffer buf, Int nbytes);

Int file_read(FD fd, ByteBuffer buf, Int nbytes);

Int file_stat(FD fd, struct file_stat *ref);

Int file_seek(FD fd, Int offset, Int whence);

void file_close(FD fd);

FileDescriptorTable cache;
