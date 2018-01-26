//
// Created by cout970 on 2016-11-01.
//

#include "dependencies.h"
#include "../include/stdio.h"
#include "../include/stdarg.h"
#include "../include/string.h"
#include "../include/stdlib.h"
#include "../include/ctype.h"
#include "../fs/filesystem.h"

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

typedef void (*PrintFunc)(int);

static int printn(PrintFunc print, int num, int base) {
    int sum = 0;
    if (num < 0) {
        print('-');
        sum++;
        num = -num;
    }
    if (num < base) {
        print("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[num]);
        sum++;
    } else {
        sum += printn(print, num / base, base);
        print("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[num % base]);
        sum++;
    }
    return sum;
}

static int printun(PrintFunc print, unsigned int num, int base) {
    int sum = 0;
    if (num < (unsigned int) base) {
        print("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[num]);
        sum++;
    } else {
        sum += printun(print, num / base, base);
        print("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[num % base]);
        sum++;
    }
    return sum;
}

int printFormat(PrintFunc print, const char *format, va_list lista) {

    int sum = 0;
    for (int i = 0; format[i] != '\0'; i++) {
        if (format[i] == '%') {
            i++;
            if (format[i] == '%') {
                print((int) format[i]);
                sum++;
            } else if (format[i] == 'd' || format[i] == 'i') {
                sum += printn(print, va_arg(lista, int), 10);
            } else if (format[i] == 'u') {
                sum += printun(print, va_arg(lista, unsigned int), 10);
            } else if (format[i] == 'x') {
                sum += printun(print, va_arg(lista, unsigned int), 16);
            } else if (format[i] == 'o') {
                sum += printun(print, va_arg(lista, unsigned int), 8);
            } else if (format[i] == 'b') {
                sum += printun(print, va_arg(lista, unsigned int), 2);
            } else if (format[i] == 'c') {
                print(va_arg(lista, int));
                sum++;
            } else if (format[i] == 's') {
                char *str = va_arg(lista, char*);
                for (int j = 0; str[j] != '\0'; j++) {
                    print((int) str[j]);
                    sum++;
                }
            }
        } else {
            print((int) format[i]);
            sum++;
        }
    }
    return sum;
}

// print to monitor
int printf(const char *ptr, ...) {
    va_list lista;
    va_start(lista, ptr);
    int sum = printFormat(putchar, ptr, lista);
    va_end(lista);
    return sum;
}


// print to file
static FILE* _printFileAux;

static void _printFile(int value){
//    fwrite(&value, 1, _printFileAux);
}

int fprintf(FILE *fd, const char *ptr, ...) {
    va_list lista;
    va_start(lista, ptr);
    _printFileAux = fd;
    int sum = printFormat(_printFile, ptr, lista);
    va_end(lista);
    return sum;
}

// print to memory
static char* _printStringAux;

static void _printString(int value){
    *_printStringAux = (char) value;
    _printStringAux++;
}

int sprintf(char *array, const char *ptr, ...) {
    va_list lista;
    va_start(lista, ptr);
    _printStringAux = array;
    int sum = printFormat(_printString, ptr, lista);
    va_end(lista);
    return sum;
}
// other utilities

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
    if (stream != NULL && stream->readFunc != NULL) {
        stream->readFunc(stream, str, n);
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

static int readStdin(FILE* fd, char *str, int n) {
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
        } else if (!iscntrl(c)) {
            str[i] = (char) c;
            putchar(c);
        }
    }
    str[i] = '\0';
    return i;
}

static int writeStdout(FILE* fd, char *str, int n) {
    int j;
    for (j = 0; j < n; j++) {
        putchar((int) str[j]);
    }
    return j;
}

FILE _stdin = {&readStdin, NULL, NULL};
FILE _stdout = {NULL, &writeStdout, NULL};
// there are no colors in the monitor so this is equivalent to stdout

FILE _stderr = {NULL, &writeStdout, NULL};

FILE *stdin = &_stdin;
FILE *stdout = &_stdout;
FILE *stderr = &_stderr;

static DiskDrive disk = NULL;

static DiskDrive getDisk(){
    if(!disk){
        disk = motherboard_get_floppy_drive();
    }
    return disk;
}

static File* processPath(const char *name){
    if(name == NULL) return NULL;
    const char* trimmed = strtrim(name);
    if(trimmed == NULL) return NULL;

    if(strcmp(trimmed, "/")){
        return file_get_root(getDisk());
    }
}

// TODO implement this to work with filesystem.h
FILE *fopen(const char *name IGNORED, const char *flags IGNORED) {
    return NULL;
}

int fclose(FILE *file IGNORED) {
    return 0;
}

int getc(FILE *file) {
    char a[1];
    fgets(a, 1, file);
    return a[0];
}

void ungetc(int c IGNORED, FILE *file IGNORED) {
    return unputchar();
}

void fflush(FILE *fd IGNORED) {}