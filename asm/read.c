//
// Created by cout970 on 2017-08-17.
//

#include "read.h"
#include "dependencies.h"

int lineNum = 0;
int useDisk = 0;
FILE* fileWord = NULL;

void readLineTerminal(char* buffer, int maxSize){
    printf("%d > ", ++lineNum);
    fgets(buffer, maxSize, stdin);
    putchar('\n');
}

void readLineDisk(char* buffer, int maxSize){
    lineNum++;
    fgets(buffer, maxSize, fileWord);
}

void init(){
    fileWord = fopen("/devices/disk0", "r");
}

void readLine(char* buffer, int maxSize) {
    if (useDisk) {
        readLineDisk(buffer, maxSize);
    } else {
        readLineTerminal(buffer, maxSize);
    }
}

int getLineNumber(){
    return lineNum;
}
