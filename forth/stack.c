//
// Created by cout970 on 2017-08-13.
//

#include "stack.h"

int dataStack[STACK_SIZE];
int dataStackPtr = 0;
int rStack[STACK_SIZE];
int rStackPtr = 0;

void emptyDataStack(){
    dataStackPtr = 0;
}

int isDataStackEmpty(){
    return dataStackPtr == 0;
}

void pushData(int data) {
    dataStack[dataStackPtr++] = data;
}

int popData() {
    return dataStack[--dataStackPtr];
}

int peekData() {
    return dataStack[dataStackPtr - 1];
}

void emptyRStack(){
    rStackPtr = 0;
}

int isRStackEmpty(){
    return rStackPtr == 0;
}

void pushR(int data) {
    rStack[rStackPtr++] = data;
}

int popR() {
    return rStack[--rStackPtr];
}

int peekR() {
    return rStack[rStackPtr - 1];
}