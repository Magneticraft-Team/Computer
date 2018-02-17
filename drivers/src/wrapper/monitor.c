//
// Created by cout970 on 2017-07-10.
//

#include "monitor.h"

Int monitor_get_num_lines(Monitor *monitor) {
    return monitor->lines;
}

Int monitor_get_num_columns(Monitor *monitor) {
    return monitor->columns;
}

Int monitor_get_cursor_pos_x(Monitor *monitor) {
    return monitor->cursorColumn;
}

Int monitor_get_cursor_pos_y(Monitor *monitor) {
    return monitor->cursorLine;
}

void monitor_set_cursor_pos_x(Monitor *monitor, Int x) {
    ((Monitor *volatile) monitor)->cursorColumn = x;
}

void monitor_set_cursor_pos_y(Monitor *monitor, Int y) {
    ((Monitor *volatile) monitor)->cursorLine = y;
}

Int monitor_get_selected_line(Monitor *monitor) {
    return monitor->currentLine;
}

void monitor_set_selected_line(Monitor *monitor, Int line) {
    ((Monitor *volatile) monitor)->currentLine = (Short) line;
}

//void monitor_signal(Monitor *monitor, Int signal) {
//    ((Monitor *volatile) monitor)->signal = (Short) signal;
//}

char volatile *monitor_get_line_buffer(Monitor *monitor) {
    return monitor->buffer;
}

Boolean monitor_has_key_events(Monitor *monitor) {
    return monitor->keyBufferSize;
}

struct key_event monitor_get_last_key_event(Monitor *monitor) {
    Int index = monitor->keyBufferPtr;
    struct key_event event = ((struct key_event *) monitor->keyBuffer)[index];
    monitor->keyBufferPtr = (Byte) ((monitor->keyBufferPtr + 1) % 14);
    monitor->keyBufferSize--;
    return event;
}

Boolean monitor_has_mouse_events(Monitor *monitor) {
    return monitor->mouseBufferSize;
}

struct mouse_event monitor_get_last_mouse_event(Monitor *monitor) {
    Int index = monitor->mouseBufferPtr;
    struct mouse_event event = ((struct mouse_event *) monitor->mouseBuffer)[index];
    monitor->mouseBufferPtr = (Byte) ((monitor->mouseBufferPtr + 1) % 14);
    monitor->mouseBufferSize--;
    return event;
}

void monitor_clear(Monitor *mon) {
    // DO nothing
}
