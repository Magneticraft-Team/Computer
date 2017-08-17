//
// Created by cout970 on 2017-08-13.
//

#ifndef MAGNETICRAFTCOMPUTER_WORDS_H
#define MAGNETICRAFTCOMPUTER_WORDS_H

#include "definitions.h"

#define TIB_SIZE 80
#define BLOCK_SIZE 1024
#define WORD_BUFFER_SIZE 128

#define IMMEDIATE_BIT_MASK 128

void readVariableAddress(Word *word);

void readVariableValue(Word *word);

void fun_forth(Word* word);

void fun_align_word(Word *a IGNORED);

void init();

void* allot(int size);

#endif //MAGNETICRAFTCOMPUTER_WORDS_H
