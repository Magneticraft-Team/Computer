//
// Created by cout970 on 2016-11-01.
//

#ifndef COMPUTER_STDIO_H
#define COMPUTER_STDIO_H

typedef struct {
    int _id;
} FILE;

extern FILE _std_io[3];

#define stdin (&_std_io[0])
#define stdout (&_std_io[1])
#define stderr (&_std_io[2])

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
