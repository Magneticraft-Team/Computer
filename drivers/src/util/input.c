//
// Created by cout970 on 3/04/18.
//

#include "util/input.h"
#include <string.h>
#include <monitor.h>
#include <motherboard.h>
#include <glib/math.h>
#include <ctype.h>
#include <debug.h>

#ifdef DEBUG_ENV
#include <stdio.h>
#endif


Int readString(String *output, Int maxSize) {
#ifdef DEBUG_ENV
    fgets(output, maxSize, stdin);
    return (Int) strlen(output);
#else
    Monitor *mon = motherboard_get_monitor();
    Int reserved = monitor_get_cursor_pos_x(mon);
    UInt buffSize = (UInt) MIN((80 - reserved), maxSize - 1);
    Int index = 0, limit = 0;

    memset(output, 0, buffSize);

    for (; index < (Int) buffSize;) {
        Char c = readChar();

        if (isprint(c) && isascii(c)) {
            // Normal character
            output[index++] = c;
            if (index == limit) limit++;
        } else if (c == 13 || c == 10) {
            // Enter
            output[index++] = '\n';
            kdebug("\n");
            break;
        } else if (c == 8) {
            // Delete
            if (index == limit) {
                limit--;
            }
            if (index > 0) {
                output[--index] = ' ';
            }
        } else if (c == 9) {
            // Tab
            if (index + 4 < (Int) buffSize) {
                if (index == limit) limit += 4;
                output[index++] = ' ';
                output[index++] = ' ';
                output[index++] = ' ';
                output[index++] = ' ';
            }
        } else if (c == (Char) 203) {
            // Left arrow
            if (index > 0) {
                index--;
            }
        } else if (c == (Char) 205) {
            // Right arrow
            if (index < limit) {
                index++;
            }
        }

        monitor_set_cursor_pos_x(mon, reserved + index);
        // repaint the line
        memcpy((void *) (monitor_get_line_buffer(mon) + reserved), output, buffSize);
        monitor_signal(mon, MONITOR_SIGNAL_WRITE);
    }
    output[index] = '\0';
    return strlen(output);
#endif
}

Char readChar() {
#ifdef DEBUG_ENV
    return (Char) getchar();
#else
    Monitor *mon = motherboard_get_monitor();
    struct key_event event;

    do {
        while (!monitor_has_key_events(mon)) {
            motherboard_sleep(1);
        }
        event = monitor_get_last_key_event(mon);
    } while (!event.press);

    return event.character;
#endif
}