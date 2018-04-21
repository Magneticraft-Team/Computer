//
// Created by cout970 on 19/09/17.
//

#include <debug.h>
#include <stdarg.h>
#include <string.h>
#include <kprint.h>
#include "../include/object.h"
#include "../include/gc.h"
#include "../include/functions.h"
#include "../include/exception.h"

const char *objectTypeNames[] = {
        "SYMBOL",
        "KEYWORD",
        "NUMBER",
        "STRING",
        "CONS",
        "FUNC",
        "NATIVE_FUN",
        "MACRO",
};

static int lastId = 0;

Object *newObject(ObjectType type, int count, ...) {
    Object *ret;
    int i;
    va_list ap;
    va_start(ap, count);

    ret = objAlloc();
    if (ret == 0) {
        kprint("Out of memory!\n");
        THROW(EXCEPTION_OUT_OF_MEMORY);
        return NULL;
    }

    ret->id = lastId++;
    ret->type = type;

    for (i = 0; i < count; i++) {
        ret->raw[i] = va_arg(ap, Object *);
    }

    va_end(ap);
    return ret;
}

Object *createSymbol(const char *symbol) {
    return newObject(SYMBOL, 1, strCopy(symbol));
}

Object *createKeyword(const char *keyword) {
    return newObject(KEYWORD, 1, strCopy(keyword));
}

Object *createNumber(int value) {
    return newObject(NUMBER, 1, value);
}

Object *createString(const char *str) {
    return newObject(STRING, 1, strCopy(str));
}

Object *createCons(Object *car, Object *cdr) {
    return newObject(CONS, 2, car, cdr);
}

Object *createFun(Object *args, Object *code, Object *env) {
    return newObject(FUNC, 3, args, code, env);
}

Object *createNativeFunc(NativeFunc func) {
    return newObject(NATIVE_FUN, 1, (Object *) func);
}

Object *createMacro(Object *args, Object *code, Object *env) {
    return newObject(MACRO, 3, args, code, env);
}

Object *getFirst(Object *obj) {
    if (obj->type == CONS) {
        return obj->car;
    }
    return obj_nil;
}

Object *getRest(Object *obj) {
    if (obj->type == CONS) {
        return obj->cdr;
    }
    return obj_nil;
}

int getNumber(Object *obj) {
    if (obj->type == NUMBER) {
        return obj->number;
    } else {
        kprint("Not a number: ");
        printObj(obj);
        kprint("\n");
        THROW(EXCEPTION_CAST_TO_NUMBER_FAILED);
        return 0;
    }
}

Object *getSymbol(Object *obj) {
    if (obj->type == SYMBOL) {
        return obj;
    } else {
        kprint("Not a symbol: ");
        printObj(obj);
        kprint("\n");
        THROW(EXCEPTION_CAST_TO_SYMBOL_FAILED);
        return 0;
    }
}

String *getString(Object *obj) {
    if (obj->type == STRING) {
        return obj->name;
    } else {
        kprint("Not a string: ");
        printObj(obj);
        kprint("\n");
        THROW(EXCEPTION_CAST_TO_NUMBER_FAILED);
        return 0;
    }
}

Object *getElem(Object *list, int index) {
    Object *aux = list;
    for (int i = 0; i < index; i++) {
        aux = getRest(aux);
    }
    return getFirst(aux);
}

int objEquals(Object *a, Object *b) {
    if (a->type != b->type) return 0;

    if (a->type == NATIVE_FUN && a->function != b->function) return 0;
    if (a->type == NUMBER && a->number != b->number) return 0;
    if (a->type == STRING && strcmp(a->name, b->name) != 0) return 0;
    if (a->type == SYMBOL && strcmp(a->name, b->name) != 0) return 0;
    if (a->type == KEYWORD && strcmp(a->name, b->name) != 0) return 0;

    if (a->type == CONS && (!objEquals(a->car, b->car) || !objEquals(a->cdr, b->cdr)))
        return 0;

    if (a->type == FUNC && (!objEquals(a->code, b->code) || !objEquals(a->args, b->args) || !objEquals(a->env, b->env)))
        return 0;

    if (a->type == MACRO && (!objEquals(a->macro_code, b->macro_code) || !objEquals(a->macro_args, b->macro_args) ||
                             !objEquals(a->macro_env, b->macro_env)))
        return 0;

    return 1;
}

int symbolEquals(Object *a, Object *b) {
    if (a->type != b->type) return 0;
    if (a->type != SYMBOL) return 0;
    return strcmp(a->name, b->name) == 0;
}

void debugObj(Object *obj) {
    kprint("Object(type=%s, id=%d", objectTypeNames[obj->type], obj->id);
    switch (obj->type) {
        case SYMBOL:
        case KEYWORD:
        case STRING:
            kprint(", name='%s'", obj->name);
            break;

        case NUMBER:
            kprint(", value='%d'", obj->number);
            break;

        case CONS:
            kprint(", car='%lx', cdr='%lx'", (long int) obj->car, (long int) obj->cdr);
            break;

        case FUNC:
            break;

        case MACRO:
            break;

        case NATIVE_FUN:
            kprint(", func='%lx'", (unsigned long int) obj->function);
            break;
    }
    kprint(", addr = 0x%lx)\n", (unsigned long int) obj);
}

void printObj(Object *obj) {
    switch (obj->type) {
        case SYMBOL:
        case KEYWORD:
            kprint("%s", obj->name);
            break;
        case NUMBER:
            kprint("%d", obj->number);
            break;
        case STRING:
            kprint("\"%s\"", obj->name);
            break;
        case CONS:
            kprint("(");
            for (;;) {
                printObj(getFirst(obj));
                if (getRest(obj) == obj_nil) {
                    kprint(")");
                    break;
                }
                obj = getRest(obj);
                if (obj->type != CONS) {
                    kprint(" . ");
                    printObj(obj);
                    kprint(")");
                    break;
                }
                kprint(" ");
            }
            break;
        case FUNC:
            kprint("func");
            break;
        case NATIVE_FUN:
            kprint("native_func");
            break;
        case MACRO:
            kprint("macro");
            break;
        default:
            kprint("corrupted (type: %d)", obj->type);
    }
}
