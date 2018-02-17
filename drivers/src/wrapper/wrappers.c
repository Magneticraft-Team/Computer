//
// Created by cout970 on 19/01/18.
//

#include <mem.h>
#include "network.h"
#include "disk_drive.h"
#include "motherboard.h"
#include "debug.h"

#define DRIVE_DISK_NAME "debug.dat"

static Char screen[50 * 80];

static Monitor monitor = {
        .header = {
                .online = 1,
                .type = DEVICE_TYPE_MONITOR,
                .status = 0
        },
        .columns = 80,
        .lines = 50,
        .currentLine = 0,
        .cursorColumn = 0,
        .cursorLine = 0,
        .currentLine = 0,
};

static DiskDrive disk = {
        .header = {
                .online = 1,
                .type = DEVICE_TYPE_FLOPPY_DRIVE,
                .status = 0
        },
        .accessTime = 1,
        .currentSector = 0,
        .hasDisk = 1,
        .numSectors = 128,
};

static NetworkCard card = {
        .header = {
                .online = 1,
                .type = DEVICE_TYPE_NETWORK_CARD,
                .status = 0
        },
        .internetAllowed = 0,
        .maxSockets = 1,
        .activeSockets = 0,
        .macAddress = 0,
        .targetMac = 0,
        .targetPort = 0,
        .connectionOpen = 0,
        .connectionError = NETWORK_ERROR_NONE,
        .inputBufferPtr = 0,
        .outputBufferPtr = 0
};

static Motherboard motherboard = {
        .memSize = 65536,
        .littleEndian = TRUE,
        .cpuTime = 0,
        .worldTime = 0,
        .online = TRUE,
        .monitor = &monitor,
        .floppy = &disk,
        .devices = {
                (struct device_header *) &card,
                NULL
        }
};

Motherboard *motherboardPtr = &motherboard;

// Monitor
void monitor_signal(Monitor *monitor, Int signal) {
    switch (signal) {
        case MONITOR_SIGNAL_READ: {
            int size = monitor->columns;
            int index = monitor->currentLine * size;
            memcpy(monitor->buffer, screen + index, (size_t) size);
            break;
        }
        case MONITOR_SIGNAL_WRITE: {
            int size = monitor->columns;
            int index = monitor->currentLine * size;
            if (index + size > (50 * 80)) {
                perror("Invalid index");
                exit(-1);
            }
            memcpy(screen + index, monitor->buffer, (size_t) size);
//            printf("%4d | %s\n", index, monitor->buffer);
        }
        default:
            break;
    }
}

// Disk drive
void disk_drive_signal(DiskDrive *drive, Int signal) {
    switch (signal) {
        case DISK_DRIVE_SIGNAL_READ: {
            FILE *file = fopen(DRIVE_DISK_NAME, "rb");
            fseek(file, drive->currentSector * DISK_DRIVE_BUFFER_SIZE, SEEK_SET);
            fread(drive->buffer, DISK_DRIVE_BUFFER_SIZE, 1, file);
            fclose(file);
            break;
        }
        case DISK_DRIVE_SIGNAL_WRITE: {
            FILE *file = fopen(DRIVE_DISK_NAME, "wb");
            fseek(file, drive->currentSector * DISK_DRIVE_BUFFER_SIZE, SEEK_SET);
            fwrite(drive->buffer, DISK_DRIVE_BUFFER_SIZE, 1, file);
            fclose(file);
            break;
        }
        case DISK_DRIVE_SIGNAL_READ_LABEL: {
            memset(drive->buffer, 0, DISK_DRIVE_BUFFER_SIZE);
            strcpy(drive->buffer, DRIVE_DISK_NAME);
            break;
        }
        default:
            break;
    }
}

// Network
void network_signal(NetworkCard *network, Byte signal) {
    // ignore
}

// Motherboard
void motherboard_signal(Int signal) {
    switch (signal) {
        case MOTHERBOARD_SIGNAL_HALT: {
            kdebug("Halt computer");
            exit(0);
        }
        case MOTHERBOARD_SIGNAL_START: {
            kdebug("Start computer");
            exit(0);
        }
        case MOTHERBOARD_SIGNAL_RESET: {
            kdebug("Reset computer");
            exit(0);
        }
        default:
            break;
    }
}

Motherboard *motherboard_get_computer_motherboard() {
    return &motherboard;
}
