//
// Created by cout970 on 2017-07-10.
//
// Api to access the monitor and the keyboard
//

#ifndef COMPUTER_MONITOR_H
#define COMPUTER_MONITOR_H

#include "types.h"
#include "devices.h"

#define MONITOR_MIN_LINE_BUFFER_SIZE 80

struct mouse_event {
    Byte button;
    Byte press;
    Short x;
    Short y;
};

struct key_event {
    Byte press;
    Char character;
};

struct monitor_header {
/*   0 0x00 */    struct device_header header;
/*   4 0x04 */    Byte keyBufferPtr;            // Position of the first key event in the keyBuffer
/*   5 0x05 */    Byte keyBufferSize;           // Number of key events in the keyBuffer
/*   6 0x06 */    Byte keyBuffer[28];           // Stores the key events (is defined in bytes for alignment)
/*  34 0x22 */    Byte mouseBufferPtr;          // Position of the first mouse event in the mouseBuffer
/*  35 0x23 */    Byte mouseBufferSize;         // Number of mouse events in the mouseBuffer
/*  36 0x24 */    Byte mouseBuffer[36];         // Stores the mouse events (is defined in bytes for alignment)
/*  72 0x48 */    const Int lines;              // Number of line in the monitor
/*  76 0x4c */    const Int columns;            // Number of columns in the monitor
/*  80 0x50 */    Int cursorLine;               // Y pos of the cursor
/*  84 0x54 */    Int cursorColumn;             // X pos of the cursor
/*  88 0x58 */    Short signal;                 // Task to run in the monitor
/*  90 0x5a */    Short currentLine;            // Line used to read/write to/from the buffer
/*  92 0x5c */    Byte buffer[MONITOR_MIN_LINE_BUFFER_SIZE]; // Stores a copy of one line of the monitor
};

typedef struct monitor_header Monitor;

#define MONITOR_SIGNAL_IGNORE 0
#define MONITOR_SIGNAL_READ 1           // read currentLine to the buffer
#define MONITOR_SIGNAL_WRITE 2          // write from the buffer to currentLine

// Number of line in the monitor
Int monitor_get_num_lines(Monitor *monitor);

// Number of columns in the monitor
Int monitor_get_num_columns(Monitor *monitor);

// Get X pos of the cursor
Int monitor_get_cursor_pos_x(Monitor *monitor);

// Get Y pos of the cursor
Int monitor_get_cursor_pos_y(Monitor *monitor);

// Set X pos of the cursor
void monitor_set_cursor_pos_x(Monitor *monitor, Int x);

// Set Y pos of the cursor
void monitor_set_cursor_pos_y(Monitor *monitor, Int y);

// Returns the current line
Int monitor_get_selected_line(Monitor *monitor);

// Sets the current line
void monitor_set_selected_line(Monitor *monitor, Int line);

// Return a pointer to the internal buffer, the volatile is important to avoid dead code elimination in gcc
Char volatile *monitor_get_line_buffer(Monitor *monitor);

// Runs a task, this is sync, doesn't require to sleep
void monitor_signal(Monitor *monitor, Int signal);

// Return TRUE if there is any key event to read, FALSE otherwise
Boolean monitor_has_key_events(Monitor *monitor);

// Return the last key event stored, if there is no more events it will return
// the last event, check monitor_has_key_events to avoid that
struct key_event monitor_get_last_key_event(Monitor *monitor);

// Return TRUE if there is any mouse event to read, FALSE otherwise
Boolean monitor_has_mouse_events(Monitor *monitor);

// Return the last mouse event stored, if there is no more events it will return
// the last event, check monitor_has_mouse_events to avoid that
struct mouse_event monitor_get_last_mouse_event(Monitor *monitor);

#endif //COMPUTER_MONITOR_H
