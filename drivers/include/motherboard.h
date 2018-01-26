//
// Created by cout970 on 2017-07-10.
//

#ifndef DRIVER_MOTHERBOARD_H
#define DRIVER_MOTHERBOARD_H

#include "devices.h"
#include "monitor.h"
#include "disk_drive.h"

#define MOTHERBOARD_MAX_DEVICES 16

/**
 * WARNING: This is the direct api to the device, the compiler will remove any code that tries to write or read
 * from this (due to dead code optimizacions), use the methods bellow or add 'volatile' to the variables to avoid that
 */
struct motherboard_header {
/*   0 0x00 */    const Byte online;
/*   1 0x01 */    Byte signal;
/*   2 0x02 */    Byte sleep;
/*   3 0x03 */    const Byte padding;
/*   4 0x04 */    const Int memSize;
/*   8 0x08 */    const Int littleEndian;
/*  12 0x0c */    const Int worldTime;
/*  16 0x10 */    const Int cpuTime;
/*  20 0x14 */    Byte logType;
/*  21 0x15 */    Byte logByte;
/*  22 0x16 */    Short logShort;
/*  24 0x18 */    Int logInt;
/*  28 0x1c */    const Ptr monitor;
/*  32 0x20 */    const Ptr floppy;
/*  36 0x24 */    const struct device_header *devices[MOTHERBOARD_MAX_DEVICES];
};

typedef struct motherboard_header Motherboard;

#define MOTHERBOARD_SIGNAL_HALT 0
#define MOTHERBOARD_SIGNAL_START 1
#define MOTHERBOARD_SIGNAL_RESET 2

#define MOTHERBOARD_LOG_TYPE_VERBOSE 0
#define MOTHERBOARD_LOG_TYPE_DECIMAL 1
#define MOTHERBOARD_LOG_TYPE_HEXADECIMAL 2
#define MOTHERBOARD_LOG_TYPE_CHAR 3

Motherboard* motherboard_get_computer_motherboard();

void motherboard_signal(Int signal);

Monitor *motherboard_get_monitor();

DiskDrive *motherboard_get_floppy_drive();

const struct device_header **motherboard_get_devices();

Int motherboard_get_memory_size();

Boolean motherboard_is_little_endian();

Int motherboard_get_minecraft_world_time();

Int motherboard_get_cpu_cycles();

void motherboard_sleep(Byte ticks);

void motherboard_set_debug_log_type(Byte type);

void motherboard_debug_print_byte(Byte value);

void motherboard_debug_print_short(Short value);

void motherboard_debug_print_int(Int value);

#endif //DRIVER_MOTHERBOARD_H
