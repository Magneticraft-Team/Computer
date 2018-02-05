//
// Created by cout970 on 19/01/18.
//
// This is a front-end to the filesystem, this abstractions allow the following:
// - Use stdin and stout as files
// - Keep a read/write pointer to "append" or "read-next-bytes" to the file instead of specify the offset where to read/write
// - Have a max amount of open files, this makes sure that you are not using too much memory and allow to detect bugs
// - This allows to use file paths, like "/file/file.txt" without navigating manually from one inode to another
// - This allow to change the implementation of the filesystem without breaking existing code
// - As any abstraction provides a simple api so you can center in what is important, you program
//
// This abstraction is inspired in the Unix syscalls for handling files

#ifndef COMPUTER_FILE_H
#define COMPUTER_FILE_H

#include <types.h>

// file_open flags, they can be OR-ed
#define FILE_OPEN_READ_ONLY    1  //Open the file so that it is read only.
#define FILE_OPEN_WRITE_ONLY   2  //Open the file so that it is write only.
#define FILE_OPEN_READ_WRITE   3  //Open the file so that it can be read from and written to.
#define FILE_OPEN_APPEND       4  //Append new information to the end of the file.
#define FILE_OPEN_TRUNCATE     8  //Initially clear all data from the file.
#define FILE_OPEN_CREATE      16  //If the file does not exist, create it.
#define FILE_OPEN_EXCLUSIVE   32  //Combined with the O_CREAT option. If the file already exists, the call will fail.

// Default file descriptors
#define STDIN_FILENO 0  // Monitor input
#define STDOUT_FILENO 1 // Monitor output
#define STDERR_FILENO 2 // Can be a file or just monitor output

#define FILE_SEEK_SET       0 // Set the cursor to a absolute position
#define FILE_SEEK_CURRENT   1 // Moves the cursor n bytes from the current pos
#define FILE_SEEK_END       2 // Set the cursor n byte before the end of the file

// Basic file info
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

// Open a file, return -1 on error
FD file_open(String *path, Int flags);

// Writes nbytes from the buffer into the file, returns the amount of bytes written
Int file_write(FD fd, const ByteBuffer buf, Int nbytes);

// Reads nbytes from the file into the buffer, returns the amount of bytes read
Int file_read(FD fd, ByteBuffer buf, Int nbytes);

// Reads the file info, see file_stat, returns -1 on error
Int file_stat(FD fd, struct file_stat *ref);

// Sets the read and write pointers, see FILE_SEEK_* for 'whence'
Int file_seek(FD fd, Int offset, Int whence);

// Close a file, return -1 on error
Int file_close(FD fd);

#endif //COMPUTER_FILE_H
