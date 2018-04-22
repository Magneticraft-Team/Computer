//
// Created by cout970 on 2017-08-13.
//

#include <string.h>
#include <stdarg.h>
#include "../include/dictionary.h"
#include "../include/words.h"

#ifdef DEBUG_ENV
char __heap[64000];
char *dp = __heap;
#else
extern char *__end;
char *dp = (char *) &__end + 340;
#endif

Word *dictionary = NULL;


String *createString(const String *src) {
    String *res = allot((int) (strlen(src) + 1));
    strcpy(res, src);
    return res;
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

    for (i = 0; i < count; i++) word->data[i] = va_arg(ap, Value);
    word->data[i] = (Value){0};

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

int strcasecmp(const char *a, const char *b) {
    int i = 0;
    while (1) {
        if (tolower(a[i]) != tolower(b[i])) return 1;
        if (a[i] == '\0') break;
        i++;
    }
    return 0;

}

Word *findIn(Word *dictionary, String *name) {
    Word *word;
    for (word = dictionary; word != NULL; word = word->next) {
        if (strcasecmp(word->name, name) == 0) {
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

Word *createVariable(const char *name, Value data) {
    return newWord(createString(name), readVariableAddress, 0, 1, data);
}

Word *createConstant(const char *name, Value data) {
    return newWord(createString(name), readVariableValue, 0, 1, data);
}