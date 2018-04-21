//
// Created by cout970 on 2017-08-13.
//

#ifndef MAGNETICRAFTCOMPUTER_DICTIONARY_H
#define MAGNETICRAFTCOMPUTER_DICTIONARY_H

#include "definitions.h"

extern Word *dictionary;
extern char* dp;

String *createString(const String *src);

Word *newWord(String *name, Func function, int inmed, int count, ...);

Word *createWord(const char *name, Func function);
Word *createImmediateWord(const char *name, Func function);

Word *createVariable(const char *name, Value data);

Word *createConstant(const char *name, Value data);

Word *extendDictionary(Word *word);

Word *findIn(Word* dictionary, String *name);

#endif //MAGNETICRAFTCOMPUTER_DICTIONARY_H
