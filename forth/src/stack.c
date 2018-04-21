//
// Created by cout970 on 2017-08-13.
//

#include "../include/stack.h"

Value dataStack[STACK_SIZE];
int dataStackPtr = 0;
Value rStack[STACK_SIZE];
int rStackPtr = 0;

void emptyDataStack(){
    dataStackPtr = 0;
}

int isDataStackEmpty(){
    return dataStackPtr == 0;
}

void pushData(Value data) {
    dataStack[dataStackPtr++] = data;
}

Value popData() {
    return dataStack[--dataStackPtr];
}

Value peekData() {
    return dataStack[dataStackPtr - 1];
}

void emptyRStack(){
    rStackPtr = 0;
}

int isRStackEmpty(){
    return rStackPtr == 0;
}

void pushR(Value data) {
    rStack[rStackPtr++] = data;
}

Value popR() {
    return rStack[--rStackPtr];
}

Value peekR() {
    return rStack[rStackPtr - 1];
}