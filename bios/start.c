//
// Created by cout970 on 2017-07-10.
//

//#define USE_DEBUG_LOG

#include <motherboard.h>
#include <debug.h>

static Monitor *monitor;
static DiskDrive *floppyDrive;

void clear_screen();

int readDisk();

void main() {

    monitor = motherboard_get_monitor();
    floppyDrive = motherboard_get_floppy_drive();

#ifdef USE_DEBUG_LOG
    motherboard_set_debug_log_type(MOTHERBOARD_LOG_TYPE_CHAR);
#else
    clear_screen();
#endif

    if (disk_drive_has_disk(floppyDrive)) {
        kdebug("Loading...\n");
        int loaded = readDisk();
        kdebug("Loaded %d sectors\n", loaded);
        if (loaded > 0) {
            kdebug("Jumping to boot...\n");
            asm volatile("jal 0");
        }
    } else {
        kdebug("No floppy disk found\n");
    }
    kdebug("Halting cpu");
}

void *memcpy(void *dest, const void *src, int n) {
    for (int i = 0; i < n; i++) {
        ((char *) dest)[i] = ((char *) src)[i];
    }
    return dest;
}

int isEmpty(char volatile *buff, int n) {
    unsigned char sum = 0;
    for (int i = 0; i < n; ++i) {
        sum |= buff[i];
    }
    return sum == 0;
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


void clear_screen() {
    int lines = monitor_get_num_lines(monitor);
    int columns = monitor_get_num_columns(monitor);
    int i;

    for (i = 0; i < columns; i++) {
        monitor_get_line_buffer(monitor)[i] = 0x20;
    }

    for (i = 0; i < lines; i++) {
        monitor_set_selected_line(monitor, i);
        monitor_signal(monitor, MONITOR_SIGNAL_WRITE);
    }

    monitor_set_selected_line(monitor, 0);
    monitor_set_cursor_pos_y(monitor, 0);
    monitor_set_cursor_pos_x(monitor, 0);
}

