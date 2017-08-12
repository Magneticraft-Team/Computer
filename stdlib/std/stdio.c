//
// Created by cout970 on 2016-11-01.
//

#include "../api/monitor.h"
#include "../api/motherboard.h"
#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "assert.h"
#include "stdlib.h"

#define SPACE ((char)32)

void clear_screen() {
    Monitor mon = motherboard_get_monitor();
    int lines = monitor_get_num_lines(mon);
    int columns = monitor_get_num_columns(mon);
    int i;

    for (i = 0; i < columns; i++) {
        monitor_get_line_buffer(mon)[i] = 0x20;
    }

    for (i = 0; i < lines; i++) {
        monitor_set_selected_line(mon, i);
        monitor_signal(mon, MONITOR_SIGNAL_WRITE);
    }
    monitor_set_selected_line(mon, 0);
    monitor_set_cursor_pos_y(mon, 0);
    monitor_set_cursor_pos_x(mon, 0);
}

static int printn(int num, int base) {
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

static int printun(unsigned int num, int base) {
    int sum = 0;
    if (num < (unsigned int) base) {
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
    va_list lista;
    va_start(lista, ptr);

    int sum = 0;
    for (int i = 0; ptr[i] != '\0'; i++) {
        if (ptr[i] == '%') {
            i++;
            if (ptr[i] == '%') {
                putchar((int) ptr[i]);
                sum++;
            } else if (ptr[i] == 'd' || ptr[i] == 'i') {
                sum += printn(va_arg(lista, int), 10);
            } else if (ptr[i] == 'u') {
                sum += printun(va_arg(lista, unsigned int), 10);
            } else if (ptr[i] == 'x') {
                sum += printun(va_arg(lista, unsigned int), 16);
            } else if (ptr[i] == 'o') {
                sum += printun(va_arg(lista, unsigned int), 8);
            } else if (ptr[i] == 'b') {
                sum += printun(va_arg(lista, unsigned int), 2);
            } else if (ptr[i] == 'c') {
                putchar(va_arg(lista, int));
                sum++;
            } else if (ptr[i] == 's') {
                char *str = va_arg(lista, char*);
                for (int j = 0; str[j] != '\0'; j++) {
                    putchar((int) str[j]);
                    sum++;
                }
            }
        } else {
            putchar((int) ptr[i]);
            sum++;
        }
    }
    va_end(lista);
    return sum;
}

int fprintf(FILE *fd IGNORED, const char *ptr, ...){
    va_list lista;
    va_start(lista, ptr);

    int sum = 0;
    for (int i = 0; ptr[i] != '\0'; i++) {
        if (ptr[i] == '%') {
            i++;
            if (ptr[i] == '%') {
                putchar((int) ptr[i]);
                sum++;
            } else if (ptr[i] == 'd' || ptr[i] == 'i') {
                sum += printn(va_arg(lista, int), 10);
            } else if (ptr[i] == 'u') {
                sum += printun(va_arg(lista, unsigned int), 10);
            } else if (ptr[i] == 'x') {
                sum += printun(va_arg(lista, unsigned int), 16);
            } else if (ptr[i] == 'o') {
                sum += printun(va_arg(lista, unsigned int), 8);
            } else if (ptr[i] == 'b') {
                sum += printun(va_arg(lista, unsigned int), 2);
            } else if (ptr[i] == 'c') {
                putchar(va_arg(lista, int));
                sum++;
            } else if (ptr[i] == 's') {
                char *str = va_arg(lista, char*);
                for (int j = 0; str[j] != '\0'; j++) {
                    putchar((int) str[j]);
                    sum++;
                }
            }
        } else {
            putchar((int) ptr[i]);
            sum++;
        }
    }
    va_end(lista);
    return sum;
}

void puts(char *str) {
    for (int j = 0; str[j] != '\0'; j++) {
        putchar((int) str[j]);
    }
}

void jumpLine() {
    Monitor mon = motherboard_get_monitor();
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

        memset((void *) monitor_get_line_buffer(mon), SPACE, (size_t) columns);
        monitor_signal(mon, MONITOR_SIGNAL_WRITE);

        monitor_set_cursor_pos_x(mon, 0);
        monitor_set_cursor_pos_y(mon, currentLine);
    }
}

char *fgets(char *str, int n, FILE *stream) {
    if (stream->_id == stdin->_id) {
        int i;
        for (i = 0; i < n - 1; i++) {
            int c = getchar();
            if (c == 13 || c == 10) {
                break;
            }
            if (c == 8) {
                if (i > 0) {
                    i--;
                    unputchar();
                }
                i--;
            } else {
                str[i] = (char) c;
                putchar(c);
            }
        }
        str[i] = '\0';
        return str;
    }
    return (char *) NULL;
}

int getchar() {
    Monitor mon = motherboard_get_monitor();
    struct key_event event;

    do {
        while (!monitor_has_key_events(mon)) {
            motherboard_sleep(1);
        }
        event = monitor_get_last_key_event(mon);
    } while (!event.press);

    return event.character;
}

void unputchar() {
    Monitor mon = motherboard_get_monitor();
    int columns = monitor_get_num_columns(mon);
    int pos = monitor_get_cursor_pos_x(mon) - 1;

    // this line
    if (pos > 0) {
        monitor_signal(mon, MONITOR_SIGNAL_READ);
        monitor_get_line_buffer(mon)[pos] = SPACE;
        monitor_set_cursor_pos_x(mon, pos);
        monitor_signal(mon, MONITOR_SIGNAL_WRITE);
        return;
    }

    //other line
    int currentLine = monitor_get_selected_line(mon);

    //deleted line
    if (currentLine <= 0) {
        return;
    }

    currentLine--;
    monitor_set_selected_line(mon, currentLine);
    monitor_set_cursor_pos_y(mon, currentLine);

    monitor_signal(mon, MONITOR_SIGNAL_READ);
    int lastPos;
    for (lastPos = columns - 1; lastPos >= 0; lastPos--) {
        if (monitor_get_line_buffer(mon)[lastPos] != SPACE) {
            break;
        }
    }
    monitor_get_line_buffer(mon)[lastPos] = SPACE;
    monitor_signal(mon, MONITOR_SIGNAL_WRITE);
    monitor_set_cursor_pos_x(mon, lastPos);
}

void putchar(int c) {
    if (c == '\t') {
        putchar(' ');
        putchar(' ');
        putchar(' ');
        putchar(' ');
        return;
    }
    if (c == '\n') {
        jumpLine();
        return;
    }
    Monitor mon = motherboard_get_monitor();
    int pos = monitor_get_cursor_pos_x(mon);
    monitor_signal(mon, MONITOR_SIGNAL_READ);
    monitor_get_line_buffer(mon)[pos] = (char) c;
    monitor_signal(mon, MONITOR_SIGNAL_WRITE);
    monitor_set_cursor_pos_x(mon, pos + 1);

    if (pos >= monitor_get_num_columns(mon) - 1) {
        jumpLine();
    }
}

///////////// FILE IO /////////////

FILE _std_io[] = {{0}, {1}, {2}};
FILE _floppy = {3};

FILE *fopen(const char *name IGNORED, const char *flags IGNORED) {
    return &_floppy;
}

int fclose(FILE *file IGNORED) {
    return 0;
}

int getc(FILE *file IGNORED){
    return getchar();
}

void ungetc(int c IGNORED, FILE *file IGNORED){
    return unputchar();
}