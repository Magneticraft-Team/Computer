//
// Created by cout970 on 26/01/18.
//

#include <types.h>
#define MAX_OPEN_FILES 8

// MEMORY

typedef struct FileDescriptorEntry {
    Byte used;
} FileDescriptorEntry;

typedef struct {
    FileDescriptorEntry entries[MAX_OPEN_FILES];
    Int count;
} FileDescriptorTable;

// DISK



FileDescriptorTable cache;
