//
// Created by cout970 on 11/02/18.
//

#ifndef MAGNETICRAFTCOMPUTER_INPUT_H
#define MAGNETICRAFTCOMPUTER_INPUT_H

#include <types.h>

void readInput(char *str, int n);

Boolean strmatch(String *a, String *b);

Boolean parsetInt(String *str, Int *result);

Int split(String *in, String *cmd, String *arg1, String *arg2, String *arg3);

#endif //MAGNETICRAFTCOMPUTER_INPUT_H
