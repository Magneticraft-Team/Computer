//
// Created by cout970 on 2017-08-12.
//

#include "lisp.h"

#define INPUT_BUFFER_SIZE 80
char inputBuffer[INPUT_BUFFER_SIZE];
int readBuffPtr = 0;
char charBuffer = '\0';

void initReader() {
    inputBuffer[0] = '\0';
}

void readLineBuffered() {
    readBuffPtr = 0;
    line_num++;
    printf("%d >", line_num);
    fgets(inputBuffer, INPUT_BUFFER_SIZE - 1, stdin);
    putchar('\n');
    int len = strlen(inputBuffer);
    inputBuffer[len] = ' ';
    inputBuffer[len + 1] = '\0';
}

char getNextChar() {
    char currentChar;

    if (charBuffer != '\0') {
        currentChar = charBuffer;
        charBuffer = '\0';
        return currentChar;
    }

    currentChar = inputBuffer[readBuffPtr++];

    if (readBuffPtr >= INPUT_BUFFER_SIZE) {
        readLineBuffered();
    }
    if (currentChar == '\0') {
        readLineBuffered();
        return '\n';
    }
    return currentChar;
}

void putNextChar(char a) {
    charBuffer = a;
}
