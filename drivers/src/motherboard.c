//
// Created by cout970 on 2017-07-10.
//

#include "motherboard.h"

static Motherboard *motherboard = (Motherboard *) 0xFFFF0000;

Motherboard *motherboard_get_computer_motherboard() {
    return motherboard;
}

void motherboard_signal(Int signal) {
    ((struct motherboard_header volatile *) motherboard)->signal = (Byte volatile) signal;
}

Monitor *motherboard_get_monitor() {
    return (Monitor *) motherboard->monitor;
}

DiskDrive *motherboard_get_floppy_drive() {
    return (DiskDrive *) motherboard->floppy;
}

const struct device_header **motherboard_get_devices() {
    return motherboard->devices;
}

Int motherboard_get_memory_size() {
    return motherboard->memSize;
}

Boolean motherboard_is_little_endian() {
    return (Boolean) motherboard->littleEndian;
}

Int motherboard_get_minecraft_world_time() {
    return motherboard->worldTime;
}

Int motherboard_get_cpu_cycles() {
    return motherboard->cpuTime;
}

void motherboard_sleep(Byte ticks) {
    ((struct motherboard_header volatile *) motherboard)->sleep = (Byte volatile) ticks;
}

void motherboard_set_debug_log_type(Byte type) {
    ((struct motherboard_header volatile *) motherboard)->logType = (Byte volatile) type;
}

void motherboard_debug_print_byte(Byte value) {
    ((struct motherboard_header volatile *) motherboard)->logByte = (Byte volatile) value;
}

void motherboard_debug_print_short(Short value) {
    ((struct motherboard_header volatile *) motherboard)->logShort = (Short volatile) value;
}

void motherboard_debug_print_int(Int value) {
    ((struct motherboard_header volatile *) motherboard)->logInt = (Int volatile) value;
}