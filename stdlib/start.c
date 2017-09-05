//
// Created by cout970 on 2017-07-27.
//

#include "dependencies.h"
#include "api/stdio.h"
#include "fs/filesystem.h"

// needed to start execution at main
#include "../driver/api/boot.h"

void main() {
    char input[80];
    DiskDrive drive = motherboard_get_floppy_drive();
    clear_screen();
    printf("Waiting for disk...\n");

    fgets(input, 80, stdin);
    printf("Loading disk\n");
    makeFs(drive);
    printf("Filesystem loaded\n");

    File* root = file_get_root(drive);
    printf("Creating file: 'test'\n");
    File* file =  file_create(drive, root, "test", FILE_TYPE_NORMAL);
    printf("File created: '%s'\n", file->name);

}