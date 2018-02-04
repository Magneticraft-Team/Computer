//
// Created by cout970 on 19/01/18.
//

#ifndef COMPUTER_FILE_H
#define COMPUTER_FILE_H

#include <types.h>

#define FILE_OPEN_READ_ONLY    1  //Open the file so that it is read only.
#define FILE_OPEN_WRITE_ONLY   2  //Open the file so that it is write only.
#define FILE_OPEN_READ_WRITE   3  //Open the file so that it can be read from and written to.
#define FILE_OPEN_APPEND       4  //Append new information to the end of the file.
#define FILE_OPEN_TRUNCATE     8  //Initially clear all data from the file.
#define FILE_OPEN_CREATE      16  //If the file does not exist, create it.
#define FILE_OPEN_EXCLUSIVE   32  //Combined with the O_CREAT option. If the file already exists, the call will fail.

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define FILE_SEEK_SET       0
#define FILE_SEEK_CURRENT   1
#define FILE_SEEK_END       2

struct file_stat {
    Int mode;     // The current permissions on the file.
    Int size;  // The size of the file
    Int inode; // The inode for the file (note that this number is unique to all files and directories on a Linux System.
    Int device;// The device that the file currently resides on.
    Int atime; // The most recent time that the file was accessed.
    Int mtime; // The most recent time that the file's contents were modified.
};

// File Descriptor
typedef int FD;

FD file_open(String *path, Int flags);

Int file_write(FD fd, const ByteBuffer buf, Int nbytes);

Int file_read(FD fd, ByteBuffer buf, Int nbytes);

Int file_stat(FD fd, struct file_stat *ref);

Int file_seek(FD fd, Int offset, Int whence);

void file_close(FD fd);

#endif //COMPUTER_FILE_H
