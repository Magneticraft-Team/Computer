//
// Created by cout970 on 2017-07-10.
//

#ifndef DRIVER_PRINTF_H
#define DRIVER_PRINTF_H

#include "api/motherboard.h"
#include "api/monitor.h"

// List of variable arguments
#define va_list struct { int* _ptr; int _count; }

// Initialization of the list, using the last argument of the function
#define va_start(_list, arg) (_list)._ptr = (int*)(&ptr + 1); (_list)._count = 0

// Retrieves a element from the list
#define va_arg(_list, arg) ((arg)*((_list)._ptr + (_list)._count++))

// Removes the list, not need in this implementation
#define va_end(_list) ((void)0)

#define va_copy(dst, src) dst._ptr = (src)._ptr; (dst)._count = (src)._count

#ifdef USE_DEBUG_LOG
void putchar(int code) {
    motherboard_debug_print_byte((i8) code);
}
#else
static int cursor = 0;

Monitor _get_primary_monitor() {
    return (Monitor) motherboard_get_monitor();
}

void newLine() {
    Monitor mon = _get_primary_monitor();
    cursor = 0;
    monitor_set_selected_line(mon, (monitor_get_selected_line(mon) + 1) % monitor_get_num_lines(mon));
    monitor_set_cursor_pos_x(mon, cursor);
    monitor_set_cursor_pos_y(mon, monitor_get_selected_line(mon));
}

void putchar(int code) {
    Monitor mon;

    if (code == '\n') {
        newLine();
        return;
    }

    mon = _get_primary_monitor();

    monitor_signal(mon, MONITOR_SIGNAL_READ);
    monitor_get_line_buffer(mon)[cursor] = (i8) code;
    monitor_signal(mon, MONITOR_SIGNAL_WRITE);

    cursor++;
    if (cursor >= monitor_get_num_columns(mon)) {
        newLine();
    } else {
        monitor_set_cursor_pos_x(mon, cursor);
        monitor_set_cursor_pos_y(mon, monitor_get_selected_line(mon));
    }
}
#endif

void puts(char *str) {
    for (int j = 0; str[j] != '\0'; j++) {
        putchar((int) str[j]);
    }
}

int printn(int num, int base) {
    int sum = 0;
    if (num < 0) {
        putchar('-');
        sum++;
        num = -num;
    }
    if (num < base) {
        putchar("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[num]);
        sum++;
    } else {
        sum += printn(num / base, base);
        putchar("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[num % base]);
        sum++;
    }
    return sum;
}

int printun(unsigned int num, int base) {
    int sum = 0;
    if (num < (unsigned)base) {
        putchar("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[num]);
        sum++;
    } else {
        sum += printun(num / base, base);
        putchar("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[num % base]);
        sum++;
    }
    return sum;
}

int printf(const char *ptr, ...) {
    va_list list;
    va_start(list, ptr);

    int sum = 0;
    for (int i = 0; ptr[i] != '\0'; i++) {
        if (ptr[i] == '%') {
            i++;
            if (ptr[i] == '%') {
                putchar((int) ptr[i]); sum++;
            } else if (ptr[i] == 'd' || ptr[i] == 'i') {
                sum += printn(va_arg(list, int), 10);
            } else if (ptr[i] == 'u') {
                sum += printun(va_arg(list, unsigned int), 10);
            } else if (ptr[i] == 'x') {
                sum += printun(va_arg(list, unsigned int), 16);
            } else if (ptr[i] == 'o') {
                sum += printun(va_arg(list, unsigned int), 8);
            } else if (ptr[i] == 'b') {
                sum += printun(va_arg(list, unsigned int), 2);
            } else if (ptr[i] == 'c') {
                putchar(va_arg(list, int)); sum++;
            } else if (ptr[i] == 's') {
                char *str = va_arg(list, char*);
                for (int j = 0; str[j] != '\0'; j++) {
                    putchar((int) str[j]); sum++;
                }
            }
        } else {
            putchar((int) ptr[i]); sum++;
        }
    }
    va_end(list);
    return sum;
}

#endif //DRIVER_PRINTF_H
