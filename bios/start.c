//
// Created by cout970 on 2017-07-10.
//

//#define USE_DEBUG_LOG

#include "../driver/api/motherboard.h"
#include "../driver/api/monitor.h"
#include "../driver/api/disk_drive.h"
#include "../driver/util/printf.h"

void clear_screen() {
    Monitor mon = motherboard_get_monitor();
    int lines = monitor_get_num_lines(mon);
    int columns = monitor_get_num_columns(mon);
    int i;

    for (i = 0; i < columns; i++) {
        monitor_get_line_buffer(mon)[i] = 0x20;
    }

    for (i = 0; i < lines; i++) {
        monitor_set_selected_line(mon, i);
        monitor_signal(mon, MONITOR_SIGNAL_WRITE);
    }
    monitor_set_selected_line(mon, 0);
    monitor_set_cursor_pos_y(mon, 0);
    monitor_set_cursor_pos_x(mon, 0);
}

void *memcpy(void *dest, const void *src, int n) {
    for (int i = 0; i < n; i++) {
        ((char *) dest)[i] = ((char *) src)[i];
    }
    return dest;
}

int isEmpty(char *buff, int n) {
    unsigned char sum = 0;
    for (int i = 0; i < n; ++i) {
        sum |= buff[i];
    }
    return sum == 0;
}

int readDisk(DiskDrive drive) {
    char *ptr = 0x0, *buffer;
    int i;
    int sectors = disk_drive_get_num_sectors(drive);

    if(sectors > 36) sectors = 36;
    int wasEmpty = 0;

    for (i = 0; i < sectors; ++i) {

        disk_drive_set_current_sector(drive, i);
        disk_drive_signal(drive, DISK_DRIVE_SIGNAL_READ);
        buffer = (char *) disk_drive_get_buffer(drive);

        motherboard_sleep((i8) disk_drive_get_access_time(drive));

        memcpy(ptr + i * 1024, (const void *) buffer, 1024);
        // stop at the second empty sector
        if (isEmpty(buffer, 1024)){
            if(wasEmpty) break;
            wasEmpty = 1;
        }
    }

    return i;
}

void main() {

#ifdef USE_DEBUG_LOG
    motherboard_set_debug_log_type(MOTHERBOARD_LOG_TYPE_CHAR);
#else
    clear_screen();
#endif

    DiskDrive floppyDrive = motherboard_get_floppy_drive();

    if (disk_drive_has_disk(floppyDrive)) {
        printf("Loading...\n");
        int loaded = readDisk(floppyDrive);
        printf("Loaded %d sectors\n", loaded);
        if(loaded > 0){
            printf("Jumping to boot...\n");
            asm volatile("jal 0");
        }
    } else {
        printf("No floppy disk found\n");
    }
    printf("Halting cpu");
}


