//
// Created by cout970 on 2017-07-10.
//

#include "motherboard.h"
#include "debug.h"

extern Motherboard *motherboardPtr;

Monitor *motherboard_get_monitor() {
    return (Monitor *) motherboardPtr->monitor;
}

DiskDrive *motherboard_get_floppy_drive() {
    return (void *) motherboardPtr->floppy;
}

const struct device_header **motherboard_get_devices() {
    return motherboardPtr->devices;
}

Int motherboard_get_memory_size() {
    return motherboardPtr->memSize;
}

Boolean motherboard_is_little_endian() {
    return (Boolean) motherboardPtr->littleEndian;
}

Int motherboard_get_minecraft_world_time() {
    return motherboardPtr->worldTime;
}

Int motherboard_get_cpu_cycles() {
    return motherboardPtr->cpuTime;
}

void motherboard_sleep(Byte ticks) {
    usleep((unsigned int) (ticks * 20000000)); // 20_000_000 nanos = 20 millis
}

void motherboard_set_debug_log_type(Byte type) {
    // ignore
}

void motherboard_debug_print_byte(Byte value) {
    kdebug("%d", value);
}

void motherboard_debug_print_short(Short value) {
    kdebug("%d", value);
}

void motherboard_debug_print_int(Int value) {
    kdebug("%d", value);
}