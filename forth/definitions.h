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
    String *name;           // heap allocated
    uint8_t flags;          // immediate flag here instead on the name size

    struct word *next;      // next word in the dictionary

    void (*code)(void);     // function to execute for this word

    int data[0];            // extra data
} Word;

typedef void (*Func)(void);

#endif //MAGNETICRAFTCOMPUTER_DEFINITIONS_H
