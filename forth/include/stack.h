//
// Created by cout970 on 2017-08-13.
//

#ifndef MAGNETICRAFTCOMPUTER_STACK_H
#define MAGNETICRAFTCOMPUTER_STACK_H

#include "definitions.h"

#define STACK_SIZE 128

extern int dataStackPtr;
extern Value dataStack[STACK_SIZE];

void emptyDataStack();

int isDataStackEmpty();

void pushData(Value data);

Value popData();

Value peekData();

void emptyRStack();

int isRStackEmpty();

void pushR(Value data);

Value popR();

Value peekR();

#endif //MAGNETICRAFTCOMPUTER_STACK_H
