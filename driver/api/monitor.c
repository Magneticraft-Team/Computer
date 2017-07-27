//
// Created by cout970 on 2017-07-10.
//

#include "monitor.h"

i32 monitor_get_num_lines(Monitor monitor) {
    return monitor->lines;
}

i32 monitor_get_num_columns(Monitor monitor) {
    return monitor->columns;
}

i32 monitor_get_cursor_pos_x(Monitor monitor) {
    return monitor->cursorColumn;
}

i32 monitor_get_cursor_pos_y(Monitor monitor) {
    return monitor->cursorLine;
}

void monitor_set_cursor_pos_x(Monitor monitor, i32 x) {
    ((Monitor volatile) monitor)->cursorColumn = x;
}

void monitor_set_cursor_pos_y(Monitor monitor, i32 y) {
    ((Monitor volatile) monitor)->cursorLine = y;
}

i32 monitor_get_selected_line(Monitor monitor) {
    return monitor->currentLine;
}

void monitor_set_selected_line(Monitor monitor, i32 line) {
    ((Monitor volatile) monitor)->currentLine = (i16) line;
}

void monitor_signal(Monitor monitor, i32 signal) {
    ((Monitor volatile) monitor)->signal = (i16) signal;
}

char volatile *monitor_get_line_buffer(Monitor monitor) {
    return monitor->buffer;
}

i32 monitor_has_key_events(Monitor monitor) {
    return monitor->keyBufferSize;
}

struct key_event monitor_get_last_key_event(Monitor monitor) {
    i32 index = monitor->keyBufferPtr;
    struct key_event event = ((struct key_event*)monitor->keyBuffer)[index];
    monitor->keyBufferPtr = (i8) ((monitor->keyBufferPtr + 1) % 14);
    monitor->keyBufferSize--;
    return event;
}

i32 monitor_has_mouse_events(Monitor monitor) {
    return monitor->mouseBufferSize;
}

struct mouse_event monitor_get_last_mouse_event(Monitor monitor) {
    i32 index = monitor->mouseBufferPtr;
    struct mouse_event event = ((struct mouse_event *) monitor->mouseBuffer)[index];
    monitor->mouseBufferPtr = (i8) ((monitor->mouseBufferPtr + 1) % 14);
    monitor->mouseBufferSize--;
    return event;
}