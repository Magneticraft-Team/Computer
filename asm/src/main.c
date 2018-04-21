/* Original implementation: https://github.com/kristianlm/small-lisp */


#include <fs/filesystem.h>
#include <debug.h>
#include <fs/file.h>
#include <util/input.h>
#include "../include/dependencies.h"
#include "../include/asm.h"
#include "../include/read.h"

#define CHECK_FS()  do {                                                                    \
                        fs_init(state.diskDrive);                                           \
                        if(fs_getDevice() == -1){ kdebug("Disk not formatted\n"); return; } \
                    } while(FALSE)

#define NEXT_ARG(input) input = strtok(NULL, " \n");                \
                        if (input == NULL) {                        \
                            kdebug("Command argument missing\n");   \
                            continue;                               \
                        }

struct {
    INodeRef currentFolder;
    Monitor *monitor;
    DiskDrive *diskDrive;
} state;

static void searchPeripherals() {
    const struct device_header **a = motherboard_get_devices();
    state.diskDrive = motherboard_get_floppy_drive();
    state.monitor = motherboard_get_monitor();
}


void f_ls() {
    CHECK_FS();
    struct DirectoryIterator iter;
    struct INode node;

    fs_iterInit(state.currentFolder, &iter);

    while (fs_iterNext(&iter)) {
        kdebug("%s", iter.entry.name);

        // Check if is directory
        fs_getINode(iter.entry.inode, &node);
        if (node.flags == FS_FLAG_DIRECTORY) {
            kdebug("/");
        }
        kdebug("\n");
    }
}

void f_cd(String *name) {
    CHECK_FS();
    INodeRef child = fs_findFile(state.currentFolder, name);
    if (child == FS_NULL_INODE_REF) {
        kdebug("Error %s doesn't exist\n", name);
        return;
    }
    struct INode node;
    fs_getINode(child, &node);
    if (node.flags == FS_FLAG_DIRECTORY) {
        state.currentFolder = child;
        return;
    } else {
        kdebug("Error %s is not a folder\n", name);
        return;
    }
}

void f_mkdir(String *name) {
    CHECK_FS();
    INodeRef res = fs_create(state.currentFolder, name, FS_FLAG_DIRECTORY);
    if (res == FS_NULL_INODE_REF) {
        kdebug("Error unable to create '%s'\n", name);
    }
}

void f_rm(String *name) {
    CHECK_FS();
    INodeRef child = fs_findFile(state.currentFolder, name);
    if (child == FS_NULL_INODE_REF) {
        kdebug("Error %s not found\n", name);
        return;
    }

    if (fs_delete(state.currentFolder, child)) {
        kdebug("Error unable to remove file '%s'\n", name);
        return;
    }
}

void main() {

    monitor_clear(state.monitor);
    kdebug("ASM 1.1\n");
    kdebug("Type 'help' for command list\n");
    String buffer[78];

    while (TRUE) {
        kdebug("> ");
        readString(buffer, 78);
        String *input = strtok(buffer, " \n");

        if (input == NULL) {
            continue;

        } else if (strcmp(input, "exit") == 0) {
            return;

        } else if (strcmp(input, "help") == 0) {
            printf("Available commands: exit, help, ls, cd, mkdir, rm, compile\n");

        } else if (strcmp(input, "ls") == 0) {
            f_ls();

        } else if (strcmp(input, "cd") == 0) {
            NEXT_ARG(input);
            f_cd(input);

        } else if (strcmp(buffer, "mkdir") == 0) {
            NEXT_ARG(input);
            f_mkdir(input);

        } else if (strcmp(buffer, "rm") == 0) {
            NEXT_ARG(input);
            f_rm(input);

        } else if (strcmp(buffer, "compile") == 0) {
            NEXT_ARG(input);
            FD src = file_open(input, 0);
            if (src == ERROR) {
                kdebug("File not found '%s'\n", input);
                continue;
            }

            FD dst;
            input = strtok(NULL, " \n");
            if (input == NULL) {
                dst = file_open(input, FILE_OPEN_CREATE | FILE_OPEN_TRUNCATE);
            } else {
                dst = file_open("boot.bin", FILE_OPEN_CREATE);
            }

            f_compile(src, dst);
            file_close(src);
            file_close(dst);

        } else {
            kdebug("Unknown command '%s'\n", input);
        }
    }
}