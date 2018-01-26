//
// Created by cout970 on 2017-07-10.
//

#ifndef DRIVER_MONITOR_H
#define DRIVER_MONITOR_H

#include "types.h"
#include "devices.h"

#define MONITOR_LINE_BUFFER_SIZE 80

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
/*   4 0x04 */    Byte keyBufferPtr;
/*   5 0x05 */    Byte keyBufferSize;
/*   6 0x06 */    Byte keyBuffer[28];
/*  34 0x22 */    Byte mouseBufferPtr;
/*  35 0x23 */    Byte mouseBufferSize;
/*  36 0x24 */    Byte mouseBuffer[36];
/*  72 0x48 */    const Int lines;
/*  76 0x4c */    const Int columns;
/*  80 0x50 */    Int cursorLine;
/*  84 0x54 */    Int cursorColumn;
/*  88 0x58 */    Short signal;
/*  90 0x5a */    Short currentLine;
/*  92 0x5c */    Byte buffer[MONITOR_LINE_BUFFER_SIZE];
};

typedef struct monitor_header Monitor;

#define MONITOR_SIGNAL_IGNORE 0
#define MONITOR_SIGNAL_READ 1
#define MONITOR_SIGNAL_WRITE 2

Int monitor_get_num_lines(Monitor *monitor);

Int monitor_get_num_columns(Monitor *monitor);

Int monitor_get_cursor_pos_x(Monitor *monitor);

Int monitor_get_cursor_pos_y(Monitor *monitor);

void monitor_set_cursor_pos_x(Monitor *monitor, Int x);

void monitor_set_cursor_pos_y(Monitor *monitor, Int y);

Int monitor_get_selected_line(Monitor *monitor);

void monitor_set_selected_line(Monitor *monitor, Int line);

Char volatile *monitor_get_line_buffer(Monitor *monitor);

void monitor_signal(Monitor *monitor, Int signal);

Boolean monitor_has_key_events(Monitor *monitor);

struct key_event monitor_get_last_key_event(Monitor *monitor);

Boolean monitor_has_mouse_events(Monitor *monitor);

struct mouse_event monitor_get_last_mouse_event(Monitor *monitor);

#endif //DRIVER_MONITOR_H
