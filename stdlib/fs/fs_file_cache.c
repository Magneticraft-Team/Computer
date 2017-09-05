//
// Created by cout970 on 2017-09-05.
//

#include "fs_utils.h"
#include "../api/string.h"
#include "../util/static_list.h"

int file_equals(File *a, File *b) {
    return a->firstBlock == b->firstBlock;
}

// non malloc list of 'File' up to MAX_OPEN_FILES
StaticList(open_files, File, MAX_OPEN_FILES, file_equals);

static int file_is_open(File *file) {
    return open_files_index_of(file) != -1;
}

static int get_file_in_folder(DiskDrive drive, File *folder, const char *name) {
    int entryCount = folder->size / sizeof(DirectoryEntry);
    DirectoryEntry entry;
    for (int i = 0; i < entryCount; ++i) {
        file_read(drive, folder, byteArrayOf(&entry, sizeof(DirectoryEntry)), sizeof(DirectoryEntry) * i);
        if (strcmp(entry.name, name) == 0) {
            return entry.firstBlock;
        }
    }
    return NULL_SECTOR;
}

static int folder_remove_child(DiskDrive drive, File *parent, File *child) {
    if (parent == NULL) return 0;
    if (parent->type != FILE_TYPE_DIRECTORY) return 0;

    // remove file from parent folder
    DirectoryEntry entry;
    int offset = 0;
    int read;
    do {
        read = file_read(drive, parent, byteArrayOf(&entry, sizeof(DirectoryEntry)), offset);
        if (read == sizeof(DirectoryEntry) && strcmp(entry.name, child->name) == 0) {
            // start moving all entries
            do {
                read = file_read(drive, parent, byteArrayOf(&entry, sizeof(DirectoryEntry)), offset + sizeof(DirectoryEntry));
                if (read == 0) break;
                file_write(drive, parent, byteArrayOf(&entry, sizeof(DirectoryEntry)), offset);
                offset += sizeof(DirectoryEntry);
            } while (1);

            file_truncate(drive, parent, parent->size - sizeof(DirectoryEntry));
            return 1;
        }
        offset += sizeof(DirectoryEntry);
    } while (read != 0);

    return 0;
}

static void makeFsIfNeeded(DiskDrive drive) {
    load_sector(drive, 0);
    SuperBlock *buffer = (SuperBlock *) disk_drive_get_buffer(drive);

    if (buffer->magicNumber == FILE_SYSTEM_MAGIC_NUMBER) {
        memcpy(&superBlockCache, buffer, sizeof(SuperBlock));

        if (file_open_count() == 0) {
            load_sector(drive, 1);
            open_files_add((File *) disk_drive_get_buffer(drive));
        }
    } else {
        makeFs(drive);
    }
}

// Root
File *file_get_root(DiskDrive drive) {
    if (file_open_count() == 0) {
        makeFsIfNeeded(drive);
    }
    load_sector(drive, superBlockCache.rootDirectory);
    FileFirstBlock *block = (FileFirstBlock *) disk_drive_get_buffer(drive);
    memcpy(&open_files.items[0].data, block, sizeof(File));

    return &open_files.items[0].data;
}

// Open/Close

File *file_open(DiskDrive drive, File *parent, const char *name) {
    if (parent == NULL) return NULL;
    if (parent->type != FILE_TYPE_DIRECTORY) return NULL;
    if (strlen(name) >= FILE_MAX_NAME_SIZE) return NULL;
    if (file_open_count() >= MAX_OPEN_FILES) return NULL;

    int sector = get_file_in_folder(drive, parent, name);
    if (sector == NULL_SECTOR) return NULL;

    load_sector(drive, sector);
    FileFirstBlock *header = (FileFirstBlock *) disk_drive_get_buffer(drive);\

    return open_files_add(&header->metadata);
}

void file_close(DiskDrive drive IGNORED, File *file) {
    if (file == NULL) return;
    // root folder cache
    if (file == &open_files.items[0].data) return;
    if (file_open_count() <= 1) return;

    if (file_is_open(file)) {
        open_files_remove(file);
    }
}

int file_open_count() {
    return open_files.used;
}

// Create/Delete

File *file_create(DiskDrive drive, File *parent, const char *name, int type) {
    if (parent == NULL) return NULL;
    if (parent->type != FILE_TYPE_DIRECTORY) return NULL;
    if (strlen(name) >= FILE_MAX_NAME_SIZE) return NULL;
    if (file_open_count() >= MAX_OPEN_FILES) return NULL;

    int sector = allocate_sector(drive);
    load_sector(drive, sector);
    FileFirstBlock *header = (FileFirstBlock *) disk_drive_get_buffer(drive);

    header->metadata.firstBlock = sector;
    header->metadata.nextBlock = NULL_SECTOR;
    strcpy(header->metadata.name, name);
    header->metadata.parent = parent->firstBlock;
    header->metadata.type = type;
    header->metadata.size = 0;
    header->metadata.lastModified = motherboard_get_minecraft_world_time();

    File *file = open_files_add(&header->metadata);

    save_sector(drive);

    DirectoryEntry newEntry;
    memset(&newEntry, 0, sizeof(DirectoryEntry));
    newEntry.firstBlock = file->firstBlock;
    memcpy(newEntry.name, file->name, FILE_MAX_NAME_SIZE);

    file_append(drive, parent, byteArrayOf(&newEntry, sizeof(DirectoryEntry)));

    // add parent folder link
    if (type == FILE_TYPE_DIRECTORY) {

        // add .
        DirectoryEntry thisEntry;
        memset(&thisEntry, 0, sizeof(DirectoryEntry));
        thisEntry.firstBlock = file->firstBlock;
        memcpy(thisEntry.name, ".", 2);
        file_append(drive, file, byteArrayOf(&thisEntry, sizeof(DirectoryEntry)));

        // add ..
        DirectoryEntry parentEntry;
        memset(&parentEntry, 0, sizeof(DirectoryEntry));
        parentEntry.firstBlock = parent->firstBlock;
        memcpy(parentEntry.name, "..", 3);
        file_append(drive, file, byteArrayOf(&parentEntry, sizeof(DirectoryEntry)));
    }

    return file;
}

void file_delete(DiskDrive drive, File *parent, File *file) {
    // you can't delete the root directory
    if (file == &open_files.items[0].data) return;
    if (!folder_remove_child(drive, parent, file)) return;

    if (file->type == FILE_TYPE_DIRECTORY) {
        DirectoryEntry entry;
        int offset = 0;
        int read;
        do {
            read = file_read(drive, file, byteArrayOf(&entry, sizeof(DirectoryEntry)), offset);
            if (read == sizeof(DirectoryEntry)) {
                File entryFile;
                load_sector(drive, entry.firstBlock);
                memcpy(&entryFile, (const void *) disk_drive_get_buffer(drive), sizeof(File));
                file_delete(drive, file, &entryFile);
            }
            offset += sizeof(DirectoryEntry);
        } while (read != 0);
    }
    file_truncate(drive, file, 0);
    free_sector(drive, file->firstBlock);

    // remove file from openFiles
    file_close(drive, file);
}