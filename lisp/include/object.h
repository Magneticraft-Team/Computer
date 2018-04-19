//
// Created by cout970 on 19/09/17.
//

#ifndef UNTITLED_OBJECT_H
#define UNTITLED_OBJECT_H

#include <types.h>

typedef enum {
    SYMBOL,
    KEYWORD,
    NUMBER,
    STRING,
    CONS,
    FUNC,
    NATIVE_FUN,
    MACRO
} ObjectType;

extern const char *objectTypeNames[];

typedef struct Object {
    ObjectType type;
    int id;

    union {
        char *name;
        int number;

        struct Object *(*const function)(struct Object *, struct Object *);

        struct Object *raw[3];

        struct {
            struct Object *car;
            struct Object *cdr;
        };

        struct {
            struct Object *args;
            struct Object *code;
            struct Object *env;
        };

        struct {
            struct Object *macro_args;
            struct Object *macro_code;
            struct Object *macro_env;
        };
    };
} Object;

typedef Object *(*NativeFunc)(Object *, Object *);

// CONSTRUCTORS

Object *createSymbol(const char *symbol);

Object *createKeyword(const char *keyword);

Object *createNumber(int value);

Object *createString(const char *str);

Object *createCons(Object *car, Object *cdr);

Object *createFun(Object *args, Object *code, Object *env);

Object *createNativeFunc(NativeFunc func);

Object *createMacro(Object *args, Object *code, Object *env);

Object *getFirst(Object *obj);

Object *getRest(Object *obj);

int getNumber(Object *obj);

Object *getSymbol(Object *obj);

String *getString(Object *obj);

Object *getElem(Object *list, int index);

int objEquals(Object *a, Object *b);

int symbolEquals(Object *a, Object *b);

void debugObj(Object *obj);

void printObj(Object *obj);

#endif //UNTITLED_OBJECT_H
