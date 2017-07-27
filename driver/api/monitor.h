//
// Created by cout970 on 2017-07-10.
//

#ifndef DRIVER_MONITOR_H
#define DRIVER_MONITOR_H

#include "types.h"
#include "devices.h"

struct mouse_event {
    i8 button;
    i8 press;
    i16 x;
    i16 y;
};

struct key_event{
    i8 press;
    char character;
};

struct monitor_header {
/*   0 0x00 */    struct device_header header;
/*   4 0x04 */    i8 keyBufferPtr;
/*   5 0x05 */    i8 keyBufferSize;
/*   6 0x06 */    i8 keyBuffer[28];
/*  34 0x22 */    i8 mouseBufferPtr;
/*  35 0x23 */    i8 mouseBufferSize;
/*  36 0x24 */    i8 mouseBuffer[36];
/*  72 0x48 */    const i32 lines;
/*  76 0x4c */    const i32 columns;
/*  80 0x50 */    i32 cursorLine;
/*  84 0x54 */    i32 cursorColumn;
/*  88 0x58 */    i16 signal;
/*  90 0x5a */    i16 currentLine;
/*  92 0x5c */    i8 buffer[80];
};

typedef struct monitor_header *Monitor;

#define MONITOR_SIGNAL_IGNORE 0
#define MONITOR_SIGNAL_READ 1
#define MONITOR_SIGNAL_WRITE 2

i32 monitor_get_num_lines(Monitor monitor);

i32 monitor_get_num_columns(Monitor monitor);

i32 monitor_get_cursor_pos_x(Monitor monitor);

i32 monitor_get_cursor_pos_y(Monitor monitor);

void monitor_set_cursor_pos_x(Monitor monitor, i32 x);

void monitor_set_cursor_pos_y(Monitor monitor, i32 y);

i32 monitor_get_selected_line(Monitor monitor);

void monitor_set_selected_line(Monitor monitor, i32 line);

void monitor_signal(Monitor monitor, i32 signal);

char volatile *monitor_get_line_buffer(Monitor monitor);

i32 monitor_has_key_events(Monitor monitor);

struct key_event monitor_get_last_key_event(Monitor monitor);

i32 monitor_has_mouse_events(Monitor monitor);

struct mouse_event monitor_get_last_mouse_event(Monitor monitor);

#endif //DRIVER_MONITOR_H
