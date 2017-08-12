//
// Created by cout970 on 2017-08-12.
//

#ifndef MAGNETICRAFTCOMPUTER_TOKENIZER_H
#define MAGNETICRAFTCOMPUTER_TOKENIZER_H

#include "object.h"

enum TokenType {
    NUMBER, STRING, LEFT_PAREN, RIGHT_PAREN, QUOTE, DOT
};

void initTokenizer();

const char *createSymbolString();

void readToken();

void nextToken();

Object *readObj();

Object *readList();

#endif //MAGNETICRAFTCOMPUTER_TOKENIZER_H
