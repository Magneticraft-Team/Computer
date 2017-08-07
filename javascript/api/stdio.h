//
// Created by cout970 on 2016-11-01.
//

#ifndef COMPUTER_STDIO_H
#define COMPUTER_STDIO_H

typedef struct {
    int _id;
} FILE;

#define stdin ((FILE){0})
#define stdout ((FILE){1})
#define stderr ((FILE){2})

void clear_screen();

int printf(const char *format, ...);

void putchar(int c);

void puts(char *s);

FILE *fopen(const char *name, const char *flags);

int fclose(FILE *file);

char *fgets(char *str, int n, FILE *stream);

int getchar();

void unputchar();

#endif //COMPUTER_STDIO_H
