//
// Created by cout970 on 11/02/18.
//

#include "../include/input.h"
#include <monitor.h>
#include <motherboard.h>
#include <glib/ctype.h>
#include <kprint.h>
#include <string.h>
#include <glib/math.h>
#include <debug.h>

#define SPACE ((char)32)

#ifdef DEBUG_ENV

#include <stdio.h>
void readInput(char *str, int n){
    fgets(str, n, stdin);
}
#else

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

Boolean strmatch(String *a, String *b) {
    return strcmp(a, b) == 0;
}

Boolean parsetInt(String *str, Int *result) {
    unsigned long int acum = 0;
    int sign = 1;
    int pos = 0;
    if (str[0] == '-') {
        pos = 1;
        sign = -1;
    } else if (str[0] == '+') {
        pos = 1;
        sign = 1;
    }

    while (str[pos] != '\0') {
        int val = str[pos] - '0';
        if (val < 0 || val > 9) {
            return FALSE;
        }
        acum = acum * 10 + val;
        pos++;
    }

    *result = ((int) acum) * sign;
    return TRUE;
}

Int split(String *in, String *cmd, String *arg1, String *arg2, String *arg3) {
    int args = 0, index = 0, aux;

    while (isspace(in[index])) index++;
    for (aux = 0; index < 80 && in[index] && !isspace(in[index]); ++index) {
        cmd[aux++] = in[index];
    }
    cmd[aux] = '\0';

    while (isspace(in[index])) index++;
    for (aux = 0; index < 80 && in[index] && !isspace(in[index]); ++index) {
        arg1[aux++] = in[index];
        args = 1;
    }
    arg1[aux] = '\0';

    while (isspace(in[index])) index++;
    for (aux = 0; index < 80 && in[index] && !isspace(in[index]); ++index) {
        arg2[aux++] = in[index];
        args = 2;
    }
    arg2[aux] = '\0';

    while (isspace(in[index])) index++;
    for (aux = 0; index < 80 && in[index] && !isspace(in[index]); ++index) {
        arg3[aux++] = in[index];
        args = 3;
    }
    arg3[aux] = '\0';
    return args;
}