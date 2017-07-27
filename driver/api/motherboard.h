//
// Created by cout970 on 2017-07-10.
//

#ifndef DRIVER_MOTHERBOARD_H
#define DRIVER_MOTHERBOARD_H

#include "types.h"
#include "devices.h"

struct motherboard_header {
/*   0 0x00 */    const i8 online;
/*   1 0x01 */    i8 signal;
/*   2 0x02 */    i8 sleep;
/*   3 0x03 */    const i8 padding;
/*   4 0x04 */    const i32 memSize;
/*   8 0x08 */    const i32 littleEndian;
/*  12 0x0c */    const i32 worldTime;
/*  16 0x10 */    const i32 cpuTime;
/*  20 0x14 */    i8 logType;
/*  21 0x15 */    i8 logByte;
/*  22 0x16 */    i16 logShort;
/*  24 0x18 */    i32 logInt;
/*  28 0x1c */    const i32 *monitor;
/*  32 0x20 */    const i32 *floppy;
/*  36 0x24 */    const struct device_header *devices[16];
};

#define MOTHERBOARD_SIGNAL_HALT 0
#define MOTHERBOARD_SIGNAL_START 1
#define MOTHERBOARD_SIGNAL_RESET 2

#define MOTHERBOARD_LOG_TYPE_VERBOSE 0
#define MOTHERBOARD_LOG_TYPE_DECIMAL 1
#define MOTHERBOARD_LOG_TYPE_HEXADECIMAL 2
#define MOTHERBOARD_LOG_TYPE_CHAR 3

void motherboard_signal(i32 signal);

void *motherboard_get_monitor();

void *motherboard_get_floppy_drive();

i32 motherboard_get_max_devices();

struct device_header **motherboard_get_devices();

i32 motherboard_get_memory_size();

boolean motherboard_is_little_endian();

i32 motherboard_get_minecraft_world_time();

i32 motherboard_get_cpu_cycles();

void motherboard_sleep(i8 ticks);

void motherboard_set_debug_log_type(i8 type);

void motherboard_debug_print_byte(i8 value);

void motherboard_debug_print_short(i16 value);

void motherboard_debug_print_int(i32 value);

#endif //DRIVER_MOTHERBOARD_H
