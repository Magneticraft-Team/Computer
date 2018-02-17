//
// Created by cout970 on 2017-07-10.
//

//#define USE_DEBUG_LOG

#include <motherboard.h>
#include <debug.h>
#include <fs/filesystem.h>
#include <string.h>
#include <glib/math.h>

static Monitor *monitor;
static DiskDrive *floppyDrive;

int readDisk();

int readFile(INodeRef file);

int loadOs();

void main() {

    monitor = motherboard_get_monitor();
    floppyDrive = motherboard_get_floppy_drive();

    motherboard_set_debug_log_type(MOTHERBOARD_LOG_TYPE_CHAR);
    monitor_clear(monitor);

    if (disk_drive_has_disk(floppyDrive)) {
        kdebug("Loading...\n");
        if (loadOs()) {
            kdebug("Jumping to boot...\n");
            asm volatile("jal 0");
        }
    } else {
        kdebug("No floppy disk found\n");
    }
    kdebug("Halting cpu");
}

int isEmpty(char volatile *buff, int n) {
    unsigned char sum = 0;
    for (int i = 0; i < n; ++i) {
        sum |= buff[i];
    }
    return sum == 0;
}

int loadOs() {
    int loaded;
    fs_init(floppyDrive);

    if (fs_getDevice() != -1) {
        INodeRef boot = fs_findFile(fs_getRoot(), "boot.bin");
        if (boot != FS_NULL_INODE_REF) {
            kdebug("Loading from boot.bin\n");
            loaded = readFile(boot);
            kdebug("Loaded %d sectors\n", loaded);
            return loaded;
        }
    }

    kdebug("Loading from disk...\n");
    loaded = readDisk();
    kdebug("Loaded %d sectors\n", loaded);
    return loaded;
}

int readDisk() {
    char *ptr = 0x0;
    char volatile *buffer;
    int i;
    int sectors = disk_drive_get_num_sectors(floppyDrive);

    // limit program size to 36K
    if (sectors > 36) sectors = 36;
    int wasEmpty = 0;

    for (i = 0; i < sectors; ++i) {

        disk_drive_set_current_sector(floppyDrive, i);
        disk_drive_signal(floppyDrive, DISK_DRIVE_SIGNAL_READ);
        buffer = disk_drive_get_buffer(floppyDrive);

        motherboard_sleep((Byte) disk_drive_get_access_time(floppyDrive));

        memcpy(ptr + i * 1024, (const Ptr) buffer, 1024);
        // stop at the second empty sector
        if (isEmpty(buffer, 1024)) {
            if (wasEmpty) break;
            wasEmpty = 1;
        }
    }

    return i;
}

int readFile(INodeRef file) {
    char *ptr = 0x0;
    struct INode node;
    if (!fs_getINode(file, &node)) return 0;

    int sectors = CEIL_DIV(node.size, DISK_DRIVE_BUFFER_SIZE);
    if (sectors > 36) sectors = 36;

    for (int i = 0; i < sectors; ++i) {
        fs_read(file, ptr + i * DISK_DRIVE_BUFFER_SIZE, i * DISK_DRIVE_BUFFER_SIZE, DISK_DRIVE_BUFFER_SIZE);
    }

    return sectors;
}
