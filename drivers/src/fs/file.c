//
// Created by cout970 on 26/01/18.
//

#include <types.h>
#include <file.h>
#include "fs/filesystem.h"

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

static struct FileDescriptorTable{
    FileDescriptorEntry entries[MAX_OPEN_FILES];
    Byte bitmap[MAX_OPEN_FILES / 8];
} Table;

