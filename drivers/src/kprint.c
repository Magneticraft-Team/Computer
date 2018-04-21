//
// Created by cout970 on 19/01/18.
//

#include <motherboard.h>
#include <stdarg.h>
#include "kprint.h"

#define SPACE ((char)32)

#ifdef DEBUG_ENV

#include <stdio.h>

void kputchar(char code) {
    putchar(code);
}

#else
#ifdef USE_DEBUG_LOG
void kputchar(char code) {
    motherboard_debug_print_byte((Byte) code);
}
#else
static Monitor *mon = NULL;

void new_line() {

    int currentLine = monitor_get_selected_line(mon) + 1;
    int lines = monitor_get_num_lines(mon);

    monitor_set_selected_line(mon, currentLine);
    monitor_set_cursor_pos_x(mon, 0);
    monitor_set_cursor_pos_y(mon, currentLine);

    if (currentLine >= lines) {
        currentLine--;
        int columns = monitor_get_num_columns(mon);

        for (int j = 0; j < currentLine; j++) {
            monitor_set_selected_line(mon, j + 1);
            monitor_signal(mon, MONITOR_SIGNAL_READ);
            monitor_set_selected_line(mon, j);
            monitor_signal(mon, MONITOR_SIGNAL_WRITE);
        }
        monitor_set_selected_line(mon, currentLine);

        Byte volatile* buff = monitor_get_line_buffer(mon);

        for (int i = 0; i < columns; ++i) {
            buff[i] = SPACE;
        }

        monitor_signal(mon, MONITOR_SIGNAL_WRITE);

        monitor_set_cursor_pos_x(mon, 0);
        monitor_set_cursor_pos_y(mon, currentLine);
    }
}

void kputchar(char code) {
    if(!mon){
        mon = motherboard_get_monitor();
        if(!mon) return;
    }

    if (code == '\n') {
        new_line();
        return;
    }
    int cursor = monitor_get_cursor_pos_x(mon);

    monitor_signal(mon, MONITOR_SIGNAL_READ);
    monitor_get_line_buffer(mon)[cursor] = code;
    monitor_signal(mon, MONITOR_SIGNAL_WRITE);

    cursor++;
    if (cursor >= monitor_get_num_columns(mon)) {
        new_line();
    } else {
        monitor_set_cursor_pos_x(mon, cursor);
        monitor_set_cursor_pos_y(mon, monitor_get_selected_line(mon));
    }
}

#endif
#endif

int kprintn(int num, int base) {
    int sum = 0;
    if (num < 0) {
        kputchar('-');
        sum++;
        num = -num;
    }
    if (num < base) {
        kputchar("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[num]);
        sum++;
    } else {
        sum += kprintn(num / base, base);
        kputchar("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[num % base]);
        sum++;
    }
    return sum;
}

int kprintun(unsigned int num, int base) {
    int sum = 0;
    if (num < (unsigned) base) {
        kputchar("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[num]);
        sum++;
    } else {
        sum += kprintun(num / base, base);
        kputchar("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[num % base]);
        sum++;
    }
    return sum;
}

int kprints(const char *str) {
    int sum = 0;
    for (; str[sum] != '\0'; sum++) {
        kputchar(str[sum]);
    }
    return sum;
}

int kprintsn(const char *str, int count) {
    int sum = 0;
    for (; str[sum] != '\0' && sum < count; sum++) {
        kputchar(str[sum]);
    }
    return sum;
}

int kprint(const char *ptr, ...) {
    va_list list;
    va_start(list, ptr);

    int sum = 0;
    for (int i = 0; ptr[i] != '\0'; i++) {
        if (ptr[i] == '%') {
            i++;
            if (ptr[i] == '%') {
                kputchar(ptr[i]);
                sum++;
            } else if (ptr[i] == 'd' || ptr[i] == 'i') {
                sum += kprintn(va_arg(list, int), 10);
            } else if (ptr[i] == 'u') {
                sum += kprintun(va_arg(list, unsigned int), 10);
            } else if (ptr[i] == 'x' || ptr[i] == 'p') {
                sum += kprintun(va_arg(list, unsigned int), 16);
            } else if (ptr[i] == 'l') {
                i++;
                if (ptr[i] == 'x') {
                    sum += kprintun(va_arg(list, unsigned int), 16);
                } else if (ptr[i] == 'i' || ptr[i] == 'd') {
                    sum += kprintun(va_arg(list, unsigned int), 10);
                }
            } else if (ptr[i] == 'o') {
                sum += kprintun(va_arg(list, unsigned int), 8);
            } else if (ptr[i] == 'b') {
                sum += kprintun(va_arg(list, unsigned int), 2);
            } else if (ptr[i] == 'c') {
                kputchar((char) va_arg(list, int));
                sum++;
            } else if (ptr[i] == 's') {
                sum += kprints(va_arg(list, char*));
            }
        } else {
            kputchar(ptr[i]);
            sum++;
        }
    }
    va_end(list);
    return sum;
}