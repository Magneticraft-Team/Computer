//
// Created by cout970 on 2016-11-01.
//

#ifndef COMPUTER_STDIO_H
#define COMPUTER_STDIO_H

typedef struct {
    int _id;
    int (*readFunc)(char* buff, int size);
    int (*writeFunc)(char* buff, int size);
    void* data;
} FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

#define EOF -1

void clear_screen();

int printf(const char *format, ...);

int fprintf(FILE *fd, const char *format, ...);

void putchar(int c);

void puts(char *s);

FILE *fopen(const char *name, const char *flags);

int fclose(FILE *file);

char *fgets(char *str, int n, FILE *stream);

int getc(FILE *file);

void ungetc(int c, FILE *file);

int getchar();

void unputchar();

void fflush(FILE* fd);

#endif //COMPUTER_STDIO_H
