//
// Created by cout970 on 2017-07-10.
//
// API to access the motherboard of the computer
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
/*   1 0x01 */    Byte signal;              // Sends a signal
/*   2 0x02 */    Byte sleep;               // Sleeps x ticks
/*   3 0x03 */    const Byte padding;       // Unused
/*   4 0x04 */    const Int memSize;        // Size of RAM
/*   8 0x08 */    const Int littleEndian;   // isLittleEndian?, by default yes
/*  12 0x0c */    const Int worldTime;      // Current minecraft world time
/*  16 0x10 */    const Int cpuTime;        // Number of cycle since the PC turn on
/*  20 0x14 */    Byte logType;             // Type of debug log
/*  21 0x15 */    Byte logByte;             // Byte to print ot the minecraft log
/*  22 0x16 */    Short logShort;           // Short to print ot the minecraft log
/*  24 0x18 */    Int logInt;               // Int to print ot the minecraft log
/*  28 0x1c */    const void *monitor;        // Address of the Monitor if there is one
/*  32 0x20 */    const void *floppy;         // Address of the Floppy driver if there is one
/*  36 0x24 */    const struct device_header *devices[MOTHERBOARD_MAX_DEVICES]; // Connected devices to this PC
};

typedef struct motherboard_header Motherboard;

#define MOTHERBOARD_SIGNAL_HALT 0
#define MOTHERBOARD_SIGNAL_START 1
#define MOTHERBOARD_SIGNAL_RESET 2

#define MOTHERBOARD_LOG_TYPE_VERBOSE 0
#define MOTHERBOARD_LOG_TYPE_DECIMAL 1
#define MOTHERBOARD_LOG_TYPE_HEXADECIMAL 2
#define MOTHERBOARD_LOG_TYPE_CHAR 3

// Get the memory address where the motherboard is mapped
Motherboard *motherboard_get_computer_motherboard();

// Send a signal to the motherboard, this is sync, no sleep needed
void motherboard_signal(Int signal);

// Returns the address of the primary monitor or NULL if there is none
Monitor *motherboard_get_monitor();

// Returns the address of the primary disk drive or NULL if there is none
DiskDrive *motherboard_get_floppy_drive();

// Address to the list of connected devices
const struct device_header **motherboard_get_devices();

// Amount of bytes in the computer RAM
Int motherboard_get_memory_size();

// Returns TRUE if the Computer is little endian, FALSE otherwise, by default this should be true
Boolean motherboard_is_little_endian();

// Gets the current world time from minecraft
Int motherboard_get_minecraft_world_time();

// Gets the amount of CPU cycles since the PC turn on
Int motherboard_get_cpu_cycles();

// Sleeps the computer for N minecraft ticks
void motherboard_sleep(Byte ticks);

// Sets the print format for the debug functions
void motherboard_set_debug_log_type(Byte type);

// Prints a byte in the minecraft log
void motherboard_debug_print_byte(Byte value);

// Prints a short in the minecraft log
void motherboard_debug_print_short(Short value);

// Prints a int in the minecraft log
void motherboard_debug_print_int(Int value);

#endif //DRIVER_MOTHERBOARD_H
