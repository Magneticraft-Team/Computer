//
// Created by cout970 on 19/02/18.
//

#include "../include/input.h"

#define SPACE ((char)32)

#ifdef DEBUG_ENV

#include <stdio.h>

void readInput(char *str, int n) {
    fgets(str, n, stdin);
}

#else
#include <types.h>
#include <motherboard.h>
#include <glib/math.h>
#include <string.h>
#include <ctype.h>
#include <debug.h>

Char nextChar(Monitor *mon) {
    struct key_event event;

    do {
        while (!monitor_has_key_events(mon)) {
            motherboard_sleep(1);
        }
        event = monitor_get_last_key_event(mon);
    } while (!event.press);

    return event.character;
}

void readInput(String *buffer, Int n) {
    Monitor *mon = motherboard_get_monitor();
    Int reserved = monitor_get_cursor_pos_x(mon);
    UInt buffSize = (UInt) MIN((80 - reserved), n - 1);
    Int index = 0, limit = 0;

    memset(buffer, 0, buffSize);

    for (; index < (Int) buffSize;) {
        Char c = nextChar(mon);

        if (isprint(c) && isascii(c)) {
            // Normal character
            buffer[index++] = c;
            if (index == limit) limit++;
        } else if (c == 13 || c == 10) {
            // Enter
            buffer[index++] = '\n';
            kdebug("\n");
            break;
        } else if (c == 8) {
            // Delete
            if (index == limit) {
                limit--;
            }
            if (index > 0) {
                buffer[--index] = ' ';
            }
        } else if (c == 9) {
            // Tab
            if (index + 4 < (Int) buffSize) {
                if (index == limit) limit += 4;
                buffer[index++] = ' ';
                buffer[index++] = ' ';
                buffer[index++] = ' ';
                buffer[index++] = ' ';
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
        memcpy((void *) (monitor_get_line_buffer(mon) + reserved), buffer, buffSize);
        monitor_signal(mon, MONITOR_SIGNAL_WRITE);
    }
    buffer[index] = '\0';
}
#endif