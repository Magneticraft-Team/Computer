//
// Created by cout970 on 2017-08-12.
//

#include "lisp.h"
#include "dependencies.h"

#define INPUT_BUFFER_SIZE 80
char inputBuffer[INPUT_BUFFER_SIZE];
int readBuffPtr = 0;
char charBuffer = '\0';

int useFile = 0;
File *inputFile = NULL;
int fileReadPtr = 0;

void setLineNumber(int val) {
    line_num = val;
}

int getLineNumber() {
    return line_num;
}

void setInputFile(File *file) {
    if (file == NULL) {
        useFile = 0;
        inputFile = NULL;

    } else {
        useFile = 1;
        inputFile = file;
    }
    fileReadPtr = 0;
}

int canReadMore() {
    if (useFile) {
        return inputFile->size < fileReadPtr;
    } else {
        return 1;
    }
}

void initReader() {
    inputBuffer[0] = '\0';
}

void readLineBuffered() {
    readBuffPtr = 0;
    line_num++;
    if (useFile) {
        DiskDrive  drive = motherboard_get_floppy_drive();
        fileReadPtr += file_read(drive, inputFile, byteArrayOf(inputBuffer, INPUT_BUFFER_SIZE - 1), fileReadPtr);
    } else {
        printf("%d >", line_num);
        fgets(inputBuffer, INPUT_BUFFER_SIZE - 1, stdin);
        putchar('\n');
    }

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
