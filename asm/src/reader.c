//
// Created by cout970 on 17/02/18.
//

#include <fs/file.h>
#include "../include/reader.h"

#define INPUT_BUFFER_SIZE 120

char inputBuffer[INPUT_BUFFER_SIZE];
int inputIndex;
int returnChar = -1;
int currentLine = 0;
int currentColumn = 0;
FD srcFile;

void initReader(FD src) {
    srcFile = src;
    currentLine = 0;
    currentColumn = 0;

    inputIndex = 0;
    inputBuffer[inputIndex] = '\0';
}

int readChar() {
    int charRead;

    if (returnChar != -1) {

        charRead = returnChar;
        returnChar = -1;
    } else {

        if (inputBuffer[inputIndex] == '\0') {
            inputIndex = 0;
            int read = file_read(srcFile, inputBuffer, INPUT_BUFFER_SIZE - 1);
            inputBuffer[read] = '\0';
        }
        charRead = inputBuffer[inputIndex++];

        if (charRead == '\n') {
            currentLine++;
            currentColumn = 0;
        } else {
            currentColumn++;
        }
    }

    return charRead;
}

void unreadChar(int character) {
    returnChar = character;
}