//
// Created by cout970 on 2017-08-13.
//

#ifndef MAGNETICRAFTCOMPUTER_DEFINITIONS_H
#define MAGNETICRAFTCOMPUTER_DEFINITIONS_H

#include "dependencies.h"

typedef struct fString {
    uint8_t size;
    const char array[1];
} String;

typedef struct word {
    String *name;
    uint8_t flags;

    struct word *next;

    void (*code)(struct word *);

    int data[0];
} Word;

typedef void (*Func)(Word *);

#endif //MAGNETICRAFTCOMPUTER_DEFINITIONS_H
