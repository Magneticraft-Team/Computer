//
// Created by cout970 on 2017-07-27.
//

#include "dependencies.h"
#include "api/stdio.h"
#include "filesystem.h"

int main() {
    char input[80];
    DiskDrive drive = motherboard_get_floppy_drive();
    clear_screen();
    printf("Waiting for disk...\n");

    fgets(input, 80, stdin);
    printf("Loading disk\n");
    makeFileSystem(drive);
    printf("Filesystem loaded\n");

    File* root = file_get_root(drive);
    printf("Creating file: 'test'\n");
    File* file =  file_create(drive, root, "test", FILE_TYPE_NORMAL);
    printf("File created: '%s'\n", file->name);



    return 0;
}