//
// Created by cout970 on 2017-08-17.
//

#include "read.h"
#include "dependencies.h"

FILE *fileWord = NULL;

void readLineTerminal(char *buffer, int maxSize) {
    printf("> ");
    fgets(buffer, maxSize, stdin);
    putchar('\n');
}

void init() {
    fileWord = fopen("/devices/disk0", "r");
}

void readLine(char *buffer, int maxSize) {
    readLineTerminal(buffer, maxSize);
}