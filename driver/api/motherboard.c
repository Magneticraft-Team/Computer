//
// Created by cout970 on 2017-07-10.
//

#include "motherboard.h"

struct motherboard_header *mb = (struct motherboard_header *) 0xFFFF0000;

void motherboard_signal(i32 signal) {
    ((struct motherboard_header volatile *) mb)->signal = (i8 volatile) signal;
}

void *motherboard_get_monitor() {
    return (void *) mb->monitor;
}

void *motherboard_get_floppy_drive() {
    return (void *) mb->floppy;
}

i32 motherboard_get_max_devices() {
    return 16;
}

struct device_header **motherboard_get_devices() {
    return (struct device_header **) mb->devices;
}

i32 motherboard_get_memory_size() {
    return mb->memSize;
}

i32 motherboard_is_little_endian() {
    return mb->littleEndian;
}

i32 motherboard_get_minecraft_world_time() {
    return mb->worldTime;
}

i32 motherboard_get_cpu_cycles() {
    return mb->cpuTime;
}

void motherboard_sleep(i8 ticks) {
    ((struct motherboard_header volatile *) mb)->sleep = (i8 volatile) ticks;
}

void motherboard_set_debug_log_type(i8 type) {
    ((struct motherboard_header volatile *) mb)->logType = (i8 volatile) type;
}

void motherboard_debug_print_byte(i8 value) {
    ((struct motherboard_header volatile *) mb)->logByte = (i8 volatile) value;
}

void motherboard_debug_print_short(i16 value) {
    ((struct motherboard_header volatile *) mb)->logShort = (i16 volatile) value;
}

void motherboard_debug_print_int(i32 value) {
    ((struct motherboard_header volatile *) mb)->logInt = (i32 volatile) value;
}