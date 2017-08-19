//
// Created by cout970 on 2017-08-12.
//

#include "tokenizer.h"
#include "globals.h"
#include "constructors.h"
#include "environment.h"
#include "read.h"

char tokenBuffer[100];
int tokenBufferPtr;

enum TokenType tokenType;
int mustReadToken = 1;

inline int isEq(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

static void readNumber(char currentChar) {
    for (;;) {
        if (!isdigit(currentChar)) {
            putNextChar(currentChar);
            tokenBuffer[tokenBufferPtr++] = '\0';
            return;
        }
        tokenBuffer[tokenBufferPtr++] = currentChar;
        currentChar = getNextChar();
    }
}

static void readSymbol(char currentChar) {
    for (;;) {
        if (strchr("()\'", currentChar) || isspace(currentChar)) {
            putNextChar(currentChar);
            tokenBuffer[tokenBufferPtr++] = '\0';
            tokenType = STRING;
            return;
        }
        tokenBuffer[tokenBufferPtr++] = currentChar;
        currentChar = getNextChar();
    }
}

void readToken() {
    char comment = 0;
    char currentChar;
    tokenBufferPtr = 0;

    do {
        currentChar = getNextChar();
        if (currentChar == '\n') comment = 0;
        if (currentChar == ';') comment = 1;
    } while (isspace(currentChar) || comment);

    if (currentChar == '(') {
        tokenType = LEFT_PAREN;
        return;
    }
    if (currentChar == ')') {
        tokenType = RIGHT_PAREN;
        return;
    }
    if (currentChar == '\'') {
        tokenType = QUOTE;
        return;
    }
    tokenBuffer[tokenBufferPtr++] = currentChar;

    // negative number?
    if (currentChar == '-') {
        currentChar = getNextChar();
        if (isdigit(currentChar)) {
            tokenType = NUMBER;
            readNumber(currentChar);
            return;
        } else {
            readSymbol(currentChar);
            return;
        }
    }

    // Number
    if (isdigit(currentChar)) {
        tokenType = NUMBER;
        readNumber();
        return;
    }

    //String
    currentChar = getNextChar();
    readSymbol(currentChar);
}

void nextToken() {
    if (mustReadToken) {
        readToken();
    }
    mustReadToken = 1;
}

Object *readObj() {

    nextToken();

    if (tokenType == LEFT_PAREN) {
        return readList();
    }
    if (tokenType == QUOTE) {
        return createCons(quote, createCons(readObj(), nil));
    }
    if (tokenType == NUMBER) {
        return createInt(atoi(tokenBuffer));
    }
    if (tokenType == STRING) {
        return getOrCreateSymbol(createSymbolString());
    }
    printf("Error: invalid token '%s'\n", tokenBuffer);
    longjmp(onError, 10);
}

Object *readList() {

    nextToken();
    if (tokenType == RIGHT_PAREN) {
        return nil;
    }
    if (tokenType == DOT) {
        return readObj();
    }
    mustReadToken = 0;
    return createCons(readObj(), readList());
}

void initTokenizer() {
    initReader();
}

const char *createSymbolString() {
    total_malloc += strlen(tokenBuffer);
    string_count++;
    return strdup(tokenBuffer);
}