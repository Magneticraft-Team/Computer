//
// Created by cout970 on 2017-08-23.
//

#ifndef MAGNETICRAFTCOMPUTER_FILESYSTEM_H
#define MAGNETICRAFTCOMPUTER_FILESYSTEM_H

#include "dependencies.h"

#define FILE_SYSTEM_MAGIC_NUMBER 0x1234CAFE
#define NULL_SECTOR 0

#define FILE_TYPE_NORMAL 0
#define FILE_TYPE_DIRECTORY 1

#define FILE_MAX_NAME_SIZE 64 //including final '\0'
#define FILE_FIRST_BLOCK_SPACE (1024 - sizeof(File))
#define FILE_NORMAL_BLOCK_SPACE 1016

#define MAX_OPEN_FILES 16


typedef struct {
    int magicNumber;
    int freeBlocks;
    int usedBlocks; // this doesn't count the first sector that has the SuperBlock
    int rootDirectory;
    int firstUnusedBlock;
    int lastRemovedBlock;
} SuperBlock;

typedef struct {
    int firstBlock;
    int nextBlock;
    char name[FILE_MAX_NAME_SIZE];
    int parent;
    int type;
    int size;
    int lastModified;
} File;

typedef struct {
    File metadata;
    char content[1024 - sizeof(File)];
} FileFirstBlock;

typedef struct {
    int firstBlock;
    int nextBlock;
    char content[1016]; // 1024 - 4 - 4
} BlockHeader;

typedef struct {
    int firstBlock;
    char name[FILE_MAX_NAME_SIZE];
} DirectoryEntry;

void makeFileSystem(DiskDrive drive);

File *file_get_root(DiskDrive drive);

File *file_create(DiskDrive drive, File *parent, const char *name, int type);

void file_delete(DiskDrive drive, File *parent, File *file);

void file_truncate(DiskDrive drive, File *file, int size) ;

File *file_open(DiskDrive drive, File *parent, const char *name);

void file_close(DiskDrive drive, File *file);

int file_read(DiskDrive drive, File *file, void *buff, int offset, int size);

int file_write(DiskDrive drive, File *file, void *buff, int offset, int size);

int file_append(DiskDrive drive, File *file, void *buff, int size);


#endif //MAGNETICRAFTCOMPUTER_FILESYSTEM_H
