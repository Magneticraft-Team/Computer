//
// Created by cout970 on 2017-08-13.
//

#ifndef MAGNETICRAFTCOMPUTER_STACK_H
#define MAGNETICRAFTCOMPUTER_STACK_H

#define STACK_SIZE 128

extern int dataStackPtr;
extern int dataStack[STACK_SIZE];

void emptyDataStack();

int isDataStackEmpty();

void pushData(int data);

int popData();

int peekData();

void emptyRStack();

int isRStackEmpty();

void pushR(int data);

int popR();

int peekR();

#endif //MAGNETICRAFTCOMPUTER_STACK_H
