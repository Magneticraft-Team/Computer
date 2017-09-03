//
// Created by cout970 on 2017-08-13.
//

#include "dictionary.h"
#include "stack.h"
#include "words.h"

extern char *__end;

Word *dictionary = NULL;

char *dp = (char *) &__end + 340;


String *createString(const char *str) {
    int size = strlen(str);
    String *header = allot(sizeof(String) + size);

    header->size = (uint8_t) size;
    memcpy((void *) header->array, str, (size_t) (size + 1));

    return header;
}

Word *newWord(String *name, Func function, int inmed, int count, ...) {
    va_list ap;
    va_start(ap, count);
    int i;
    fun_align_word();
    Word *word = allot(sizeof(Word) + sizeof(int) * count);

    word->name = name;
    if (inmed) {
        word->flags = IMMEDIATE_BIT_MASK;
    } else {
        word->flags = 0;
    };
    word->next = NULL;
    word->code = function;

    for (i = 0; i < count; i++) word->data[i] = va_arg(ap, int);
    word->data[i] = 0;

    va_end(ap);
    return word;
}

Word *extendDictionary(Word *word) {
    word->next = NULL;

    if (dictionary == NULL) {
        dictionary = word;
        return word;
    }

    Word *last;
    for (last = dictionary; last->next != NULL; last = last->next);
    last->next = word;

    return word;
}

int strEq(String *a, String *b) {
    if (a->size != b->size) return 0;
    int i;
    for (i = 0; i < a->size; i++) {
        if (a->array[i] != b->array[i]) return 0;
    }
    return 1;
}

Word *findIn(Word *dictionary, String *name) {
    Word *word;
    for (word = dictionary; word != NULL; word = word->next) {
        if (strEq(word->name, name)) {
            return word;
        }
    }
    return NULL;
}

Word *createWord(const char *name, Func fun) {
    return newWord(createString(name), fun, 0, 0);
}

Word *createImmediateWord(const char *name, Func fun) {
    return newWord(createString(name), fun, 1, 0);
}

Word *createVariable(const char *name, int data) {
    return newWord(createString(name), readVariableAddress, 0, 1, data);
}

Word *createConstant(const char *name, int data) {
    return newWord(createString(name), readVariableValue, 0, 1, data);
}