//
// Created by cout970 on 2017-08-13.
//

#ifndef MAGNETICRAFTCOMPUTER_DICTIONARY_H
#define MAGNETICRAFTCOMPUTER_DICTIONARY_H

#include "definitions.h"

extern Word *dictionary;
extern char* dp;

String *createString(const char *str);

Word *newWord(String *name, Func function, int count, ...);

Word *createWord(const char *name, Func function);

Word *createVariable(const char *name, int data);

Word *createConstant(const char *name, int data);

Word *extendDictionary(Word *word);

Word *findIn(Word* dictionary, String *name);

#endif //MAGNETICRAFTCOMPUTER_DICTIONARY_H
